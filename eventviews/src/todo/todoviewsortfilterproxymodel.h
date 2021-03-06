/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef CALENDARVIEWS_TODOVIEWSORTFILTERPROXYMODEL_H
#define CALENDARVIEWS_TODOVIEWSORTFILTERPROXYMODEL_H

#include "prefs.h"

#include <QSortFilterProxyModel>
#include <QStringList>

class TodoViewSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TodoViewSortFilterProxyModel(const EventViews::PrefsPtr &prefs,
                                          QObject *parent = Q_NULLPTR);

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) Q_DECL_OVERRIDE;

    const QStringList &categories() const
    {
        return mCategories;
    }

    const QStringList &priorities() const
    {
        return mPriorities;
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const Q_DECL_OVERRIDE;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void setCategoryFilter(const QStringList &categories);
    void setPriorityFilter(const QStringList &priorities);

private:
    int compareStartDates(const QModelIndex &left, const QModelIndex &right) const;
    int compareDueDates(const QModelIndex &left, const QModelIndex &right) const;
    int comparePriorities(const QModelIndex &left, const QModelIndex &right) const;
    int compareCompletion(const QModelIndex &left, const QModelIndex &right) const;
    QStringList mCategories;
    QStringList mPriorities;
    Qt::SortOrder mSortOrder;
    EventViews::PrefsPtr mPreferences;
};

#endif
