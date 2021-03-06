/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "yousenditutil.h"
#include <QVariant>
#include <QDateTime>
#include <QJsonParseError>

QStringList PimCommon::YouSendItUtil::getListFolder(const QString &data)
{
    QStringList listFolder;

    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return listFolder;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();

    if (info.contains(QStringLiteral("folders"))) {
        QVariantMap mapFolder = info.value(QStringLiteral("folders")).toMap();
        QVariantList folders = mapFolder.value(QStringLiteral("folder")).toList();
        Q_FOREACH (const QVariant &v, folders) {
            QVariantMap map = v.toMap();
            if (map.contains(QStringLiteral("name"))) {
                listFolder.append(map.value(QStringLiteral("name")).toString());
            }
        }
    }
    return listFolder;
}

QDateTime PimCommon::YouSendItUtil::convertToDateTime(QString dateTime, bool removeTimeZone)
{
    if (removeTimeZone) {
        dateTime.chop(6);    // chop() removes the time zone
    }
    const QDateTime t = QDateTime(QDateTime::fromString(dateTime, QStringLiteral("yyyy-MM-ddThh:mm:ss")));
    return t;
}
