#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
EMSDK="$ROOT/.tools/emsdk"
PYTHON="${EMSDK_PYTHON:-$EMSDK/python/3.13.3_64bit/bin/python3}"

if [ ! -x "$EMSDK/upstream/emscripten/emcc" ]; then
    echo "缺少项目内的 Emscripten：$EMSDK" >&2
    exit 1
fi

# shellcheck disable=SC1091
. "$EMSDK/emsdk_env.sh"

BUILD="$ROOT/build-wasm"
mkdir -p "$BUILD" "$ROOT/web/dist" "$ROOT/vscode-extension/media"

emcc -O2 -c "$ROOT/vendor/miniaudio/miniaudio.c" -o "$BUILD/miniaudio.o" -I"$ROOT/vendor/miniaudio"

em++ -O2 -std=c++17 \
    -I"$ROOT/include" -I"$ROOT/vendor/miniaudio" \
    "$ROOT/main.cpp" \
    "$ROOT"/src/*.cpp \
    "$ROOT"/src/APU/*.cpp \
    "$ROOT/src/wasm/WasmHost.cpp" \
    "$BUILD/miniaudio.o" \
    -o "$BUILD/SimpleNES.js" \
    -sMODULARIZE=1 \
    -sEXPORT_NAME=createSimpleNES \
    -sSINGLE_FILE=1 \
    -sENVIRONMENT=web \
    -sALLOW_MEMORY_GROWTH=1 \
    -sINITIAL_MEMORY=67108864 \
    -sEXPORTED_FUNCTIONS='[_nes_load_rom,_nes_set_key,_nes_release_keys,_malloc,_free,_ma_device_process_pcm_frames_playback__webaudio,_ma_device_process_pcm_frames_capture__webaudio,_ma_device__on_notification_unlocked]' \
    -sEXPORTED_RUNTIME_METHODS='[HEAPU8]' \
    -sEXIT_RUNTIME=0

cp "$BUILD/SimpleNES.js" "$ROOT/web/dist/SimpleNES.js"
cp "$BUILD/SimpleNES.js" "$ROOT/vscode-extension/media/SimpleNES.js"
cp "$ROOT/web/index.html" "$ROOT/web/runtime.js" "$ROOT/web/dist/"
cp "$ROOT/web/runtime.js" "$ROOT/vscode-extension/media/runtime.js"
"$PYTHON" "$ROOT/tools/make_test_rom.py" "$ROOT/web/dist/solid.nes"
echo "浏览器页面：$ROOT/web/dist/index.html"
echo "插件产物：$ROOT/vscode-extension/media/SimpleNES.js"
