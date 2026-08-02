import QtQuick
import Md3
import GitDesk

/// Window About body — driven by resources/version.json via VersionInfo
Flickable {
    id: root
    width: parent ? parent.width : 360
    height: parent ? Math.min(parent.height, 420) : 280
    contentWidth: width
    contentHeight: col.implicitHeight
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    flickableDirection: Flickable.VerticalFlick
    implicitHeight: Math.min(420, col.implicitHeight)

    readonly property var info: GitDeskApp.versionInfo

    Column {
        id: col
        width: root.width
        spacing: Md3Theme.spacingSm

        Md3Text {
            width: parent.width
            wrapMode: Text.Wrap
            role: Md3Text.BodyMedium
            text: root.info.tagline
            visible: root.info.tagline.length > 0
        }
        Md3Text {
            width: parent.width
            wrapMode: Text.Wrap
            role: Md3Text.BodySmall
            tone: Md3Text.OnSurfaceVariant
            text: root.info.description
            visible: root.info.description.length > 0
        }
        Md3Text {
            visible: root.info.channel.length > 0 || root.info.buildDate.length > 0
            role: Md3Text.LabelSmall
            tone: Md3Text.OnSurfaceVariant
            text: qsTr("渠道 %1 · %2").arg(root.info.channel).arg(root.info.buildDate)
        }

        Repeater {
            model: root.info.highlights
            delegate: Md3Text {
                required property var modelData
                width: col.width
                wrapMode: Text.Wrap
                role: Md3Text.BodySmall
                tone: Md3Text.OnSurfaceVariant
                text: "· " + String(modelData)
            }
        }

        Md3Divider {
            width: parent.width
            visible: root.info.changelog.length > 0
        }
        Md3Text {
            visible: root.info.changelog.length > 0
            text: qsTr("更新日志")
            role: Md3Text.TitleSmall
        }

        Repeater {
            model: root.info.changelog
            delegate: Column {
                required property var modelData
                width: col.width
                spacing: 2

                Md3Text {
                    width: parent.width
                    wrapMode: Text.Wrap
                    role: Md3Text.LabelMedium
                    text: String(modelData.version || "")
                          + " (" + String(modelData.date || "") + ") — "
                          + String(modelData.title || "")
                }
                Repeater {
                    model: modelData.changes || []
                    delegate: Md3Text {
                        required property var modelData
                        width: col.width
                        wrapMode: Text.Wrap
                        role: Md3Text.BodySmall
                        tone: Md3Text.OnSurfaceVariant
                        text: "  [" + String(modelData.type || "") + "] "
                              + String(modelData.text || "")
                    }
                }
                Item { width: 1; height: 6 }
            }
        }

        Md3Text {
            visible: root.info.author.length > 0
            role: Md3Text.LabelSmall
            tone: Md3Text.OnSurfaceVariant
            text: qsTr("作者：%1").arg(root.info.author)
        }
    }
}
