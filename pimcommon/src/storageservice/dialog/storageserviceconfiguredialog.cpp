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

#include "storageserviceconfiguredialog.h"
#include "storageservice/widgets/storageserviceconfigurewidget.h"
#include "storageservice/settings/storageservicesettingswidget.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace PimCommon;

StorageServiceConfigureDialog::StorageServiceConfigureDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Configure"));
    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);
    mMainLayout = new QVBoxLayout;

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &StorageServiceConfigureDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &StorageServiceConfigureDialog::reject);
    mStorageServiceConfigureWidget = new PimCommon::StorageServiceConfigureWidget;

    connect(mStorageServiceConfigureWidget, &PimCommon::StorageServiceConfigureWidget::serviceRemoved, this, &StorageServiceConfigureDialog::serviceRemoved);
    mMainLayout->addWidget(mStorageServiceConfigureWidget);

    topLayout->addLayout(mMainLayout);
    topLayout->addWidget(buttonBox);
    readConfig();
}

StorageServiceConfigureDialog::~StorageServiceConfigureDialog()
{
    writeConfig();
}

void StorageServiceConfigureDialog::addExtraWidget(QWidget *w)
{
    mMainLayout->addWidget(w);
}

void StorageServiceConfigureDialog::writeSettings()
{
    //Reimplement it
}

void StorageServiceConfigureDialog::loadSettings()
{
    //Reimplement it
}

QMap<QString, PimCommon::StorageServiceAbstract *> StorageServiceConfigureDialog::listService() const
{
    return mStorageServiceConfigureWidget->storageServiceSettingsWidget()->listService();
}

void StorageServiceConfigureDialog::setListService(const QMap<QString, PimCommon::StorageServiceAbstract *> &lst)
{
    mStorageServiceConfigureWidget->storageServiceSettingsWidget()->setListService(lst);
}

void StorageServiceConfigureDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "StorageServiceConfigureDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void StorageServiceConfigureDialog::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group(QStringLiteral("StorageServiceConfigureDialog"));
    group.writeEntry("Size", size());
}

