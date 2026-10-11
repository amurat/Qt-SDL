// Runs rendertests in a page served by tests/web/run_rendertests.py.
// The URL carries ?src=<source dir>&approve=0|1. ApprovalTests expects the test source and
// tests/approved/webgl at their host paths, so both are mirrored into MEMFS before main().
// Output lines and the resulting images are sent back to the server.
(function() {
  var params = new URLSearchParams(location.search);
  var sourceDir = params.get('src') || '';
  var approve = params.get('approve') === '1';
  var approvedDir = sourceDir + '/tests/approved/webgl';
  // approved images as fetched, to find the ones an approve run replaced
  var fetched = {};
  var finished = false;

  // synchronous, so lines arrive in order and the last ones aren't lost on exit
  function send(method, path, body) {
    var xhr = new XMLHttpRequest();
    xhr.open(method, path, false);
    xhr.send(body);
  }

  function sameBytes(a, b) {
    if (!a || !b || a.length !== b.length) return false;
    for (var i = 0; i < a.length; ++i) {
      if (a[i] !== b[i]) return false;
    }
    return true;
  }

  function uploadResults() {
    var names;
    try {
      names = FS.readdir(approvedDir);
    } catch (e) {
      return;
    }
    names.forEach(function(name) {
      if (!/^rendertests\..*\.png$/.test(name)) return;
      var data = FS.readFile(approvedDir + '/' + name);
      var changed = /\.(received|diff)\.png$/.test(name) ||
          (/\.approved\.png$/.test(name) && !sameBytes(data, fetched[name]));
      if (changed) {
        send('PUT', '/approved/' + encodeURIComponent(name), data);
      }
    });
  }

  function finish(code) {
    if (finished) return;
    finished = true;
    try {
      uploadResults();
    } finally {
      send('POST', '/exit?code=' + code, '');
    }
  }

  // the output goes to a log, not a terminal
  Module.arguments = ['--no-colors'];

  Module.print = function(text) {
    console.log(text);
    send('POST', '/log', text + '\n');
  };
  Module.printErr = function(text) {
    console.error(text);
    send('POST', '/log', text + '\n');
  };

  Module.preRun = Module.preRun || [];
  Module.preRun.push(function() {
    ENV.RENDERTESTS_APPROVE = approve ? '1' : '0';
    FS.mkdirTree(approvedDir);
    // ApprovalTests checks that the test's __FILE__ exists
    FS.writeFile(sourceDir + '/tests/rendertests.cpp', '');

    addRunDependency('approved-images');
    fetch('/approved-list')
      .then(function(response) { return response.json(); })
      .then(function(names) {
        return Promise.all(names.map(function(name) {
          return fetch('/approved/' + encodeURIComponent(name))
            .then(function(response) { return response.arrayBuffer(); })
            .then(function(buffer) {
              var data = new Uint8Array(buffer);
              fetched[name] = data;
              FS.writeFile(approvedDir + '/' + name, data);
            });
        }));
      })
      .then(function() {
        removeRunDependency('approved-images');
      })
      .catch(function(e) {
        Module.printErr('unable to fetch the approved images: ' + e);
        finish(1);
      });
  });

  Module.onExit = function(code) { finish(code); };
  Module.onAbort = function(what) {
    Module.printErr('aborted: ' + what);
    finish(134);
  };
  window.addEventListener('error', function(event) {
    Module.printErr('uncaught: ' + event.message);
    finish(1);
  });
})();
