/*
  This file is part of the KOrganizer interfaces.

  Copyright (c) 1999,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004           Reinhold Kainhofer   <reinhold@kainhofer.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "baseview.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/calendarsearch.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/utils.h>

#include <akonadi_next/kcheckableproxymodel.h>
#include <akonadi_next/kcolumnfilterproxymodel.h>

#include <Akonadi/EntityTreeView>

#include <KConfigGroup>
#include <KRandom>

#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>


using namespace CalendarSupport;
using namespace Future;
using namespace KOrg;

CollectionSelection* BaseView::sGlobalCollectionSelection = 0;

void BaseView::setGlobalCollectionSelection( CollectionSelection* s )
{
  sGlobalCollectionSelection = s;
}

CollectionSelection* BaseView::globalCollectionSelection()
{
  return sGlobalCollectionSelection;
}

class BaseView::Private
{
  BaseView *const q;

  public:
    explicit Private( BaseView* qq )
      : q( qq ),
        mChanges( EventViews::EventView::IncidencesAdded |
                  EventViews::EventView::DatesChanged ),
        calendar( 0 ),
        customCollectionSelection( 0 ),
        collectionSelectionModel( 0 )
    {
      QByteArray cname = q->metaObject()->className();
      cname.replace( ":", "_" );
      identifier = cname + "_" + KRandom::randomString( 8 ).toLatin1();
      calendarSearch = new CalendarSearch( q );
      connect( calendarSearch->model(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
               q, SLOT( rowsInserted( const QModelIndex&, int, int ) ) );
      connect( calendarSearch->model(), SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
               q, SLOT( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ) );
      connect( calendarSearch->model(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
               q, SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
      connect( calendarSearch->model(), SIGNAL( modelReset() ), q, SLOT( calendarReset() ) );
    }

    ~Private()
    {
      delete collectionSelectionModel;
    }

    EventViews::EventView::Changes mChanges;
    Calendar *calendar;
    CalendarSearch *calendarSearch;
    CollectionSelection *customCollectionSelection;
    KCheckableProxyModel* collectionSelectionModel;
    QByteArray identifier;
    KDateTime startDateTime;
    KDateTime endDateTime;
    KDateTime actualStartDateTime;
    KDateTime actualEndDateTime;
    void setUpModels();
    void reconnectCollectionSelection();
};

void BaseView::Private::setUpModels()
{
  delete customCollectionSelection;
  customCollectionSelection = 0;
  if ( collectionSelectionModel ) {
    customCollectionSelection = new CollectionSelection( collectionSelectionModel->selectionModel() );
    calendarSearch->setSelectionModel( collectionSelectionModel->selectionModel() );

  } else {
    calendarSearch->setSelectionModel( globalCollectionSelection()->model() );
  }
#if 0
  QDialog* dlg = new QDialog( q );
  dlg->setModal( false );
  QVBoxLayout* layout = new QVBoxLayout( dlg );
  EntityTreeView* testview = new EntityTreeView( dlg );
  layout->addWidget( testview );
  testview->setModel( calendarSearch->model() );
  dlg->show();
#endif
  reconnectCollectionSelection();
}

void BaseView::Private::reconnectCollectionSelection()
{
  if ( q->globalCollectionSelection() )
    q->globalCollectionSelection()->disconnect( q );

  if ( customCollectionSelection )
    customCollectionSelection->disconnect( q );

  QObject::connect( q->collectionSelection(), SIGNAL(selectionChanged(Akonadi::Collection::List,Akonadi::Collection::List)), q, SLOT(collectionSelectionChanged()) );
}


BaseView::BaseView( QWidget *parent )
  : QWidget( parent ), mChanger( 0 ), d( new Private( this ) )
{
}

BaseView::~BaseView()
{
  delete d;
}

void BaseView::setCalendar( Calendar *cal )
{
  if ( d->calendar == cal )
    return;
  d->calendar = cal;
  if ( cal && d->collectionSelectionModel )
    d->collectionSelectionModel->setSourceModel( cal->model() );
}

CalPrinterBase::PrintType BaseView::printType() const
{
  return CalPrinterBase::Month;
}

Calendar *BaseView::calendar()
{
  return d->calendar;
}

CalendarSearch* BaseView::calendarSearch() const
{
  return d->calendarSearch;
}

bool BaseView::isEventView()
{
  return false;
}

void BaseView::dayPassed( const QDate & )
{
  updateView();
}

void BaseView::setIncidenceChanger( IncidenceChanger *changer )
{
  mChanger = changer;
}

void BaseView::flushView()
{}

BaseView* BaseView::viewAt( const QPoint & )
{
  return this;
}

void BaseView::updateConfig()
{
}

bool BaseView::hasConfigurationDialog() const
{
  return false;
}

void BaseView::setDateRange( const KDateTime& start, const KDateTime& end )
{
#if 0 //AKONADI_PORT the old code called showDates() (below), which triggers a repaint, which the old code relies on
  if ( d->startDateTime == start && d->endDateTime == end )
    return;
#endif
  d->startDateTime = start;
  d->endDateTime = end;
  showDates( start.date(), end.date() );
  const QPair<KDateTime,KDateTime> adjusted = actualDateRange( start, end );
  d->actualStartDateTime = adjusted.first;
  d->actualEndDateTime = adjusted.second;
  d->calendarSearch->setStartDate( d->actualStartDateTime );
  d->calendarSearch->setEndDate( d->actualEndDateTime );
}

KDateTime BaseView::startDateTime() const
{
  return d->startDateTime;
}

KDateTime BaseView::endDateTime() const
{
  return d->endDateTime;
}

KDateTime BaseView::actualStartDateTime() const
{
  return d->actualStartDateTime;
}

KDateTime BaseView::actualEndDateTime() const
{
  return d->actualEndDateTime;
}

void BaseView::showConfigurationDialog( QWidget* )
{
}

QByteArray BaseView::identifier() const
{
  return d->identifier;
}

void BaseView::setIdentifier( const QByteArray& identifier )
{
  d->identifier = identifier;
}

void BaseView::restoreConfig( const KConfigGroup &configGroup )
{
  const bool useCustom = configGroup.readEntry( "UseCustomCollectionSelection", false );
  if ( !d->collectionSelectionModel && !useCustom ) {
    delete d->collectionSelectionModel;
    d->collectionSelectionModel = 0;
    d->setUpModels();
  } else if ( useCustom ) {

    if ( !d->collectionSelectionModel ) {
      // Sort the calendar model on calendar name
      QSortFilterProxyModel *sortProxy = new QSortFilterProxyModel( this );
      sortProxy->setDynamicSortFilter( true );
      sortProxy->setSortCaseSensitivity( Qt::CaseInsensitive );

      if ( d->calendar ) {
        sortProxy->setSourceModel( d->calendar->treeModel() );
      }

      // Only show the first column.
      KColumnFilterProxyModel *columnFilterProxy = new KColumnFilterProxyModel( this );
      columnFilterProxy->setVisibleColumn( CalendarSupport::CalendarModel::CollectionTitle );
      columnFilterProxy->setSourceModel( sortProxy );

      // Make the calendar model checkable.
      d->collectionSelectionModel = new KCheckableProxyModel( this );
      d->collectionSelectionModel->setSourceModel( columnFilterProxy );

      d->setUpModels();
    }
  }

  doRestoreConfig( configGroup );
}

void BaseView::saveConfig( KConfigGroup &configGroup )
{
  configGroup.writeEntry( "UseCustomCollectionSelection", d->collectionSelectionModel != 0 );
  doSaveConfig( configGroup );
}

void BaseView::doRestoreConfig( const KConfigGroup & )
{
}

void BaseView::doSaveConfig( KConfigGroup & )
{
}

CollectionSelection* BaseView::collectionSelection() const
{
  return d->customCollectionSelection ? d->customCollectionSelection : globalCollectionSelection();
}

void BaseView::setCustomCollectionSelectionProxyModel( KCheckableProxyModel* model )
{
  if ( d->collectionSelectionModel == model )
    return;

  delete d->collectionSelectionModel;
  d->collectionSelectionModel = model;
  d->setUpModels();
}

void BaseView::collectionSelectionChanged()
{

}

KCheckableProxyModel *BaseView::customCollectionSelectionProxyModel() const
{
  return d->collectionSelectionModel;
}

KCheckableProxyModel *BaseView::takeCustomCollectionSelectionProxyModel()
{
  KCheckableProxyModel* m = d->collectionSelectionModel;
  d->collectionSelectionModel = 0;
  d->setUpModels();
  return m;
}

CollectionSelection *BaseView::customCollectionSelection() const
{
  return d->customCollectionSelection;
}

void BaseView::clearSelection()
{
}

bool BaseView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  Q_UNUSED( startDt );
  Q_UNUSED( endDt );
  Q_UNUSED( allDay );
  return false;
}

void BaseView::getHighlightMode( bool &highlightEvents,
                                 bool &highlightTodos,
                                 bool &highlightJournals )
{
  highlightEvents   = true;
  highlightTodos    = false;
  highlightJournals = false;
}

void BaseView::handleBackendError( const QString &errorString )
{
  kError() << errorString;
}

void BaseView::backendErrorOccurred()
{
  handleBackendError( d->calendarSearch->errorString() );
}

bool BaseView::usesFullWindow()
{
  return false;
}

bool BaseView::supportsZoom()
{
  return false;
}

bool BaseView::supportsDateRangeSelection()
{
  return true;
}

void BaseView::incidencesAdded( const Akonadi::Item::List & )
{
}

void BaseView::incidencesAboutToBeRemoved( const Akonadi::Item::List & )
{
}

void BaseView::incidencesChanged( const Akonadi::Item::List& )
{
}

void BaseView::calendarReset()
{
}

QPair<KDateTime,KDateTime> BaseView::actualDateRange( const KDateTime& start, const KDateTime& end ) const
{
  return qMakePair( start, end );
}

void BaseView::dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
  Q_ASSERT( topLeft.parent() == bottomRight.parent() );

  incidencesChanged( itemsFromModel( d->calendarSearch->model(), topLeft.parent(),
                     topLeft.row(), bottomRight.row() ) );
}

void BaseView::rowsInserted( const QModelIndex& parent, int start, int end )
{
  incidencesAdded( itemsFromModel( d->calendarSearch->model(), parent, start, end ) );
}

void BaseView::rowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
  incidencesAboutToBeRemoved( itemsFromModel( d->calendarSearch->model(), parent, start, end ) );
}

void BaseView::setFilter( KCalCore::CalFilter *filter )
{
  calendarSearch()->setFilter( filter );
}

void BaseView::setChanges( EventViews::EventView::Changes changes )
{
  d->mChanges = changes;
}

EventViews::EventView::Changes BaseView::changes() const
{
  return d->mChanges;
}

#include "baseview.moc"
