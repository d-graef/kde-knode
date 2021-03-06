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

#ifndef INVALIDFILTERLISTITEMDELEGATE_H
#define INVALIDFILTERLISTITEMDELEGATE_H

#include <KWidgetItemDelegate>

namespace MailCommon
{
class InvalidFilterListItemDelegate : public KWidgetItemDelegate
{
    Q_OBJECT
public:
    explicit InvalidFilterListItemDelegate(QAbstractItemView *itemView, QObject *parent = Q_NULLPTR);
    virtual ~InvalidFilterListItemDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QList<QWidget *> createItemWidgets(const QModelIndex &) const Q_DECL_OVERRIDE;

    void updateItemWidgets(const QList<QWidget *> widgets,
                           const QStyleOptionViewItem &option,
                           const QPersistentModelIndex &index) const Q_DECL_OVERRIDE;
private Q_SLOTS:
    void slotShowDetails();
Q_SIGNALS:
    void showDetails(const QString &details);
};
}
#endif // INVALIDFILTERLISTITEMDELEGATE_H
