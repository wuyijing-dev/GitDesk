import QtQuick
import Md3
import GitDesk

/// Git graph — custom node layer (专用可视化手写；缩放控件用 Md3).
Item {
    id: root

    property real nodeRadius: 7
    property real rowPitch: 44
    property real colPitch: 28
    property real leftPad: 28
    property real topPad: 24
    property real scaleFactor: 1.0

    readonly property int rows: GitDeskApp.graph ? GitDeskApp.graph.rowCountValue : 0
    readonly property int cols: GitDeskApp.graph ? GitDeskApp.graph.maxColumn + 1 : 1

    clip: true

    Rectangle {
        anchors.fill: parent
        radius: Md3Theme.shape.medium
        color: Md3Theme.colorScheme.surfaceContainerLowest
        border.width: 1
        border.color: Md3Theme.colorScheme.outlineVariant
    }

    Flickable {
        id: flick
        anchors.fill: parent
        anchors.margins: 4
        contentWidth: Math.max(width, leftPad + cols * colPitch * scaleFactor + 420)
        contentHeight: Math.max(height, topPad + rows * rowPitch * scaleFactor + 80)
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Item {
            id: graphLayer
            width: flick.contentWidth
            height: flick.contentHeight

            Canvas {
                id: edgeCanvas
                anchors.fill: parent
                antialiasing: true
                renderTarget: Canvas.FramebufferObject

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    ctx.clearRect(0, 0, width, height)
                    const count = nodeRepeater.count
                    const s = root.scaleFactor
                    const rp = root.rowPitch * s
                    const cp = root.colPitch * s
                    const map = ({})
                    for (let i = 0; i < count; ++i) {
                        const item = nodeRepeater.itemAt(i)
                        if (!item)
                            continue
                        map[item.commitId] = {
                            x: item.nodeX,
                            y: item.nodeY,
                            color: item.nodeColor
                        }
                    }
                    for (let i = 0; i < count; ++i) {
                        const item = nodeRepeater.itemAt(i)
                        if (!item)
                            continue
                        const parents = item.parents || []
                        const pcols = item.parentColumns || []
                        for (let p = 0; p < parents.length; ++p) {
                            const pid = parents[p]
                            const target = map[pid]
                            let tx
                            let ty
                            let col = item.nodeColor
                            if (target) {
                                tx = target.x
                                ty = target.y
                                col = target.color
                            } else {
                                const pc = pcols[p] !== undefined ? pcols[p] : item.column
                                tx = root.leftPad + pc * cp
                                ty = item.nodeY + rp
                            }
                            ctx.strokeStyle = String(col)
                            ctx.globalAlpha = 0.85
                            ctx.lineWidth = 2
                            ctx.beginPath()
                            ctx.moveTo(item.nodeX, item.nodeY)
                            const midY = (item.nodeY + ty) / 2
                            ctx.bezierCurveTo(item.nodeX, midY, tx, midY, tx, ty)
                            ctx.stroke()
                        }
                    }
                    ctx.globalAlpha = 1
                }
            }

            Repeater {
                id: nodeRepeater
                model: GitDeskApp.graph
                onCountChanged: Qt.callLater(function () { edgeCanvas.requestPaint() })

                delegate: Item {
                    id: node
                    required property string commitId
                    required property string shortId
                    required property string subject
                    required property string author
                    required property string date
                    required property var refs
                    required property int column
                    required property int row
                    required property color nodeColor
                    required property var parents
                    required property var parentColumns

                    readonly property real nodeX: root.leftPad + column * root.colPitch * root.scaleFactor
                    readonly property real nodeY: root.topPad + row * root.rowPitch * root.scaleFactor

                    width: 1
                    height: 1
                    x: nodeX
                    y: nodeY

                    Rectangle {
                        id: dot
                        width: root.nodeRadius * 2 * root.scaleFactor
                        height: width
                        radius: width / 2
                        anchors.centerIn: parent
                        color: nodeColor
                        border.width: GitDeskApp.selectedCommitId === commitId ? 3 : 0
                        border.color: Md3Theme.colorScheme.primary

                        scale: 0.2
                        Component.onCompleted: scale = 1
                        Behavior on scale {
                            NumberAnimation {
                                duration: Md3Motion.medium2
                                easing.type: Easing.OutBack
                            }
                        }
                    }

                    Md3HStack {
                        anchors.left: dot.right
                        anchors.leftMargin: 12
                        anchors.verticalCenter: dot.verticalCenter
                        spacing: Md3Theme.spacingSm

                        Md3Text {
                            text: shortId
                            role: Md3Text.LabelSmall
                            tone: Md3Text.OnSurfaceVariant
                        }
                        Md3Text {
                            text: subject
                            role: Md3Text.BodyMedium
                            elide: Text.ElideRight
                            width: Math.min(360, Math.max(120, flick.width - nodeX - 80))
                        }
                        Repeater {
                            model: refs
                            delegate: Md3AssistChip {
                                required property var modelData
                                text: String(modelData)
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: dot
                        anchors.margins: -8
                        cursorShape: Qt.PointingHandCursor
                        onClicked: GitDeskApp.selectedCommitId = commitId
                    }

                    Component.onCompleted: Qt.callLater(function () { edgeCanvas.requestPaint() })
                    onNodeXChanged: edgeCanvas.requestPaint()
                    onNodeYChanged: edgeCanvas.requestPaint()
                }
            }
        }
    }

    Md3HStack {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Md3Theme.spacingMd
        spacing: Md3Theme.spacingXs

        Md3IconButton {
            icon: "remove"
            onClicked: {
                root.scaleFactor = Math.max(0.6, root.scaleFactor - 0.1)
                edgeCanvas.requestPaint()
            }
        }
        Md3IconButton {
            icon: "add"
            onClicked: {
                root.scaleFactor = Math.min(1.8, root.scaleFactor + 0.1)
                edgeCanvas.requestPaint()
            }
        }
        Md3IconButton {
            icon: "fit_screen"
            onClicked: {
                root.scaleFactor = 1
                flick.contentX = 0
                flick.contentY = 0
                edgeCanvas.requestPaint()
            }
        }
    }

    Connections {
        target: GitDeskApp.graph
        function onGraphLayoutChanged() {
            edgeCanvas.requestPaint()
        }
    }
}
