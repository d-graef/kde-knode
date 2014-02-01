/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "webdavjob.h"
#include "webdavsettingsdialog.h"
#include "pimcommon/storageservice/authdialog/logindialog.h"
#include "pimcommon/storageservice/webdav/protocol/webdav.h"

#include <KLocalizedString>

#include <qjson/parser.h>

#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

using namespace PimCommon;

WebDavJob::WebDavJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent),
      mReqId(-1)
{
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
    connect(mNetworkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
}

WebDavJob::~WebDavJob()
{

}

void WebDavJob::initializeToken(const QString &publicLocation, const QString &serviceLocation, const QString &username, const QString &password)
{
    mUserName = username;
    mPassword = password;
    mPublicLocation = publicLocation;
    mServiceLocation = serviceLocation;
}

void WebDavJob::slotAuthenticationRequired(QNetworkReply *,QAuthenticator *auth)
{
    QPointer<LoginDialog> dlg = new LoginDialog;
    dlg->setCaption(i18n("WebDav"));
    if (dlg->exec()) {
        mUserName = dlg->username();
        mPassword = dlg->password();
        auth->setUser(mUserName);
        auth->setPassword(mPassword);
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        deleteLater();
    }
    delete dlg;
}

void WebDavJob::requestTokenAccess()
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;
    QPointer<WebDavSettingsDialog> dlg = new WebDavSettingsDialog;
    if (dlg->exec()) {
        mServiceLocation = dlg->serviceLocation();
        mPublicLocation = dlg->publicLocation();
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        deleteLater();
    }
    delete dlg;
    QUrl url(mServiceLocation);
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void WebDavJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFile;
    mError = false;
    qDebug()<<" copyFile :source :"<<source<<" destination "<<destination;
    QWebdav *webdav = new QWebdav(this);
    QUrl url = configureWebDav(webdav);
    connect(webdav, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)),
            this, SLOT(slotRequired(QString,quint16,QAuthenticator*)));
    connect(webdav, SIGNAL(requestFinished(int, bool)), this, SLOT(slotRequestFinished(int, bool)));
    mReqId = webdav->copy(url.toString() + QLatin1Char('/') + source, url.toString() + QLatin1Char('/') + destination + QLatin1Char('/') + source);
}

void WebDavJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolder;
    mError = false;
    qDebug()<<" copyFolder :source :"<<source<<" destination "<<destination;
    QWebdav *webdav = new QWebdav(this);
    QUrl url = configureWebDav(webdav);
    connect(webdav, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)),
            this, SLOT(slotRequired(QString,quint16,QAuthenticator*)));
    connect(webdav, SIGNAL(requestFinished(int, bool)), this, SLOT(slotRequestFinished(int, bool)));
    mReqId = webdav->copy(url.toString() + source, url.toString() + destination);
}

void WebDavJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFile;
    mError = false;
    QWebdav *webdav = new QWebdav(this);
    QUrl url = configureWebDav(webdav);
    connect(webdav, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)),
            this, SLOT(slotRequired(QString,quint16,QAuthenticator*)));
    connect(webdav, SIGNAL(requestFinished(int, bool)), this, SLOT(slotRequestFinished(int, bool)));
    mReqId = webdav->remove(url.toString() + QLatin1Char('/') + filename);
}

void WebDavJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolder;
    mError = false;
    QWebdav *webdav = new QWebdav(this);
    QUrl url = configureWebDav(webdav);
    connect(webdav, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)),
            this, SLOT(slotRequired(QString,quint16,QAuthenticator*)));
    connect(webdav, SIGNAL(requestFinished(int, bool)), this, SLOT(slotRequestFinished(int, bool)));
    mReqId = webdav->rmdir(url.toString() + QLatin1Char('/') + foldername + QLatin1Char('/'));
}


QNetworkReply *WebDavJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::UploadFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
    return 0;
}

