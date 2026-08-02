#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class VersionInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use GitDeskApp.versionInfo")

    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString version READ version NOTIFY changed)
    Q_PROPERTY(QString buildDate READ buildDate NOTIFY changed)
    Q_PROPERTY(QString channel READ channel NOTIFY changed)
    Q_PROPERTY(QString organization READ organization NOTIFY changed)
    Q_PROPERTY(QString author READ author NOTIFY changed)
    Q_PROPERTY(QString summary READ summary NOTIFY changed)
    Q_PROPERTY(QString tagline READ tagline NOTIFY changed)
    Q_PROPERTY(QString description READ description NOTIFY changed)
    Q_PROPERTY(QStringList highlights READ highlights NOTIFY changed)
    Q_PROPERTY(QVariantList changelog READ changelog NOTIFY changed)
    Q_PROPERTY(QString aboutPlainText READ aboutPlainText NOTIFY changed)

public:
    explicit VersionInfo(QObject *parent = nullptr);

    QString name() const { return m_name; }
    QString version() const { return m_version; }
    QString buildDate() const { return m_buildDate; }
    QString channel() const { return m_channel; }
    QString organization() const { return m_organization; }
    QString author() const { return m_author; }
    QString summary() const { return m_summary; }
    QString tagline() const { return m_tagline; }
    QString description() const { return m_description; }
    QStringList highlights() const { return m_highlights; }
    QVariantList changelog() const { return m_changelog; }
    QString aboutPlainText() const;

signals:
    void changed();

private:
    void load();

    QString m_name = QStringLiteral("GitDesk");
    QString m_version = QStringLiteral("0.1.0");
    QString m_buildDate;
    QString m_channel = QStringLiteral("dev");
    QString m_organization = QStringLiteral("wuyijing-dev");
    QString m_author = QStringLiteral("wuyijing");
    QString m_summary;
    QString m_tagline;
    QString m_description;
    QStringList m_highlights;
    QVariantList m_changelog;
};
