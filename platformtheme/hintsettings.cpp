/*
 * Copyright (C) 2021 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hintsettings.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QString>
#include <QFileInfo>
#include <QToolBar>
#include <QPalette>
#include <QToolButton>
#include <QMainWindow>
#include <QApplication>
#include <QGuiApplication>
#include <QDialogButtonBox>
#include <QScreen>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <qpa/qplatformdialoghelper.h>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QFontDatabase>

static const QByteArray s_systemFontName = QByteArrayLiteral("Font");
static const QByteArray s_systemFixedFontName = QByteArrayLiteral("FixedFont");
static const QByteArray s_systemPointFontSize = QByteArrayLiteral("FontSize");
static const QByteArray s_darkModeName = QByteArrayLiteral("DarkMode");
static const QByteArray s_lightIconName = (QFile::exists(QStringLiteral("/usr/share/icons/Aevis")) || QFile::exists(QStringLiteral("/usr/share/icons/Aevis-Dark")))
    ? QByteArrayLiteral("Aevis")
    : QByteArrayLiteral("Cutefish");
static const QByteArray s_darkIconName = (QFile::exists(QStringLiteral("/usr/share/icons/Aevis-Dark")) || QFile::exists(QStringLiteral("/usr/share/icons/Cutefish-Dark")))
    ? (QFile::exists(QStringLiteral("/usr/share/icons/Aevis-Dark")) ? QByteArrayLiteral("Aevis-Dark") : QByteArrayLiteral("Cutefish-Dark"))
    : QByteArrayLiteral("Cutefish-Dark");

static QString availableFontFamily(const QString &requested, const QString &fallback)
{
    QFontDatabase database;
    const QStringList families = database.families();

    if (families.contains(requested))
        return requested;
    if (families.contains(fallback))
        return fallback;
    if (families.contains(QStringLiteral("SF Pro Text")))
        return QStringLiteral("SF Pro Text");
    if (families.contains(QStringLiteral("SF Pro Display")))
        return QStringLiteral("SF Pro Display");
    if (families.contains(QStringLiteral("SF Pro")))
        return QStringLiteral("SF Pro");
    if (families.contains(QStringLiteral("Noto Sans")))
        return QStringLiteral("Noto Sans");

    return QStringLiteral("Sans Serif");
}

HintsSettings::HintsSettings(QObject *parent)
    : QObject(parent),
      m_settings(new QSettings(QSettings::UserScope,
                               QFile::exists(QDir::homePath() + QStringLiteral("/.config/aevis/theme.conf")) ? QStringLiteral("aevis") : QStringLiteral("cutefishos"),
                               QStringLiteral("theme")))
{
    const QString defaultTheme = QString::fromLatin1(darkMode() ? s_darkIconName : s_lightIconName);
    m_iconTheme = m_settings->value("IconTheme", defaultTheme).toString();

    if (m_iconTheme.isEmpty() || m_iconTheme.contains(QLatin1String("Crule"))
        || m_iconTheme == QLatin1String("cutefish-dark") || m_iconTheme == QLatin1String("cutefish-light")
        || m_iconTheme == QLatin1String("aevis-dark") || m_iconTheme == QLatin1String("aevis-light")
        || m_iconTheme.startsWith(QLatin1String("WhiteSur"), Qt::CaseInsensitive)) {
        m_iconTheme = defaultTheme;
    }

    if (m_iconTheme.startsWith(QLatin1String("Aevis"), Qt::CaseInsensitive)
        || m_iconTheme.startsWith(QLatin1String("Cutefish"), Qt::CaseInsensitive)) {
        m_hints[QPlatformTheme::SystemIconThemeName] = darkMode() ? s_darkIconName : s_lightIconName;
    } else {
        m_hints[QPlatformTheme::SystemIconThemeName] = m_iconTheme;
    }

    m_hints[QPlatformTheme::SystemIconFallbackThemeName] = s_lightIconName;
    m_hints[QPlatformTheme::StyleNames] = QStringList() << QStringLiteral("aevis") << QStringLiteral("cutefish");
    m_hints[QPlatformTheme::SystemIconFallbackThemeName] = QStringLiteral("hicolor");
    m_hints[QPlatformTheme::IconThemeSearchPaths] = xdgIconThemePaths();
    m_hints[QPlatformTheme::UseFullScreenForPopupMenu] = false;
    m_hints[QPlatformTheme::DialogButtonBoxLayout] = QPlatformDialogHelper::MacLayout;

    m_settingsFile = m_settings->fileName();

    QMetaObject::invokeMethod(this, "lazyInit", Qt::QueuedConnection);
}

HintsSettings::~HintsSettings()
{
}

void HintsSettings::lazyInit()
{
    m_fileWatcher = new QFileSystemWatcher();
    m_fileWatcher->addPath(m_settingsFile);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &HintsSettings::onFileChanged);
}

QStringList HintsSettings::xdgIconThemePaths() const
{
    QStringList paths;

    // make sure we have ~/.local/share/icons in paths if it exists
    paths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory);

    const QFileInfo homeIconDir(QDir::homePath() + QStringLiteral("/.icons"));
    if (homeIconDir.isDir()) {
        paths << homeIconDir.absoluteFilePath();
    }

    return paths;
}

QString HintsSettings::systemFont() const
{
    return availableFontFamily(m_settings->value(s_systemFontName, "SF Pro Text").toString(),
                               QStringLiteral("SF Pro"));
}

QString HintsSettings::systemFixedFont() const
{
    return availableFontFamily(m_settings->value(s_systemFixedFontName, "Monospace").toString(),
                               QStringLiteral("DejaVu Sans Mono"));
}

qreal HintsSettings::systemFontPointSize() const
{
    return m_settings->value(s_systemPointFontSize, 9).toDouble();
}

bool HintsSettings::darkMode()
{
    return m_settings->value(s_darkModeName, false).toBool();
}

void HintsSettings::onFileChanged(const QString &path)
{
    Q_UNUSED(path);

    QVariantMap map;
    for (const QString &value : m_settings->allKeys()) {
        map[value] = m_settings->value(value);
    }

    m_settings->sync();

    for (const QString &value : m_settings->allKeys()) {
        const QVariant &oldValue = map.value(value);
        const QVariant &newValue = m_settings->value(value);

        if (oldValue != newValue) {
            if (value == s_systemFontName)
                emit systemFontChanged(newValue.toString());
            else if (value == s_systemFixedFontName)
                emit systemFixedFontChanged(newValue.toString());
            else if (value == s_systemPointFontSize)
                emit systemFontPointSizeChanged(newValue.toDouble());
            else if (value == s_darkModeName) {
                emit darkModeChanged(newValue.toBool());
                if (m_iconTheme.startsWith(QLatin1String("Aevis"), Qt::CaseInsensitive)
                    || m_iconTheme.startsWith(QLatin1String("Cutefish"), Qt::CaseInsensitive)) {
                    m_hints[QPlatformTheme::SystemIconThemeName] = darkMode() ? s_darkIconName : s_lightIconName;
                }
                emit iconThemeChanged();
            } else if (value == "IconTheme") {
                QString newTheme = newValue.toString();
                if (newTheme.isEmpty() || newTheme.contains(QLatin1String("Crule"))
                    || newTheme == QLatin1String("cutefish-dark") || newTheme == QLatin1String("cutefish-light")
                    || newTheme == QLatin1String("aevis-dark") || newTheme == QLatin1String("aevis-light")
                    || newTheme.startsWith(QLatin1String("WhiteSur"), Qt::CaseInsensitive)) {
                    newTheme = QString::fromLatin1(darkMode() ? s_darkIconName : s_lightIconName);
                }
                m_iconTheme = newTheme;
                if (m_iconTheme.startsWith(QLatin1String("Aevis"), Qt::CaseInsensitive)
                    || m_iconTheme.startsWith(QLatin1String("Cutefish"), Qt::CaseInsensitive)) {
                    m_hints[QPlatformTheme::SystemIconThemeName] = darkMode() ? s_darkIconName : s_lightIconName;
                } else {
                    m_hints[QPlatformTheme::SystemIconThemeName] = m_iconTheme;
                }
                emit iconThemeChanged();
            }
        }
    }

    bool fileDeleted = !m_fileWatcher->files().contains(m_settingsFile);
    if (fileDeleted)
        m_fileWatcher->addPath(m_settingsFile);
}
