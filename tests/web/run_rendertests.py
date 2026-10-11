#!/usr/bin/env python3
"""Runs the Emscripten rendertests in headless Chrome.

Serves the build directory and the approved images on localhost, loads rendertests.html,
prints the test output and exits with the tests' exit code. Received, diff and (with
--approve) approved images come back from the page and land in --approved-dir.

SwiftShader is forced, so a local run renders like the CI run.
"""

import argparse
import http.server
import json
import os
import platform
import re
import shutil
import socket
import subprocess
import sys
import tempfile
import threading
import urllib.parse

IMAGE_NAME = re.compile(r"^rendertests\.[A-Za-z0-9._-]+\.png$")


def find_chrome():
    if os.environ.get("CHROME"):
        return os.environ["CHROME"]
    candidates = [
        "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
        "google-chrome",
        "google-chrome-stable",
        "chromium",
        "chromium-browser",
    ]
    for candidate in candidates:
        path = candidate if os.path.isabs(candidate) else shutil.which(candidate)
        if path and os.path.exists(path):
            return path
    sys.exit("Chrome not found; set CHROME to its executable")


def make_handler(build_dir, approved_dir, result):
    class Handler(http.server.SimpleHTTPRequestHandler):
        extensions_map = dict(http.server.SimpleHTTPRequestHandler.extensions_map,
                              **{".wasm": "application/wasm", ".js": "text/javascript"})

        def __init__(self, *args, **kwargs):
            super().__init__(*args, directory=build_dir, **kwargs)

        def log_message(self, format, *args):
            pass

        def image_path(self):
            name = urllib.parse.unquote(urllib.parse.urlparse(self.path).path[len("/approved/"):])
            if not IMAGE_NAME.match(name):
                self.send_error(400, "bad image name")
                return None
            return os.path.join(approved_dir, name)

        def reply(self, code, body=b"", content_type="text/plain"):
            self.send_response(code)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def read_body(self):
            return self.rfile.read(int(self.headers.get("Content-Length", 0)))

        def do_GET(self):
            path = urllib.parse.urlparse(self.path).path
            if path == "/approved-list":
                names = sorted(n for n in os.listdir(approved_dir)
                               if IMAGE_NAME.match(n) and n.endswith(".approved.png"))
                self.reply(200, json.dumps(names).encode(), "application/json")
            elif path.startswith("/approved/"):
                image = self.image_path()
                if image is None:
                    return
                if not os.path.isfile(image):
                    self.send_error(404)
                    return
                with open(image, "rb") as f:
                    self.reply(200, f.read(), "image/png")
            else:
                super().do_GET()

        def do_PUT(self):
            if not self.path.startswith("/approved/"):
                self.send_error(404)
                return
            image = self.image_path()
            if image is None:
                return
            with open(image, "wb") as f:
                f.write(self.read_body())
            self.reply(200)

        def do_POST(self):
            url = urllib.parse.urlparse(self.path)
            if url.path == "/log":
                sys.stdout.write(self.read_body().decode("utf-8", "replace"))
                sys.stdout.flush()
                self.reply(200)
            elif url.path == "/exit":
                self.read_body()
                code = urllib.parse.parse_qs(url.query).get("code", ["1"])[0]
                self.reply(200)
                result["code"] = int(code) if code.lstrip("-").isdigit() else 1
                result["done"].set()
            else:
                self.send_error(404)

    return Handler


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build-dir", required=True, help="directory with rendertests.html")
    parser.add_argument("--source-dir", required=True, help="the repository root the tests were built from")
    parser.add_argument("--approved-dir", required=True, help="tests/approved/webgl")
    parser.add_argument("--approve", action="store_true",
                        default=os.environ.get("RENDERTESTS_APPROVE", "0") not in ("", "0"),
                        help="accept every received image as the reference (default: $RENDERTESTS_APPROVE)")
    parser.add_argument("--timeout", type=float, default=300, help="seconds")
    args = parser.parse_args()

    build_dir = os.path.abspath(args.build_dir)
    approved_dir = os.path.abspath(args.approved_dir)
    os.makedirs(approved_dir, exist_ok=True)
    for name in os.listdir(approved_dir):
        if name.endswith(".received.png") or name.endswith(".diff.png"):
            os.remove(os.path.join(approved_dir, name))

    result = {"code": None, "done": threading.Event()}
    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0),
                                             make_handler(build_dir, approved_dir, result))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    port = server.server_address[1]

    query = urllib.parse.urlencode({"src": args.source_dir, "approve": "1" if args.approve else "0"})
    url = f"http://127.0.0.1:{port}/rendertests.html?{query}"

    profile = tempfile.mkdtemp(prefix="rendertests-chrome-")
    command = [
        find_chrome(),
        "--headless=new",
        "--use-angle=swiftshader",
        "--enable-unsafe-swiftshader",
        "--no-first-run",
        "--no-default-browser-check",
        "--disable-extensions",
        f"--user-data-dir={profile}",
        "--remote-debugging-port=0",
    ]
    if platform.system() == "Linux":
        command.append("--no-sandbox")
    command.append(url)

    chrome = subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        if not result["done"].wait(args.timeout):
            print(f"rendertests timed out after {args.timeout:.0f} s", file=sys.stderr)
            return 1
        return result["code"]
    finally:
        chrome.terminate()
        try:
            chrome.wait(10)
        except subprocess.TimeoutExpired:
            chrome.kill()
        server.shutdown()
        shutil.rmtree(profile, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
