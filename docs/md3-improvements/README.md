# Md3 组件库易用性改进清单（GitDesk 消费方反馈）

> **来源：** GitDesk（Qt 6.10 / C++20 / QML · Md3 v1.1.x）接入与 IDE 壳层实作  
> **目的：** 汇总布局与 API 上阻碍「开箱即用」的点，供库侧迭代优先级参考  
> **原则：** 能用组件就用；库应减少隐蔽约束与文档外知识，而不是要求应用侧记忆一堆例外

---

## 1. 总览：当前最大摩擦

GitDesk 是 **IDE 风格**（顶栏 + 左栏 + 多页工作区 + 旁路详情），不是 Gallery / 单页表单。接入中最耗时的不是「缺控件」，而是：

1. **布局契约不显式**（谁设宽高、谁禁止 `anchors.fill`、谁会强制撑高）
2. **组合组件互相打架**（`ScrollView` × `VStack expand` × `SplitView` × `TreeView`）
3. **命名/语义不一致**（如 EmptyState 的 `body` vs 直觉上的 `description`）
4. **壳层模式偏「目的地导航」**，对「自定义 IDE 壳」文档与示例不足
5. **对话框 `open` 与外部状态双向同步易断**（设置页第二次打不开）

建议库侧把「消费方易踩坑」提升到与控件 API 表同等优先级。

---

## 2. 布局系统（最高优先级）

### 2.1 `Md3SplitView`：子项禁止 `anchors.fill`（隐蔽、破坏力大）

**现象：** 子项再写 `anchors.fill: parent` 会与分栏手写 `width/height` 抢几何，出现重叠、黑块、拖不动。

**现状：** 实现里对 pane 直接赋值 `x/y/width/height`；文档示例未强调「子项禁止填满锚点」。

**改进建议：**

| 项 | 建议 |
|----|------|
| 文档 | 在 API / layout 指南置顶警告，附正确/错误对照 |
| API | `property bool manageGeometry: true`；或提供 `Md3SplitPane { fill: true }` 内部托管 |
| 运行时 | Debug 下若子项 `anchors.fill` 打 `console.warn` |
| 示例 | SerialFlow 式「外层 Split 不定锚、内层 Item 再 `anchors.fill`」官方样例 |

### 2.2 `Md3SplitView`：无法真正收起一侧

**现象：** `splitRatio` 被夹在 `[0.05, 0.95]`，`minPane2` 再大也会占位；详情栏「关闭」仍占宽。

**改进建议：**

- 支持 `collapsed` / `pane2Visible`，或允许 `splitRatio` 到 `0`/`1`
- 文档明确：可折叠详情优先用 `Md3SideSheet`，并给 IDE「固定旁栏 vs SideSheet」选型表

### 2.3 `Md3VStack` / `Md3HStack`：`expand: true` 难学、易误用

**现象：** IDE 侧栏 / Changes 用 `expand: true` 的 `Md3ScrollView` 时，视口被撑满或与底部 Commit 叠压。

**改进建议：**

- 文档专节：「何时用 `expand` / 何时用 anchors 定高」
- 提供 `Md3Fill` / `Md3ExpandingScrollView` 语义更清晰的封装
- Gallery 增加「侧栏：固定头 + 剩余滚动 + sticky footer」标准片段（与 GitDesk 同构）

### 2.4 `Md3ScrollView`：`contentHeight = max(viewport, content)` 的副作用

**现象：** 内容比视口矮时，下方仍是可滚「空区」，深色主题下像黑矩形盖住后续区块。

**改进建议：**

- 默认改为「内容矮于视口则 `contentHeight = content`」（可选 `minContentHeight: viewport`）
- 或文档写明：ScrollView 只包「自然高度内容」，不要把 expand 视口当成「内容高度」

### 2.5 `Md3TabBar`：不适合作为「铺满剩余高度的页容器」

**现象：** `height: implicitHeight`；带 `pages` 时靠 `pageAreaHeight`，IDE 主区很难「Tab + 下面填满」。

