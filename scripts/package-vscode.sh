#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
NODE_BIN=$(find "$ROOT/.tools/emsdk/node" -type f -path '*/bin/node' 2>/dev/null | head -1)
if [ -z "$NODE_BIN" ] || [ ! -x "$NODE_BIN" ]; then
    echo "缺少项目内的 Node。Emscripten 工具链应在 .tools/emsdk。" >&2
    exit 1
fi
export PATH="$(dirname "$NODE_BIN"):$PATH"

"$ROOT/scripts/build-wasm.sh"

VSCE_DIR="$ROOT/.tools/vsce"
if [ ! -x "$VSCE_DIR/node_modules/.bin/vsce" ]; then
    mkdir -p "$VSCE_DIR"
    npm install --prefix "$VSCE_DIR" --no-fund --no-audit @vscode/vsce
fi

OUT="$ROOT/simplenes-0.1.0.vsix"
cd "$ROOT/vscode-extension"
"$VSCE_DIR/node_modules/.bin/vsce" package \
    --allow-missing-repository \
    --skip-license \
    --out "$OUT"

echo "已生成 $OUT"
echo "安装：cursor --install-extension \"$OUT\""
