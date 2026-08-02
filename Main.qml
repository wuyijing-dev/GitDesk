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
    aboutText: GitDeskApp.versionInfo.summary
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
        model: [
            { title: qsTr("打开仓库"), icon: "folder_open",
              action: () => GitDeskApp.pickRepository() },
            { title: qsTr("设置"), icon: "settings",
              action: () => { window.settingsOpen = true } },
            { title: qsTr("刷新"), icon: "refresh",
              action: () => GitDeskApp.refresh() },
            { title: qsTr("Fetch"), icon: "cloud_download",
              action: () => GitDeskApp.fetch() },
            { title: qsTr("Pull"), icon: "download",
              action: () => GitDeskApp.pull() },
            { title: qsTr("Push"), icon: "upload",
              action: () => GitDeskApp.push() },
            { title: qsTr("Overview"), icon: "dashboard",
              action: () => GitDeskApp.workspaceTab = 0 },
            { title: qsTr("Graph"), icon: "account_tree",
              action: () => GitDeskApp.workspaceTab = 1 },
            { title: qsTr("Changes"), icon: "difference",
              action: () => GitDeskApp.workspaceTab = 2 },
            { title: qsTr("History"), icon: "history",
              action: () => GitDeskApp.workspaceTab = 3 },
            { title: qsTr("新建分支"), icon: "add",
              action: () => window.openCreateBranchDialog() }
        ]
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
                       ? qsTr("%1 · %2 changes")
                         .arg(GitDeskApp.currentBranch)
                         .arg(GitDeskApp.changedFileCount)
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
                    model: [
                        { text: qsTr("Overview") },
                        { text: qsTr("Graph") },
                        { text: qsTr("Changes") },
                        { text: qsTr("History") }
                    ]
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
