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

#include "dropboxstorageservice.h"
#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/widgets/storageservicetreewidgetitem.h"
#include "storageservice/storageservicemanager.h"
#include "dropboxutil.h"
#include "dropboxjob.h"
#include "storageservice/settings/storageservicesettings.h"
#include "storageservice/storageservicejobconfig.h"
#include "storageservice/utils/storageserviceutils.h"

#include <kwallet.h>

#include <KLocalizedString>

#include "pimcommon_debug.h"
#include <KFormat>
#include <QLocale>
#include <QJsonParseError>

using namespace PimCommon;

DropBoxStorageService::DropBoxStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

DropBoxStorageService::~DropBoxStorageService()
{
}

void DropBoxStorageService::shutdownService()
{
    mAccessToken.clear();
    mAccessTokenSecret.clear();
    mAccessOauthSignature.clear();
}

bool DropBoxStorageService::hasValidSettings() const
{
    return (!PimCommon::StorageServiceJobConfig::self()->dropboxOauthConsumerKey().isEmpty() &&
            !PimCommon::StorageServiceJobConfig::self()->dropboxOauthSignature().isEmpty() &&
            !PimCommon::StorageServiceJobConfig::self()->dropboxRootPath().isEmpty());
}

void DropBoxStorageService::removeConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            wallet->removeEntry(walletEntry);
        }
    }
}

void DropBoxStorageService::readConfig()
{
    if (StorageServiceSettings::self()->createDefaultFolder()) {
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QStringList lst = wallet->entryList();
            if (lst.contains(storageServiceName())) {
                QMap<QString, QString> map;
                wallet->readMap(storageServiceName(), map);
                if (map.contains(QStringLiteral("Access Token"))) {
                    mAccessToken = map.value(QStringLiteral("Access Token"));
                }
                if (map.contains(QStringLiteral("Access Token Secret"))) {
                    mAccessTokenSecret = map.value(QStringLiteral("Access Token Secret"));
                }
                if (map.contains(QStringLiteral("Access Oauth Signature"))) {
                    mAccessOauthSignature = map.value(QStringLiteral("Access Oauth Signature"));
                }
            }
            mNeedToReadConfigFirst = false;
        }
    }
}

void DropBoxStorageService::storageServiceauthentication()
{
    DropBoxJob *job = new DropBoxJob(this);
    connect(job, &DropBoxJob::authorizationDone, this, &DropBoxStorageService::slotAuthorizationDone);
    connect(job, &DropBoxJob::authorizationFailed, this, &DropBoxStorageService::slotAuthorizationFailed);
    connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
    job->requestTokenAccess();
}

bool DropBoxStorageService::checkNeedAuthenticate()
{
    if (mNeedToReadConfigFirst) {
        readConfig();
    }
    return mAccessToken.isEmpty();
}

void DropBoxStorageService::storageServiceShareLink(const QString &root, const QString &path)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(ShareLinkAction);
        mNextAction->setRootPath(root);
        mNextAction->setPath(path);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::shareLinkDone, this, &DropBoxStorageService::slotShareLinkDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->shareLink(root, path);
    }
}

void DropBoxStorageService::storageServicecreateServiceFolder()
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(CreateServiceFolderAction);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::createFolderDone, this, &DropBoxStorageService::slotCreateFolderDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->createServiceFolder();
    }
}

void DropBoxStorageService::slotAuthorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature)
{
    mAccessToken = accessToken;
    mAccessTokenSecret = accessTokenSecret;
    mAccessOauthSignature = accessOauthSignature;

    if (StorageServiceSettings::self()->createDefaultFolder()) {
        const QString walletEntry = storageServiceName();
        KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
        if (wallet) {
            QMap<QString, QString> map;
            map[QStringLiteral("Access Token")] = mAccessToken;
            map[QStringLiteral("Access Token Secret")] = mAccessTokenSecret;
            map[QStringLiteral("Access Oauth Signature")] = mAccessOauthSignature;
            wallet->writeMap(walletEntry, map);
        }
    }

    emitAuthentificationDone();
}

