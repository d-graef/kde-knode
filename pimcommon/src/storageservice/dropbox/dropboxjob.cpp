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

#include "dropboxjob.h"
#include "storageservice/authdialog/storageauthviewdialog.h"
#include "storageservice/storageserviceabstract.h"
#include "storageservice/utils/storageserviceutils.h"
#include "storageservice/storageservicejobconfig.h"

#include <KLocalizedString>
#include <QUrl>

#include <QFile>

#include <QNetworkRequest>
#include <QDateTime>
#include <QStringList>
#include "pimcommon_debug.h"
#include <QPointer>
#include <QJsonDocument>

using namespace PimCommon;

DropBoxJob::DropBoxJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mApiPath = QStringLiteral("https://api.dropbox.com/1/");
    mOauthconsumerKey = PimCommon::StorageServiceJobConfig::self()->dropboxOauthConsumerKey();
    mOauthSignature = PimCommon::StorageServiceJobConfig::self()->dropboxOauthSignature();
    mRootPath = PimCommon::StorageServiceJobConfig::self()->dropboxRootPath();
    mOauthVersion = QStringLiteral("1.0");
    mOauthSignatureMethod = QStringLiteral("PLAINTEXT");
    mTimestamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);
    mNonce = PimCommon::StorageServiceUtils::generateNonce(8);
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished, this, &DropBoxJob::slotSendDataFinished);
}

DropBoxJob::~DropBoxJob()
{

}

void DropBoxJob::initializeToken(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature)
{
    mOauthToken = accessToken;
    mOauthTokenSecret = accessTokenSecret;
    mAccessOauthSignature = accessOauthSignature;
}

