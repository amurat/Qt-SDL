#!/bin/bash
# Configures the Emscripten (browser) build in build-web: webhelloworld and rendertests.
# EMSDK points at an emsdk checkout with an activated SDK.
set -e
EMSDK="${EMSDK:-$HOME/Development/emsdk}"
. "$EMSDK/emsdk_env.sh" > /dev/null
emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Debug "$@"
