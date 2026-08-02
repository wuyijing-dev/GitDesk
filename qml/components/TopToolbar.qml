import QtQuick
import Md3
import GitDesk

/// Compact tool strip — Md3AppToolBar + AppBarButtons（避免一排大 Button 挤爆）
Md3AppToolBar {
    id: root
    barHeight: 48
    contentSpacing: Md3Theme.spacingXs
    horizontalPadding: Md3Theme.spacingMd

    signal createBranchRequested()
    signal settingsRequested()

    Md3HStack {
        spacing: Md3Theme.spacingSm
        fillHeight: true

        Md3Icon {
            icon: "hub"
            size: 22
            iconColor: Md3Theme.colorScheme.primary
        }
        Md3Text {
            text: GitDeskApp.hasRepo ? GitDeskApp.repoName : "GitDesk"
            role: Md3Text.TitleSmall
        }
    }

    Md3DropDownButton {
        visible: GitDeskApp.hasRepo
        text: GitDeskApp.currentBranch.length ? GitDeskApp.currentBranch : qsTr("分支")
        icon: "call_split"
        menuModel: {
            const items = []
            const names = GitDeskApp.localBranchNames
            for (let i = 0; i < names.length; ++i)
                items.push({
                    text: names[i],
                    icon: names[i] === GitDeskApp.currentBranch ? "check" : "commit"
                })
            items.push({ divider: true })
            items.push({ text: qsTr("新建分支…"), icon: "add" })
            return items
        }
        onMenuItemClicked: function (index) {
            const names = GitDeskApp.localBranchNames
            if (index >= 0 && index < names.length)
                GitDeskApp.checkoutBranch(names[index])
            else
                root.createBranchRequested()
        }
    }

    Md3Text {
        visible: GitDeskApp.hasRepo && GitDeskApp.hasUpstream
        text: qsTr("↑%1 ↓%2").arg(GitDeskApp.ahead).arg(GitDeskApp.behind)
        role: Md3Text.LabelSmall
        tone: (GitDeskApp.ahead > 0 || GitDeskApp.behind > 0)
              ? Md3Text.Primary
              : Md3Text.OnSurfaceVariant
    }

    Md3Spacer { expand: true }

    Md3AppBarButton {
        visible: GitDeskApp.hasRepo
        icon: "cloud_download"
        label: qsTr("获取")
        enabled: !GitDeskApp.busy
        onClicked: GitDeskApp.fetch()
    }
    Md3DropDownButton {
        visible: GitDeskApp.hasRepo
        text: qsTr("拉取")
        icon: "download"
        enabled: !GitDeskApp.busy
        menuModel: {
            const _ = GitDeskApp.locale.revision
            return [
                { text: qsTr("拉取 (ff-only)"), icon: "download" },
                { text: qsTr("拉取 --rebase"), icon: "sync_alt" }
            ]
        }
        onMenuItemClicked: function (index) {
            if (index === 1)
                GitDeskApp.pullRebase()
            else
                GitDeskApp.pull()
        }
    }
    Md3AppBarButton {
        visible: GitDeskApp.hasRepo && GitDeskApp.rebaseInProgress
        icon: "play_arrow"
        label: qsTr("继续 Rebase")
        enabled: !GitDeskApp.busy
        onClicked: GitDeskApp.continueRebase()
    }
    Md3AppBarButton {
        visible: GitDeskApp.hasRepo
        icon: "upload"
        label: qsTr("推送")
        enabled: !GitDeskApp.busy
        onClicked: GitDeskApp.push()
    }
    Md3Button {
        visible: GitDeskApp.hasRepo
        text: qsTr("提交")
        icon: "commit"
        variant: Md3Button.Filled
        enabled: !GitDeskApp.busy
        onClicked: {
            GitDeskApp.workspaceTab = 2
            GitDeskApp.detailOpen = false
        }
    }

    Md3AppBarButton {
        visible: GitDeskApp.hasRepo && !GitDeskApp.hasUpstream
        icon: "upload"
        label: qsTr("推送上游")
        enabled: !GitDeskApp.busy
        onClicked: GitDeskApp.pushSetUpstream()
    }

    Md3AppBarButton {
        visible: GitDeskApp.hasRepo
        icon: "folder"
        label: qsTr("文件夹")
        onClicked: GitDeskApp.openRepoFolder()
    }
    Md3AppBarButton {
        visible: GitDeskApp.hasRepo
        icon: "compare_arrows"
        label: qsTr("对比")
        enabled: !GitDeskApp.busy
        onClicked: {
            const w = Window.window
            if (w && w.openCompareBranchesDialog)
                w.openCompareBranchesDialog()
        }
    }
    Md3AppBarButton {
        icon: "folder_open"
        label: qsTr("打开")
        onClicked: GitDeskApp.pickRepository()
    }
    Md3AppBarButton {
        visible: GitDeskApp.hasRepo
        icon: "refresh"
        label: qsTr("刷新")
        enabled: !GitDeskApp.busy
        onClicked: GitDeskApp.refresh()
    }
    Md3ToggleIconButton {
        visible: GitDeskApp.hasRepo
        icon: "info"
        checkable: true
        checked: GitDeskApp.detailOpen
        onToggled: function (on) { GitDeskApp.detailOpen = on }
    }
    Md3AppBarButton {
        icon: "settings"
        label: qsTr("设置")
        onClicked: root.settingsRequested()
    }
}
