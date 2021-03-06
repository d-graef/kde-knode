/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef STORAGESERVICECONFIGUREWIDGET_H
#define STORAGESERVICECONFIGUREWIDGET_H

#include <QWidget>
#include "pimcommon_export.h"
class KUrlRequester;
namespace PimCommon
{
class StorageServiceSettingsWidget;
class PIMCOMMON_EXPORT StorageServiceConfigureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StorageServiceConfigureWidget(QWidget *parent = Q_NULLPTR);
    ~StorageServiceConfigureWidget();

    virtual void loadSettings();
    virtual void writeSettings();
    PimCommon::StorageServiceSettingsWidget *storageServiceSettingsWidget() const;
    KUrlRequester *downloadFolder() const;

Q_SIGNALS:
    void changed();
    void serviceRemoved(const QString &);

protected:
    PimCommon::StorageServiceSettingsWidget *mStorageSettings;
    KUrlRequester *mDownloadFolder;
};
}

#endif // STORAGESERVICECONFIGUREWIDGET_H
