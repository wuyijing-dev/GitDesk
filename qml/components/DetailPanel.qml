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
        }
    }

    Md3EmptyState {
        width: parent.width
        visible: GitDeskApp.selectedCommitId.length === 0 && GitDeskApp.selectedFilePath.length === 0
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
