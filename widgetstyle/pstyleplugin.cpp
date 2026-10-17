#include "pstyleplugin.h"
#include "basestyle.h"

#include <QApplication>
#include <QStyleFactory>
#include <QDebug>

QStringList ProxyStylePlugin::keys() const
{
    return {QStringLiteral("aevis"), QStringLiteral("cutefish")};
}

QStyle *ProxyStylePlugin::create(const QString &key)
{
    if (key.compare(QStringLiteral("aevis"), Qt::CaseInsensitive) != 0 &&
        key.compare(QStringLiteral("cutefish"), Qt::CaseInsensitive) != 0) {
        return nullptr;
    }

    return new BaseStyle;
}
