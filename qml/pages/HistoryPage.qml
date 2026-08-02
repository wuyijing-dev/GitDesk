import QtQuick
import Md3
import GitDesk

Item {
    id: root
    anchors.fill: parent

    Md3SplitView {
        anchors.fill: parent
        anchors.margins: 4
        splitRatio: 0.45
        minPane1: 260
        minPane2: 240

        Item {
            clip: true
            Md3VStack {
                anchors.fill: parent
                spacing: Md3Theme.spacingSm
                padding: Md3Theme.spacingSm
                stretchChildren: true

                Md3Text {
                    text: qsTr("History · %1").arg(GitDeskApp.commitCount)
                    role: Md3Text.TitleSmall
                }

                Md3ScrollView {
                    property bool expand: true
                    clip: true

                    Md3VStack {
                        width: parent.width
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
                                height: 48
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