**改进建议：**

- `fillHeight: true` / `Layout.fillHeight` 友好模式：页区吃掉剩余高度
- 文档推荐两种模式：`tabsOnly`（条） vs `tabsWithPages`（条+页），并说明 IDE 常用「条 + 外部 pageHost」

### 2.6 `Md3ContainerBody`（Fit）与 `anchors.fill` 子项

**现象：** FullscreenDialog / Card 内 Fit 宿主与子项填满规则复杂；设置页曾因宽度上限 + Fit 显得「没铺满」。

**改进建议：**

- 文档用一张表说明：Fit / Scroll × 有无 `anchors.fill` × 高度从哪来
- FullscreenDialog 提供 `contentMargins` / `contentPadding` 可配（现写死 24）
- Gallery：FullscreenDialog 内「全宽设置页」官方例

### 2.7 `default property` 是 `content` 还是 `data`

**现象：** `Md3HStack`/`Md3VStack` 的 default 是 `content`；`PageHeader` 尾钮若误用 HStack 当 default 宿主会丢子项。规则已写在 Cursor rule，但对新人仍属「暗知识」。

**改进建议：**

- 所有布局壳统一文档句式：「子项必须进入 `content`，禁止 alias 到 `data`」
- qmllint 自定义检查（若可行）或 Gallery 错误示例页

### 2.8 `Md3Divider` 仅水平

**现象：** 工具栏需要竖分割线时只能手写 `Rectangle`。

**改进建议：** `orientation: Qt.Horizontal | Qt.Vertical`（或 `vertical: true`）

### 2.9 `Md3TreeView` 高度与 Scroll 嵌套

**现象：** 靠 `HeightSync` + `preferredMaxHeight`；放进大高度 ScrollView 时，列表空白区容易被误解为「下面列表坏了」。

**改进建议：**

- 在 Scroll 内默认「内容高度」模式文档化
- `preferredMaxHeight` 默认示例与侧栏片段绑定说明
- `unloadWhenPageInactive` 在非 PageHost 场景的行为写清楚（GitDesk 侧栏需关掉）

---

## 3. 控件 API 与命名一致性

### 3.1 `Md3EmptyState`

| 现状 | 直觉 |
|------|------|
| `body` | 很多人写 `description` → 直接加载失败 |

**改进：** 增加 `property alias description: body`，或文档/示例统一用词，并在破坏性变更说明里列出。

### 3.2 多行文本

**现状：** 无独立 `Md3TextArea`，靠 `Md3TextField { multiline: true }`。

**改进：** 增加 `Md3TextArea` 别名类型（或文档「表单多行」一节置顶），降低搜索成本。

### 3.3 `Md3Icon` 颜色属性

**现状：** `iconColor`，易误写成 `color`。

**改进：** `property alias color: iconColor` 或文档/示例统一。

### 3.4 列表模型与 Repeater

**现象：** `QAbstractListModel` + `required property` 角色绑定偶发失败；`QVariantList` 更稳。

**改进：**

- 文档：「侧栏短列表用 `QVariantList` / `ListModel`；长列表用 `ListView` + 角色」
- Gallery：Branch/File 列表两种绑定对照

### 3.5 `Md3AssistChip` / 轻操作

文档已有 glue-less；建议补充「可点击 Chip 是否冒泡 / 是否该用 Button」的选用表（与 design-guidelines 对齐）。

---

## 4. 应用壳层（ApplicationWindow）

### 4.1 强项

- `toolBar` / `statusBar` 槽位清晰
- `destinations` + Rail 对「多页工具型 App」很强（CleanSpace / SerialFlow）
- `Md3SideSheet` 做旁路详情符合设计指南，比硬塞第三栏 Split 更稳

### 4.2 缺口：IDE / 自定义壳示例

GitDesk 不用 destinations，而是：

`toolBar + Split(Explorer | Workspace(Tab+Pages)) + SideSheet(Detail)`

**改进建议：**

