/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef MERGECONTACTWIDGETLIST_H
#define MERGECONTACTWIDGETLIST_H

#include <QListWidget>
#include <AkonadiCore/Item>
#include <KContacts/Addressee>

namespace KABMergeContacts
{

class MergeContactWidgetListItem : public QListWidgetItem
{
public:
    MergeContactWidgetListItem(const Akonadi::Item &item, QListWidget *parent = Q_NULLPTR);
    Akonadi::Item item() const;

private:
    Akonadi::Item mItem;
};

class MergeContactWidgetList :  public QListWidget
{
    Q_OBJECT
public:
    explicit MergeContactWidgetList(QWidget *parent = Q_NULLPTR);
    ~MergeContactWidgetList();

    void fillListContact(const Akonadi::Item::List &items);
    Akonadi::Item::List listSelectedContacts() const;
    Akonadi::Item currentAkonadiItem() const;
private:
    QString itemName(const KContacts::Addressee &address) const;
};
}

#endif // MERGECONTACTWIDGETLIST_H
