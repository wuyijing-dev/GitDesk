import QtQuick
import Md3

/// Compact stat tile — 不用 Md3Card body+anchors.fill（易测高为 0）
Rectangle {
    id: root
    property string label: ""
    property string value: "0"
    property string icon: "analytics"
    property color accent: Md3Theme.colorScheme.primary

    width: 148
    height: 88
    radius: Md3Theme.shape.medium
    color: Md3Theme.colorScheme.surfaceContainerHigh

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Row {
            spacing: 8
            Md3Icon {
                icon: root.icon
                size: 18
                iconColor: root.accent
            }
            Md3Text {
                anchors.verticalCenter: parent.verticalCenter
                text: root.label
                role: Md3Text.LabelSmall
                tone: Md3Text.OnSurfaceVariant
            }
        }
        Md3Text {
            text: root.value
            role: Md3Text.HeadlineSmall
        }
    }
}
