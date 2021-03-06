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

#ifndef STORAGESERVICECONFIGUREDIALOG_H
#define STORAGESERVICECONFIGUREDIALOG_H

#include "PimCommon/StorageServiceConfigureDialog"

class QCheckBox;

class StorageServiceConfigureDialog : public PimCommon::StorageServiceConfigureDialog
{
    Q_OBJECT
public:
    explicit StorageServiceConfigureDialog(QWidget *parent = Q_NULLPTR);
    ~StorageServiceConfigureDialog();

    void writeSettings() Q_DECL_OVERRIDE;

private:
    void loadSettings() Q_DECL_OVERRIDE;
    QCheckBox *mCloseWallet;
};

#endif // STORAGESERVICECONFIGUREDIALOG_H
