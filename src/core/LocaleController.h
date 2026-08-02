#pragma once

#include <QObject>
#include <QTranslator>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

class QQmlEngine;

class LocaleController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use GitDeskApp.locale")

    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(int revision READ revision NOTIFY languageChanged)
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages CONSTANT)

public:
    explicit LocaleController(QObject *parent = nullptr);

    void setEngine(QQmlEngine *engine);

    QString language() const { return m_language; }
    int revision() const { return m_revision; }
    QStringList availableLanguages() const;

    Q_INVOKABLE bool apply(const QString &languageCode);
    Q_INVOKABLE QString displayName(const QString &languageCode) const;
    Q_INVOKABLE QString toMd3Language(const QString &languageCode) const;
    Q_INVOKABLE QString normalize(const QString &languageCode) const;

signals:
    void languageChanged();

private:
    bool installAppTranslator(const QString &normalized);

    QQmlEngine *m_engine = nullptr;
    QTranslator m_translator;
    QString m_language = QStringLiteral("zh-CN");
    int m_revision = 0;
};
