import QtQuick
import Md3
import GitDesk

Item {
    id: root
    anchors.fill: parent

    property string searchText: ""

    function commitMatches(subject, author, shortId, commitId) {
        const q = root.searchText.trim().toLowerCase()
        if (!q.length)
            return true
        return String(subject).toLowerCase().indexOf(q) >= 0
                || String(author).toLowerCase().indexOf(q) >= 0
                || String(shortId).toLowerCase().indexOf(q) >= 0
                || String(commitId).toLowerCase().indexOf(q) >= 0
    }

    Md3SplitView {
        anchors.fill: parent
        anchors.margins: 4
        splitRatio: 0.45
        minPane1: 260
        minPane2: 240

        Item {
            clip: true

            Item {
                id: histHeader
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: Md3Theme.spacingSm
                height: histHeaderCol.implicitHeight

                Column {
                    id: histHeaderCol
                    width: parent.width
                    spacing: Md3Theme.spacingSm

                    Md3Text {
                        text: qsTr("历史 · %1").arg(GitDeskApp.commitCount)
                        role: Md3Text.TitleSmall
                    }
                    Md3TextField {
                        width: parent.width
                        label: qsTr("搜索提交…")
                        text: root.searchText
                        onTextChanged: root.searchText = text
                    }
                }
            }

            Md3ScrollView {
                id: histScroller
                anchors.top: histHeader.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: Md3Theme.spacingSm
                anchors.topMargin: Md3Theme.spacingSm
                clip: true
                fillContentWidth: true

                Column {
                    width: histScroller.width > 0 ? histScroller.width - 4 : 240
                    spacing: 2

                    Repeater {
                        model: GitDeskApp.commits
                        delegate: Md3ListTile {
                            required property string commitId
                            required property string shortId
                            required property string subject
                            required property string author
                            required property string date
                            width: parent.width
                            visible: root.commitMatches(subject, author, shortId, commitId)
                            title: subject
                            subtitle: author + " · " + date
                            leadingIcon: "commit"
                            selected: GitDeskApp.selectedCommitId === commitId
                            trailing: Md3Text {
                                text: shortId
                                role: Md3Text.LabelSmall
                                tone: Md3Text.OnSurfaceVariant
                            }
                            onClicked: {
                                GitDeskApp.selectedCommitId = commitId
                                GitDeskApp.detailOpen = true
                            }
                        }
                    }
                }
            }
        }

        Item {
            clip: true
            DiffViewer {
                anchors.fill: parent
                anchors.margins: 4
                compact: true
            }
        }
    }
}
