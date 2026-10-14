const vscode = require("vscode");
const path = require("path");

function nonce() {
    const alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    let value = "";
    for (let i = 0; i < 32; i++) {
        value += alphabet[Math.floor(Math.random() * alphabet.length)];
    }
    return value;
}

class NesEditorProvider {
    constructor(context) {
        this.context = context;
    }

    openCustomDocument(uri) {
        return { uri, dispose() {} };
    }

    async resolveCustomEditor(document, webviewPanel) {
        const webview = webviewPanel.webview;
        const media = vscode.Uri.file(path.join(this.context.extensionPath, "media"));
        webview.options = {
            enableScripts: true,
            localResourceRoots: [media],
        };

        const scriptUri = webview.asWebviewUri(vscode.Uri.joinPath(media, "SimpleNES.js"));
        const runtimeUri = webview.asWebviewUri(vscode.Uri.joinPath(media, "runtime.js"));
        const token = nonce();
        const csp = webview.cspSource;

        webview.html = `<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8">
  <meta http-equiv="Content-Security-Policy" content="default-src 'none'; style-src ${csp} 'nonce-${token}'; script-src ${csp} 'nonce-${token}' 'wasm-unsafe-eval';">
  <style nonce="${token}">
    html, body { margin: 0; width: 100%; height: 100%; background: #000; overflow: hidden; }
    canvas {
      position: fixed;
      inset: 0;
      width: 100%;
      height: 100%;
      background: #000;
      object-fit: contain;
      image-rendering: pixelated;
      image-rendering: crisp-edges;
      outline: none;
    }
    #hud {
      position: fixed;
      left: 16px;
      bottom: 16px;
      z-index: 2;
      max-width: min(420px, calc(100% - 32px));
      margin: 0;
      padding: 12px 36px 12px 14px;
      border-radius: 8px;
      background: rgba(20, 20, 20, 0.82);
      color: #ddd;
      font: 13px/1.45 sans-serif;
      box-shadow: 0 8px 24px rgba(0, 0, 0, 0.35);
    }
    #hud[hidden] { display: none; }
    #hud p { margin: 0; }
    #hud p + p { margin-top: 6px; color: #bbb; }
    #hud-close {
      position: absolute;
      top: 6px;
      right: 8px;
      width: 24px;
      height: 24px;
      padding: 0;
      border: 0;
      border-radius: 4px;
      background: transparent;
      color: #fff;
      font: 18px/24px sans-serif;
      cursor: pointer;
    }
    #hud-close:hover { background: rgba(255, 255, 255, 0.12); }
  </style>
</head>
<body>
  <canvas id="screen" width="256" height="240" tabindex="0"></canvas>
  <aside id="hud">
    <button id="hud-close" type="button" aria-label="关闭">×</button>
    <p id="status">正在加载模拟器…</p>
    <p>W A S D 方向，J = A，K = B，Enter = Start，右 Shift = Select。先点一下画面。</p>
  </aside>
  <script nonce="${token}" src="${scriptUri}"></script>
  <script nonce="${token}" src="${runtimeUri}"></script>
  <script nonce="${token}">
    const vscodeApi = acquireVsCodeApi();
    const statusEl = document.getElementById("status");
    const canvas = document.getElementById("screen");
    const hud = document.getElementById("hud");
    if (vscodeApi.getState() && vscodeApi.getState().hudHidden) hud.hidden = true;
    document.getElementById("hud-close").addEventListener("click", () => {
      hud.hidden = true;
      vscodeApi.setState({ hudHidden: true });
      canvas.focus();
    });
    createSimpleNES().then((Module) => {
      const player = attachSimpleNES(Module, canvas);
      window.addEventListener("message", (event) => {
        const message = event.data;
        if (!message || message.type !== "rom") return;
        const binary = atob(message.base64);
        const bytes = new Uint8Array(binary.length);
        for (let i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i);
        const rc = player.loadRom(bytes);
        statusEl.textContent = rc === 0 ? "运行中" : "这个文件不是可运行的 iNES ROM";
      });
      statusEl.textContent = "等待 ROM…";
      vscodeApi.postMessage({ type: "ready" });
    }).catch((error) => {
      statusEl.textContent = "模拟器加载失败：" + error;
    });
  </script>
</body>
</html>`;

        const bytes = await vscode.workspace.fs.readFile(document.uri);
        const base64 = Buffer.from(bytes).toString("base64");
        const subscription = webview.onDidReceiveMessage((message) => {
            if (message && message.type === "ready") {
                webview.postMessage({ type: "rom", base64 });
            }
        });
        webviewPanel.onDidDispose(() => subscription.dispose());
    }
}

function activate(context) {
    context.subscriptions.push(
        vscode.window.registerCustomEditorProvider(
            "simplenes.player",
            new NesEditorProvider(context),
            {
                webviewOptions: { retainContextWhenHidden: true },
                supportsMultipleEditorsPerDocument: false,
            }
        )
    );
}

function deactivate() {}

module.exports = { activate, deactivate };
