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

#ifndef UNITCOMBOBOX_H
#define UNITCOMBOBOX_H

#include <QComboBox>
#include "archivemailinfo.h"

class UnitComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit UnitComboBox(QWidget *parent = Q_NULLPTR);
    ~UnitComboBox();
    ArchiveMailInfo::ArchiveUnit unit() const;
    void setUnit(ArchiveMailInfo::ArchiveUnit unit);
};

#endif // UNITCOMBOBOX_H
