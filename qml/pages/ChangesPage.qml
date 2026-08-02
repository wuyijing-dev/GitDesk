import QtQuick
import Md3
import GitDesk

Item {
    id: root
    // Parent is pageHost — fill it. Do NOT put anchors.fill on Md3SplitView children.
    anchors.fill: parent

    Md3SplitView {
        anchors.fill: parent
        anchors.margins: 4
        splitRatio: 0.38
        minPane1: 220
        minPane2: 280

        // Pane 1 — sized by SplitView (no anchors.fill)
        Item {
            clip: true

            Md3VStack {
                anchors.fill: parent
                spacing: Md3Theme.spacingSm
                padding: Md3Theme.spacingSm
                stretchChildren: true

                Md3HStack {
                    width: parent.width
                    spacing: Md3Theme.spacingSm
                    Md3Text {
                        text: qsTr("Changes")
                        role: Md3Text.TitleSmall
                    }
                    Md3Spacer { expand: true }
                    Md3IconButton {
                        icon: "add"
                        onClicked: GitDeskApp.stageAll()
                    }
                    Md3IconButton {
                        icon: "remove"
                        onClicked: GitDeskApp.unstageAll()
                    }
                }

                Md3ScrollView {
                    property bool expand: true
                    clip: true

                    Md3VStack {
                        width: parent.width
                        spacing: Md3Theme.spacingMd

                        Md3PageSection {
                            width: parent.width
                            title: qsTr("Staged")
                            Md3VStack {
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
                                        height: 40
                                        visible: staged
                                        title: path
                                        subtitle: displayStatus
                                        leadingIcon: "edit"
                                        selected: GitDeskApp.selectedFilePath === path && GitDeskApp.selectedFileStaged
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
                            title: qsTr("Unstaged")
                            Md3VStack {
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
                                        height: 40
                                        visible: !staged
                                        title: path
                                        subtitle: displayStatus
                                        leadingIcon: status === "??" ? "fiber_new" : "edit"
                                        selected: GitDeskApp.selectedFilePath === path && !GitDeskApp.selectedFileStaged
                                        onClicked: GitDeskApp.selectChange(path, false)
                                        trailing: Md3IconButton {
                                            icon: "add"
                                            onClicked: GitDeskApp.stageFile(path)
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

                CommitComposer {
                    width: parent.width
                }
            }
        }

        // Pane 2 — Diff (no anchors.fill on this Item)
        Item {
            clip: true
            DiffViewer {
                anchors.fill: parent
                anchors.margins: 4
            }
        }
    }
}