QUrl WebDavJob::configureWebDav(QWebdav *webdav)
{
    webdav->setUser(mUserName, mPassword);

    QUrl url(mServiceLocation);

    QHttp::ConnectionMode mode = QHttp::ConnectionModeHttp;

    if (url.port() == -1) {
        if (mServiceLocation.startsWith(QLatin1String("https://"))) {
            url.setPort(443);
        } else if (mServiceLocation.startsWith(QLatin1String("http://"))) {
            url.setPort(80);
        }
    }

    if (mServiceLocation.startsWith(QLatin1String("https://")))
        mode = QHttp::ConnectionModeHttps;
    else
        mode = QHttp::ConnectionModeHttp;
    webdav->setHost(url.host(), mode, url.port());
    return url;
}

void WebDavJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    qDebug()<<" folder"<<folder;
    QWebdav *webdav = new QWebdav(this);
    QUrl url = configureWebDav(webdav);
    if (!folder.isEmpty())
        url.setPath(folder);

    connect(webdav, SIGNAL(listInfo(QString)), this, SLOT(slotListInfo(QString)));
    connect(webdav, SIGNAL(sslErrors(QList<QSslError>)),
            webdav, SLOT(ignoreSslErrors()));
    connect(webdav, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)),
            this, SLOT(slotRequired(QString,quint16,QAuthenticator*)));
    qDebug()<<" url.toString()"<<url.toString();
    webdav->list(url.toString());
}

void WebDavJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolder;
    mError = false;
    QWebdav *webdav = new QWebdav(this);
    QUrl url = configureWebDav(webdav);
    connect(webdav, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)),
            this, SLOT(slotRequired(QString,quint16,QAuthenticator*)));
    connect(webdav, SIGNAL(requestFinished(int, bool)), this, SLOT(slotRequestFinished(int, bool)));
    //TODO add destination
    qDebug()<<" url.toString() "<<url.toString() <<" foldername"<<foldername;
    mReqId = webdav->mkdir(url.toString() + QLatin1Char('/') + destination + QLatin1Char('/') + foldername + QLatin1Char('/'));
}

void WebDavJob::slotRequired(const QString &, quint16 , QAuthenticator *authenticator)
{
    QPointer<LoginDialog> dlg = new LoginDialog;
    dlg->setCaption(i18n("WebDav"));
    if (dlg->exec()) {
        mUserName = dlg->username();
        mPassword = dlg->password();
        authenticator->setUser(mUserName);
        authenticator->setPassword(mPassword);
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        deleteLater();
    }
    delete dlg;
    authenticator->setUser(mUserName);
    authenticator->setPassword(mPassword);
}

void WebDavJob::slotListInfo(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void WebDavJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfo;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}


void WebDavJob::slotRequestFinished(int id, bool)
{
    if (id == mReqId) {
        switch(mActionType) {
        case PimCommon::StorageServiceAbstract::CreateFolder:
            Q_EMIT createFolderDone(QString());
            break;
        case PimCommon::StorageServiceAbstract::CopyFile:
            Q_EMIT copyFileDone(QString());
            break;
        case PimCommon::StorageServiceAbstract::CopyFolder:
            Q_EMIT copyFolderDone(QString());
            break;
        case PimCommon::StorageServiceAbstract::DeleteFile:
            Q_EMIT deleteFileDone(QString());
            break;
        case PimCommon::StorageServiceAbstract::DeleteFolder:
            Q_EMIT deleteFolderDone(QString());
            break;
        case PimCommon::StorageServiceAbstract::NoneAction:
        case PimCommon::StorageServiceAbstract::RequestToken:
        case PimCommon::StorageServiceAbstract::AccessToken:
        case PimCommon::StorageServiceAbstract::UploadFile:
        case PimCommon::StorageServiceAbstract::DownLoadFile:
        case PimCommon::StorageServiceAbstract::AccountInfo:
        case PimCommon::StorageServiceAbstract::ListFolder:
        case PimCommon::StorageServiceAbstract::CreateServiceFolder:
        case PimCommon::StorageServiceAbstract::RenameFolder:
        case PimCommon::StorageServiceAbstract::RenameFile:
        case PimCommon::StorageServiceAbstract::MoveFolder:
        case PimCommon::StorageServiceAbstract::MoveFile:
        case PimCommon::StorageServiceAbstract::ShareLink:
            break;
        }
        deleteLater();
    }
}