void DropBoxStorageService::storageServicelistFolder(const QString &folder)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(ListFolderAction);
        mNextAction->setNextActionFolder(folder);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::listFolderDone, this, &DropBoxStorageService::slotListFolderDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->listFolder(folder);
    }
}

void DropBoxStorageService::storageServiceaccountInfo()
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(AccountInfoAction);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::accountInfoDone, this, &DropBoxStorageService::slotAccountInfoDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->accountInfo();
    }
}

void DropBoxStorageService::storageServicecreateFolder(const QString &name, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(CreateFolderAction);
        mNextAction->setNextActionName(name);
        mNextAction->setNextActionFolder(destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::createFolderDone, this, &DropBoxStorageService::slotCreateFolderDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->createFolder(name, destination);
    }
}

void DropBoxStorageService::storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(UploadFileAction);
        mNextAction->setNextActionName(filename);
        mNextAction->setNextActionFolder(destination);
        mNextAction->setUploadAsName(uploadAsName);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::uploadFileDone, this, &DropBoxStorageService::slotUploadFileDone);
        connect(job, &DropBoxJob::shareLinkDone, this, &DropBoxStorageService::slotShareLinkDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        connect(job, &DropBoxJob::uploadDownloadFileProgress, this, &DropBoxStorageService::slotuploadDownloadFileProgress);
        connect(job, &DropBoxJob::uploadFileFailed, this, &DropBoxStorageService::slotUploadFileFailed);
        mUploadReply = job->uploadFile(filename, uploadAsName, destination);
    }
}

void DropBoxStorageService::slotAuthorizationFailed(const QString &errorMessage)
{
    mAccessToken.clear();
    mAccessTokenSecret.clear();
    mAccessOauthSignature.clear();
    emitAuthentificationFailed(errorMessage);
}

QString DropBoxStorageService::name()
{
    return i18n("Dropbox");
}

QString DropBoxStorageService::description()
{
    return i18n("Dropbox is a file hosting service operated by Dropbox, Inc. that offers cloud storage, file synchronization, and client software.");
}

QUrl DropBoxStorageService::serviceUrl()
{
    return QUrl(QStringLiteral("https://www.dropbox.com/"));
}

QString DropBoxStorageService::serviceName()
{
    return QStringLiteral("dropbox");
}

QString DropBoxStorageService::iconName()
{
    return QStringLiteral("kdepim-dropbox");
}

StorageServiceAbstract::Capabilities DropBoxStorageService::serviceCapabilities()
{
    StorageServiceAbstract::Capabilities cap;
    cap |= AccountInfoCapability;
    cap |= UploadFileCapability;
    cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= DeleteFileCapability;
    cap |= ListFolderCapability;
    cap |= ShareLinkCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    cap |= MoveFileCapability;
    cap |= MoveFolderCapability;
    cap |= CopyFileCapability;
    cap |= CopyFolderCapability;

    return cap;
}

QString DropBoxStorageService::storageServiceName() const
{
    return serviceName();
}

void DropBoxStorageService::storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(DownLoadFileAction);
        mNextAction->setNextActionName(name);
        mNextAction->setDownloadDestination(destination);
        mNextAction->setFileId(fileId);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::downLoadFileDone, this, &DropBoxStorageService::slotDownLoadFileDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        connect(job, &DropBoxJob::downLoadFileFailed, this, &DropBoxStorageService::slotDownLoadFileFailed);
        connect(job, &DropBoxJob::uploadDownloadFileProgress, this, &DropBoxStorageService::slotuploadDownloadFileProgress);
        mDownloadReply = job->downloadFile(name, fileId, destination);
    }
}

