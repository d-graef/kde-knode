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

#ifndef SEARCHDUPLICATERESULTWIDGET_H
#define SEARCHDUPLICATERESULTWIDGET_H

#include <QWidget>
#include <Akonadi/Item>
#include "kaddressbook_export.h"
class QTreeWidget;
namespace KAddressBookGrantlee {
class GrantleeContactViewer;
}
namespace KABMergeContacts {
class KADDRESSBOOK_EXPORT SearchDuplicateResultWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchDuplicateResultWidget(QWidget *parent=0);
    ~SearchDuplicateResultWidget();
    void setContacts(const QList<Akonadi::Item::List> &lstItem);
private:
    QTreeWidget *mResult;
    KAddressBookGrantlee::GrantleeContactViewer *mContactViewer;
};
}

#endif // SEARCHDUPLICATERESULTWIDGET_H