- 新增文档：`guides/ide-shell.md`（或 `desktop-ide-patterns.md`）
- Gallery / examples：`hello-ide-shell`（假数据即可）
- 明确推荐：详情 → SideSheet；可拖分栏 → SplitView；设置 → FullscreenDialog

### 4.3 `Md3StatusBar`

**现象：** `text` 与 `centerText` 易重复展示同一仓库状态。

**改进：** 文档给「左 / 中 / 右」职责示例；或 `secondaryText` 语义更清晰。

### 4.4 设置页全宽

**现象：** 消费方曾 `Math.min(720, width)` 居中，显得「没铺满」；FullscreenDialog 边距固定。

**改进：** 官方设置页模板默认全宽；可选 `contentMaxWidth` 由应用决定，而不是示例暗示必须窄栏。

### 4.5 消费方启动契约

`RESOURCE_PREFIX`、`QT_RESOURCE_ALIAS`、共享 `Md3.dll` 部署等已有 `consumer-app-main-qml`——建议在 quickstart 第一屏用检查清单（GitDesk / auto_deploy_Qt 已验证）。

### 4.6 `Md3FullscreenDialog.open` 与外部状态双向绑定易断（新）

**现象（GitDesk 设置页）：**

```qml
Md3FullscreenDialog {
    open: window.settingsOpen   // 绑定
    onConfirmed: window.settingsOpen = false
}
```

对话框内部 `accept()` / `reject()` 会执行 `open = false`，**打断**对 `window.settingsOpen` 的绑定。之后再把 `settingsOpen = true`，对话框不再打开。欢迎页 / 顶栏「设置」表现为「点了没反应」。

**改进建议：**

| 项 | 建议 |
|----|------|
| API | `accept()`/`reject()` 改为发 `requestClose`，由外部写 `open`；或提供不打断绑定的关闭路径 |
| 文档 | 明确警告：`open: someProp` 与内部写 `open = false` 冲突；推荐 `Binding` / 强制同步函数 |
| 示例 | 设置页：`openSettings()` / `closeSettings()` 同时写窗口状态与 `dialog.open` |

**GitDesk 规避：** 去掉单向绑定，改用 `openSettings()` / `closeSettings()` 双写，并在 `onOpenChanged` 回写窗口状态。

### 4.7 欢迎页 / 子页通过 `Window.window` 写壳层属性（新）

**现象：** 子组件内 `Window.window.settingsOpen = true` 难调试；用函数 `w.openCloneDialog()` 相对稳，但仍不如信号清晰。

**改进建议：**

- 文档推荐：子组件用 **signal** 冒泡（`settingsRequested`），由 `ApplicationWindow` 统一打开对话框
- Gallery：欢迎页 CTA 打开 FullscreenDialog 的官方片段

### 4.8 `aboutContent` 与 About 对话框固定高度（新）

**现象：** About 对话框默认高度偏小；长 `aboutContent`（changelog）易被裁切，需内容侧自带 `Flickable`。

**改进建议：** About 默认加滚动；或暴露 `aboutDialogHeight`；文档注明长内容必须可滚。

### 4.9 `Md3DropDownButton`：主按钮只开菜单，无「默认动作」（新）

**现象（GitDesk 顶栏「拉取」）：** 想要「点一下 = ff-only Pull，下拉选 rebase」时，组件只有 `onClicked → toggleMenu()`，没有 `defaultAction` / `onPrimaryClicked`。

**改进建议：**

| 项 | 建议 |
|----|------|
| API | `signal primaryClicked` + `property bool menuOnly: true`（默认兼容）；或 `split: true` 主区动作 / 箭头开菜单 |
| 文档 | 工具栏「带默认动作的下拉」选用表：DropDown vs MenuButton vs 两个 AppBarButton |
| 示例 | IDE 工具栏：Fetch / Pull(split) / Push |

**GitDesk 规避：** 用 DropDown 两项菜单（ff-only / --rebase）；命令面板再暴露同名命令。