void DropBoxStorageService::storageServicedeleteFile(const QString &filename)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(DeleteFileAction);
        mNextAction->setNextActionName(filename);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::deleteFileDone, this, &DropBoxStorageService::slotDeleteFileDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->deleteFile(filename);
    }
}

void DropBoxStorageService::storageServicedeleteFolder(const QString &foldername)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(DeleteFolderAction);
        mNextAction->setNextActionFolder(foldername);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::deleteFolderDone, this, &DropBoxStorageService::slotDeleteFolderDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->deleteFolder(foldername);
    }
}

void DropBoxStorageService::storageServiceRenameFolder(const QString &source, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(RenameFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::renameFolderDone, this, &DropBoxStorageService::slotRenameFolderDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->renameFolder(source, destination);
    }
}

void DropBoxStorageService::storageServiceRenameFile(const QString &source, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(RenameFileAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::renameFileDone, this, &DropBoxStorageService::slotRenameFileDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->renameFile(source, destination);
    }
}

void DropBoxStorageService::storageServiceMoveFolder(const QString &source, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(MoveFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::moveFolderDone, this, &DropBoxStorageService::slotMoveFileDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->moveFolder(source, destination);
    }
}

void DropBoxStorageService::storageServiceMoveFile(const QString &source, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(RenameFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::moveFileDone, this, &DropBoxStorageService::slotMoveFileDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->moveFile(source, destination);
    }
}

void DropBoxStorageService::storageServiceCopyFile(const QString &source, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(CopyFileAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::copyFileDone, this, &DropBoxStorageService::slotCopyFileDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->copyFile(source, destination);
    }
}

void DropBoxStorageService::storageServiceCopyFolder(const QString &source, const QString &destination)
{
    if (checkNeedAuthenticate()) {
        mNextAction->setNextActionType(CopyFolderAction);
        mNextAction->setRenameFolder(source, destination);
        storageServiceauthentication();
    } else {
        DropBoxJob *job = new DropBoxJob(this);
        job->initializeToken(mAccessToken, mAccessTokenSecret, mAccessOauthSignature);
        connect(job, &DropBoxJob::copyFolderDone, this, &DropBoxStorageService::slotCopyFolderDone);
        connect(job, &DropBoxJob::actionFailed, this, &DropBoxStorageService::slotActionFailed);
        job->copyFolder(source, destination);
    }
}

QString DropBoxStorageService::fileIdentifier(const QVariantMap &variantMap)
{
    if (variantMap.contains(QStringLiteral("path"))) {
        return variantMap.value(QStringLiteral("path")).toString();
    }
    return QString();
}

QString DropBoxStorageService::fileShareRoot(const QVariantMap &variantMap)
{
    if (variantMap.contains(QStringLiteral("root"))) {
        return variantMap.value(QStringLiteral("root")).toString();
    }
    return QString();
}

StorageServiceAbstract::Capabilities DropBoxStorageService::capabilities() const
{
    return serviceCapabilities();
}

