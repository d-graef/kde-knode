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

#ifndef MERGECONTACTSHOWRESULTTABWIDGET_H
#define MERGECONTACTSHOWRESULTTABWIDGET_H

#include <QTabWidget>
#include <AkonadiCore/item.h>

namespace KABMergeContacts
{

class MergeContactShowResultTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit MergeContactShowResultTabWidget(QWidget *parent = Q_NULLPTR);
    ~MergeContactShowResultTabWidget();

    void updateTabWidget();

    bool tabBarVisible() const;

    void showMergedContacts(const Akonadi::Item::List &lstItem);
    void addMergedContact(const Akonadi::Item &item, bool updateTab = false);
};
}
#endif // MERGECONTACTSHOWRESULTTABWIDGET_H
