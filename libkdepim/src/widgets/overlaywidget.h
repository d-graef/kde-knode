/* -*- c++ -*-
 * overlaywidget.h
 *
 *  Copyright (c) 2004 David Faure <faure@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */
#ifndef KDEPIM_OVERLAYWIDGET_H
#define KDEPIM_OVERLAYWIDGET_H

#include "kdepim_export.h"

#include <QHBoxLayout>
#include <QFrame>

class QResizeEvent;
class QEvent;

namespace KPIM
{

/**
 * This is a widget that can align itself with another one, without using a layout,
 * so that it can actually be on top of other widgets.
 * Currently the only supported type of alignment is "right aligned, on top of the other widget".
 *
 */
class OverlayWidgetPrivate;
class KDEPIM_EXPORT OverlayWidget : public QFrame
{
    Q_OBJECT

public:
    OverlayWidget(QWidget *alignWidget, QWidget *parent);
    ~OverlayWidget();

    QWidget *alignWidget() const;
    void setAlignWidget(QWidget *alignWidget);

protected:
    void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;

private:
    void reposition();

private:
    OverlayWidgetPrivate *const d;
};

} // namespace

#endif /* OVERLAYWIDGET_H */

