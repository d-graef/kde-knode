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

#ifndef LINEEDITWITHCOMPLETER_H
#define LINEEDITWITHCOMPLETER_H

#include <KLineEdit>
#include "pimcommon_export.h"

namespace PimCommon
{
class PIMCOMMON_EXPORT LineEditWithCompleter : public KLineEdit
{
    Q_OBJECT
public:
    explicit LineEditWithCompleter(QWidget *parent = Q_NULLPTR);
    ~LineEditWithCompleter();

protected:
    void contextMenuEvent(QContextMenuEvent *e) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotClearHistory();
};
}
#endif // LINEEDITWITHCOMPLETER_H
