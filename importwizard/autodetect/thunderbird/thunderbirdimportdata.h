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
#ifndef THUNDERBIRDIMPORTDATA_H
#define THUNDERBIRDIMPORTDATA_H

#include "abstractimporter.h"
class ImportWizard;

class ThunderbirdImportData : public AbstractImporter
{
public:
    explicit ThunderbirdImportData(ImportWizard *parent);
    ~ThunderbirdImportData();

    TypeSupportedOptions supportedOption() Q_DECL_OVERRIDE;
    bool foundMailer() const Q_DECL_OVERRIDE;

    bool importSettings() Q_DECL_OVERRIDE;
    bool importMails() Q_DECL_OVERRIDE;
    bool importFilters() Q_DECL_OVERRIDE;
    bool importAddressBook() Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
private:
    QString defaultProfile();

    QString mDefaultProfile;
};

#endif /* THUNDERBIRDIMPORTDATA_H */