### 4.10 危险操作确认：`Md3Dialog` 文案动态化（新）

**现象：** Reset Soft / Mixed / Hard 共用一个对话框，靠外部 `pendingResetMode` 改 `text`。可行，但易漏 `confirmTone`（Hard 应更醒目）。

**改进建议：** 文档给「危险分级确认」片段：`confirmTone` × Soft/Hard；或 `Md3ConfirmDanger` 预设。

### 4.11 长列表塞进 SideSheet（Blame / 文件历史）（新）

**现象：** Blame 最多 2000 行用 `Repeater` + `Md3ListTile` 塞进 SideSheet 滚动区，首开卡顿、滚动卡顿。

**改进建议：**

- Gallery：SideSheet 内 `ListView` / `Md3VirtualList`（若有）官方片段
- 文档警告：SideSheet 内容区禁止大 `Repeater`；超过 N 行改虚拟列表
- API：SideSheet `content` 默认可滚时注明与内层 ListView 嵌套滚动冲突

**GitDesk 规避：** Blame 上限 2000；后续应改 `ListView`。

---

## 5. 文档与发现性

| 缺口 | 建议 |
|------|------|
| 布局「反模式」少 | 增加「会导致重叠的 5 个写法」 |
| IDE 壳用例少 | 见 4.2 |
| 属性同义名 | EmptyState / Icon 等 |
| 性能页与壳层交叉 | `md3PageActive`、TreeView unload 与自定义壳共存说明 |
| 控件选用表 | design-guidelines 很好；请链到 layout 反模式 |
| **对话框状态同步** | FullscreenDialog / Dialog 与外部 `open` 绑定的正确写法（见 4.6） |
| **工具栏下拉** | DropDown「仅菜单」vs「主动作+菜单」选型（见 4.9） |
| **SideSheet 大数据** | Blame / log 类长列表禁用 Repeater（见 4.11） |

建议目录：

```
docs/guides/
  layout.md              # 已有 → 加强反模式
  ide-shell.md           # 新增
  layout-antipatterns.md # 新增（可与 layout 合并）
  dialogs-and-open.md    # 新增：open 绑定 / Binding / 强制同步
```

---

## 6. 建议优先级（库侧）

### P0 — 少踩坑、少崩溃式 QML 错误

1. SplitView：文档警告 + debug 检测 `anchors.fill`
2. EmptyState：`description` 别名
3. ScrollView：可选不强制 `contentHeight ≥ viewport`
4. IDE 壳 + 侧栏「头固定 + 下滚动」官方片段
5. **FullscreenDialog：勿在 accept 中打断外部 `open` 绑定（或文档强制同步写法）**

### P1 — 更好用

6. SplitView 可折叠 / SideSheet 选型表  
7. TabBar `fillHeight` 或 IDE pageHost 模式文档  
8. Divider 垂直  
9. FullscreenDialog `contentMargins`  
10. `Md3TextArea` 别名  
11. About 对话框可滚 / 可调高  
12. DropDownButton split / primaryClicked（工具栏默认动作）  
13. SideSheet + VirtualList 官方片段（Blame / 长历史）  

### P2 — 提升

14. qmllint / 静态检查布局契约  
15. `Md3SplitPane` 声明式 API  
16. 设置页 / IDE shell 示例工程进 examples  
17. 危险确认对话框分级预设（Reset Hard 等） 

---

## 7. GitDesk 侧已采用的规避策略（供对照）

| 问题 | 规避 |
|------|------|
| 三栏 Split 关不掉 | 详情改 `Md3SideSheet` |
| Split 子项锚点冲突 | 子项不用 `anchors.fill`；内层 Item 再填满 |
| 侧栏 / Changes 叠压或大片留白 | 去掉 ScrollView `expand`；**header + Scroll + sticky footer** |
| 分支列表绑定不稳 | `localBranchNames`（`QVariantList`） |
| 设置页不宽 | 去掉 `maxWidth: 720`，`width: scroller.width` |
| 空状态加载失败 | `description` → `body` |
| 设置第二次打不开 | `openSettings()` / `closeSettings()` 双写；欢迎页 `settingsRequested` |
| Tab 工作区高度为 0 | 不用 `VStack expand` 包 pageHost；TabBar + anchors 填满剩余 |
| Pull 要默认动作 + rebase | DropDown 两项菜单；命令面板同步暴露 |
| Reset Hard 误触 | 共用确认框 + 文案按 mode 切换；Hard 用 Error tone |
| Blame 塞 SideSheet | 上限截断；后续改 ListView |

