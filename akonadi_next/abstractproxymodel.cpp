/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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

#include "abstractproxymodel.h"
#include "abstractitemmodel_p.h"

#include "entitytreemodel.h"

#include <KDE/KDebug>

class AbstractProxyModelPrivate : public AbstractItemModelPrivate
{
  public:
    AbstractProxyModelPrivate( QAbstractProxyModel *proxy )
      : AbstractItemModelPrivate( proxy ),
        mHeaderDataSet( 0 )
    {

    }

    int mHeaderDataSet;
};

AbstractProxyModel::AbstractProxyModel( QObject *parent )
  : QAbstractProxyModel( parent ),
    d_ptr( new AbstractProxyModelPrivate( this ) )
{
}

AbstractProxyModel::~AbstractProxyModel()
{
}

void AbstractProxyModel::beginMoveRows( const QModelIndex &srcParent, int start, int end,
                                        const QModelIndex &destinationParent, int destinationRow )
{
  Q_D(AbstractProxyModel);
  return d->beginMoveRows( srcParent, start, end, destinationParent, destinationRow );
}

void AbstractProxyModel::endMoveRows()
{
  Q_D(AbstractProxyModel);
  d->endMoveRows();
}


void AbstractProxyModel::beginResetModel()
{
  Q_D(AbstractProxyModel);
  d->beginResetModel();
}

void AbstractProxyModel::endResetModel()
{
  Q_D(AbstractProxyModel);
  d->endResetModel();
}

void AbstractProxyModel::beginChangeChildOrder( const QModelIndex &index )
{
  Q_D(AbstractProxyModel);
  d->beginChangeChildOrder( index );
}

void AbstractProxyModel::endChangeChildOrder()
{
  Q_D(AbstractProxyModel);
  d->endChangeChildOrder();
}

void AbstractProxyModel::setHeaderSet( int set )
{
  Q_D(AbstractProxyModel);
  d->mHeaderDataSet = set;
}

int AbstractProxyModel::headerSet() const
{
  Q_D(const AbstractProxyModel);
  return d->mHeaderDataSet;
}

QVariant AbstractProxyModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_D(const AbstractProxyModel);
  role += ( Akonadi::EntityTreeModel::TerminalUserRole * d->mHeaderDataSet );
  return sourceModel()->headerData( section, orientation, role );
}

bool AbstractProxyModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column,
                                       const QModelIndex & parent )
{
  Q_ASSERT( sourceModel() );

  const QModelIndex sourceParent = mapToSource( parent );
  return sourceModel()->dropMimeData( data, action, row, column, sourceParent );
}

QMimeData* AbstractProxyModel::mimeData( const QModelIndexList & indexes ) const
{
  Q_ASSERT( sourceModel() );

  QModelIndexList sourceIndexes;
  foreach ( const QModelIndex& index, indexes )
    sourceIndexes << mapToSource( index );

  return sourceModel()->mimeData( sourceIndexes );
}

QStringList AbstractProxyModel::mimeTypes() const
{
  Q_ASSERT( sourceModel() );
  return sourceModel()->mimeTypes();
}

QModelIndexList AbstractProxyModel::match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const
{
  QModelIndexList proxyList;
  QModelIndexList sourceList = sourceModel()->match(mapToSource(start), role, value, hits, flags);
  QModelIndexList::const_iterator it;
  const QModelIndexList::const_iterator begin = sourceList.constBegin();
  const QModelIndexList::const_iterator end = sourceList.constEnd();
  QModelIndex proxyIndex;
  for (it = begin; it != end; ++it)
  {
    proxyIndex = mapFromSource(*it);

    if (!proxyIndex->isValid())
      continue;

    proxyList << proxyIndex;
  }
  return proxyList;
}

