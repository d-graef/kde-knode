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

#ifndef RESULTDUPLICATETREEWIDGET_H
#define RESULTDUPLICATETREEWIDGET_H

#include <QTreeWidget>
#include <AkonadiCore/Item>
#include "kaddressbook_export.h"
namespace KContacts
{
class Addressee;
}
namespace KABMergeContacts
{

class KADDRESSBOOK_EXPORT ResultDuplicateTreeWidgetItem : public QTreeWidgetItem
{
public:
    explicit ResultDuplicateTreeWidgetItem(QTreeWidget *parent = Q_NULLPTR);
    ~ResultDuplicateTreeWidgetItem();

    Akonadi::Item item() const;
    void setItem(const Akonadi::Item &item);

private:
    void setDisplayName();
    QString contactName(const KContacts::Addressee &address);
    Akonadi::Item mItem;
};

class KADDRESSBOOK_EXPORT ResultDuplicateTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit ResultDuplicateTreeWidget(QWidget *parent = Q_NULLPTR);
    ~ResultDuplicateTreeWidget();
    void setContacts(const QVector<Akonadi::Item::List> &lstItem);

    QVector<Akonadi::Item::List> selectedContactsToMerge() const;

Q_SIGNALS:
    void showContactPreview(const Akonadi::Item &item);
private Q_SLOTS:
    void slotItemActivated(QTreeWidgetItem *item, int column);
    void slotItemChanged(QTreeWidgetItem *item, int column);
private:
    void changeState(QTreeWidgetItem *item, bool b);
};
}

#endif // RESULTDUPLICATETREEWIDGET_H
