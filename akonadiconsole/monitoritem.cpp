/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "monitoritem.h"
#include "monitorsmodel.h"
#include "notificationsourceinterface.h"
#include "akonadiconsole_debug.h"
#include <AkonadiCore/servermanager.h>

#include <QStringList>
#include <QTimer>
#include <QVector>

MonitorItem::MonitorItem(const QDBusObjectPath &identifier_, MonitorsModel *model):
    QObject(model),
    identifier(identifier_),
    allMonitored(false)
{

    QString service = QStringLiteral("org.freedesktop.Akonadi");
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        service += QLatin1String(".") + Akonadi::ServerManager::instanceIdentifier();
    }

    mInterface = new org::freedesktop::Akonadi::NotificationSource(
        service, identifier.path(),
        QDBusConnection::sessionBus(), this);
    if (!mInterface) {
        qCWarning(AKONADICONSOLE_LOG) << "Failed to connect to org.freedesktop.Akonadi.NotificationSource of subscriber" << identifier_.path();
        return;
    }

    if (mInterface->lastError().isValid()) {
        qCWarning(AKONADICONSOLE_LOG) << mInterface->lastError().message();
        return;
    }

    connect(mInterface, &org::freedesktop::Akonadi::NotificationSource::monitoredCollectionsChanged, this, &MonitorItem::monitoredCollectionsChanged);
    connect(mInterface, &org::freedesktop::Akonadi::NotificationSource::monitoredItemsChanged, this, &MonitorItem::monitoredItemsChanged);
    connect(mInterface, &org::freedesktop::Akonadi::NotificationSource::monitoredResourcesChanged, this, &MonitorItem::monitoredResourcesChanged);
    connect(mInterface, &org::freedesktop::Akonadi::NotificationSource::monitoredMimeTypesChanged, this, &MonitorItem::monitoredMimeTypesChanged);
    connect(mInterface, &org::freedesktop::Akonadi::NotificationSource::isAllMonitoredChanged, this, &MonitorItem::isAllMonitoredChanged);
    connect(mInterface, &org::freedesktop::Akonadi::NotificationSource::ignoredSessionsChanged, this, &MonitorItem::ignoredSessionsChanged);

    QTimer::singleShot(0, this, &MonitorItem::init);
}

MonitorItem::~MonitorItem()
{
}

void MonitorItem::init()
{
    isAllMonitoredChanged();
    monitoredCollectionsChanged();
    monitoredItemsChanged();
    monitoredMimeTypesChanged();
    monitoredResourcesChanged();
    ignoredSessionsChanged();
}

void MonitorItem::isAllMonitoredChanged()
{
    allMonitored = mInterface->isAllMonitored();
    Q_EMIT changed(MonitorsModel::IsAllMonitoredColumn);
}

void MonitorItem::monitoredCollectionsChanged()
{
    const QVector<qint64> list = mInterface->monitoredCollections();
    monitoredCollections.clear();
    for (int i = 0; i < list.size(); i++) {
        if (i > 0) {
            monitoredCollections += QLatin1String(", ");
        }
        monitoredCollections += QString::number(list[i]);
    }

    Q_EMIT changed(MonitorsModel::MonitoredCollectionsColumn);
}

void MonitorItem::monitoredItemsChanged()
{
    const QVector<qint64> list = mInterface->monitoredItems();
    monitoredItems.clear();
    for (int i = 0; i < list.size(); i++) {
        if (i > 0) {
            monitoredItems += QLatin1String(", ");
        }
        monitoredItems += QString::number(list[i]);
    }

    Q_EMIT changed(MonitorsModel::MonitoredItemsColumn);
}

void MonitorItem::monitoredMimeTypesChanged()
{
    const QStringList mimeTypes = mInterface->monitoredMimeTypes();
    monitoredMimeTypes = mimeTypes.join(QStringLiteral(", "));
    Q_EMIT changed(MonitorsModel::MonitoredMimeTypesColumn);
}

void MonitorItem::monitoredResourcesChanged()
{
    const QVector<QByteArray> list = mInterface->monitoredResources();
    monitoredResources.clear();
    for (int i = 0; i < list.size(); i++) {
        if (i > 0) {
            monitoredResources += QStringLiteral(", ");
        }
        monitoredResources += QLatin1String(list[i]);
    }

    Q_EMIT changed(MonitorsModel::MonitoredResourcesColumn);
}

void MonitorItem::ignoredSessionsChanged()
{
    const QVector<QByteArray> list = mInterface->ignoredSessions();
    ignoredSessions.clear();
    for (int i = 0; i < list.size(); i++) {
        if (i > 0) {
            ignoredSessions += QLatin1String(", ");
        }
        ignoredSessions += QLatin1String(list[i]);
    }

    Q_EMIT changed(MonitorsModel::IgnoredSessionsColumn);
}

