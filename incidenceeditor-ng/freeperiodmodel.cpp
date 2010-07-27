/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include "freeperiodmodel.h"

#include <kcalcore/duration.h>

#include <KSystemTimeZones>
#include <KCalendarSystem>
#include <KGlobal>
#include <KDebug>

using namespace IncidenceEditorsNG;

FreePeriodModel::FreePeriodModel( QObject* parent ): QAbstractListModel( parent )
{
    qRegisterMetaType<KCalCore::Period>("KCalCore::Period");
}

IncidenceEditorsNG::FreePeriodModel::~FreePeriodModel()
{
}

QVariant FreePeriodModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() )
      return QVariant();

    if( index.row() > mPeriodList.size() - 1 )
      return QVariant();

    switch( role ) {
      case Qt::DisplayRole:
        return stringify( index.row() );
      case Qt::ToolTipRole:
        return tooltipify( index.row() );
      case FreePeriodModel::PeriodRole:
        return QVariant::fromValue( mPeriodList.at( index.row() ) );
      default:
        return QVariant();
    }
}

int FreePeriodModel::rowCount( const QModelIndex& parent ) const
{
  if( !parent.isValid() )
    return mPeriodList.size();
  return 0;
}

QVariant FreePeriodModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    return QAbstractItemModel::headerData( section, orientation, role );
}

void FreePeriodModel::slotNewFreePeriods( const KCalCore::Period::List& freePeriods )
{
    mPeriodList = splitPeriodsByDay( freePeriods );
    reset();
}

KCalCore::Period::List FreePeriodModel::splitPeriodsByDay( const KCalCore::Period::List& freePeriods )
{
    KCalCore::Period::List splitList;
    foreach( KCalCore::Period period, freePeriods ) {
        if( period.start().date() == period.end().date() )  {
            splitList << period; // period occurs on the same day
            continue;
        }

        while( period.start().date() != period.end().date() ) {
            const KDateTime midnight( period.start().date(), QTime( 23, 59, 59, 999 ), period.start().timeSpec() );
            KCalCore::Period firstPeriod( period.start(), midnight );
            KCalCore::Period secondPeriod( midnight.addMSecs( 1 ), period.end() );
            kDebug() << firstPeriod.start() << firstPeriod.end();
            kDebug() << secondPeriod.start() << secondPeriod.end();
            kDebug() << midnight << midnight.addMSecs( 1 );
            splitList << firstPeriod;
            period = secondPeriod;
        }
        splitList << period;
    }
    return splitList;
}


QString FreePeriodModel::stringify( int index ) const
{
    KCalCore::Period period = mPeriodList.at( index );
    const KCalendarSystem *calSys = KGlobal::locale()->calendar();

    const QDate startDate = period.start().date();
    const QString startTime = KGlobal::locale()->formatTime( period.start().time(), false, true );
    const QString endTime = KGlobal::locale()->formatTime( period.end().time(), false, true );
    const QString longMonthName = calSys->monthName( startDate );
    const QString dayofWeek = calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName );
    // TODO i18n, ping chusslove
    return i18nc( "A time period duration. KLocale is used to format the components. example: Monday, 12 June, 8:00am to 9:30am",
                  "%1, %2 %3, %4 to %5").arg( dayofWeek )
                                        .arg( startDate.day() )
                                        .arg( longMonthName )
                                        .arg( startTime )
                                        .arg( endTime );
}

QString FreePeriodModel::tooltipify( int index ) const
{
    KDateTime::Spec timeSpec = KSystemTimeZones::local();
    KCalCore::Period period = mPeriodList.at( index );
    unsigned long duration = period.duration().asSeconds() * 1000; // we want milliseconds
    QString toolTip = "<qt>";
    toolTip += "<b>" + i18nc( "@info:tooltip", "Free Period" ) + "</b>";
    toolTip += "<hr>";
    toolTip += "<i>" + i18nc( "@info:tooltip period start time", "Start:" ) + "</i>" + "&nbsp;";
    toolTip += KGlobal::locale()->formatDateTime( period.start().toTimeSpec( timeSpec ).dateTime() );
    toolTip += "<br>";
    toolTip += "<i>" + i18nc( "@info:tooltip period end time", "End:" ) + "</i>" + "&nbsp;";
    toolTip += KGlobal::locale()->formatDateTime( period.end().toTimeSpec( timeSpec ).dateTime() );
    toolTip += "<br>";
    toolTip += "<i>" + i18nc( "@info:tooltip period duration", "Duration:" ) + "</i>" + "&nbsp;";
    toolTip += KGlobal::locale()->prettyFormatDuration( duration );
    toolTip += "</qt>";
    return toolTip;
}