void DropBoxJob::requestTokenAccess()
{
    mActionType = PimCommon::StorageServiceAbstract::RequestTokenAction;
    mError = false;
    QNetworkRequest request(QUrl(mApiPath + QLatin1String("oauth/request_token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QUrl postData;

    postData.addQueryItem(QStringLiteral("oauth_consumer_key"), mOauthconsumerKey);
    postData.addQueryItem(QStringLiteral("oauth_nonce"), mNonce);
    postData.addQueryItem(QStringLiteral("oauth_signature"), mOauthSignature);
    postData.addQueryItem(QStringLiteral("oauth_signature_method"), mOauthSignatureMethod);
    postData.addQueryItem(QStringLiteral("oauth_timestamp"), mTimestamp);
    postData.addQueryItem(QStringLiteral("oauth_version"), mOauthVersion);

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::getTokenAccess()
{
    mActionType = PimCommon::StorageServiceAbstract::AccessTokenAction;
    mError = false;
    QNetworkRequest request(QUrl(mApiPath + QLatin1String("oauth/access_token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QUrl postData;
    postData.addQueryItem(QStringLiteral("oauth_consumer_key"), mOauthconsumerKey);
    postData.addQueryItem(QStringLiteral("oauth_nonce"), mNonce);
    postData.addQueryItem(QStringLiteral("oauth_signature"), mAccessOauthSignature);
    postData.addQueryItem(QStringLiteral("oauth_signature_method"), mOauthSignatureMethod);
    postData.addQueryItem(QStringLiteral("oauth_timestamp"), mTimestamp);
    postData.addQueryItem(QStringLiteral("oauth_version"), mOauthVersion);
    postData.addQueryItem(QStringLiteral("oauth_token"), mOauthToken);

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        QJsonParseError parsingError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
        if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
            errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
            deleteLater();
            return;
        }
        const QMap<QString, QVariant> error = jsonDoc.toVariant().toMap();
        if (error.contains(QStringLiteral("error"))) {
            const QString errorStr = error.value(QStringLiteral("error")).toString();
            switch (mActionType) {
            case PimCommon::StorageServiceAbstract::NoneAction:
                break;
            case PimCommon::StorageServiceAbstract::AccessTokenAction:
            case PimCommon::StorageServiceAbstract::RequestTokenAction:
                Q_EMIT authorizationFailed(i18n("Dropbox access is not authorized. Error message: %1", errorStr));
                break;
            case PimCommon::StorageServiceAbstract::UploadFileAction:
                Q_EMIT uploadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                break;
            case PimCommon::StorageServiceAbstract::DownLoadFileAction:
                Q_EMIT downLoadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                break;
            case PimCommon::StorageServiceAbstract::CreateFolderAction:
            case PimCommon::StorageServiceAbstract::AccountInfoAction:
            case PimCommon::StorageServiceAbstract::ListFolderAction:
            case PimCommon::StorageServiceAbstract::ShareLinkAction:
            case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
            case PimCommon::StorageServiceAbstract::DeleteFileAction:
            case PimCommon::StorageServiceAbstract::DeleteFolderAction:
            case PimCommon::StorageServiceAbstract::RenameFolderAction:
            case PimCommon::StorageServiceAbstract::RenameFileAction:
            case PimCommon::StorageServiceAbstract::MoveFolderAction:
            case PimCommon::StorageServiceAbstract::MoveFileAction:
            case PimCommon::StorageServiceAbstract::CopyFileAction:
            case PimCommon::StorageServiceAbstract::CopyFolderAction:
                errorMessage(mActionType, errorStr);
                break;
            }
        } else {
            errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
        }
        deleteLater();
        return;
    }
    switch (mActionType) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        break;
    case PimCommon::StorageServiceAbstract::RequestTokenAction:
        parseRequestToken(data);
        break;
    case PimCommon::StorageServiceAbstract::AccessTokenAction:
        parseResponseAccessToken(data);
        break;
    case PimCommon::StorageServiceAbstract::UploadFileAction:
        parseUploadFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateFolderAction:
        parseCreateFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::AccountInfoAction:
        parseAccountInfo(data);
        break;
    case PimCommon::StorageServiceAbstract::ListFolderAction:
        parseListFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::ShareLinkAction:
        parseShareLink(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::DeleteFileAction:
        parseDeleteFile(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFolderAction:
        parseDeleteFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFileAction:
        parseDownLoadFile(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFolderAction:
        parseRenameFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFileAction:
        parseRenameFile(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFolderAction:
        parseMoveFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFileAction:
        parseMoveFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFileAction:
        parseCopyFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFolderAction:
        parseCopyFolder(data);
        break;
    }
}

QString DropBoxJob::extractPathFromData(const QString &data)
{
    QString name;
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return name;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    if (info.contains(QStringLiteral("path"))) {
        name = info.value(QStringLiteral("path")).toString();
    }
    return name;
}

void DropBoxJob::parseCopyFile(const QString &data)
{
    //qCDebug(PIMCOMMON_LOG)<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT copyFileDone(name);
    deleteLater();
}

void DropBoxJob::parseCopyFolder(const QString &data)
{
    //qCDebug(PIMCOMMON_LOG)<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT copyFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseMoveFolder(const QString &data)
{
    //qCDebug(PIMCOMMON_LOG)<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT moveFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseMoveFile(const QString &data)
{
    //qCDebug(PIMCOMMON_LOG)<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT moveFileDone(name);
    deleteLater();
}

void DropBoxJob::parseRenameFile(const QString &data)
{
    //qCDebug(PIMCOMMON_LOG)<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT renameFileDone(name);
    deleteLater();
}

void DropBoxJob::parseRenameFolder(const QString &data)
{
    //qCDebug(PIMCOMMON_LOG)<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT renameFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseDeleteFolder(const QString &data)
{
    const QString name = extractPathFromData(data);
    Q_EMIT deleteFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseDeleteFile(const QString &data)
{
    const QString name = extractPathFromData(data);
    Q_EMIT deleteFileDone(name);
    deleteLater();
}

void DropBoxJob::parseAccountInfo(const QString &data)
{
    QJsonParseError error;
    const QJsonDocument json = QJsonDocument::fromJson(data.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || json.isNull()) {
        errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
        deleteLater();
        return;
    }
    const QMap<QString, QVariant> info = json.toVariant().toMap();

    PimCommon::AccountInfo accountInfo;
    if (info.contains(QStringLiteral("display_name"))) {
        accountInfo.displayName = info.value(QStringLiteral("display_name")).toString();
    }
    if (info.contains(QStringLiteral("quota_info"))) {
        QMap<QString, QVariant> quotaInfo = info.value(QStringLiteral("quota_info")).toMap();
        if (quotaInfo.contains(QStringLiteral("quota"))) {
            accountInfo.quota = quotaInfo.value(QStringLiteral("quota")).toLongLong();
        }
        if (quotaInfo.contains(QStringLiteral("normal"))) {
            accountInfo.accountSize = quotaInfo.value(QStringLiteral("normal")).toLongLong();
        }
        if (quotaInfo.contains(QStringLiteral("shared"))) {
            accountInfo.shared = quotaInfo.value(QStringLiteral("shared")).toLongLong();
        }
    }

    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

void DropBoxJob::parseResponseAccessToken(const QString &data)
{
    if (data.contains(QStringLiteral("error"))) {
        Q_EMIT authorizationFailed(data);
    } else {
        QStringList split           = data.split(QLatin1Char('&'));
        QStringList tokenSecretList = split.at(0).split(QLatin1Char('='));
        mOauthTokenSecret          = tokenSecretList.at(1);
        QStringList tokenList       = split.at(1).split(QLatin1Char('='));
        mOauthToken = tokenList.at(1);
        mAccessOauthSignature = mOauthSignature + mOauthTokenSecret;

        Q_EMIT authorizationDone(mOauthToken, mOauthTokenSecret, mAccessOauthSignature);
    }
    deleteLater();
}

void DropBoxJob::parseRequestToken(const QString &result)
{
    const QStringList split = result.split(QLatin1Char('&'));
    if (split.count() == 2) {
        const QStringList tokenSecretList = split.at(0).split(QLatin1Char('='));
        mOauthTokenSecret = tokenSecretList.at(1);
        const QStringList tokenList = split.at(1).split(QLatin1Char('='));
        mOauthToken = tokenList.at(1);
        mAccessOauthSignature = mOauthSignature + mOauthTokenSecret;

        //qCDebug(PIMCOMMON_LOG)<<" mOauthToken" <<mOauthToken<<"mAccessOauthSignature "<<mAccessOauthSignature<<" mOauthSignature"<<mOauthSignature;

    } else {
        qCDebug(PIMCOMMON_LOG) << " data is not good: " << result;
    }
    doAuthentication();
}

void DropBoxJob::doAuthentication()
{
    QUrl url(mApiPath + QLatin1String("oauth/authorize"));
    url.addQueryItem(QStringLiteral("oauth_token"), mOauthToken);
    QPointer<StorageAuthViewDialog> dlg = new StorageAuthViewDialog;
    dlg->setUrl(url);
    if (dlg->exec()) {
        getTokenAccess();
        delete dlg;
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        delete dlg;
        deleteLater();
    }
}

void DropBoxJob::createFolderJob(const QString &foldername, const QString &destination)
{
    if (foldername.isEmpty()) {
        qCDebug(PIMCOMMON_LOG) << " folder empty!";
    }
    QUrl url(mApiPath + QLatin1String("fileops/create_folder"));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    url.addQueryItem(QStringLiteral("path"), destination + QLatin1Char('/') + foldername);
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolderAction;
    mError = false;
    createFolderJob(foldername, destination);
}

QNetworkReply *DropBoxJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFileAction;
        mError = false;
        if (file->open(QIODevice::ReadOnly)) {
            const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);
            const QString r = mAccessOauthSignature.replace(QLatin1Char('&'), QStringLiteral("%26"));
            const QString str = QStringLiteral("https://api-content.dropbox.com/1/files_put/dropbox///%7/%1?oauth_consumer_key=%2&oauth_nonce=%3&oauth_signature=%4&oauth_signature_method=PLAINTEXT&oauth_timestamp=%6&oauth_version=1.0&oauth_token=%5&overwrite=false").
                                arg(uploadAsName).arg(mOauthconsumerKey).arg(mNonce).arg(r).arg(mOauthToken).arg(mTimestamp).arg(defaultDestination);
            QUrl url(str);
            QNetworkRequest request(url);
            QNetworkReply *reply = mNetworkAccessManager->put(request, file);
            file->setParent(reply);
            connect(reply, &QNetworkReply::uploadProgress, this, &DropBoxJob::slotuploadDownloadFileProgress);
            connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
            return reply;
        }
    }
    delete file;
    return Q_NULLPTR;
}

void DropBoxJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfoAction;
    mError = false;
    QUrl url(mApiPath + QLatin1String("account/info"));
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolderAction;
    mError = false;
    QUrl url(mApiPath + QLatin1String("metadata/dropbox/") + folder);
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::parseUploadFile(const QString &data)
{
    QJsonParseError error;
    const QJsonDocument json = QJsonDocument::fromJson(data.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || json.isNull()) {
        errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
        deleteLater();
        return;
    }
    const QMap<QString, QVariant> info = json.toVariant().toMap();
    QString root;
    QString path;
    if (info.contains(QStringLiteral("root"))) {
        root = info.value(QStringLiteral("root")).toString();
    }
    if (info.contains(QStringLiteral("path"))) {
        path = info.value(QStringLiteral("path")).toString();
    }
    QString itemName;
    if (!path.isEmpty()) {
        itemName = path.right((path.length() - path.lastIndexOf(QLatin1Char('/'))) - 1);
    }
    Q_EMIT uploadFileDone(itemName);
    shareLink(root, path);
}

void DropBoxJob::shareLink(const QString &root, const QString &path)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLinkAction;
    mError = false;

    QUrl url = QUrl(mApiPath + QStringLiteral("shares/%1/%2").arg(root).arg(path));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    addDefaultUrlItem(url);
    QNetworkRequest request(url);

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolderAction;
    mError = false;
    createFolderJob(PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder(), QString());
}

QNetworkReply *DropBoxJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    Q_UNUSED(fileId);
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFileAction;
    mError = false;
    const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);
    delete mDownloadFile;
    mDownloadFile = new QFile(defaultDestination + QLatin1Char('/') + name);
    if (mDownloadFile->open(QIODevice::WriteOnly)) {
        const QString r = mAccessOauthSignature.replace(QLatin1Char('&'), QStringLiteral("%26"));
        const QString str = QStringLiteral("https://api-content.dropbox.com/1/files/dropbox///%1?oauth_consumer_key=%2&oauth_nonce=%3&oauth_signature=%4&oauth_signature_method=PLAINTEXT&oauth_timestamp=%6&oauth_version=1.0&oauth_token=%5").
                            arg(name).arg(mOauthconsumerKey).arg(mNonce).arg(r).arg(mOauthToken).arg(mTimestamp);
        QUrl url(str);
        QNetworkRequest request(url);
        QNetworkReply *reply = mNetworkAccessManager->get(request);
        mDownloadFile->setParent(reply);
        connect(reply, &QNetworkReply::readyRead, this, &DropBoxJob::slotDownloadReadyRead);
        connect(reply, &QNetworkReply::downloadProgress, this, &DropBoxJob::slotuploadDownloadFileProgress);
        connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
        return reply;
    } else {
        delete mDownloadFile;
    }
    return Q_NULLPTR;
}

void DropBoxJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFileAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/delete"));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    url.addQueryItem(QStringLiteral("path"), filename);
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolderAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/delete"));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    url.addQueryItem(QStringLiteral("path"), foldername);
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolderAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/move"));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QStringLiteral("from_path"), source);

    QStringList parts = source.split(QStringLiteral("/"), QString::SkipEmptyParts);
    parts.removeLast();
    QString destinationFolder = parts.join(QStringLiteral("/"));
    if (destinationFolder.isEmpty()) {
        destinationFolder = QStringLiteral("/");
    }
    if (!destinationFolder.endsWith(QLatin1Char('/'))) {
        destinationFolder += QLatin1String("/");
    }
    if (!destinationFolder.startsWith(QLatin1Char('/'))) {
        destinationFolder.prepend(QLatin1String("/"));
    }

    const QString newPath = destinationFolder.append(destination);
    url.addQueryItem(QStringLiteral("to_path"), newPath);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFileAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/move"));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    addDefaultUrlItem(url);

    //Generate new path.
    QStringList parts = oldName.split(QStringLiteral("/"), QString::SkipEmptyParts);
    parts.removeLast();
    QString destinationFolder = parts.join(QStringLiteral("/"));
    if (destinationFolder.isEmpty()) {
        destinationFolder = QStringLiteral("/");
    }
    if (!destinationFolder.endsWith(QLatin1Char('/'))) {
        destinationFolder += QLatin1String("/");
    }
    if (!destinationFolder.startsWith(QLatin1Char('/'))) {
        destinationFolder.prepend(QLatin1String("/"));
    }

    const QString newPath = destinationFolder.append(newName);

    url.addQueryItem(QStringLiteral("from_path"), oldName);
    url.addQueryItem(QStringLiteral("to_path"), newPath);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolderAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/move"));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QStringLiteral("from_path"), source);
    url.addQueryItem(QStringLiteral("to_path"), destination + source);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFileAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/move"));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QStringLiteral("from_path"), source);
    url.addQueryItem(QStringLiteral("to_path"), destination + source);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFileAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/copy"));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QStringLiteral("from_path"), source);
    url.addQueryItem(QStringLiteral("to_path"), destination + source);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolderAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/copy"));
    url.addQueryItem(QStringLiteral("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QStringLiteral("from_path"), source);
    url.addQueryItem(QStringLiteral("to_path"), destination + source);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DropBoxJob::slotError);
}

void DropBoxJob::addDefaultUrlItem(QUrl &url)
{
    url.addQueryItem(QStringLiteral("oauth_consumer_key"), mOauthconsumerKey);
    url.addQueryItem(QStringLiteral("oauth_nonce"), mNonce);
    url.addQueryItem(QStringLiteral("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'), QStringLiteral("%26")));
    url.addQueryItem(QStringLiteral("oauth_signature_method"), mOauthSignatureMethod);
    url.addQueryItem(QStringLiteral("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QStringLiteral("oauth_version"), mOauthVersion);
    url.addQueryItem(QStringLiteral("oauth_token"), mOauthToken);
}

void DropBoxJob::parseShareLink(const QString &data)
{
    QJsonParseError error;
    const QJsonDocument json = QJsonDocument::fromJson(data.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || json.isNull()) {
        errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
        deleteLater();
        return;
    }
    const QMap<QString, QVariant> info = json.toVariant().toMap();
    QString url;
    if (info.contains(QStringLiteral("url"))) {
        url = info.value(QStringLiteral("url")).toString();
    }
    Q_EMIT shareLinkDone(url);
    deleteLater();
}

void DropBoxJob::parseCreateFolder(const QString &data)
{
    const QString name = extractPathFromData(data);
    Q_EMIT createFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseListFolder(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void DropBoxJob::parseDownLoadFile(const QString &data)
{
    //qCDebug(PIMCOMMON_LOG)<<" data "<<data;
    Q_EMIT downLoadFileDone(QString());
    deleteLater();
}
