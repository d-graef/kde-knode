/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef AKONADI_CONTACT_CONTACTVIEWITEM_H
#define AKONADI_CONTACT_CONTACTVIEWITEM_H

#include <QtCore/QTimer>
#include <QtDeclarative/QDeclarativeItem>

namespace Akonadi {

class ContactViewer;

namespace Contact {

class ContactViewItem : public QDeclarativeItem
{
  Q_OBJECT
  Q_PROPERTY( int contactItemId READ contactItemId WRITE setContactItemId )

  public:
    explicit ContactViewItem( QDeclarativeItem *parent = 0 );
    ~ContactViewItem();

    qint64 contactItemId() const;
    void setContactItemId( qint64 id );

  protected:
    void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry);

  private:
    ContactViewer *m_viewer;
    QGraphicsProxyWidget *m_proxy;
};

}
}

#endif