void WebDavJob::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        qDebug()<<" error type "<<data;
        QJson::Parser parser;
        bool ok;

        QMap<QString, QVariant> error = parser.parse(data.toUtf8(), &ok).toMap();
        if (error.contains(QLatin1String("error"))) {
            const QString errorStr = error.value(QLatin1String("error")).toString();
            switch(mActionType) {
            case PimCommon::StorageServiceAbstract::NoneAction:
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::RequestToken:
                Q_EMIT authorizationFailed(errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::AccessToken:
                Q_EMIT authorizationFailed(errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::UploadFile:
                Q_EMIT uploadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::DownLoadFile:
                Q_EMIT downLoadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::CreateFolder:
            case PimCommon::StorageServiceAbstract::AccountInfo:
            case PimCommon::StorageServiceAbstract::ListFolder:
            case PimCommon::StorageServiceAbstract::CreateServiceFolder:
            case PimCommon::StorageServiceAbstract::DeleteFile:
            case PimCommon::StorageServiceAbstract::DeleteFolder:
            case PimCommon::StorageServiceAbstract::RenameFolder:
            case PimCommon::StorageServiceAbstract::RenameFile:
            case PimCommon::StorageServiceAbstract::MoveFolder:
            case PimCommon::StorageServiceAbstract::MoveFile:
            case PimCommon::StorageServiceAbstract::CopyFile:
            case PimCommon::StorageServiceAbstract::CopyFolder:
            case PimCommon::StorageServiceAbstract::ShareLink:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            }
        } else {
            errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
            deleteLater();
        }
        return;
    }
    switch(mActionType) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::RequestToken:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::AccessToken:
        parseAccessToken(data);
        break;
    case PimCommon::StorageServiceAbstract::UploadFile:
        parseUploadFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateFolder:
        parseCreateFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::AccountInfo:
        parseAccountInfo(data);
        break;
    case PimCommon::StorageServiceAbstract::ListFolder:
        parseListFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFile:
    case PimCommon::StorageServiceAbstract::DeleteFile:
    case PimCommon::StorageServiceAbstract::DeleteFolder:
    case PimCommon::StorageServiceAbstract::RenameFolder:
    case PimCommon::StorageServiceAbstract::RenameFile:
    case PimCommon::StorageServiceAbstract::MoveFolder:
    case PimCommon::StorageServiceAbstract::MoveFile:
    case PimCommon::StorageServiceAbstract::CopyFile:
    case PimCommon::StorageServiceAbstract::CopyFolder:
    case PimCommon::StorageServiceAbstract::ShareLink:
    case PimCommon::StorageServiceAbstract::CreateServiceFolder:

        deleteLater();
        break;
    }
}

void WebDavJob::parseAccessToken(const QString &data)
{
    qDebug()<<" void WebDavJob::parseAccessToken(const QString &data)"<<data;
    Q_EMIT authorizationDone(mPublicLocation, mServiceLocation, mUserName, mPassword);
    deleteLater();
}

void WebDavJob::parseUploadFile(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}

void WebDavJob::parseCreateFolder(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}

void WebDavJob::parseAccountInfo(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}

void WebDavJob::parseListFolder(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}


void WebDavJob::shareLink(const QString &root, const QString &path)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

void WebDavJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolder;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

QNetworkReply *WebDavJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
    return 0;
}

void WebDavJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));

    //TODO
    deleteLater();
}

void WebDavJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));

    //TODO
    deleteLater();
}

void WebDavJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));

    //TODO
    deleteLater();
}

void WebDavJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));

    //TODO
    deleteLater();
}

#include "moc_webdavjob.cpp"
