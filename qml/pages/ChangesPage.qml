import QtQuick
import Md3
import GitDesk

Item {
    id: root
    anchors.fill: parent

    signal stashRequested()
    signal discardAllRequested()
    signal discardFileRequested(string path)

    Md3SplitView {
        anchors.fill: parent
        anchors.margins: 4
        splitRatio: 0.38
        minPane1: 240
        minPane2: 280

        Item {
            clip: true

            Item {
                id: changesHeader
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: Md3Theme.spacingSm
                height: headerCol.implicitHeight

                Column {
                    id: headerCol
                    width: parent.width
                    spacing: 4

                    Md3HStack {
                        width: parent.width
                        spacing: Md3Theme.spacingSm
                        Md3Text {
                            text: qsTr("变更")
                            role: Md3Text.TitleSmall
                        }
                        Md3Spacer { expand: true }
                        Md3IconButton {
                            icon: "inventory_2"
                            onClicked: root.stashRequested()
                        }
                        Md3IconButton {
                            icon: "add"
                            onClicked: GitDeskApp.stageAll()
                        }
                        Md3IconButton {
                            icon: "remove"
                            onClicked: GitDeskApp.unstageAll()
                        }
                        Md3IconButton {
                            icon: "delete"
                            onClicked: root.discardAllRequested()
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: conflictRow.implicitHeight + 12
                        visible: GitDeskApp.conflictCount > 0
                                 || GitDeskApp.mergeInProgress
                                 || GitDeskApp.rebaseInProgress
                        radius: Md3Theme.shape.small
                        color: Md3Theme.colorScheme.errorContainer

                        Md3HStack {
                            id: conflictRow
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 8
                            spacing: Md3Theme.spacingSm
                            Md3Text {
                                text: {
                                    if (GitDeskApp.rebaseInProgress)
                                        return qsTr("Rebase 进行中 · %1 个冲突")
                                            .arg(GitDeskApp.conflictCount)
                                    if (GitDeskApp.mergeInProgress)
                                        return qsTr("合并进行中 · %1 个冲突")
                                            .arg(GitDeskApp.conflictCount)
                                    return qsTr("%1 个冲突文件").arg(GitDeskApp.conflictCount)
                                }
                                role: Md3Text.LabelMedium
                                tone: Md3Text.Custom
                                customColor: Md3Theme.colorScheme.colorOnErrorContainer
                                width: Math.max(80, parent.width - 220)
                                wrapMode: Text.Wrap
                            }
                            Md3Spacer { expand: true }
                            Md3Button {
                                visible: GitDeskApp.rebaseInProgress
                                text: qsTr("继续")
                                variant: Md3Button.Text
                                enabled: !GitDeskApp.busy
                                onClicked: GitDeskApp.continueRebase()
                            }
                            Md3Button {
                                visible: GitDeskApp.rebaseInProgress
                                text: qsTr("中止")
                                variant: Md3Button.Text
                                enabled: !GitDeskApp.busy
                                onClicked: GitDeskApp.abortRebase()
                            }
                            Md3Button {
                                visible: GitDeskApp.mergeInProgress && !GitDeskApp.rebaseInProgress
                                text: qsTr("中止")
                                variant: Md3Button.Text
                                onClicked: GitDeskApp.abortMerge()
                            }
                        }
                    }

                    Md3HStack {
                        width: parent.width
                        spacing: Md3Theme.spacingSm
                        visible: GitDeskApp.stashes.length > 0
                        Md3Text {
                            text: qsTr("贮藏 · %1").arg(GitDeskApp.stashes.length)
                            role: Md3Text.LabelSmall
                            tone: Md3Text.OnSurfaceVariant
                        }
                        Md3Spacer { expand: true }
                        Md3Button {
                            text: qsTr("弹出")
                            variant: Md3Button.Text
                            icon: "unarchive"
                            onClicked: GitDeskApp.stashPop()
                        }
                    }
                }
            }

            CommitComposer {
                id: composer
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: Md3Theme.spacingSm
                onStashRequested: root.stashRequested()
            }

            Md3ScrollView {
                id: changeScroller
                anchors.top: changesHeader.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: composer.top
                anchors.topMargin: Md3Theme.spacingSm
                anchors.bottomMargin: Md3Theme.spacingSm
                anchors.leftMargin: Md3Theme.spacingSm
                anchors.rightMargin: Md3Theme.spacingXs
                clip: true
                fillContentWidth: true

                Column {
                    width: changeScroller.width > 0 ? changeScroller.width - 4 : 220
                    spacing: Md3Theme.spacingMd

                    Md3PageSection {
                        width: parent.width
                        title: qsTr("已暂存")

                        Column {
                            width: parent.width
                            spacing: 2
                            Repeater {
                                model: GitDeskApp.changes
                                delegate: Md3ListTile {
                                    required property string path
                                    required property string displayStatus
                                    required property bool staged
                                    width: parent.width
                                    visible: staged
                                    title: path
                                    subtitle: displayStatus
                                    leadingIcon: "edit"
                                    selected: GitDeskApp.selectedFilePath === path
                                              && GitDeskApp.selectedFileStaged
                                    onClicked: GitDeskApp.selectChange(path, true)
                                    trailing: Md3IconButton {
                                        icon: "remove"
                                        onClicked: GitDeskApp.unstageFile(path)
                                    }
                                }
                            }
                        }
                    }

                    Md3PageSection {
                        width: parent.width
                        title: qsTr("未暂存")

                        Column {
                            width: parent.width
                            spacing: 2
                            Repeater {
                                model: GitDeskApp.changes
                                delegate: Md3ListTile {
                                    required property string path
                                    required property string displayStatus
                                    required property bool staged
                                    required property string status
                                    width: parent.width
                                    visible: !staged
                                    title: path
                                    subtitle: displayStatus
                                    leadingIcon: status === "??" ? "fiber_new" : "edit"
                                    selected: GitDeskApp.selectedFilePath === path
                                              && !GitDeskApp.selectedFileStaged
                                    onClicked: GitDeskApp.selectChange(path, false)
                                    trailing: Md3HStack {
                                        spacing: 0
                                        Md3IconButton {
                                            visible: status === "U"
                                            icon: "call_merge"
                                            accessibleName: qsTr("采用我们的")
                                            onClicked: GitDeskApp.resolveConflict(path, "ours")
                                        }
                                        Md3IconButton {
                                            visible: status === "U"
                                            icon: "call_split"
                                            accessibleName: qsTr("采用他们的")
                                            onClicked: GitDeskApp.resolveConflict(path, "theirs")
                                        }
                                        Md3IconButton {
                                            icon: "add"
                                            onClicked: GitDeskApp.stageFile(path)
                                        }
                                        Md3IconButton {
                                            icon: "delete"
                                            onClicked: root.discardFileRequested(path)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Md3PageSection {
                        width: parent.width
                        title: qsTr("贮藏")
                        visible: GitDeskApp.stashes.length > 0

                        Column {
                            width: parent.width
                            spacing: 2
                            Repeater {
                                model: GitDeskApp.stashes
                                delegate: Md3ListTile {
                                    required property var modelData
                                    width: parent.width
                                    title: String(modelData.message || modelData.ref || "")
                                    subtitle: String(modelData.ref || "")
                                    leadingIcon: "inventory_2"
                                    trailing: Md3IconButton {
                                        icon: "delete"
                                        onClicked: GitDeskApp.stashDrop(Number(modelData.index))
                                    }
                                }
                            }
                        }
                    }

                    Md3EmptyState {
                        visible: GitDeskApp.changedFileCount === 0
                        width: parent.width
                        icon: "check_circle"
                        title: qsTr("工作区干净")
                        body: qsTr("没有未提交的变更")
                    }
                }
            }
        }

        Item {
            clip: true
            DiffViewer {
                anchors.fill: parent
                anchors.margins: 4
            }
        }
    }
}
