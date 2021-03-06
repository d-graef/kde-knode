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

#ifndef STORAGESERVICEMANAGER_H
#define STORAGESERVICEMANAGER_H

#include <QObject>
#include <QMap>
#include "pimcommon_export.h"
#include "storageserviceabstract.h"

class KActionMenu;
namespace PimCommon
{
class StorageServiceManagerPrivate;
class PIMCOMMON_EXPORT StorageServiceManager : public QObject
{
    Q_OBJECT
public:
    enum ServiceType {
        Unknown = -1,
        DropBox = 0,
        Hubic,
        YouSendIt,
        WebDav,
        Box,
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
        GDrive,
#endif
        //Last element
        EndListService
    };

    explicit StorageServiceManager(QObject *parent = Q_NULLPTR);
    ~StorageServiceManager();

    QString ourIdentifier() const;

    KActionMenu *menuShareLinkServices(QWidget *parent) const;
    KActionMenu *menuDownloadServices(QWidget *parent) const;
    KActionMenu *menuUploadServices(QWidget *parent) const;
    KActionMenu *menuWithCapability(PimCommon::StorageServiceAbstract::Capability mainCapability, const QList<PimCommon::StorageServiceAbstract::Capability> &lstCapability, QWidget *parent) const;

    QMap<QString, StorageServiceAbstract *> listService() const;
    void setListService(const QMap<QString, StorageServiceAbstract *> &lst);
    void setDefaultUploadFolder(const QString &folder);
    QString defaultUploadFolder() const;

    static QString serviceToI18n(ServiceType type);
    static QString serviceName(ServiceType type);
    static QString description(ServiceType type);
    static QUrl serviceUrl(ServiceType type);
    static QString icon(ServiceType type);
    static StorageServiceAbstract::Capabilities capabilities(ServiceType type);
    static QString kconfigName();

    void removeService(const QString &serviceName);

Q_SIGNALS:
    void servicesChanged();
    void deleteFolderDone(const QString &serviceName, const QString &filename);
    void uploadFileDone(const QString &serviceName, const QString &filename);
    void uploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void uploadFileFailed(const QString &serviceName, const QString &filename);
    void shareLinkDone(const QString &serviceName, const QString &link);
    void authenticationDone(const QString &serviceName);
    void authenticationFailed(const QString &serviceName, const QString &error);
    void actionFailed(const QString &serviceName, const QString &error);
    void deleteFileDone(const QString &serviceName, const QString &filename);
    void accountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &accountInfo);
    void uploadFileStart(PimCommon::StorageServiceAbstract *service);
    void configChanged(const QString &id);

private Q_SLOTS:
    void slotAccountInfo();
    void slotShareFile();
    void slotDeleteFile();
    void slotDeleteFolder();
    void slotDownloadFile();

    void slotConfigChanged(const QString &id);
private:
    void defaultConnect(StorageServiceAbstract *service);
    void readConfig();
    void writeConfig();
    StorageServiceManagerPrivate *const d;
};
}

#endif // STORAGESERVICEMANAGER_H
