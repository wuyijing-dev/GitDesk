import QtQuick
import Md3
import GitDesk

/// Empty / welcome — Md3EmptyState + recent list (Md3VStack / Md3ListTile).
Item {
    id: root

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: Md3Theme.colorScheme.surface }
            GradientStop { position: 1.0; color: Md3Theme.colorScheme.surfaceContainerLow }
        }
    }

    Md3VStack {
        anchors.centerIn: parent
        width: Math.min(480, parent.width - 48)
        spacing: Md3Theme.spacingXl
        stretchChildren: true

        Md3VStack {
            spacing: Md3Theme.spacingSm
            Md3Icon {
                icon: "hub"
                size: 56
                iconColor: Md3Theme.colorScheme.primary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Md3Text {
                text: "GitDesk"
                role: Md3Text.DisplaySmall
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Md3Text {
                text: qsTr("把 Git 变成可操作的软件工程地图")
                role: Md3Text.BodyLarge
                tone: Md3Text.OnSurfaceVariant
                wrapMode: Text.Wrap
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Md3HStack {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Md3Theme.spacingMd
            Md3Button {
                text: qsTr("打开仓库")
                icon: "folder_open"
                onClicked: GitDeskApp.pickRepository()
            }
            Md3Button {
                text: qsTr("克隆仓库")
                icon: "cloud_download"
                variant: Md3Button.Outlined
                onClicked: {
                    const w = Window.window
                    if (w && w.openCloneDialog)
                        w.openCloneDialog()
                }
            }
            Md3Button {
                text: qsTr("初始化仓库")
                icon: "create_new_folder"
                variant: Md3Button.Outlined
                onClicked: GitDeskApp.pickAndInitRepository()
            }
            Md3Button {
                text: qsTr("设置")
                icon: "settings"
                variant: Md3Button.Outlined
                onClicked: {
                    const w = Window.window
                    if (w)
                        w.settingsOpen = true
                }
            }
        }

        Md3PageSection {
            width: parent.width
            visible: GitDeskApp.recentRepos.length > 0
            title: qsTr("最近打开")

            Md3VStack {
                width: parent.width
                spacing: 2
                Repeater {
                    model: GitDeskApp.recentRepos
                    delegate: Md3ListTile {
                        required property var modelData
                        width: parent.width
                        title: {
                            const p = String(modelData)
                            const i = Math.max(p.lastIndexOf("/"), p.lastIndexOf("\\"))
                            return i >= 0 ? p.substring(i + 1) : p
                        }
                        subtitle: String(modelData)
                        leadingIcon: "folder"
                        onClicked: GitDeskApp.openRepository(String(modelData))
                    }
                }
            }
        }
    }
}
