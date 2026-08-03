# Md3 易用性 Gaps — GitDesk 消费方跟踪

> **来源：** GitDesk（Qt 6.10 / C++20 / QML · 当前 **0.5.0**）  
> **库基线：** **Md3 main / Unreleased**（在 **1.1.3** 之上；待下一 patch 发版）  
> **目的：** 库侧缺口已闭环；本表只跟踪 **App 尚未切换** 的项  
> **官方：** [layout](https://wuyijing-dev.github.io/QML_MD3_Document/guides/layout/) · [dialogs-and-open](https://wuyijing-dev.github.io/QML_MD3_Document/guides/dialogs-and-open/) · [feedback](https://wuyijing-dev.github.io/QML_MD3_Document/guides/feedback/) · [collections](https://wuyijing-dev.github.io/QML_MD3_Document/guides/collections/)

---

## 状态图例

| 前缀 | 含义 |
|------|------|
| **【库已合】** | 库 main 已有 API；App 仍待切换 / 等发版 |
| **【观察】** | 库已有能力，App 可选未切 |
| **【须知】** | 非缺陷：官方契约 / 反模式，遵守即可 |

---

## 1. 总览

| 优先级 | 主题 |
|--------|------|
| **全部库已合** | 原 P0–P2 开放项均已在库 main 落地；GitDesk 待迁 API |
| **观察 / 须知** | Divider / Icon 别名；侧栏三段式；Fit 单宿主 |

---

## 2–5. 库 API 对照（原开放项）

| 原编号 | 库 API | App 待做 |
|--------|--------|----------|
| §2.2 | `verticalScrollbarGutter` / `contentAvailableWidth` | 替换 `width - 4` |
| §2.3 | TreeView `preferredHeightFraction` | 替换 `scroller.height * 0.42` 手算 |
| §2.4 | `Md3ScrollPage` | Overview 可改用 |
| §2.5–2.6 | layout 指南 | 保持 anchors / 单宿主 |
| §2.7 | `Md3PageScaffold` | Changes 页可迁 |
| §2.8 | `Md3InspectorLayout` | 嵌套 Split 可迁 |
| §3.2 | InfoBar `secondaryActionText` | 去「中止」另起行 |
| §3.3 | `Md3I18n.revision` / `bump()` | 模型绑定 revision |
| §3.4 | ListTile `trailingActions` + overflow | 去手写 5 图标 Row |
| §3.5 | `writeCheckedOnToggle: false` | 顶栏详情钮 |
| §3.6 | TextField `boundText` + `syncBoundText` | CommitComposer |
| §3.7 | Card `fillFallbackHeight` | StatCard 可回 Card |
| §3.8 | `Md3DiffBlock` | 统一 DiffViewer |
| §3.9 | PageSection `trailing` | Explorer 节头 |
| §3.10 | CodeBlock `fill: true` | 去手减 56px |
| §3.11 | `Md3ActionRow` / Card `actionsMaxVisible` | Detail 长按钮列 |
| §3.12 / §4.3 | List `fillAvailableHeight` / `preferredMaxHeight` | Blame 280px |
| §4.1 / §4.6 | Sheet/Flyout `writeOpenOnClose` | 删 `syncDetailSheet` |
| §4.4 | About 滚动（1.1.3） | 纯 Column content |
| §4.5 | AppToolBar `density` + `trailing` | 顶栏密度 / 更多 |
| §4.7 / §4.8 | `Md3DialogHost.confirm` / `prompt` | 减 pending* / Window.window |
| §4.9 | Dialog `preferredWidth` / `contentWidth` | 去 `parent ? … : 280` |
| §4.10 | CommandPalette `section` / `visibleWhen` | 分组模型 |

### 【须知】侧栏三段式 / Fit 单宿主 / visible 严格 bool

见 layout / dialogs-and-open 官方指南。

### 【观察】Divider `vertical` · Icon `color`/`iconColor`

App 可选未全面替换。

---

## 6. App 临时规避（发版后可删）

| 模式 | 改用 |
|------|------|
| `syncDetailSheet()` | `writeOpenOnClose: false` + `onDismissed` |
| `scroller.width - 4` | `verticalScrollbarGutter` |
| `locale.revision` 手读 | `Md3I18n.revision` |
| InfoBar 下「中止」行 | `secondaryActionText` |
| Diff/Blame 固定高度 | `preferredMaxHeight` / `fillAvailableHeight` / DiffBlock |
| StatCard 手写 Rectangle | Card + `fillFallbackHeight` |
| Overview 手写 Flickable | `Md3ScrollPage` |
| Diff hunk Card+Text | `Md3DiffBlock` |
| 二十多个 Dialog + pending* | `Md3DialogHost.confirm` / `prompt` |
| `Window.window.confirmX` | DialogHost |
| Explorer 节头 HStack | PageSection `trailing` |
| CodeBlock 手减 chrome | `fill: true` |
| 长操作按钮列 | `Md3ActionRow` |

---

## 7. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-08-03 | 初稿 → 0.5.0 对齐 |
| 2026-08-03 | 库合入 P0/部分 P1 |
| 2026-08-03 | **库合入全部剩余 gaps**；本表改为 App 迁移清单 |

---

*面向 QML_MD3 维护者与 GitDesk 贡献者。官方 API 以文档站为准。*
