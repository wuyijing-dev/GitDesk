import QtQuick
import Md3
import GitDesk

/// Left explorer — header + scroll fill + sticky footer（铺满侧栏）
Item {
    id: root
    clip: true

    signal createBranchRequested()
    signal createTagRequested()
    signal deleteTagRequested(string name)

    Rectangle {
        anchors.fill: parent
        color: Md3Theme.colorScheme.surfaceContainerLow
    }

    Md3VStack {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: Md3Theme.spacingMd
        spacing: 4
        stretchChildren: true

        Md3Text {
            text: qsTr("仓库")
            role: Md3Text.TitleSmall
        }
        Md3Text {
            text: GitDeskApp.repoPath
            role: Md3Text.LabelSmall
            tone: Md3Text.OnSurfaceVariant
            elide: Text.ElideMiddle
            width: parent.width
        }
    }

    Md3Divider {
        id: headerLine
        anchors.top: header.bottom
        anchors.topMargin: Md3Theme.spacingSm
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Md3Theme.spacingMd
        anchors.rightMargin: Md3Theme.spacingMd
    }

    // Sticky footer — 贴底铺满
    Item {
        id: footer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: createBtn.implicitHeight + Md3Theme.spacingMd * 2

        Md3Divider {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Md3Theme.spacingMd
            anchors.rightMargin: Md3Theme.spacingMd
        }

        Md3Button {
            id: createBtn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: Md3Theme.spacingMd
            text: qsTr("新建分支")
            icon: "add"
            variant: Md3Button.Outlined
            onClicked: root.createBranchRequested()
        }
    }

    Md3ScrollView {
        id: scroller
        anchors.top: headerLine.bottom
        anchors.topMargin: Md3Theme.spacingSm
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: footer.top
        anchors.leftMargin: Md3Theme.spacingMd
        anchors.rightMargin: Md3Theme.spacingSm
        anchors.bottomMargin: Md3Theme.spacingSm
        clip: true
        fillContentWidth: true

        Column {
            width: scroller.width > 0 ? scroller.width - 4 : 240
            spacing: Md3Theme.spacingMd

            Md3Text {
                text: qsTr("项目")
                role: Md3Text.LabelMedium
                tone: Md3Text.OnSurfaceVariant
            }
            Md3TreeView {
                width: parent.width
                // 随侧栏高度伸展，避免大片留白
                preferredMaxHeight: Math.max(180, Math.floor(scroller.height * 0.42))
                showConnectors: true
                unloadWhenPageInactive: false
                model: GitDeskApp.projectTree
            }

            Md3Text {
                text: qsTr("分支")
                role: Md3Text.LabelMedium
                tone: Md3Text.OnSurfaceVariant
            }
            Repeater {
                model: GitDeskApp.localBranchNames
                delegate: Md3ListTile {
                    required property var modelData
                    required property int index
                    width: parent.width
                    title: String(modelData)
                    leadingIcon: String(modelData) === GitDeskApp.currentBranch
                                 ? "check_circle" : "call_split"
                    selected: String(modelData) === GitDeskApp.currentBranch
                    onClicked: GitDeskApp.checkoutBranch(String(modelData))
                }
            }
            Md3Text {
                visible: GitDeskApp.localBranchNames.length === 0
                text: qsTr("无本地分支")
                role: Md3Text.BodySmall
                tone: Md3Text.OnSurfaceVariant
            }

            Md3Text {
                text: qsTr("远程")
                role: Md3Text.LabelMedium
                tone: Md3Text.OnSurfaceVariant
            }
            Repeater {
                model: GitDeskApp.remotes
                delegate: Md3ListTile {
                    required property var modelData
                    width: parent.width
                    title: String(modelData)
                    leadingIcon: "cloud"
                }
            }
            Md3Text {
                visible: GitDeskApp.remotes.length === 0
                text: qsTr("无远程")
                role: Md3Text.BodySmall
                tone: Md3Text.OnSurfaceVariant
            }

            Md3HStack {
                width: parent.width
                spacing: Md3Theme.spacingSm
                Md3Text {
                    text: qsTr("标签")
                    role: Md3Text.LabelMedium
                    tone: Md3Text.OnSurfaceVariant
                }
                Md3Spacer { expand: true }
                Md3IconButton {
                    icon: "add"
                    onClicked: root.createTagRequested()
                }
            }
            Flow {
                width: parent.width
                spacing: Md3Theme.spacingXs
                Repeater {
                    model: GitDeskApp.tags
                    delegate: Md3InputChip {
                        required property var modelData
                        text: String(modelData)
                        onRemoved: root.deleteTagRequested(String(modelData))
                    }
                }
            }
            Md3Text {
                visible: GitDeskApp.tags.length === 0
                text: qsTr("无标签")
                role: Md3Text.BodySmall
                tone: Md3Text.OnSurfaceVariant
            }
        }
    }
}
