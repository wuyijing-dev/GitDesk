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
        title: qsTr("Commit")
        subtitle: GitDeskApp.selectedCommit.shortId || ""
        variant: Md3Card.Filled
        layoutMode: Md3ContainerBody.Fit

        Md3VStack {
            width: parent.width
            spacing: Md3Theme.spacingSm

            Md3Text {
                text: qsTr("Author")
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
                text: qsTr("Date")
                role: Md3Text.LabelSmall
                tone: Md3Text.OnSurfaceVariant
            }
            Md3Text {
                text: GitDeskApp.selectedCommit.date || ""
                role: Md3Text.BodyMedium
            }
            Md3Text {
                text: qsTr("Message")
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
                text: qsTr("Copy SHA")
                icon: "content_copy"
                variant: Md3Button.Outlined
                onClicked: Md3Notify.copy(GitDeskApp.selectedCommitId, { feedback: qsTr("已复制") })
            }
        }
    }

    Md3Card {
        width: parent.width
        visible: GitDeskApp.selectedFilePath.length > 0 && GitDeskApp.selectedCommitId.length === 0
        title: qsTr("File")
        subtitle: GitDeskApp.selectedFilePath
        variant: Md3Card.Filled

        Md3VStack {
            width: parent.width
            spacing: Md3Theme.spacingSm
            Md3Text {
                text: GitDeskApp.selectedFileStaged ? qsTr("Staged") : qsTr("Unstaged")
                role: Md3Text.LabelMedium
                tone: Md3Text.OnSurfaceVariant
            }
            Md3HStack {
                spacing: Md3Theme.spacingSm
                Md3Button {
                    visible: !GitDeskApp.selectedFileStaged
                    text: qsTr("Stage")
                    icon: "add"
                    onClicked: GitDeskApp.stageFile(GitDeskApp.selectedFilePath)
                }
                Md3Button {
                    visible: GitDeskApp.selectedFileStaged
                    text: qsTr("Unstage")
                    icon: "remove"
                    variant: Md3Button.Outlined
                    onClicked: GitDeskApp.unstageFile(GitDeskApp.selectedFilePath)
                }
            }
        }
    }

    Md3EmptyState {
        width: parent.width
        visible: GitDeskApp.selectedCommitId.length === 0 && GitDeskApp.selectedFilePath.length === 0
        icon: "info"
        title: qsTr("选择 Commit 或文件")
        body: qsTr("在 Graph / Changes / History 中点击条目")
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
