/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "evolutioncalendar.h"
#include "evolutionutil.h"

#include "importwizard_debug.h"

#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>

EvolutionCalendar::EvolutionCalendar(ImportWizard *parent)
    : AbstractCalendar(parent)
{
}

EvolutionCalendar::~EvolutionCalendar()
{

}

void EvolutionCalendar::loadCalendar(const QString &filename)
{
    //Read gconf file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(IMPORTWIZARD_LOG) << " We can't open file" << filename;
        return;
    }
    QDomDocument doc;
    if (!EvolutionUtil::loadInDomDocument(&file, doc)) {
        return;
    }
    QDomElement config = doc.documentElement();

    if (config.isNull()) {
        qCDebug(IMPORTWIZARD_LOG) << "No config found";
        return;
    }
    mCalendarPath = QDir::homePath() + QLatin1String("/.local/share/evolution/calendar/");
    for (QDomElement e = config.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        const QString tag = e.tagName();
        if (tag == QLatin1String("entry")) {
            if (e.hasAttribute("name")) {
                const QString attr = e.attribute("name");
                if (attr == QLatin1String("sources")) {
                    readCalendar(e);
                } else {
                    qCDebug(IMPORTWIZARD_LOG) << " attr unknown " << attr;
                }
            }
        }
    }
}

void EvolutionCalendar::readCalendar(const QDomElement &calendar)
{
    for (QDomElement calendarConfig = calendar.firstChildElement(); !calendarConfig.isNull(); calendarConfig = calendarConfig.nextSiblingElement()) {
        if (calendarConfig.tagName() == QLatin1String("li")) {
            const QDomElement stringValue = calendarConfig.firstChildElement();
            extractCalendarInfo(stringValue.text());
        }
    }
}

void EvolutionCalendar::extractCalendarInfo(const QString &info)
{
    //qCDebug(IMPORTWIZARD_LOG)<<" info "<<info;
    //Read QDomElement
    QDomDocument cal;
    if (!EvolutionUtil::loadInDomDocument(info, cal)) {
        return;
    }
    QDomElement domElement = cal.documentElement();

    if (domElement.isNull()) {
        qCDebug(IMPORTWIZARD_LOG) << "Account not found";
        return;
    }
    QString base_uri;
    if (domElement.hasAttribute(QStringLiteral("base_uri"))) {
        base_uri = domElement.attribute(QStringLiteral("base_uri"));
    }
    if (base_uri == QLatin1String("local:")) {
        for (QDomElement e = domElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString tag = e.tagName();
            if (tag == QLatin1String("source")) {
                QString name;
                QMap<QString, QVariant> settings;
                if (e.hasAttribute(QStringLiteral("uid"))) {
                }
                if (e.hasAttribute(QStringLiteral("name"))) {
                    name = e.attribute(QStringLiteral("name"));
                    settings.insert(QStringLiteral("DisplayName"), name);
                }
                if (e.hasAttribute(QStringLiteral("relative_uri"))) {
                    const QString path = mCalendarPath + e.attribute(QStringLiteral("relative_uri")) + QLatin1String("/calendar.ics");
                    settings.insert(QStringLiteral("Path"), path);
                }
                if (e.hasAttribute(QStringLiteral("color_spec"))) {
                    //const QString color = e.attribute(QStringLiteral("color_spec"));
                    //Need id.
                    //TODO: Need to get id for collection to add color.
                }
                QDomElement propertiesElement = e.firstChildElement();
                if (!propertiesElement.isNull()) {
                    for (QDomElement property = propertiesElement.firstChildElement(); !property.isNull(); property = property.nextSiblingElement()) {
                        const QString propertyTag = property.tagName();
                        if (propertyTag == QLatin1String("property")) {
                            if (property.hasAttribute(QStringLiteral("name"))) {
                                const QString propertyName = property.attribute(QStringLiteral("name"));
                                if (propertyName == QLatin1String("custom-file-readonly")) {
                                    if (property.hasAttribute(QStringLiteral("value"))) {
                                        if (property.attribute(QStringLiteral("value")) == QLatin1String("1")) {
                                            settings.insert(QStringLiteral("ReadOnly"), true);
                                        }
                                    }
                                } else if (propertyName == QLatin1String("alarm")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " need to implement alarm property";
                                } else {
                                    qCDebug(IMPORTWIZARD_LOG) << " property unknown :" << propertyName;
                                }
                            }
                        } else {
                            qCDebug(IMPORTWIZARD_LOG) << " tag unknown :" << propertyTag;
                        }
                    }
                }
                AbstractBase::createResource(QStringLiteral("akonadi_ical_resource"), name, settings);
            } else {
                qCDebug(IMPORTWIZARD_LOG) << " tag unknown :" << tag;
            }
        }
    } else if (base_uri == QLatin1String("webcal://")) {
        qCDebug(IMPORTWIZARD_LOG) << " need to implement webcal protocol";
    } else if (base_uri == QLatin1String("google://")) {
        qCDebug(IMPORTWIZARD_LOG) << " need to implement google protocol";
    } else if (base_uri == QLatin1String("caldav://")) {
        qCDebug(IMPORTWIZARD_LOG) << " need to implement caldav protocol";
    } else {
        qCDebug(IMPORTWIZARD_LOG) << " base_uri unknown" << base_uri;
    }
}
