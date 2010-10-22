/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Sérgio Martins <iamsergio@gmail.com>

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
#include "incidencechanger2.h"
#include "incidencechanger2_p.h"
#include "calendar.h"

#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemDeleteJob>

#include <KJob>

using namespace Akonadi;
using namespace KCalCore;
using namespace CalendarSupport;


IncidenceChanger2::Private::Private( IncidenceChanger2 *qq ) : q( qq )
{
}

IncidenceChanger2::Private::~Private()
{
}

void IncidenceChanger2::Private::handleCreateJobResult( KJob *job )
{
  Q_UNUSED( job );
}

void IncidenceChanger2::Private::handleDeleteJobResult( KJob *job )
{
  Q_UNUSED( job );
}

void IncidenceChanger2::Private::handleModifyJobResult( KJob *job )
{
  Q_UNUSED( job );
}

IncidenceChanger2::IncidenceChanger2( CalendarSupport::Calendar *calendar ) : QObject(),
                                                                              d( new Private( this ) )
{
  Q_UNUSED( calendar );
  d->mLatestOperationId = 0;
}

IncidenceChanger2::~IncidenceChanger2()
{
  delete d;
}

int IncidenceChanger2::createIncidence( const Incidence::Ptr &incidence,
                                        const Collection &collection,
                                        uint atomicOperationId,
                                        QWidget *parent )
{
  Q_UNUSED( parent );
  Q_UNUSED( atomicOperationId );

  Q_ASSERT_X( incidence, "createIncidence()", "Invalid incidences not allowed" );


  Item item;
  item.setPayload<Incidence::Ptr>( incidence );
  item.setMimeType( incidence->mimeType() );
  ItemCreateJob *createJob = new ItemCreateJob( item, collection );

  // TODO: remove sync exec calls from Akonadi::Groupware
  connect( createJob, SIGNAL(result(KJob*)),
           d, SLOT(handleCreateJobResult(KJob*)), Qt::QueuedConnection );

  return ++d->mLatestOperationId;
}

int IncidenceChanger2::deleteIncidence( const Item &item,
                                        uint atomicOperationId,
                                        QWidget *parent )
{
  // Too harsh?
  Q_UNUSED( atomicOperationId );
  Q_UNUSED( parent );
  Q_ASSERT_X( item.isValid(), "deleteIncidence()", "Invalid items not allowed" );

  ItemDeleteJob *deleteJob = new ItemDeleteJob( item );
  connect( deleteJob, SIGNAL(result(KJob *)),
           d, SLOT(handleDeleteJobResult(KJob *)) );

  return ++d->mLatestOperationId;
}

int IncidenceChanger2::modifyIncidence( const Item &changedItem,
                                        const Item &originalItem,
                                        uint atomicOperationId,
                                        QWidget *parent )
{
  Q_UNUSED( parent );
  Q_UNUSED( atomicOperationId );
  Q_UNUSED( originalItem );

  ItemModifyJob *modifyJob = new ItemModifyJob( changedItem );
  connect( modifyJob, SIGNAL(result( KJob *)),
           d, SLOT(handleModifyJobResult(KJob *)) );

  return ++d->mLatestOperationId;
}

uint IncidenceChanger2::startAtomicOperation()
{
  static uint latestAtomicOperationId = 0;
  return ++latestAtomicOperationId;
}

void IncidenceChanger2::endAtomicOperation( uint atomicOperationId )
{
  Q_UNUSED( atomicOperationId );
  //d->mOperationStatus.remove( atomicOperationId );
}

