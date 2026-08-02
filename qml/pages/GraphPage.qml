import QtQuick
import Md3
import GitDesk

Item {
    id: root
    anchors.fill: parent

    Md3Text {
        id: title
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 12
        text: qsTr("图谱")
        role: Md3Text.TitleSmall
    }

    GitGraphCanvas {
        anchors.top: title.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 8
        anchors.margins: 8
    }
}
