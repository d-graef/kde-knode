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

#ifndef FILTERIMPORTERGMAIL_H
#define FILTERIMPORTERGMAIL_H

#include "filter/filterimporter/filterimporterabstract.h"
class QFile;
namespace MailCommon
{

class MAILCOMMON_EXPORT FilterImporterGmail : public FilterImporterAbstract
{
public:
    FilterImporterGmail(QFile *file);
    ~FilterImporterGmail();
    static QString defaultFiltersSettingsPath();
private:
    QString createUniqFilterName();
    void parseFilters(const QDomElement &e);
    int mFilterCount;
};
}

#endif // FILTERIMPORTERGMAIL_H
