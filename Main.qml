import QtQuick
import Md3
import GitDesk

Md3ApplicationWindow {
    id: window
    width: 1180
    height: 720
    minimumWidth: 900
    minimumHeight: 560
    visible: true
    title: GitDeskApp.hasRepo
           ? qsTr("GitDesk — %1").arg(GitDeskApp.repoName)
           : qsTr("GitDesk")
    color: Md3Theme.colorScheme.surface
    roundedCorners: true
    cornerRadius: Md3WindowCapabilities.windowCornerRadius
    syncImmersiveDarkMode: true
    pagePadding: 0
    layoutMode: Md3ContainerBody.Fit
    defaultShowFocusRings: false
    showAboutButton: true
    showPerformanceButton: false
    aboutAppName: GitDeskApp.versionInfo.name
    aboutVersion: GitDeskApp.versionInfo.version
    aboutOrganization: GitDeskApp.versionInfo.organization
    aboutText: GitDeskApp.versionInfo.aboutPlainText
    aboutContent: AboutContent { }
    persistSession: true
    settingsOrganization: "wuyijing-dev"
    settingsApplication: "GitDesk"

    Component.onCompleted: {
        window.applyStoredSettings()
        Qt.callLater(function () {
            const key = "shell/compactWindowV1"
            const done = Md3AppSettings.value(key, false)
            if (done === true || done === "true" || done === 1)
                return
            width = 1180
            height = 720
            Md3AppSettings.setValue("window/width", 1180)
            Md3AppSettings.setValue("window/height", 720)
            Md3AppSettings.setValue(key, true)
            Md3AppSettings.sync()
        })
    }

    function asBool(v, fallback) {
        if (v === undefined || v === null)
            return fallback
        if (v === true || v === 1 || v === "1" || v === "true")
            return true
        if (v === false || v === 0 || v === "0" || v === "false")
            return false
        return fallback
    }

    function applyStoredSettings() {
        const lang = String(Md3AppSettings.value("settings/language", "zh-CN") || "zh-CN")
        GitDeskApp.locale.apply(lang)

        const dark = window.asBool(Md3AppSettings.value("settings/darkMode", true), true)
        const density = Number(Md3AppSettings.value("settings/density", 1))
        const seed = String(Md3AppSettings.value("settings/seedColor", "#1B6B4A") || "#1B6B4A")
        const focusRings = window.asBool(Md3AppSettings.value("settings/showFocusRings", false), false)
        Md3Theme.dark = dark
        Md3Theme.density = (!isNaN(density) && density >= 1) ? 1 : 0
        Md3Theme.applySeed(seed)
        window.defaultShowFocusRings = focusRings
        if (typeof Md3Accessibility !== "undefined")
            Md3Accessibility.showFocusRings = focusRings

        const gitPath = String(Md3AppSettings.value("settings/gitExecutable", "") || "")
        if (gitPath.length)
            GitDeskApp.setGitExecutable(gitPath)

        const tmpl = String(Md3AppSettings.value("settings/commitTemplate", "") || "")
        if (tmpl.length && !GitDeskApp.commitMessage.length)
            GitDeskApp.commitMessage = tmpl

        GitDeskApp.detailOpen = window.asBool(
                    Md3AppSettings.value("settings/detailOpenByDefault", true), true)
        window.showDiffLineNumbers = window.asBool(
                    Md3AppSettings.value("settings/showDiffLineNumbers", true), true)
    }

    property bool settingsOpen: false
    property bool showDiffLineNumbers: true

    onSettingsOpenChanged: {
        if (settingsOpen && settingsPage)
            settingsPage.load()
    }

    Connections {
        target: GitDeskApp
        function onNotify(message, severity) {
            if (severity === "error")
                Md3Notify.snackbar(message)
            else if (severity === "success")
                Md3Notify.toast(message, { severity: Md3Toast.Success })
            else
                Md3Notify.toast(message)
        }
        function onRepoChanged() {
            if (GitDeskApp.hasRepo) {
                GitDeskApp.detailOpen = window.asBool(
                            Md3AppSettings.value("settings/detailOpenByDefault", true), true)
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+O"
        context: Qt.ApplicationShortcut
        onActivated: GitDeskApp.pickRepository()
    }
    Shortcut {
        sequence: "Ctrl+R"
        context: Qt.ApplicationShortcut
        onActivated: GitDeskApp.refresh()
    }
    Shortcut {
        sequence: "Ctrl+Return"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (GitDeskApp.hasRepo && GitDeskApp.commitMessage.length > 0)
                GitDeskApp.commit()
        }
    }
    Shortcut {
        sequence: "Ctrl+K"
        context: Qt.ApplicationShortcut
        onActivated: commandPalette.open = !commandPalette.open
    }
    Shortcut {
        sequence: "Ctrl+,"
        context: Qt.ApplicationShortcut
        onActivated: window.settingsOpen = true
    }

    Md3CommandPalette {
        id: commandPalette
        model: {
            const _ = GitDeskApp.locale.revision
            return [
                { title: qsTr("打开仓库"), icon: "folder_open",
                  action: () => GitDeskApp.pickRepository() },
                { title: qsTr("克隆仓库"), icon: "cloud_download",
                  action: () => window.openCloneDialog() },
                { title: qsTr("初始化仓库"), icon: "create_new_folder",
                  action: () => GitDeskApp.pickAndInitRepository() },
                { title: qsTr("设置"), icon: "settings",
                  action: () => { window.settingsOpen = true } },
                { title: qsTr("刷新"), icon: "refresh",
                  action: () => GitDeskApp.refresh() },
                { title: qsTr("获取"), icon: "cloud_download",
                  action: () => GitDeskApp.fetch() },
                { title: qsTr("拉取"), icon: "download",
                  action: () => GitDeskApp.pull() },
                { title: qsTr("推送"), icon: "upload",
                  action: () => GitDeskApp.push() },
                { title: qsTr("Push 并设置上游"), icon: "upload",
                  action: () => GitDeskApp.pushSetUpstream() },
                { title: qsTr("贮藏"), icon: "inventory_2",
                  action: () => window.openStashDialog() },
                { title: qsTr("弹出贮藏"), icon: "unarchive",
                  action: () => GitDeskApp.stashPop() },
                { title: qsTr("丢弃全部未暂存"), icon: "delete",
                  action: () => window.openDiscardAllDialog() },
                { title: qsTr("新建标签"), icon: "sell",
                  action: () => window.openCreateTagDialog() },
                { title: qsTr("概览"), icon: "dashboard",
                  action: () => GitDeskApp.workspaceTab = 0 },
                { title: qsTr("图谱"), icon: "account_tree",
                  action: () => GitDeskApp.workspaceTab = 1 },
                { title: qsTr("变更"), icon: "difference",
                  action: () => GitDeskApp.workspaceTab = 2 },
                { title: qsTr("历史"), icon: "history",
                  action: () => GitDeskApp.workspaceTab = 3 },
                { title: qsTr("新建分支"), icon: "add",
                  action: () => window.openCreateBranchDialog() }
            ]
        }
        onActivated: (item) => { if (item.action) item.action() }
    }

    toolBar: TopToolbar {
        onCreateBranchRequested: window.openCreateBranchDialog()
        onSettingsRequested: window.settingsOpen = true
    }

    statusBar: Md3StatusBar {
        text: GitDeskApp.hasRepo ? GitDeskApp.repoName : qsTr("GitDesk")
        leadingIcon: "commit"
        centerText: GitDeskApp.busy
                    ? GitDeskApp.busyText
                    : (GitDeskApp.hasRepo
                       ? (GitDeskApp.hasUpstream
                          ? qsTr("%1 · ↑%2 ↓%3 · %4 changes")
                            .arg(GitDeskApp.currentBranch)
                            .arg(GitDeskApp.ahead)
                            .arg(GitDeskApp.behind)
                            .arg(GitDeskApp.changedFileCount)
                          : qsTr("%1 · %2 changes")
                            .arg(GitDeskApp.currentBranch)
                            .arg(GitDeskApp.changedFileCount))
                       : qsTr("Ctrl+O 打开仓库"))
    }

    // Single body host — prevents multi-child Fit layout fighting
    Item {
        anchors.fill: parent
        clip: true

        WelcomeView {
            anchors.fill: parent
            visible: !GitDeskApp.hasRepo
            z: 1
        }

        // Only ONE Md3SplitView: Explorer | Workspace
        // Detail uses Md3SideSheet (no nested split / no minPane steal)
        Md3SplitView {
            anchors.fill: parent
            anchors.margins: 4
            visible: GitDeskApp.hasRepo
            z: 0
            splitRatio: 0.22
            minPane1: 220
            minPane2: 480

            // Pane 1 — do NOT use anchors.fill (SplitView sets geometry)
            RepoExplorer {
                onCreateBranchRequested: window.openCreateBranchDialog()
                onCreateTagRequested: window.openCreateTagDialog()
                onDeleteTagRequested: function (name) {
                    window.pendingDeleteTag = name
                    deleteTagDialog.open = true
                }
            }

            // Pane 2 — workspace：纯 anchors，不用 VStack expand（会把 pageHost 高度算成 0）
            Item {
                id: workspacePane
                clip: true

                Md3TabBar {
                    id: workspaceTabs
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    variant: Md3TabBar.Primary
                    currentIndex: GitDeskApp.workspaceTab
                    onCurrentIndexChangedByUser: function (index) {
                        GitDeskApp.workspaceTab = index
                    }
                    model: {
                        // Depend on revision so tab labels retranslate after language switch
                        const _ = GitDeskApp.locale.revision
                        return [
                            { text: qsTr("概览") },
                            { text: qsTr("图谱") },
                            { text: qsTr("变更") },
                            { text: qsTr("历史") }
                        ]
                    }
                }

                Item {
                    id: pageHost
                    anchors.top: workspaceTabs.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.topMargin: 4
                    clip: true

                    OverviewPage {
                        anchors.fill: parent
                        visible: GitDeskApp.workspaceTab === 0
                    }
                    GraphPage {
                        anchors.fill: parent
                        visible: GitDeskApp.workspaceTab === 1
                    }
                    ChangesPage {
                        anchors.fill: parent
                        visible: GitDeskApp.workspaceTab === 2
                        onStashRequested: window.openStashDialog()
                        onDiscardAllRequested: window.openDiscardAllDialog()
                        onDiscardFileRequested: function (path) {
                            window.pendingDiscardPath = path
                            discardFileDialog.open = true
                        }
                    }
                    HistoryPage {
                        anchors.fill: parent
                        visible: GitDeskApp.workspaceTab === 3
                    }
                }
            }
        }

        // Detail as SideSheet — MD3 旁路详情，不挤主分栏
        Md3SideSheet {
            id: detailSheet
            edge: Md3SideSheet.End
            sheetWidth: 340
            modal: false
            open: GitDeskApp.hasRepo && GitDeskApp.detailOpen
            title: qsTr("详情")
            layoutMode: Md3ContainerBody.Scroll
            onDismissed: GitDeskApp.detailOpen = false

            DetailPanel {
                width: parent ? parent.width : 320
            }
        }
    }

    property string pendingDiscardPath: ""
    property string pendingDeleteTag: ""
    property string cloneParentDir: ""

    Md3Dialog {
        id: createBranchDialog
        title: qsTr("创建分支")
        text: qsTr("基于当前 HEAD 创建新分支")
        confirmText: qsTr("创建")
        dismissText: qsTr("取消")

        Md3TextField {
            id: branchNameField
            width: parent ? parent.width : 280
            label: qsTr("分支名")
            placeholderText: "feature/…"
        }

        onConfirmed: {
            const name = branchNameField.text.trim()
            if (name.length > 0)
                GitDeskApp.createBranch(name)
            branchNameField.text = ""
        }
    }

    function openCreateBranchDialog() {
        branchNameField.text = ""
        createBranchDialog.open = true
    }

    Md3Dialog {
        id: createTagDialog
        title: qsTr("新建标签")
        text: qsTr("在当前 HEAD 上创建标签")
        confirmText: qsTr("创建")
        dismissText: qsTr("取消")

        Md3VStack {
            width: parent ? parent.width : 280
            spacing: Md3Theme.spacingSm
            Md3TextField {
                id: tagNameField
                width: parent.width
                label: qsTr("标签名")
                placeholderText: "v0.2.0"
            }
            Md3TextField {
                id: tagMessageField
                width: parent.width
                label: qsTr("说明（可选，有说明则为 annotated）")
                placeholderText: qsTr("Release notes…")
            }
        }

        onConfirmed: {
            const name = tagNameField.text.trim()
            if (name.length > 0)
                GitDeskApp.createTag(name, tagMessageField.text.trim())
            tagNameField.text = ""
            tagMessageField.text = ""
        }
    }

    function openCreateTagDialog() {
        tagNameField.text = ""
        tagMessageField.text = ""
        createTagDialog.open = true
    }

    Md3Dialog {
        id: deleteTagDialog
        title: qsTr("删除标签？")
        text: qsTr("将删除本地标签 %1").arg(window.pendingDeleteTag)
        confirmText: qsTr("删除")
        confirmTone: Md3Dialog.Error
        dismissText: qsTr("取消")
        onConfirmed: {
            if (window.pendingDeleteTag.length)
                GitDeskApp.deleteTag(window.pendingDeleteTag)
            window.pendingDeleteTag = ""
        }
    }

    Md3Dialog {
        id: stashDialog
        title: qsTr("Stash")
        text: qsTr("保存当前工作区变更（含未跟踪文件）")
        confirmText: qsTr("保存")
        dismissText: qsTr("取消")

        Md3TextField {
            id: stashMessageField
            width: parent ? parent.width : 280
            label: qsTr("说明（可选）")
            placeholderText: qsTr("WIP…")
        }

        onConfirmed: {
            GitDeskApp.stashSave(stashMessageField.text.trim())
            stashMessageField.text = ""
        }
    }

    function openStashDialog() {
        stashMessageField.text = ""
        stashDialog.open = true
    }

    Md3Dialog {
        id: discardFileDialog
        title: qsTr("丢弃文件变更？")
        text: qsTr("将丢弃未暂存变更：%1").arg(window.pendingDiscardPath)
        confirmText: qsTr("丢弃")
        confirmTone: Md3Dialog.Error
        dismissText: qsTr("取消")
        onConfirmed: {
            if (window.pendingDiscardPath.length)
                GitDeskApp.discardFile(window.pendingDiscardPath)
            window.pendingDiscardPath = ""
        }
    }

    Md3Dialog {
        id: discardAllDialog
        title: qsTr("丢弃全部未暂存？")
        text: qsTr("将还原所有已跟踪文件的未暂存修改（不影响已暂存与未跟踪文件）。")
        confirmText: qsTr("丢弃全部")
        confirmTone: Md3Dialog.Error
        dismissText: qsTr("取消")
        onConfirmed: GitDeskApp.discardAll()
    }

    function openDiscardAllDialog() {
        discardAllDialog.open = true
    }

    Md3Dialog {
        id: cloneDialog
        title: qsTr("克隆仓库")
        text: qsTr("输入远程地址，并选择本地目标文件夹路径")
        confirmText: qsTr("克隆")
        dismissText: qsTr("取消")

        Md3VStack {
            width: parent ? parent.width : 320
            spacing: Md3Theme.spacingSm
            Md3TextField {
                id: cloneUrlField
                width: parent.width
                label: qsTr("仓库 URL")
                placeholderText: "https://github.com/org/repo.git"
            }
            Md3TextField {
                id: cloneDestField
                width: parent.width
                label: qsTr("目标目录（完整路径）")
                placeholderText: "D:/src/repo"
            }
            Md3Button {
                text: qsTr("选择父目录…")
                variant: Md3Button.Outlined
                icon: "folder_open"
                onClicked: {
                    const parent = GitDeskApp.pickCloneDirectory()
                    if (!parent || !parent.length)
                        return
                    window.cloneParentDir = parent
                    const url = cloneUrlField.text.trim()
                    let name = "repo"
                    const m = url.match(/([^/\\]+?)(?:\.git)?\/?$/)
                    if (m && m[1])
                        name = m[1]
                    const sep = parent.indexOf("\\") >= 0 ? "\\" : "/"
                    cloneDestField.text = parent.replace(/[\\/]+$/, "") + sep + name
                }
            }
        }

        onConfirmed: {
            GitDeskApp.cloneRepository(cloneUrlField.text.trim(), cloneDestField.text.trim())
            cloneUrlField.text = ""
            cloneDestField.text = ""
        }
    }

    function openCloneDialog() {
        cloneUrlField.text = ""
        cloneDestField.text = ""
        cloneDialog.open = true
    }

    Md3FullscreenDialog {
        id: settingsDialog
        open: window.settingsOpen
        title: qsTr("设置")
        confirmText: qsTr("完成")
        layoutMode: Md3ContainerBody.Fit
        onConfirmed: {
            if (settingsPage)
                settingsPage.save()
            window.settingsOpen = false
        }
        onDismissed: window.settingsOpen = false

        SettingsPage {
            id: settingsPage
            anchors.fill: parent
            onCloseRequested: window.settingsOpen = false
            onSaved: {
                window.showDiffLineNumbers = settingsPage.showDiffLineNumbers
            }
        }
    }
}
