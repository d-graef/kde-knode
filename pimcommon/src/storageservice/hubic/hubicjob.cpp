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

#include "hubicjob.h"
#include "storageservice/storageserviceabstract.h"
#include "storageservice/storageservicejobconfig.h"
#include "storageservice/authdialog/storageauthviewdialog.h"

#include <KLocalizedString>

#include "pimcommon_debug.h"
#include <QFile>
#include <QJsonParseError>

using namespace PimCommon;

HubicJob::HubicJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished, this, &HubicJob::slotSendDataFinished);
    mScope = PimCommon::StorageServiceJobConfig::self()->hubicScope();
    mClientId = PimCommon::StorageServiceJobConfig::self()->hubicClientId();
    mClientSecret = PimCommon::StorageServiceJobConfig::self()->hubicClientSecret();
    mRedirectUri = PimCommon::StorageServiceJobConfig::self()->oauth2RedirectUrl();
    mServiceUrl = QStringLiteral("https://api.hubic.com");
    mApiUrl = QStringLiteral("https://api.hubic.com");
    mAuthorizePath = QStringLiteral("/oauth/auth/");
    mPathToken = QStringLiteral("/oauth/token/");
    mCurrentAccountInfoPath = QStringLiteral("/1.0/account/usage");
    mSwiftTokenPath = QStringLiteral("/1.0/account/credentials");
}

HubicJob::~HubicJob()
{

}

void HubicJob::initializeToken(const QString &refreshToken, const QString &token)
{
    mError = false;
    mRefreshToken = refreshToken;
    mToken = token;
}

void HubicJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolderAction;
    mError = false;
    //TODO
    Q_EMIT actionFailed(QStringLiteral("Not Implemented"));
    qCDebug(PIMCOMMON_LOG) << " not implemented";
    deleteLater();
}

void HubicJob::requestTokenAccess()
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::RequestTokenAction;
    QUrl url(mServiceUrl + mAuthorizePath);
    url.addQueryItem(QStringLiteral("response_type"), QStringLiteral("code"));
    url.addQueryItem(QStringLiteral("client_id"), mClientId);
    url.addQueryItem(QStringLiteral("redirect_uri"), mRedirectUri);
    if (!mScope.isEmpty()) {
        url.addQueryItem(QStringLiteral("scope"), mScope);
    }
    mAuthUrl = url;
    //qCDebug(PIMCOMMON_LOG)<<" url"<<url;
    delete mAuthDialog;
    mAuthDialog = new PimCommon::StorageAuthViewDialog;
    connect(mAuthDialog.data(), &PimCommon::StorageAuthViewDialog::urlChanged, this, &HubicJob::slotRedirect);
    mAuthDialog->setUrl(url);
    if (mAuthDialog->exec()) {
        delete mAuthDialog;
    } else {
        Q_EMIT authorizationFailed(i18n("Authorization canceled."));
        delete mAuthDialog;
        deleteLater();
    }
}

void HubicJob::slotRedirect(const QUrl &url)
{
    if (url != mAuthUrl) {
        //qCDebug(PIMCOMMON_LOG)<<" Redirect !"<<url;
        mAuthDialog->accept();
        parseRedirectUrl(url);
    }
}

void HubicJob::parseRedirectUrl(const QUrl &url)
{
    const QList<QPair<QString, QString> > listQuery = url.queryItems();
    //qCDebug(PIMCOMMON_LOG)<< "listQuery "<<listQuery;

    QString authorizeCode;
    QString errorStr;
    QString errorDescription;
    for (int i = 0; i < listQuery.size(); ++i) {
        const QPair<QString, QString> item = listQuery.at(i);
        if (item.first == QLatin1String("code")) {
            authorizeCode = item.second;
            break;
        } else if (item.first == QLatin1String("error")) {
            errorStr = item.second;
        } else if (item.first == QLatin1String("error_description")) {
            errorDescription = item.second;
        }
    }
    if (!authorizeCode.isEmpty()) {
        getTokenAccess(authorizeCode);
    } else {
        Q_EMIT authorizationFailed(errorStr + QLatin1Char(' ') + errorDescription);
        deleteLater();
    }
}

void HubicJob::getTokenAccess(const QString &authorizeCode)
{
    mActionType = PimCommon::StorageServiceAbstract::AccessTokenAction;
    mError = false;
    QNetworkRequest request(QUrl(mServiceUrl + mPathToken));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QUrl postData;
    postData.addQueryItem(QStringLiteral("code"), authorizeCode);
    postData.addQueryItem(QStringLiteral("redirect_uri"), mRedirectUri);
    postData.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("authorization_code"));
    postData.addQueryItem(QStringLiteral("client_id"), mClientId);
    postData.addQueryItem(QStringLiteral("client_secret"), mClientSecret);
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &HubicJob::slotError);
}