QString DropBoxStorageService::fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder)
{
    Q_UNUSED(currentFolder);
    listWidget->clear();
    QJsonParseError error;
    QString parentFolder;
    const QJsonDocument json = QJsonDocument::fromJson(data.toString().toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || json.isNull()) {
        return parentFolder;
    }
    const QMap<QString, QVariant> info = json.toVariant().toMap();
    listWidget->createMoveUpItem();
    if (info.contains(QStringLiteral("path"))) {
        const QString path = info.value(QStringLiteral("path")).toString();
        if (parentFolder.isEmpty()) {
            if (!path.isEmpty()) {
                if (path == QLatin1String("/")) {
                    parentFolder = path;
                } else {
                    QStringList parts = path.split(QStringLiteral("/"));
                    parts.removeLast();
                    parentFolder = parts.join(QStringLiteral("/"));
                    if (parentFolder.isEmpty()) {
                        parentFolder = QStringLiteral("/");
                    }
                }
            }
        }
    }
    if (info.contains(QStringLiteral("contents"))) {
        const QVariantList lst = info.value(QStringLiteral("contents")).toList();
        Q_FOREACH (const QVariant &variant, lst) {
            const QVariantMap qwer = variant.toMap();
            //qCDebug(PIMCOMMON_LOG)<<" qwer "<<qwer;
            if (qwer.contains(QStringLiteral("is_dir"))) {
                const bool isDir = qwer.value(QStringLiteral("is_dir")).toBool();
                const QString name = qwer.value(QStringLiteral("path")).toString();

                const QString itemName = name.right((name.length() - name.lastIndexOf(QLatin1Char('/'))) - 1);

                StorageServiceTreeWidgetItem *item;
                if (isDir) {
                    item = listWidget->addFolder(itemName, name);
                } else {
                    QString mimetype;
                    if (qwer.contains(QStringLiteral("mime_type"))) {
                        mimetype = qwer.value(QStringLiteral("mime_type")).toString();
                    }
                    item = listWidget->addFile(itemName, name, mimetype);
                    if (qwer.contains(QStringLiteral("bytes"))) {
                        item->setSize(qwer.value(QStringLiteral("bytes")).toULongLong());
                    }
                }
                if (qwer.contains(QStringLiteral("client_mtime"))) {
                    QString tmp = qwer.value(QStringLiteral("client_mtime")).toString();
                    item->setDateCreated(PimCommon::DropBoxUtil::convertToDateTime(tmp));
                }
                if (qwer.contains(QStringLiteral("modified"))) {
                    QString tmp = qwer.value(QStringLiteral("modified")).toString();
                    item->setLastModification(PimCommon::DropBoxUtil::convertToDateTime(tmp));
                }
                item->setStoreInfo(qwer);
            }
        }
    }
    return parentFolder;
}

QMap<QString, QString> DropBoxStorageService::itemInformation(const QVariantMap &variantMap)
{
    QMap<QString, QString> information;

    const bool isDir = variantMap.value(QStringLiteral("is_dir")).toBool();
    const QString name = variantMap.value(QStringLiteral("path")).toString();

    const QString itemName = name.right((name.length() - name.lastIndexOf(QLatin1Char('/'))) - 1);
    information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Type), isDir ? i18n("Folder") : i18n("File"));
    information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Name), itemName);

    if (variantMap.contains(QStringLiteral("bytes"))) {
        const qulonglong size = variantMap.value(QStringLiteral("bytes")).toULongLong();
        if (!(isDir && size == 0)) {
            information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Size), KFormat().formatByteSize(size));
        }
    }
    if (variantMap.contains(QStringLiteral("client_mtime"))) {
        const QString tmp = variantMap.value(QStringLiteral("client_mtime")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::Created), QLocale().toString((PimCommon::DropBoxUtil::convertToDateTime(tmp)), QLocale::ShortFormat));
    }
    if (variantMap.contains(QStringLiteral("modified"))) {
        const QString tmp = variantMap.value(QStringLiteral("modified")).toString();
        information.insert(PimCommon::StorageServiceUtils::propertyNameToI18n(PimCommon::StorageServiceUtils::LastModified), QLocale().toString((PimCommon::DropBoxUtil::convertToDateTime(tmp)), QLocale::ShortFormat));
    }
    if (variantMap.contains(QStringLiteral("root"))) {
        information.insert(i18n("Storage path:"), variantMap.value(QStringLiteral("root")).toString());
    }
    return information;
}

QIcon DropBoxStorageService::icon() const
{
    return QIcon::fromTheme(iconName());
}

QString DropBoxStorageService::disallowedSymbols() const
{
    return QLatin1String("[/:?*<>\"|]");
}

QString DropBoxStorageService::disallowedSymbolsStr() const
{
    return QStringLiteral("\\ / : ? * < > \" |");
}

qlonglong DropBoxStorageService::maximumUploadFileSize() const
{
    return 150000000;
}

