# SimpleNES

在 VS Code / Cursor 里打开 `.nes` 文件，用 WebAssembly 运行这个 NES 模拟器。

玩家 1：W A S D 方向，J = A，K = B，Enter = Start，右 Shift = Select。先点一下画面。

安装打好的包：

```sh
cursor --install-extension simplenes-0.1.0.vsix
```

重新打包：

```sh
scripts/package-vscode.sh
```