这些规避说明：**库缺的是契约与模板，不是能力本身。**

### 7.1 推荐片段：侧栏 / 变更页「三段式」

```qml
Item { // SplitView 的直接子项：不要 anchors.fill
    Item { id: header; anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right; height: … }
    Item { id: footer; anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right; height: … }
    Md3ScrollView {
        anchors.top: header.bottom
        anchors.bottom: footer.top
        anchors.left: parent.left
        anchors.right: parent.right
        // 不要 property bool expand: true
    }
}
```

建议库 Gallery 直接收录（标题：「IDE 侧栏三段式」）。

---

## 8. 期望的「易用」定义（验收口径）

做到以下时，可认为布局易用性达标：

1. 新人按 `ide-shell` 示例 30 分钟内搭出「顶栏 + 左栏 + Tab 工作区 + 详情 Sheet」且无重叠  
2. Split / Scroll / VStack 组合在文档中有明确允许矩阵  
3. 常见别名（`description` / `color` / TextArea）不导致整页 `Type unavailable`  
4. 侧栏、设置全宽、可折叠详情均有可复制片段，无需读组件源码猜高度策略  
5. **FullscreenDialog / 设置页可反复打开关闭，无需应用侧记忆「双写 open」**  
6. **工具栏下拉可「一点默认动作 / 箭头开菜单」**，无需拆成两个按钮或纯菜单  
7. **SideSheet 内万级行列表有官方虚拟列表模板**，不靠 Repeater 硬撑  

---

## 9. GitDesk 0.3.0 消费方观察（强操作 UI）

以下不是 Md3 缺陷，而是 IDE 类 Git 客户端叠在壳上时的组合经验，供 Gallery / 示例选型参考：

| 场景 | 做法 | 对库的启示 |
|------|------|------------|
| 冲突横幅 | Changes 顶栏 `errorContainer` + 中止/继续 | 需要「页面内持久 banner」模式，不只 Snackbar |
| Rebase / Merge 双态 | 同一条横幅按 `rebaseInProgress` 切换文案与按钮 | 状态机文案模板可进 design-guidelines |
| ours / theirs | ListTile trailing 两个 IconButton + 详情面板双按钮 | 列表行「双危险动作」密度指南 |
| 分支对比结果 | 写入 SideSheet 卡片 + 提交列表可点选 | SideSheet 适合「操作结果面板」，不止「当前选中」 |
| 命令面板镜像 | 每个顶栏/危险操作在 CommandPalette 再挂一份 | 文档：CommandPalette 与 ToolBar 命令同源清单 |

建议在 `ide-shell.md` 增加小节「异步任务 + 忙碌锁 + toast」与「确认对话框挂在 ApplicationWindow」两小节（GitDesk 全部确认挂在 `Main.qml`）。

---

## 10. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-08-03 | 初稿：基于 GitDesk 0.1 接入 Md3 v1.1.x 的实作反馈 |
| 2026-08-03 | 增补：FullscreenDialog `open` 绑定打断、欢迎页信号、About 高度、三段式侧栏、设置二次打开 |
| 2026-08-03 | 增补（0.3.0）：DropDown 无默认动作、危险确认分级、SideSheet Blame 大列表、强操作 UI 观察、优先级与规避表同步 |

---

*本文档位于 GitDesk 仓库，面向 **QML_MD3 / Md3** 维护者与消费方；不替代官方 API 表，而是补充「真实产品壳」视角的易用性债清单。*
