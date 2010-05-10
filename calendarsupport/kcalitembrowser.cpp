/*
    Copyright (c) 2008 Bruno Virlet <bvirlet@kdemail.net>

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

#include "kcalitembrowser.h"

#include <akonadi/item.h>
#include <akonadi/itemfetchscope.h>

#include <kcal/incidence.h>
#include <kcal/incidenceformatter.h>

#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

using namespace Akonadi;

KCalItemBrowser::KCalItemBrowser( QWidget* parent )
    : ItemBrowser( parent ), d( 0 ), mAttachmentModel( 0 )
{
  fetchScope().fetchFullPayload();
}

KCalItemBrowser::~KCalItemBrowser()
{
}


QString KCalItemBrowser::itemPath() const
{
  return mItemPath;
}

void KCalItemBrowser::setItemPath(const QString& path)
{
  mItemPath = path;
}

QString KCalItemBrowser::itemToRichText( const Item& item )
{
    if ( !item.hasPayload<IncidencePtr>() ) {
        return QString();
    }

    mAttachmentModel->setItem( item );
    IncidencePtr incidence = item.payload<IncidencePtr>();
    QString headerData;
    if ( !mItemPath.isEmpty() )
    {
      headerData += "<table><tr><td>" + mItemPath + "</td></tr></table>";
    }
    return headerData + KCal::IncidenceFormatter::extensiveDisplayStr( incidence.get(), KDateTime::Spec() );
}

QAbstractItemModel* KCalItemBrowser::attachmentModel() const
{
  if ( !mAttachmentModel )
  {
    mAttachmentModel = new IncidenceAttachmentModel( const_cast<KCalItemBrowser *>( this ) );
  }
  return mAttachmentModel;
}

