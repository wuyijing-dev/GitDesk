import QtQuick
import Md3
import GitDesk

Item {
    id: root
    anchors.fill: parent

    property bool darkMode: true
    property int density: 1
    property string seedColor: "#1B6B4A"
    property bool showFocusRings: false
    property bool detailOpenByDefault: true
    property bool showDiffLineNumbers: true
    property string commitTemplate: ""
    property string gitExecutableHint: ""
    property string languageCode: "zh-CN"

    signal saved()
    signal closeRequested()

    function asBool(v, fallback) {
        if (v === undefined || v === null)
            return fallback
        if (v === true || v === 1 || v === "1" || v === "true")
            return true
        if (v === false || v === 0 || v === "0" || v === "false")
            return false
        return fallback
    }

    function load() {
        darkMode = asBool(Md3AppSettings.value("settings/darkMode", true), true)
        const d = Number(Md3AppSettings.value("settings/density", 1))
        density = (!isNaN(d) && d >= 1) ? 1 : 0
        seedColor = String(Md3AppSettings.value("settings/seedColor", "#1B6B4A") || "#1B6B4A")
        showFocusRings = asBool(Md3AppSettings.value("settings/showFocusRings", false), false)
        detailOpenByDefault = asBool(Md3AppSettings.value("settings/detailOpenByDefault", true), true)
        showDiffLineNumbers = asBool(Md3AppSettings.value("settings/showDiffLineNumbers", true), true)
        commitTemplate = String(Md3AppSettings.value("settings/commitTemplate", "") || "")
        gitExecutableHint = String(Md3AppSettings.value("settings/gitExecutable", "") || "")
        if (!gitExecutableHint.length)
            gitExecutableHint = GitDeskApp.gitExecutable()
        languageCode = GitDeskApp.locale.normalize(
                    String(Md3AppSettings.value("settings/language", "zh-CN") || "zh-CN"))
        applyTheme()
    }

    function applyTheme() {
        Md3Theme.dark = darkMode
        Md3Theme.density = density
        Md3Theme.applySeed(seedColor)
        if (typeof Md3Accessibility !== "undefined")
            Md3Accessibility.showFocusRings = showFocusRings
    }

    function save() {
        Md3AppSettings.setValue("settings/darkMode", darkMode)
        Md3AppSettings.setValue("settings/density", density)
        Md3AppSettings.setValue("settings/seedColor", seedColor)
        Md3AppSettings.setValue("settings/showFocusRings", showFocusRings)
        Md3AppSettings.setValue("settings/detailOpenByDefault", detailOpenByDefault)
        Md3AppSettings.setValue("settings/showDiffLineNumbers", showDiffLineNumbers)
        Md3AppSettings.setValue("settings/commitTemplate", commitTemplate)
        Md3AppSettings.setValue("settings/gitExecutable", gitExecutableHint)
        Md3AppSettings.setValue("settings/language", languageCode)
        Md3AppSettings.sync()
        applyTheme()
        GitDeskApp.locale.apply(languageCode)
        GitDeskApp.setGitExecutable(gitExecutableHint)
        if (commitTemplate.length && !GitDeskApp.commitMessage.length)
            GitDeskApp.commitMessage = commitTemplate
        Md3Notify.snackbar(qsTr("设置已保存"))
        root.saved()
    }

    Component.onCompleted: load()

    Md3ScrollView {
        id: scroller
        anchors.fill: parent
        clip: true
        fillContentWidth: true
        showScrollToTop: true

        Md3VStack {
            // 铺满对话框内容区，不再限制 720
            width: scroller.width > 1 ? scroller.width : 400
            spacing: Md3Theme.spacingLg
            padding: Md3Theme.spacingMd
            stretchChildren: true

            Md3HStack {
                width: parent.width
                spacing: Md3Theme.spacingSm
                Md3Text {
                    text: qsTr("偏好设置")
                    role: Md3Text.TitleMedium
                }
                Md3Spacer { expand: true }
                Md3Button {
                    text: qsTr("保存")
                    icon: "save"
                    onClicked: root.save()
                }
                Md3Button {
                    text: qsTr("关闭")
                    icon: "close"
                    variant: Md3Button.Text
                    onClicked: root.closeRequested()
                }
            }

            Md3PageSection {
                width: parent.width
                title: qsTr("语言")
                subtitle: qsTr("界面语言")

                Md3HStack {
                    width: parent.width
                    spacing: Md3Theme.spacingSm
                    Repeater {
                        model: GitDeskApp.locale.availableLanguages
                        delegate: Md3Button {
                            required property var modelData
                            text: GitDeskApp.locale.displayName(String(modelData))
                            variant: root.languageCode === String(modelData)
                                     ? Md3Button.Filled
                                     : Md3Button.Outlined
                            onClicked: {
                                root.languageCode = String(modelData)
                                GitDeskApp.locale.apply(root.languageCode)
                                Md3AppSettings.setValue("settings/language", root.languageCode)
                                Md3AppSettings.sync()
                            }
                        }
                    }
                }
            }

            Md3PageSection {
                width: parent.width
                title: qsTr("外观")
                subtitle: qsTr("主题与密度")

                Md3VStack {
                    width: parent.width
                    spacing: Md3Theme.spacingMd

                    Md3Switch {
                        text: qsTr("深色模式")
                        checked: root.darkMode
                        onToggled: function (checked) {
                            root.darkMode = checked
                            root.applyTheme()
                        }
                    }
                    Md3Switch {
                        text: qsTr("紧凑密度（IDE）")
                        checked: root.density === 1
                        onToggled: function (checked) {
                            root.density = checked ? 1 : 0
                            root.applyTheme()
                        }
                    }
                    Md3Switch {
                        text: qsTr("显示键盘焦点环")
                        checked: root.showFocusRings
                        onToggled: function (checked) {
                            root.showFocusRings = checked
                            root.applyTheme()
                        }
                    }
                    Md3TextField {
                        width: parent.width
                        label: qsTr("主题色 Seed")
                        text: root.seedColor
                        placeholderText: "#1B6B4A"
                        onEditingFinished: {
                            if (text.length >= 4) {
                                root.seedColor = text.trim()
                                root.applyTheme()
                            }
                        }
                    }
                    Md3HStack {
                        spacing: Md3Theme.spacingSm
                        Repeater {
                            model: ["#1B6B4A", "#006A6A", "#6750A4", "#8B4513", "#1565C0"]
                            delegate: Rectangle {
                                required property var modelData
                                width: 28
                                height: 28
                                radius: 14
                                color: String(modelData)
                                border.width: root.seedColor.toLowerCase() === String(modelData).toLowerCase() ? 2 : 0
                                border.color: Md3Theme.colorScheme.primary
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        root.seedColor = String(modelData)
                                        root.applyTheme()
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Md3PageSection {
                width: parent.width
                title: qsTr("工作区")
                subtitle: qsTr("面板与 Diff")

                Md3VStack {
                    width: parent.width
                    spacing: Md3Theme.spacingMd

                    Md3Switch {
                        text: qsTr("打开仓库时显示详情面板")
                        checked: root.detailOpenByDefault
                        onToggled: function (checked) {
                            root.detailOpenByDefault = checked
                        }
                    }
                    Md3Switch {
                        text: qsTr("Diff 显示行号")
                        checked: root.showDiffLineNumbers
                        onToggled: function (checked) {
                            root.showDiffLineNumbers = checked
                        }
                    }
                }
            }

            Md3PageSection {
                width: parent.width
                title: qsTr("Git")
                subtitle: qsTr("提交模板与可执行文件")

                Md3VStack {
                    width: parent.width
                    spacing: Md3Theme.spacingMd

                    Md3TextField {
                        width: parent.width
                        label: qsTr("Commit 模板")
                        multiline: true
                        maximumLineCount: 4
                        text: root.commitTemplate
                        placeholderText: qsTr("例如：feat: ")
                        onTextChanged: root.commitTemplate = text
                    }
                    Md3TextField {
                        width: parent.width
                        label: qsTr("git 路径（可选，留空则自动查找）")
                        text: root.gitExecutableHint
                        placeholderText: "C:/Program Files/Git/cmd/git.exe"
                        onEditingFinished: root.gitExecutableHint = text.trim()
                    }
                    Md3Text {
                        width: parent.width
                        wrapMode: Text.Wrap
                        role: Md3Text.BodySmall
                        tone: Md3Text.OnSurfaceVariant
                        text: GitDeskApp.hasRepo
                              ? qsTr("当前仓库：%1").arg(GitDeskApp.repoPath)
                              : qsTr("当前未打开仓库")
                    }
                }
            }

            Md3PageSection {
                width: parent.width
                title: qsTr("数据")
                subtitle: qsTr("最近仓库")

                Md3VStack {
                    width: parent.width
                    spacing: Md3Theme.spacingMd

                    Md3Text {
                        width: parent.width
                        role: Md3Text.BodyMedium
                        text: qsTr("最近打开 %1 个仓库").arg(GitDeskApp.recentRepos.length)
                    }
                    Md3Button {
                        text: qsTr("清空最近列表")
                        icon: "delete"
                        variant: Md3Button.Outlined
                        onClicked: clearRecentsDialog.open = true
                    }
                }
            }

            Md3PageSection {
                width: parent.width
                title: qsTr("快捷键")
                subtitle: qsTr("全局")

                Md3VStack {
                    width: parent.width
                    spacing: 2
                    Repeater {
                        model: [
                            { k: "Ctrl+O", v: qsTr("打开仓库") },
                            { k: "Ctrl+R", v: qsTr("刷新") },
                            { k: "Ctrl+K", v: qsTr("命令面板") },
                            { k: "Ctrl+,", v: qsTr("设置") },
                            { k: "Ctrl+Enter", v: qsTr("提交（有 message 时）") }
                        ]
                        delegate: Md3ListTile {
                            required property var modelData
                            width: parent.width
                            title: modelData.v
                            trailing: Md3Text {
                                text: modelData.k
                                role: Md3Text.LabelMedium
                                tone: Md3Text.OnSurfaceVariant
                            }
                        }
                    }
                }
            }

            Md3PageSection {
                width: parent.width
                title: qsTr("关于")

                Md3VStack {
                    width: parent.width
                    spacing: Md3Theme.spacingSm
                    Md3Text {
                        text: GitDeskApp.versionInfo.name + " " + GitDeskApp.versionInfo.version
                        role: Md3Text.TitleSmall
                    }
                    Md3Text {
                        width: parent.width
                        wrapMode: Text.Wrap
                        role: Md3Text.BodyMedium
                        tone: Md3Text.OnSurfaceVariant
                        text: GitDeskApp.versionInfo.tagline
                    }
                    Md3Text {
                        width: parent.width
                        wrapMode: Text.Wrap
                        role: Md3Text.BodySmall
                        tone: Md3Text.OnSurfaceVariant
                        text: GitDeskApp.versionInfo.description
                    }
                    Md3Text {
                        text: qsTr("渠道 %1 · %2")
                              .arg(GitDeskApp.versionInfo.channel)
                              .arg(GitDeskApp.versionInfo.buildDate)
                        role: Md3Text.LabelSmall
                        tone: Md3Text.OnSurfaceVariant
                    }
                    Md3Text {
                        text: GitDeskApp.versionInfo.organization
                              + " · " + GitDeskApp.versionInfo.author
                        role: Md3Text.LabelMedium
                        tone: Md3Text.OnSurfaceVariant
                    }
                    Repeater {
                        model: GitDeskApp.versionInfo.highlights
                        delegate: Md3Text {
                            required property var modelData
                            width: parent.width
                            wrapMode: Text.Wrap
                            role: Md3Text.BodySmall
                            tone: Md3Text.OnSurfaceVariant
                            text: "· " + String(modelData)
                        }
                    }
                }
            }
        }
    }

    Md3Dialog {
        id: clearRecentsDialog
        title: qsTr("清空最近仓库？")
        text: qsTr("将清除欢迎页上的最近打开列表。")
        confirmText: qsTr("清空")
        confirmTone: Md3Dialog.Error
        dismissText: qsTr("取消")
        onConfirmed: {
            Md3AppSettings.setValue("recentRepos", [])
            Md3AppSettings.sync()
            GitDeskApp.clearRecentRepos()
            Md3Notify.toast(qsTr("已清空"), { severity: Md3Toast.Success })
        }
    }
}
