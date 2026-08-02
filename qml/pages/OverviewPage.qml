import QtQuick
import Md3
import GitDesk

Item {
    id: root
    anchors.fill: parent

    // 保证即使滚动测量失败也能看到底色与标题
    Rectangle {
        anchors.fill: parent
        color: Md3Theme.colorScheme.surface
    }

    Flickable {
        id: flick
        anchors.fill: parent
        anchors.margins: 12
        clip: true
        contentWidth: width
        contentHeight: Math.max(height, col.implicitHeight + 24)
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: col
            width: flick.width
            spacing: 16

            Md3Text {
                text: qsTr("Overview")
                role: Md3Text.TitleLarge
            }
            Md3Text {
                width: parent.width
                wrapMode: Text.Wrap
                role: Md3Text.BodyMedium
                tone: Md3Text.OnSurfaceVariant
                text: qsTr("%1 · %2 · %3 commits · %4 changes")
                      .arg(GitDeskApp.repoName)
                      .arg(GitDeskApp.currentBranch)
                      .arg(GitDeskApp.commitCount)
                      .arg(GitDeskApp.changedFileCount)
            }

            Flow {
                width: parent.width
                spacing: 12

                StatCard {
                    label: qsTr("Commits")
                    value: String(GitDeskApp.commitCount)
                    icon: "commit"
                }
                StatCard {
                    label: qsTr("Branches")
                    value: String(GitDeskApp.branchCount)
                    icon: "call_split"
                }
                StatCard {
                    label: qsTr("Tags")
                    value: String(GitDeskApp.tagCount)
                    icon: "sell"
                }
                StatCard {
                    label: qsTr("Contributors")
                    value: String(GitDeskApp.contributorCount)
                    icon: "groups"
                }
                StatCard {
                    label: qsTr("Changes")
                    value: String(GitDeskApp.changedFileCount)
                    icon: "difference"
                    accent: Md3Theme.colorScheme.error
                }
            }

            Md3Text {
                text: qsTr("最近活动")
                role: Md3Text.TitleSmall
            }

            Repeater {
                model: GitDeskApp.recentActivity
                delegate: Md3ListTile {
                    required property var modelData
                    width: col.width
                    title: modelData.title || ""
                    subtitle: (modelData.author || "") + " · " + (modelData.date || "")
                    leadingIcon: "history"
                    onClicked: {
                        GitDeskApp.selectedCommitId = modelData.id || ""
                        GitDeskApp.workspaceTab = 1
                        GitDeskApp.detailOpen = true
                    }
                }
            }

            Md3Text {
                visible: !GitDeskApp.recentActivity || GitDeskApp.recentActivity.length === 0
                text: qsTr("暂无提交记录")
                role: Md3Text.BodyMedium
                tone: Md3Text.OnSurfaceVariant
            }
        }
    }
}