void HubicJob::slotSendDataFinished(QNetworkReply *reply)
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
        qCDebug(PIMCOMMON_LOG) << " error " << error;
        if (error.contains(QStringLiteral("message"))) {
            const QString errorStr = error.value(QStringLiteral("message")).toString();
            switch (mActionType) {
            case PimCommon::StorageServiceAbstract::NoneAction:
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::RequestTokenAction:
                Q_EMIT authorizationFailed(errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::AccessTokenAction:
                Q_EMIT authorizationFailed(errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::UploadFileAction:
                Q_EMIT uploadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::DownLoadFileAction:
                Q_EMIT downLoadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::DeleteFileAction:
            case PimCommon::StorageServiceAbstract::CreateFolderAction:
            case PimCommon::StorageServiceAbstract::AccountInfoAction:
            case PimCommon::StorageServiceAbstract::ListFolderAction:
            case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
            case PimCommon::StorageServiceAbstract::DeleteFolderAction:
            case PimCommon::StorageServiceAbstract::RenameFolderAction:
            case PimCommon::StorageServiceAbstract::RenameFileAction:
            case PimCommon::StorageServiceAbstract::MoveFolderAction:
            case PimCommon::StorageServiceAbstract::MoveFileAction:
            case PimCommon::StorageServiceAbstract::CopyFileAction:
            case PimCommon::StorageServiceAbstract::CopyFolderAction:
            case PimCommon::StorageServiceAbstract::ShareLinkAction:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            }
        } else {
            if (!mErrorMsg.isEmpty()) {
                errorMessage(mActionType, mErrorMsg);
            } else {
                errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
            }
            deleteLater();
        }
        return;
    }
    qCDebug(PIMCOMMON_LOG) << " data: " << data;
    switch (mActionType) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::RequestTokenAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::AccessTokenAction:
        parseAccessToken(data);
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
    case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
        parseCreateServiceFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFileAction:
        parseDeleteFile(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFolderAction:
        parseDeleteFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFileAction:
        parseCopyFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFolderAction:
        parseCopyFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFileAction:
        parseRenameFile(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFolderAction:
        parseRenameFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFolderAction:
        parseMoveFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFileAction:
        parseMoveFile(data);
        break;
    case PimCommon::StorageServiceAbstract::ShareLinkAction:
        parseShareLink(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFileAction:
        parseDownloadFile(data);
        break;
    }
}

void HubicJob::parseAccountInfo(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    //qCDebug(PIMCOMMON_LOG)<<" info"<<info;
    PimCommon::AccountInfo accountInfo;
    if (info.contains(QStringLiteral("used"))) {
        accountInfo.shared = info.value(QStringLiteral("used")).toLongLong();
    }
    if (info.contains(QStringLiteral("quota"))) {
        accountInfo.quota = info.value(QStringLiteral("quota")).toLongLong();
    }
    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

void HubicJob::refreshToken()
{
    mActionType = PimCommon::StorageServiceAbstract::AccessTokenAction;
    QNetworkRequest request(QUrl(QStringLiteral("https://api.hubic.com/oauth/token/")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QUrl postData;
    postData.addQueryItem(QStringLiteral("refresh_token"), mRefreshToken);
    postData.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));
    postData.addQueryItem(QStringLiteral("client_id"), mClientId);
    postData.addQueryItem(QStringLiteral("client_secret"), mClientSecret);
    qCDebug(PIMCOMMON_LOG) << "refreshToken postData: " << postData;

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &HubicJob::slotError);
}

void HubicJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFileAction;
    mError = false;
}

void HubicJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolderAction;
    mError = false;
}

void HubicJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolderAction;
    mError = false;
}

void HubicJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFileAction;
    mError = false;
}

void HubicJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolderAction;
    mError = false;
}

void HubicJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFileAction;
    mError = false;
}

void HubicJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFileAction;
    mError = false;
}

void HubicJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolderAction;
    mError = false;
}

QNetworkReply *HubicJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFileAction;
        mError = false;
        if (file->open(QIODevice::ReadOnly)) {
#if 0
            QUrl url;
            url.setUrl(QLatin1String("https://upload.box.com/api/2.0/files/content"));
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
            request.setRawHeader("Authorization", "Bearer " + mToken.toLatin1());
            QUrl postData;
            postData.addQueryItem(QStringLiteral("parent_id"), destination);
            postData.addQueryItem(QStringLiteral("filename"), uploadAsName);
            QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
            file->setParent(reply);
            connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &HubicJob::slotError);
            return reply;
#else
            return Q_NULLPTR;
#endif
        }
    }
    delete file;
    return Q_NULLPTR;
}

void HubicJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolderAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mSwiftTokenPath);
    qCDebug(PIMCOMMON_LOG) << "url" << url;
    QNetworkRequest request(url);
    qCDebug(PIMCOMMON_LOG) << " mToken" << mToken;
    request.setRawHeader("X-Auth-Token", mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &HubicJob::slotError);

#if 0
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + (folder.isEmpty() ? QLatin1String("0") : folder) + QLatin1String("/items?fields=name,created_at,size,modified_at,id"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer " + mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &HubicJob::slotError);
#endif
}

void HubicJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfoAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mCurrentAccountInfoPath);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer " + mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &HubicJob::slotError);
}

void HubicJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolderAction;
    mError = false;
}

void HubicJob::shareLink(const QString &root, const QString &fileId)
{
    Q_UNUSED(root);
    mActionType = PimCommon::StorageServiceAbstract::ShareLinkAction;
    mError = false;
}

void HubicJob::parseDeleteFolder(const QString &data)
{
    /*
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qCDebug(PIMCOMMON_LOG)<<" info"<<info;
    */
    Q_EMIT deleteFolderDone(QString());
}

void HubicJob::parseDeleteFile(const QString &data)
{
    /*
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qCDebug(PIMCOMMON_LOG)<<" info"<<info;
    */
    Q_EMIT deleteFileDone(QString());
}

void HubicJob::parseCreateServiceFolder(const QString &data)
{
#if 0
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qCDebug(PIMCOMMON_LOG) << " info" << info;
#endif
    Q_EMIT actionFailed(QStringLiteral("Not Implemented"));
    deleteLater();
}

void HubicJob::parseListFolder(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void HubicJob::parseCreateFolder(const QString &data)
{
    const QString folderName = parseNameInfo(data);
    Q_EMIT createFolderDone(folderName);
    deleteLater();
}

void HubicJob::parseUploadFile(const QString &data)
{
    const QString folderName = parseNameInfo(data);
    qCDebug(PIMCOMMON_LOG) << " data" << data;
    Q_EMIT uploadFileDone(folderName);
    //shareLink(QString());
    deleteLater();
}

void HubicJob::parseCopyFile(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT copyFileDone(filename);
    deleteLater();
}

void HubicJob::parseRenameFile(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT renameFileDone(filename);
    deleteLater();
}

void HubicJob::parseRenameFolder(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT renameFolderDone(filename);
    deleteLater();
}

void HubicJob::parseCopyFolder(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT copyFolderDone(filename);
    deleteLater();
}

void HubicJob::parseMoveFolder(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT moveFolderDone(filename);
    deleteLater();
}

void HubicJob::parseMoveFile(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT moveFileDone(filename);
    deleteLater();
}

QString HubicJob::parseNameInfo(const QString &data)
{
    QString filename;
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return filename;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();

    if (info.contains(QStringLiteral("name"))) {
        filename = info.value(QStringLiteral("name")).toString();
    }
    return filename;

}

void HubicJob::parseShareLink(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    QString url;
    if (info.contains(QStringLiteral("shared_link"))) {
        const QVariantMap map = info.value(QStringLiteral("shared_link")).toMap();
        if (map.contains(QStringLiteral("url"))) {
            url = map.value(QStringLiteral("url")).toString();
        }
    }
    Q_EMIT shareLinkDone(url);
    deleteLater();
}

void HubicJob::parseDownloadFile(const QString &data)
{
    qCDebug(PIMCOMMON_LOG) << " Data " << data;
    Q_EMIT downLoadFileDone(QString());
}

QNetworkReply *HubicJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFileAction;
    mError = false;
#if 0
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
    const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);
    delete mDownloadFile;
    mDownloadFile = new QFile(defaultDestination + QLatin1Char('/') + name);
    if (mDownloadFile->open(QIODevice::WriteOnly)) {
        QUrl url;
        qCDebug(PIMCOMMON_LOG) << " fileId " << fileId << " name " << name;
        url.setUrl(mApiUrl + mFileInfoPath + fileId + QLatin1String("/content"));
        qCDebug(PIMCOMMON_LOG) << "url!" << url;
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
        request.setRawHeader("Authorization", "Bearer " + mToken.toLatin1());
        QNetworkReply *reply = mNetworkAccessManager->get(request);
        mDownloadFile->setParent(reply);
        connect(reply, &QNetworkReply::readyRead, this, &HubicJob::slotDownloadReadyRead);
        connect(reply, &QNetworkReply::downloadProgress, this, &HubicJob::slotuploadDownloadFileProgress);
        connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &HubicJob::slotError);
        return reply;
    } else {
        delete mDownloadFile;
    }
#endif
    return Q_NULLPTR;
}

void HubicJob::parseAccessToken(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        qCDebug(PIMCOMMON_LOG) << " parseAccessToken error" << data;
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    qCDebug(PIMCOMMON_LOG) << " info" << info;
    if (info.contains(QStringLiteral("refresh_token"))) {
        mRefreshToken = info.value(QStringLiteral("refresh_token")).toString();
    }
    if (info.contains(QStringLiteral("access_token"))) {
        mToken = info.value(QStringLiteral("access_token")).toString();
    }
    qint64 expireInTime = 0;
    if (info.contains(QStringLiteral("expires_in"))) {
        expireInTime = info.value(QStringLiteral("expires_in")).toLongLong();
    }
    qCDebug(PIMCOMMON_LOG) << " parseAccessToken";
    Q_EMIT authorizationDone(mRefreshToken, mToken, expireInTime);
    deleteLater();
}

