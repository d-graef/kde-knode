/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef FOLLOWUPREMINDERINFOWIDGET_H
#define FOLLOWUPREMINDERINFOWIDGET_H

#include <QWidget>
#include <KConfigGroup>
class QTreeWidget;
class FollowUpReminderInfo;
class FollowUpReminderInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FollowUpReminderInfoWidget(QWidget *parent=0);
    ~FollowUpReminderInfoWidget();

    void restoreTreeWidgetHeader(const QByteArray &data);
    void saveTreeWidgetHeader(KConfigGroup &group);

    void setInfo(const QList<FollowUpReminderInfo *> &info);
private slots:
    void customContextMenuRequested(const QPoint &pos);
    void slotRemoveItem();

private:
    enum FollowUpReminderColumn {
        To = 0,
        Subject,
        MessageId
    };
    QTreeWidget *mTreeWidget;
};

#endif // FOLLOWUPREMINDERINFOWIDGET_H