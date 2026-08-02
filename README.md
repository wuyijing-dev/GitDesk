# GitDesk

现代化 Git 可视化工作台（Qt 6.10+ / C++20 / QML / Md3）。

## 布局（Md3）

| 区域 | 组件 |
|------|------|
| 顶栏 | `Md3ApplicationWindow.toolBar` → `Md3AppToolBar` + `Md3HStack` / `Md3Spacer` |
| 底栏 | `statusBar` → `Md3StatusBar` |
| 分栏 | `Md3SplitView`（Explorer / Workspace / Detail） |
| 工作区 | `Md3VStack` + `Md3TabBar` + `expand` 页宿主 |
| 列表/树 | `Md3ListTile` / `Md3TreeView` / `Md3PageSection` |
| Graph | 手写 Canvas（专用可视化）；缩放控件仍用 Md3 |

## 构建

```powershell
# 需 ./Md3（junction/copy 自 QML_MD3 dist/Md3，建议 pin v1.1.2）
cmake -S . -B build -DCMAKE_PREFIX_PATH="D:/Qt/6.10.2/msvc2022_64"
cmake --build build --config Release
```

## 快捷键

- `Ctrl+O` 打开仓库
- `Ctrl+R` 刷新
- `Ctrl+K` 命令面板
- `Ctrl+Enter` 提交（有 message 时）
