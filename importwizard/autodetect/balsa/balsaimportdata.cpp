/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "balsa/balsaimportdata.h"
#include "balsa/balsasettings.h"
#include "balsa/balsaaddressbook.h"
#include "mailimporter/filterbalsa.h"
#include "mailimporter/filterinfo.h"
#include "MailCommon/FilterImporterBalsa"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include <KLocalizedString>

#include <QDir>

BalsaImportData::BalsaImportData(ImportWizard *parent)
    : AbstractImporter(parent)
{
    mPath = MailImporter::FilterBalsa::defaultSettingsPath();
}

BalsaImportData::~BalsaImportData()
{
}

bool BalsaImportData::foundMailer() const
{
    QDir directory(mPath);
    if (directory.exists()) {
        return true;
    }
    return false;
}

QString BalsaImportData::name() const
{
    return QStringLiteral("Balsa");
}

bool BalsaImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();

    MailImporter::FilterBalsa balsa;
    balsa.setFilterInfo(info);
    info->setStatusMessage(i18n("Import in progress"));
    QDir directory(balsa.localMailDirPath());
    if (directory.exists()) {
        balsa.importMails(directory.absolutePath());
    } else {
        balsa.import();
    }
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

bool BalsaImportData::importAddressBook()
{
    const QString addressbookFile(mPath + QStringLiteral("config"));
    BalsaAddressBook addressbook(addressbookFile, mImportWizard);
    return true;
}

bool BalsaImportData::importSettings()
{
    const QString settingFile(mPath + QStringLiteral("config"));
    BalsaSettings settings(settingFile, mImportWizard);
    return true;
}

bool BalsaImportData::importFilters()
{
    const QString filterPath = mPath + QStringLiteral("config");
    return addFilters(filterPath, MailCommon::FilterImporterExporter::BalsaFilter);
}

AbstractImporter::TypeSupportedOptions BalsaImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |= AbstractImporter::Mails;
    options |= AbstractImporter::AddressBooks;
    options |= AbstractImporter::Settings;
    options |= AbstractImporter::Filters;
    return options;
}
