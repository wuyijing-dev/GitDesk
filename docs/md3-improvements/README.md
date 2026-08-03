# Md3 易用性 Gaps — GitDesk 消费方跟踪

> **来源：** GitDesk（Qt 6.10 / C++20 / QML · 当前 **0.5.0**）  
> **库基线：** **Md3 v1.1.3**（下列【库已合】项在 **main / Unreleased**，待下一 patch 发版后可删 App 规避）  
> **目的：** 只跟踪**尚未闭环**的易用性债  
> **官方：** [layout](https://wuyijing-dev.github.io/QML_MD3_Document/guides/layout/) · [dialogs-and-open](https://wuyijing-dev.github.io/QML_MD3_Document/guides/dialogs-and-open/) · [feedback](https://wuyijing-dev.github.io/QML_MD3_Document/guides/feedback/) · [collections](https://wuyijing-dev.github.io/QML_MD3_Document/guides/collections/)

---

## 状态图例

| 前缀 | 含义 |
|------|------|
| **【开放】** | 建议库侧新增 / 改进；App 已有规避或临时写法 |
| **【库已合】** | 库 main 已有 API；App 仍待切换 / 等发版 |
| **【观察】** | 库已有能力，App 可选未切 |
| **【须知】** | 非缺陷：官方契约 / 反模式，遵守即可 |

---

## 1. 总览

| 优先级 | 主题 |
|--------|------|
| **P0（库已合）** | SideSheet / BottomSheet / Flyout `writeOpenOnClose`（§4.1 / §4.6）— App 可去 `syncDetailSheet` |
| **P1** | `menuModel` locale、ListTile trailing 溢出、SideSheet 嵌套高度、Diff hunk 统一视图、Dialog 工厂 |
| **P1（库已合）** | InfoBar 多动作（§3.2）；PageSection trailing（§3.9）；Scroll gutter API（§2.2） |
| **P2** | TreeView flex / Scroll 测高、受控 Toggle / 双向 text、Card 测高、工具栏密度、PageScaffold、CommandPalette 分组、DialogHost、CodeBlock fill、Card actions |
| **P3** | Icon `color`；Divider 竖线；嵌套 Split 预设（观察） |

---

## 2. 布局系统

### 【观察】2.1 Divider 垂直

- **库：** `orientation` / `vertical: true`
- **App：** 暂未全面替换手写竖线（可选）

### 【须知】侧栏三段式

```qml
Item { // SplitView 直接子项：不要 anchors.fill
    Item { id: header; anchors.top: parent.top; /* … */ height: … }
    Item { id: footer; anchors.bottom: parent.bottom; /* … */ height: … }
    Md3ScrollView {
        anchors.top: header.bottom
        anchors.bottom: footer.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
```

### 【库已合】2.2 ScrollView 内容宽 gutter

**库：** `verticalScrollbarGutter` + `contentAvailableWidth`（默认 gutter `0`，侧栏可设 `4` 或 `scrollBarThickness`）。

**App：** 可将 `width: scroller.width - 4` 改为 gutter / `contentAvailableWidth`。

### 【开放】2.3 TreeView 在滚动列中的弹性高度

**现象：** `RepoExplorer` 用 `preferredMaxHeight: Math.max(180, Math.floor(scroller.height * 0.42))` 让树随侧栏变高。

**库侧建议：** `flex` / `expandInScrollColumn` / `preferredHeightFraction`.

### 【开放】2.4 页级 Scroll 测高不可靠时退回 Flickable

**现象：** `OverviewPage` 不用 `Md3ScrollView`，改手写 `Flickable`。

**库侧建议：** 加固 `Md3ScrollView` 在 Tab / Fit 宿主中的测高，或提供 `Md3ScrollPage`。

### 【开放】2.5 SplitView 内 VStack `expand` → pageHost 高度为 0（0.5.x）

**现象：** Tab + pageHost 必须用纯 `anchors`；`Md3VStack` + `expand` 会把 `pageHost` 算成 **0**。

**库：** layout 指南已写明；仍无 `expandInParent` 安全模式 / `Md3WorkspacePane` 可选。

### 【开放】2.6 ApplicationWindow Fit 多子项互抢（0.5.x）

**现象：** 欢迎页与仓库 Split 靠 `visible` 切换。

**库：** layout 指南已写「单宿主」；仍缺 `activeView` API。

### 【开放】2.7 页三段式：header · scroll · sticky footer（0.5.x）

**库侧建议：** `Md3PageScaffold { header; body; stickyFooter }`（通用名，非产品壳）。

### 【观察】2.8 嵌套 SplitView（explorer | workspace 再套 list | diff）

**库侧建议：** 嵌套 Split cookbook，或 `Md3InspectorLayout`（列表 + 详情）预设。

---

## 3. 控件 API 与命名

### 【观察】3.1 Icon `color` / `iconColor`

- **库：** 文档 / 别名
- **App：** 仍多用 `iconColor`（可用，非阻塞）

### 【库已合】3.2 InfoBar 多动作

**库：** `secondaryActionText` + `secondaryActionClicked`。

**App：** 可去掉 InfoBar 下另挂「中止」行。

### 【开放】3.3 动态 `menuModel` / `model` 需手动吃 `locale.revision`

**库侧建议：** 内建 locale 失效（或官方 `retranslate()` / 绑定 `Qt.uiLanguage`）。

### 【开放】3.4 ListTile trailing 挤满 3～5 个 IconButton

**库侧建议：** `trailingActions` + overflow、`maxVisibleTrailingActions`。

### 【开放】3.5 ToggleIconButton 外驱 `checked` 需双向手写

**库侧建议：** 受控模式 / `bindChecked`，语义对齐 `writeOpenOnClose`。

### 【开放】3.6 TextArea / 表单控件与 singleton 双向 `text`

**库侧建议：** 安全双向 `text`（如 `bindText`）。

### 【开放】3.7 Md3Card body + `anchors.fill` → 隐式高度为 0

**库侧建议：** 修 implicitHeight，或提供 `Md3StatTile`。

### 【开放】3.8 Diff hunk 视图与 CodeBlock 分叉（0.5.0）

**库侧建议：** `Md3DiffBlock` 或 CodeBlock **hunk 槽**（`hunkActions` / 每段 footer）。

### 【库已合】3.9 PageSection header 动作槽

**库：** `Md3PageSection.trailing`。

**App：** Explorer「远程 / 标签」节头可改用 `trailing:`。

### 【开放】3.10 CodeBlock `maxHeight` 需手减 chrome（0.5.x）

**库侧建议：** `Md3CodeBlock { fill: true }` 在拉伸布局中自动吃剩余高度。

### 【开放】3.11 Card / SideSheet 内长操作列（0.5.0）

**库侧建议：** `Md3Card.actions` + overflow；或紧凑 `Md3ActionRow`。

### 【开放】3.12 嵌入 ListView / VirtualList 固定高度公式（延伸 4.3）

**库侧建议：** `fillAvailableHeight` / `flexInScrollColumn`；稳定的 delegate 宽绑定。

---

## 4. 应用壳层 / 对话框 / 面板

### 【库已合】4.1 SideSheet `writeOpenOnClose`

**库：** `writeOpenOnClose`（默认 `true`）；绑定外驱时设 `false` 并在 `onDismissed` 清状态。见 [dialogs-and-open](https://wuyijing-dev.github.io/QML_MD3_Document/guides/dialogs-and-open/)。

**App：** 发版后可删 `syncDetailSheet()`。

### 【须知】4.2 `visible` 须为严格 bool

消费方写 `visible` 时避免 `a && b` 短路返回 `undefined`。

### 【开放】4.3 SideSheet 内嵌套滚动体固定高度

**库侧建议：** Scroll vs Fit 指南；嵌入列表 `fillAvailableHeight`。

### 【观察】4.4 About `aboutContent` 滚动

**库（1.1.3）：** About 宿主已可滚；`aboutDialogHeight` / `aboutDialogWidth`。`aboutContent` 仍宜纯布局列，勿再套 Flickable。

### 【开放】4.5 AppToolBar 密度：DropDown / AppBarButton / Filled 混排

**库侧建议：** `Md3AppBarDropDown`、`density: Compact`、或 overflow「更多」槽。

### 【库已合】4.6 Overlay 族 `writeOpenOnClose`

BottomSheet / Flyout 已与 FullscreenDialog 同契约。

### 【开放】4.7 Dialog 农场 + pending 样板（0.5.0）

**库侧建议：** `Md3ConfirmDialog.show({…})` / `Md3PromptDialog` / `DialogService`。

### 【开放】4.8 深层组件经 `Window.window` 调对话框（0.5.x）

**库侧建议：** `Md3DialogHost` attached、或窗口级 `dialogs`。

### 【开放】4.9 Dialog 内容宽 `parent ? parent.width : 280`（0.5.x）

**库侧建议：** Dialog 暴露稳定 `contentWidth` / 默认表单宽。

### 【开放】4.10 CommandPalette 扁平无分组（0.5.0）

**库侧建议：** `{ section, items }` 分组、`visibleWhen` / context tags。

---

## 5. 文档与发现性

### 【库已合】dialogs-and-open 已扩写 SideSheet / BottomSheet / Flyout

### 【库已合】layout 指南补 SplitView × VStack expand / Fit 单宿主 / scrollbar gutter

### 【开放】collections / feedback：Diff hunk、Confirm 工厂、Palette 分组

对齐 §3.8 / §4.7 / §4.10。（InfoBar 双动作已写入 feedback）

---

## 6. 优先级（给库）

| 优先级 | 项 |
|--------|-----|
| **P0 已合** | SideSheet / BottomSheet / Flyout `writeOpenOnClose` |
| **P1** | locale 失效（§3.3）；ListTile trailing（§3.4）；SideSheet / 列表可填高度（§4.3 / §3.12）；**Diff hunk（§3.8）**；**Dialog 工厂（§4.7）** |
| **P1 已合** | InfoBar 多动作；PageSection trailing；Scroll gutter |
| **P2** | Tree flex / ScrollPage（§2.3–2.4）；PageScaffold（§2.7）；受控 Toggle / 双向 text（§3.5–3.6）；Card / StatTile（§3.7）；CodeBlock fill（§3.10）；Card actions（§3.11）；工具栏密度（§4.5）；DialogHost（§4.8）；Dialog contentWidth（§4.9）；Palette 分组（§4.10） |
| P3 | Icon / Divider（§2.1 / §3.1）；嵌套 Split 预设（§2.8） |

---

## 7. App 临时规避（待库闭环后可删）

| 模式 | 待 |
|------|-----|
| `syncDetailSheet()` 双写 | §4.1 — 库已合，切 `writeOpenOnClose: false` 后可删 |
| `scroller.width - 4` | §2.2 — 库已合，改 `verticalScrollbarGutter` |
| `locale.revision` 读一下 | §3.3 内建失效 |
| InfoBar 下再挂「中止」行 | §3.2 — 库已合，改 `secondaryActionText` |
| Diff / Blame 固定 280px / count×56 | §4.3 / §3.12 |
| StatCard 手写 Rectangle | §3.7 Card 测高 |
| Overview 手写 Flickable | §2.4 Scroll 测高 |
| Diff hunk 用 Card+Text 分叉 | §3.8 DiffBlock |
| Main 二十多个 Dialog + pending* | §4.7 Dialog 工厂 |
| `Window.window.confirmX` | §4.8 DialogHost |
| 工作区纯 anchors 避 VStack expand | §2.5（文档已写；API 仍开放） |
| 欢迎/仓库同 Item 切 visible | §2.6（文档已写） |
| Explorer 节头手写 HStack 动作 | §3.9 — 库已合，改 `trailing:` |
| CodeBlock 手减 56px chrome | §3.10 fill |

---

## 8. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-08-03 | 初稿 → 跟踪表；采用 1.1.3 API；标记【完成】 |
| 2026-08-03 | 新增【开放】一批；**清除全部【完成】项** |
| 2026-08-03 | **对齐 0.5.0**：新增 §2.5–2.8、§3.8–3.12、§4.7–4.10、§5 文档项 |
| 2026-08-03 | **对照库 main**：§4.1/4.6/3.2/3.9/2.2 标【库已合】；About §4.4→观察；刷新总览 |

---

*面向 QML_MD3 维护者与 GitDesk 贡献者。官方 API 以文档站为准。*
