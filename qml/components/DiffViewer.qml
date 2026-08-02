import QtQuick
import Md3
import GitDesk

/// Diff viewer — Md3CodeBlock chrome; line coloring via formatted text when possible.
Item {
    id: root
    property bool compact: false
    property bool splitView: false

    Md3VStack {
        anchors.fill: parent
        spacing: Md3Theme.spacingSm
        stretchChildren: true

        Md3HStack {
            width: parent.width
            spacing: Md3Theme.spacingSm
            visible: !root.compact

            Md3Text {
                text: qsTr("Diff")
                role: Md3Text.TitleSmall
            }
            Md3Spacer { expand: true }
            Md3ButtonGroup {
                layout: Md3ButtonGroup.Connected
                currentIndex: root.splitView ? 1 : 0
                model: [
                    { text: qsTr("Inline"), icon: "view_agenda" },
                    { text: qsTr("Split"), icon: "vertical_split" }
                ]
                onClicked: function (index) { root.splitView = index === 1 }
            }
        }

        Md3EmptyState {
            visible: GitDeskApp.currentDiff.length === 0 && !GitDeskApp.diffLoading
            width: parent.width
            property bool expand: true
            icon: "difference"
            title: qsTr("无 Diff")
            body: qsTr("选择变更文件或 Commit")
        }

        Md3Text {
            visible: GitDeskApp.diffLoading
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("加载 Diff…")
            role: Md3Text.BodyMedium
            tone: Md3Text.OnSurfaceVariant
        }

        Md3CodeBlock {
            visible: GitDeskApp.currentDiff.length > 0
            property bool expand: true
            width: parent.width
            code: GitDeskApp.currentDiff
            language: GitDeskApp.selectedFilePath.length
                      ? GitDeskApp.languageForPath(GitDeskApp.selectedFilePath)
                      : "plain"
showLineNumbers: {
                const w = Window.window
                if (w && w.showDiffLineNumbers !== undefined)
                    return w.showDiffLineNumbers
                return true
            }
            wrap: false
            fontSize: 12
            maxHeight: Math.max(160, root.height - (root.compact ? 16 : 56))
            scrollable: true
            showCopyButton: !root.compact
            onCopied: function (text) {
                Md3Notify.copy(text, { feedback: qsTr("Diff 已复制") })
            }
        }
    }
}
