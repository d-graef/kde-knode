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

#ifndef CALENDARVIEWS_TODODELEGATES_H
#define CALENDARVIEWS_TODODELEGATES_H

#include <Akonadi/Calendar/ETMCalendar>
#include <QStyledItemDelegate>

class QPainter;
class QSize;
class QStyleOptionViewItem;
class QTextDocument;

/**
  This delegate is responsible for displaying progress bars for the completion
  status of indivitual todos. It also provides a slider to change the completion
  status of the todo when in editing mode.

  @author Thomas Thrainer
*/
class TodoCompleteDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TodoCompleteDelegate(QObject *parent = Q_NULLPTR);

    ~TodoCompleteDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    void initStyleOptionProgressBar(QStyleOptionProgressBar *option,
                                    const QModelIndex &index) const;
};

class TodoCompleteSlider : public QSlider
{
    Q_OBJECT

public:
    explicit TodoCompleteSlider(QWidget *parent);

private Q_SLOTS:
    void updateTip(int value);
};

/**
  This delegate is responsible for displaying the priority of todos.
  It also provides a combo box to change the priority of the todo
  when in editing mode.

  @author Thomas Thrainer
 */
class TodoPriorityDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TodoPriorityDelegate(QObject *parent = Q_NULLPTR);

    ~TodoPriorityDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const Q_DECL_OVERRIDE;
};

/**
  This delegate is responsible for displaying the due date of todos.
  It also provides a combo box to change the due date of the todo
  when in editing mode.

  @author Thomas Thrainer
 */
class TodoDueDateDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TodoDueDateDelegate(QObject *parent = Q_NULLPTR);

    ~TodoDueDateDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const Q_DECL_OVERRIDE;
};

/**
  This delegate is responsible for displaying the categories of todos.
  It also provides a combo box to change the categories of the todo
  when in editing mode.

  @author Thomas Thrainer
 */
class TodoCategoriesDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TodoCategoriesDelegate(QObject *parent = Q_NULLPTR);

    ~TodoCategoriesDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const Q_DECL_OVERRIDE;

    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal);
private:
    Akonadi::ETMCalendar::Ptr mCalendar;
};

/**
  This delegate is responsible for displaying possible rich text elements
  of a todo. That's the summary and the description.

  @author Thomas Thrainer
 */
class TodoRichTextDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TodoRichTextDelegate(QObject *parent = Q_NULLPTR);

    ~TodoRichTextDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;
private:
    QTextDocument *m_textDoc;
};

#endif
