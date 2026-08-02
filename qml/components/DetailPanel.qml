import QtQuick
import Md3
import GitDesk

/// SideSheet body — 只放内容，标题/关闭由 Md3SideSheet 负责
Md3VStack {
    id: root
    spacing: Md3Theme.spacingMd
    stretchChildren: true
    width: parent ? parent.width : 320

    Md3Card {
        width: parent.width
        visible: GitDeskApp.selectedCommitId.length > 0
        title: qsTr("提交")
        subtitle: GitDeskApp.selectedCommit.shortId || ""
        variant: Md3Card.Filled
        layoutMode: Md3ContainerBody.Fit

        Md3VStack {
            width: parent.width
            spacing: Md3Theme.spacingSm

            Md3Text {
                text: qsTr("作者")
                role: Md3Text.LabelSmall
                tone: Md3Text.OnSurfaceVariant
            }
            Md3Text {
                text: (GitDeskApp.selectedCommit.author || "")
                      + " <" + (GitDeskApp.selectedCommit.email || "") + ">"
                role: Md3Text.BodyMedium
                wrapMode: Text.Wrap
                width: parent.width
            }
            Md3Text {
                text: qsTr("日期")
                role: Md3Text.LabelSmall
                tone: Md3Text.OnSurfaceVariant
            }
            Md3Text {
                text: GitDeskApp.selectedCommit.date || ""
                role: Md3Text.BodyMedium
            }
            Md3Text {
                text: qsTr("说明")
                role: Md3Text.LabelSmall
                tone: Md3Text.OnSurfaceVariant
            }
            Md3Text {
                text: GitDeskApp.selectedCommit.subject || ""
                role: Md3Text.BodyLarge
                wrapMode: Text.Wrap
                width: parent.width
            }
            Md3Button {
                text: qsTr("复制 SHA")
                icon: "content_copy"
                variant: Md3Button.Outlined
                onClicked: Md3Notify.copy(GitDeskApp.selectedCommitId, { feedback: qsTr("已复制") })
            }
            Md3Button {
                text: qsTr("从此创建分支…")
                icon: "call_split"
                variant: Md3Button.Outlined
                onClicked: {
                    const w = Window.window
                    if (w && w.openCreateBranchFromCommitDialog)
                        w.openCreateBranchFromCommitDialog(GitDeskApp.selectedCommitId)
                }
            }
            Md3Button {
                text: qsTr("Cherry-pick")
                icon: "content_paste"
                variant: Md3Button.Outlined
                enabled: !GitDeskApp.busy
                onClicked: {
                    const w = Window.window
                    if (w && w.confirmCherryPick)
                        w.confirmCherryPick(GitDeskApp.selectedCommitId)
                }
            }
            Md3Button {
                text: qsTr("Revert")
                icon: "undo"
                variant: Md3Button.Outlined
                enabled: !GitDeskApp.busy
                onClicked: {
                    const w = Window.window
                    if (w && w.confirmRevert)
                        w.confirmRevert(GitDeskApp.selectedCommitId)
                }
            }
            Md3Button {
                text: qsTr("Reset Soft")
                icon: "history"
                variant: Md3Button.Text
                enabled: !GitDeskApp.busy
                onClicked: {
                    const w = Window.window
                    if (w && w.confirmReset)
                        w.confirmReset(GitDeskApp.selectedCommitId, "soft")
                }
            }
            Md3Button {
                text: qsTr("Reset Mixed")
                icon: "history"
                variant: Md3Button.Text
                enabled: !GitDeskApp.busy
                onClicked: {
                    const w = Window.window
                    if (w && w.confirmReset)
                        w.confirmReset(GitDeskApp.selectedCommitId, "mixed")
                }
            }
            Md3Button {
                text: qsTr("Reset Hard")
                icon: "warning"
                variant: Md3Button.Text
                enabled: !GitDeskApp.busy
                onClicked: {
                    const w = Window.window
                    if (w && w.confirmReset)
                        w.confirmReset(GitDeskApp.selectedCommitId, "hard")
                }
            }
        }
    }

    Md3Card {
        width: parent.width
        visible: GitDeskApp.selectedFilePath.length > 0 && GitDeskApp.selectedCommitId.length === 0
        title: qsTr("文件")
        subtitle: GitDeskApp.selectedFilePath
        variant: Md3Card.Filled

        Md3VStack {
            width: parent.width
            spacing: Md3Theme.spacingSm
            Md3Text {
                text: GitDeskApp.selectedFileStaged ? qsTr("已暂存") : qsTr("未暂存")
                role: Md3Text.LabelMedium
                tone: Md3Text.OnSurfaceVariant
            }
            Md3HStack {
                spacing: Md3Theme.spacingSm
                Md3Button {
                    visible: !GitDeskApp.selectedFileStaged
                    text: qsTr("暂存")
                    icon: "add"
                    onClicked: GitDeskApp.stageFile(GitDeskApp.selectedFilePath)
                }
                Md3Button {
                    visible: GitDeskApp.selectedFileStaged
                    text: qsTr("取消暂存")
                    icon: "remove"
                    variant: Md3Button.Outlined
                    onClicked: GitDeskApp.unstageFile(GitDeskApp.selectedFilePath)
                }
            }

            Md3HStack {
                spacing: Md3Theme.spacingSm
                visible: GitDeskApp.conflictCount > 0
                         || GitDeskApp.mergeInProgress
                         || GitDeskApp.rebaseInProgress
                Md3Button {
                    text: qsTr("采用我们的")
                    icon: "call_merge"
                    variant: Md3Button.Outlined
                    enabled: !GitDeskApp.busy
                    onClicked: GitDeskApp.resolveConflict(GitDeskApp.selectedFilePath, "ours")
                }
                Md3Button {
                    text: qsTr("采用他们的")
                    icon: "call_split"
                    variant: Md3Button.Outlined
                    enabled: !GitDeskApp.busy
                    onClicked: GitDeskApp.resolveConflict(GitDeskApp.selectedFilePath, "theirs")
                }
            }

            Md3Switch {
                text: qsTr("显示 Blame")
                checked: GitDeskApp.showBlame
                onToggled: function (on) { GitDeskApp.showBlame = on }
            }

            Md3Text {
                visible: GitDeskApp.showBlame && GitDeskApp.fileBlame.length > 0
                text: qsTr("Blame（最多 2000 行）")
                role: Md3Text.LabelMedium
            }
            Repeater {
                model: GitDeskApp.showBlame ? GitDeskApp.fileBlame : []
                delegate: Md3ListTile {
                    required property var modelData
                    width: parent.width
                    title: String(modelData.text || "")
                    subtitle: String(modelData.shortId || "") + " · "
                              + String(modelData.author || "") + " · "
                              + String(modelData.date || "")
                    leadingIcon: "person"
                }
            }

            Md3Text {
                visible: GitDeskApp.fileHistory.length > 0
                text: qsTr("文件历史")
                role: Md3Text.LabelMedium
            }
            Repeater {
                model: GitDeskApp.fileHistory
                delegate: Md3ListTile {
                    required property var modelData
                    width: parent.width
                    title: String(modelData.subject || "")
                    subtitle: String(modelData.author || "") + " · " + String(modelData.date || "")
                    leadingIcon: "history"
                    trailing: Md3Text {
                        text: String(modelData.shortId || "")
                        role: Md3Text.LabelSmall
                        tone: Md3Text.OnSurfaceVariant
                    }
                    onClicked: {
                        if (modelData && modelData.id)
                            GitDeskApp.selectedCommitId = String(modelData.id)
                    }
                }
            }
        }
    }

    Md3Card {
        width: parent.width
        visible: {
            const c = GitDeskApp.branchCompare
            return c && (c.ahead > 0 || c.behind > 0 || (c.commits && c.commits.length > 0))
        }
        title: qsTr("分支对比")
        subtitle: {
            const c = GitDeskApp.branchCompare
            if (!c)
                return ""
            return qsTr("%1 → %2 · 超前 %3 · 落后 %4")
                .arg(c.base || "")
                .arg(c.head || "")
                .arg(c.ahead || 0)
                .arg(c.behind || 0)
        }
        variant: Md3Card.Filled
        layoutMode: Md3ContainerBody.Fit

        Md3VStack {
            width: parent.width
            spacing: Md3Theme.spacingXs
            Repeater {
                model: (GitDeskApp.branchCompare && GitDeskApp.branchCompare.commits)
                       ? GitDeskApp.branchCompare.commits : []
                delegate: Md3ListTile {
                    required property var modelData
                    width: parent.width
                    title: String(modelData.subject || "")
                    subtitle: String(modelData.author || "") + " · " + String(modelData.date || "")
                    leadingIcon: "commit"
                    trailing: Md3Text {
                        text: String(modelData.shortId || "")
                        role: Md3Text.LabelSmall
                        tone: Md3Text.OnSurfaceVariant
                    }
                    onClicked: {
                        if (modelData && modelData.id)
                            GitDeskApp.selectedCommitId = String(modelData.id)
                    }
                }
            }
        }
    }

    Md3EmptyState {
        width: parent.width
        visible: GitDeskApp.selectedCommitId.length === 0 && GitDeskApp.selectedFilePath.length === 0
                 && !(GitDeskApp.branchCompare && GitDeskApp.branchCompare.commits
                      && GitDeskApp.branchCompare.commits.length > 0)
        icon: "info"
        title: qsTr("选择提交或文件")
        body: qsTr("在图谱 / 变更 / 历史中点击条目")
    }

    DiffViewer {
        visible: GitDeskApp.selectedCommitId.length > 0 || GitDeskApp.selectedFilePath.length > 0
        width: parent.width
        compact: true
        // SideSheet scrolls; give diff a bounded height
        implicitHeight: 280
        height: 280
    }
}
