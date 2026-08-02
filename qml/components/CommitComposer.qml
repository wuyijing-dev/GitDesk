import QtQuick
import Md3
import GitDesk

/// Commit composer — Md3Form + multiline Md3TextField.
Md3Card {
    id: root
    title: qsTr("Commit")
    subtitle: qsTr("暂存文件后填写说明")
    variant: Md3Card.Elevated
    layoutMode: Md3ContainerBody.Fit

    Md3Form {
        id: form
        width: parent.width
        requiredFields: GitDeskApp.amendCommit ? [] : ["message"]
        liveGate: true

        Md3TextField {
            name: "message"
            label: qsTr("Commit Message")
            placeholderText: qsTr("简述本次变更…")
            multiline: true
            maximumLineCount: 6
            text: GitDeskApp.commitMessage
            onTextChanged: {
                if (GitDeskApp.commitMessage !== text)
                    GitDeskApp.commitMessage = text
            }
        }

        Md3HStack {
            spacing: Md3Theme.spacingMd
            Md3Checkbox {
                text: qsTr("Amend")
                checked: GitDeskApp.amendCommit
                onCheckedToggled: function (on) { GitDeskApp.amendCommit = on }
            }
            Md3Spacer { expand: true }
            Md3Button {
                text: qsTr("Stage All")
                variant: Md3Button.Text
                icon: "select_all"
                onClicked: GitDeskApp.stageAll()
            }
        }

        Md3Button {
            text: GitDeskApp.amendCommit ? qsTr("Amend Commit") : qsTr("Commit")
            icon: "commit"
            busy: GitDeskApp.busy
            enabled: form.canSubmit || GitDeskApp.amendCommit
            onClicked: GitDeskApp.commit()
        }
    }
}
