/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef DROPBOXPLUGIN_H
#define DROPBOXPLUGIN_H

#include <storageservice/storageserviceplugin.h>

#include <QVariant>


namespace PimCommon
{
class DropBoxPlugin : public PimCommon::StorageServicePlugin
{
    Q_OBJECT
public:
    explicit DropBoxPlugin(QObject *parent = Q_NULLPTR, const QList<QVariant> & = QList<QVariant>());
    ~DropBoxPlugin();

    QString storageServiceName() const Q_DECL_OVERRIDE;

    StorageServicePlugin::Capabilities capabilities() const Q_DECL_OVERRIDE;
    QString description() const Q_DECL_OVERRIDE;
    QUrl serviceUrl() const Q_DECL_OVERRIDE;

    QString dropboxOauthConsumerKey() const;
    QString dropboxOauthSignature() const;
    QString dropboxRootPath() const;
    QRegExp disallowedSymbols() const Q_DECL_OVERRIDE;
    QString disallowedSymbolsStr() const Q_DECL_OVERRIDE;
    qlonglong maximumUploadFileSize() const Q_DECL_OVERRIDE;

};
}
#endif // DROPBOXPLUGIN_H
