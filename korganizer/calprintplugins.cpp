/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown
    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qpainter.h>
#include <qdatetimeedit.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kprinter.h>
#include <kconfig.h>
#include <kcalendarsystem.h>

#include <libkcal/todo.h>
#include <libkcal/calendar.h>

#include <libkdepim/kdateedit.h>

#include "koprefs.h"
#include "koglobals.h"
#include "calprintplugins.h"
#ifndef KORG_NOPRINTER

#include "calprintplugins.moc"

#include "calprintdayconfig_base.h"
#include "calprintweekconfig_base.h"
#include "calprintmonthconfig_base.h"
#include "calprinttodoconfig_base.h"


/**************************************************************
 *           Print Day
 **************************************************************/

CalPrintDay::CalPrintDay( KPrinter *printer, Calendar *cal, KConfig *cfg )
  : CalPrintBase( printer, cal, cfg )
{
}

CalPrintDay::~CalPrintDay()
{
}

QWidget *CalPrintDay::configWidget( QWidget *w )
{
  mConfigWidget = new CalPrintDayConfig_Base( w );
  setSettingsWidget();
  return mConfigWidget;
}

void CalPrintDay::readSettingsWidget()
{
  CalPrintDayConfig_Base *cfg =
      dynamic_cast<CalPrintDayConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mStartTime = cfg->mFromTime->time();
    mEndTime = cfg->mToTime->time();
    mIncludeAllEvents = cfg->mIncludeAllEvents->isChecked();

    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mUseColors = cfg->mColors->isChecked();
  }
}

void CalPrintDay::setSettingsWidget()
{
  CalPrintDayConfig_Base *cfg =
      dynamic_cast<CalPrintDayConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mFromTime->setTime( mStartTime );
    cfg->mToTime->setTime( mEndTime );
    cfg->mIncludeAllEvents->setChecked( mIncludeAllEvents );

    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mColors->setChecked( mUseColors );
  }
}

void CalPrintDay::loadConfig()
{
  if ( mConfig ) {
    QDate dt;
    QTime tm1( KOPrefs::instance()->mDayBegins.time() );
    QDateTime startTm( dt, tm1  );
    QDateTime endTm( dt, tm1.addSecs( 12 * 60 * 60 ) );
    mStartTime = mConfig->readDateTimeEntry( "Start time", &startTm ).time();
    mEndTime = mConfig->readDateTimeEntry( "End time", &endTm ).time();
    mIncludeTodos = mConfig->readBoolEntry( "Include todos", false );
    mIncludeAllEvents = mConfig->readBoolEntry( "Include all events", false );
  }
  setSettingsWidget();
}

void CalPrintDay::saveConfig()
{
  kdDebug() << "CalPrintDay::saveConfig()" << endl;

  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Start time", QDateTime( QDate(), mStartTime ) );
    mConfig->writeEntry( "End time", QDateTime( QDate(), mEndTime ) );
    mConfig->writeEntry( "Include todos", mIncludeTodos );
    mConfig->writeEntry( "Include all events", mIncludeAllEvents );
  }
}

void CalPrintDay::setDateRange( const QDate& from, const QDate& to )
{
  CalPrintBase::setDateRange( from, to );
  CalPrintDayConfig_Base *cfg =
      dynamic_cast<CalPrintDayConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintDay::print( QPainter &p, int width, int height )
{
  QDate curDay( mFromDate );

  do {
    int x = 0;
    int y = 0;
    int currHeight=( height - y ) / 20;
    QTime curStartTime( mStartTime );
    QTime curEndTime( mEndTime );
    if ( curStartTime.secsTo( curEndTime ) <= 3600 ) {
      if ( curStartTime.hour() == 0 ) {
        curStartTime = QTime( 0, 0, 0 );
        curEndTime = curStartTime.addSecs( 3600 );
      } else if ( curEndTime.hour() == 23 ) {
        curEndTime=QTime( 23, 59, 59 );
        if ( curStartTime > QTime( 23, 0, 0 ) ) {
          curStartTime = QTime( 23, 0, 0 );
        }
      } else {
        curStartTime = curStartTime.addSecs( -1200 );
      }
      curEndTime = curEndTime.addSecs( 1200 );
    }

    KLocale *local = KGlobal::locale();
    drawHeader( p, local->formatDate( curDay, false ),
                curDay, QDate(), 0, 0, width, mHeaderHeight );

    y += mHeaderHeight + 5;
    x = 80;
    Event::List eventList = mCalendar->events( curDay, true );

    p.setFont( QFont( "helvetica", 14 ) );
    drawAllDayBox( p, eventList, curDay, true, x, y, width - x, currHeight );
    y += currHeight;
    drawAgendaDayBox( p, eventList, curDay, mIncludeAllEvents,
                      curStartTime, curEndTime, x, y, width - x, height - y );
    drawTimeLine( p, curStartTime, curEndTime, 0, y, x - 5, height - y );
    curDay = curDay.addDays( 1 );
    if ( curDay <= mToDate ) mPrinter->newPage();
  } while ( curDay <= mToDate );
}



/**************************************************************
 *           Print Week
 **************************************************************/

CalPrintWeek::CalPrintWeek(KPrinter *printer, Calendar *cal, KConfig *cfg)
  :CalPrintBase(printer,cal,cfg)
{
}

CalPrintWeek::~CalPrintWeek()
{
}

QWidget *CalPrintWeek::configWidget( QWidget *w )
{
  mConfigWidget = new CalPrintWeekConfig_Base( w );
  setSettingsWidget();
  return mConfigWidget;
}

void CalPrintWeek::readSettingsWidget()
{
  CalPrintWeekConfig_Base *cfg =
      dynamic_cast<CalPrintWeekConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mWeekPrintType = (eWeekPrintType)( cfg->mPrintType->id(
      cfg->mPrintType->selected() ) );

    mStartTime = cfg->mFromTime->time();
    mEndTime = cfg->mToTime->time();

    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mUseColors = cfg->mColors->isChecked();
  }
}

void CalPrintWeek::setSettingsWidget()
{
  CalPrintWeekConfig_Base *cfg =
      dynamic_cast<CalPrintWeekConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mPrintType->setButton( mWeekPrintType );

    cfg->mFromTime->setTime( mStartTime );
    cfg->mToTime->setTime( mEndTime );

    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mColors->setChecked( mUseColors );
  }
}

void CalPrintWeek::loadConfig()
{
  if ( mConfig ) {
    QDate dt;
    QTime tm1( KOPrefs::instance()->mDayBegins.time() );
    QDateTime startTm( dt, tm1  );
    QDateTime endTm( dt, tm1.addSecs( 43200 ) );
    mStartTime = mConfig->readDateTimeEntry( "Start time", &startTm ).time();
    mEndTime = mConfig->readDateTimeEntry( "End time", &endTm ).time();
    mIncludeTodos = mConfig->readBoolEntry( "Include todos", false );
    mWeekPrintType =(eWeekPrintType)( mConfig->readNumEntry( "Print type", (int)Filofax ) );
  }
  setSettingsWidget();
}

void CalPrintWeek::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Start time", QDateTime( QDate(), mStartTime ) );
    mConfig->writeEntry( "End time", QDateTime( QDate(), mEndTime ) );
    mConfig->writeEntry( "Include todos", mIncludeTodos );
    mConfig->writeEntry( "Print type", int( mWeekPrintType ) );
  }
}

KPrinter::Orientation CalPrintWeek::orientation()
{
  if ( mWeekPrintType == Filofax ) return KPrinter::Portrait;
  else return KPrinter::Landscape;
}

void CalPrintWeek::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintBase::setDateRange( from, to );
  CalPrintWeekConfig_Base *cfg =
      dynamic_cast<CalPrintWeekConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintWeek::print( QPainter &p, int width, int height )
{
  QDate curWeek, fromWeek, toWeek;

  // correct begin and end to first and last day of week
  int weekdayCol = weekdayColumn( mFromDate.dayOfWeek() );
  fromWeek = mFromDate.addDays( -weekdayCol );
  weekdayCol = weekdayColumn( mFromDate.dayOfWeek() );
  toWeek = mToDate.addDays( 6 - weekdayCol );

  curWeek = fromWeek.addDays( 6 );
  KLocale *local = KGlobal::locale();

  switch ( mWeekPrintType ) {
    case Filofax:
      do {
        QString line1( local->formatDate( curWeek.addDays( -6 ) ) );
        QString line2( local->formatDate( curWeek ) );
        drawHeader( p, line1 + "\n" + line2, curWeek.addDays( -6 ), QDate(),
                    0, 0, width, mHeaderHeight );
        int top = mHeaderHeight + 10;
        drawWeek( p, curWeek, 0, top, width, height - top );
        curWeek = curWeek.addDays( 7 );
        if ( curWeek <= toWeek )
          mPrinter->newPage();
      } while ( curWeek <= toWeek );
      break;

    case Timetable:
    default:
      do {
        QString line1( local->formatDate( curWeek.addDays( -6 ) ) );
        QString line2( local->formatDate( curWeek ) );
        int hh = int(mHeaderHeight * 2./3.);
        drawHeader( p, i18n("date from - to", "%1 - %2").arg( line1 ).arg( line2 ),
                    curWeek, QDate(), 0, 0, width, hh );
        drawTimeTable( p, fromWeek, curWeek,
                       mStartTime, mEndTime, 0, hh + 5,
                       width, height - hh - 5 );
        fromWeek = fromWeek.addDays( 7 );
        curWeek = fromWeek.addDays( 6 );
        if ( curWeek <= toWeek )
          mPrinter->newPage();
      } while ( curWeek <= toWeek );
      break;

    case SplitWeek:
      drawSplitWeek( p, fromWeek, toWeek );
      break;
  }
}




/**************************************************************
 *           Print Month
 **************************************************************/

CalPrintMonth::CalPrintMonth( KPrinter *printer, Calendar *cal, KConfig *cfg )
  : CalPrintBase( printer, cal, cfg )
{
}

CalPrintMonth::~CalPrintMonth()
{
}

QWidget *CalPrintMonth::configWidget( QWidget *w )
{
  mConfigWidget = new CalPrintMonthConfig_Base( w );
  return mConfigWidget;
}

void CalPrintMonth::readSettingsWidget()
{
  CalPrintMonthConfig_Base *cfg =
      dynamic_cast<CalPrintMonthConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mWeekNumbers =  cfg->mWeekNumbers->isChecked();

    mIncludeTodos = cfg->mIncludeTodos->isChecked();
//    mUseColors = cfg->mColors->isChecked();
  }
}

void CalPrintMonth::setSettingsWidget()
{
  CalPrintMonthConfig_Base *cfg =
      dynamic_cast<CalPrintMonthConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mWeekNumbers->setChecked( mWeekNumbers );

    cfg->mIncludeTodos->setChecked( mIncludeTodos );
//    cfg->mColors->setChecked( mUseColors );
  }
}

void CalPrintMonth::loadConfig()
{
  if ( mConfig ) {
    mWeekNumbers = mConfig->readBoolEntry( "Print week numbers", true );
    mIncludeTodos = mConfig->readBoolEntry( "Include todos", false );
  }
  setSettingsWidget();
}

void CalPrintMonth::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Print week numbers", mWeekNumbers );
    mConfig->writeEntry( "Include todos", mIncludeTodos );
  }
}

void CalPrintMonth::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintBase::setDateRange( from, to );
  CalPrintMonthConfig_Base *cfg =
      dynamic_cast<CalPrintMonthConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintMonth::print( QPainter &p, int width, int height )
{
  QDate curMonth, fromMonth, toMonth;

  fromMonth = mFromDate.addDays( -( mFromDate.day() - 1 ) );
  toMonth = mToDate.addDays( mToDate.daysInMonth() - mToDate.day() );

  curMonth = fromMonth;
  do {
    QString title( i18n("monthname year", "%1 %2") );
    title = title.arg( KOGlobals::self()->calendarSystem()->monthName( curMonth ) )
                 .arg( curMonth.year() );
    QDate tmp( fromMonth );
    int weekdayCol = weekdayColumn( tmp.dayOfWeek() );
    tmp = tmp.addDays( -weekdayCol );

    drawHeader( p, title,
                curMonth.addMonths( -1 ), curMonth.addMonths( 1 ),
                0, 0, width, mHeaderHeight );
    drawMonth( p, curMonth, mWeekNumbers, 0, mHeaderHeight + 5,
               width, height - mHeaderHeight - 5 );
    curMonth = curMonth.addDays( curMonth.daysInMonth() );
    if ( curMonth <= toMonth ) mPrinter->newPage();
  } while ( curMonth <= toMonth );

}




/**************************************************************
 *           Print Todos
 **************************************************************/

CalPrintTodos::CalPrintTodos( KPrinter *printer, Calendar *cal, KConfig *cfg )
  : CalPrintBase( printer, cal, cfg )
{
}

CalPrintTodos::~CalPrintTodos()
{
}

QWidget *CalPrintTodos::configWidget( QWidget *w )
{
  mConfigWidget = new CalPrintTodoConfig_Base( w );
  return mConfigWidget;
}

void CalPrintTodos::readSettingsWidget()
{
  CalPrintTodoConfig_Base *cfg =
      dynamic_cast<CalPrintTodoConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    mPageTitle = cfg->mTitle->text();

    mTodoPrintType = (eTodoPrintType)( cfg->mPrintType->id(
      cfg->mPrintType->selected() ) );

    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mIncludeDescription = cfg->mDescription->isChecked();
    mIncludePriority = cfg->mPriority->isChecked();
    mIncludeDueDate = cfg->mDueDate->isChecked();
    mConnectSubTodos = cfg->mConnectSubTodos->isChecked();
  }
}

void CalPrintTodos::setSettingsWidget()
{
  CalPrintTodoConfig_Base *cfg =
      dynamic_cast<CalPrintTodoConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    cfg->mTitle->setText( mPageTitle );

    cfg->mPrintType->setButton( mTodoPrintType );

    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mDescription->setChecked( mIncludeDescription );
    cfg->mPriority->setChecked( mIncludePriority );
    cfg->mDueDate->setChecked( mIncludeDueDate );
    cfg->mConnectSubTodos->setChecked( mConnectSubTodos );
  }
}

void CalPrintTodos::loadConfig()
{
  if ( mConfig ) {
    mPageTitle = mConfig->readEntry( "Page title", i18n("Todo list") );
    mTodoPrintType = (eTodoPrintType)mConfig->readNumEntry( "Print type", (int)TodosAll );
    mIncludeDescription = mConfig->readBoolEntry( "Include description", true );
    mIncludePriority = mConfig->readBoolEntry( "Include priority", true );
    mIncludeDueDate = mConfig->readBoolEntry( "Include due date", true );
    mConnectSubTodos = mConfig->readBoolEntry( "Connect subtodos", true );
  }
  setSettingsWidget();
}

void CalPrintTodos::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Page title", mPageTitle );
    mConfig->writeEntry( "Print type", int( mTodoPrintType ) );
    mConfig->writeEntry( "Include description", mIncludeDescription );
    mConfig->writeEntry( "Include priority", mIncludePriority );
    mConfig->writeEntry( "Include due date", mIncludeDueDate );
    mConfig->writeEntry( "Connect subtodos", mConnectSubTodos );
  }
}

void CalPrintTodos::print( QPainter &p, int width, int height )
{
  int pospriority = 10;
  int possummary = 60;
  int posdue = width - 85;
  int lineSpacing = 15;
  int fontHeight = 10;

  drawHeader( p, mPageTitle, mFromDate, QDate(),
              0, 0, width, mHeaderHeight );

  int mCurrentLinePos = mHeaderHeight + 5;
  QString outStr;

  p.setFont( QFont( "helvetica", 10 ) );
  lineSpacing = p.fontMetrics().lineSpacing();
  mCurrentLinePos += lineSpacing;
  // draw the headers
  p.setFont( QFont("helvetica", 10, QFont::Bold ) );
  if ( mIncludePriority ) {
    outStr += i18n("Priority");
    p.drawText( pospriority, mCurrentLinePos - 2, outStr);
  } else {
    possummary = 10;
    pospriority = -1;
  }

  outStr.truncate( 0 );
  outStr += i18n("Summary");
  p.drawText( possummary, mCurrentLinePos - 2, outStr );

  if ( mIncludeDueDate ) {
    outStr.truncate( 0 );
    outStr += i18n("Due");
    p.drawText( posdue, mCurrentLinePos - 2, outStr );
  } else {
    posdue = -1;
  }

  p.setFont( QFont( "helvetica", 10 ) );

  fontHeight = p.fontMetrics().height();

  Todo::List todoList;
//   if (mTodoPrintType==TodosSelected) {
//     todoList.append(selectedTodoo);
//   } else {
    todoList = mCalendar->todos();
//   }
  // TODO_RK: filter out todos

  int count = 0;
  for( int cprior = 1; cprior <= 6; cprior++ ) {
    Todo::List::ConstIterator it;
    for( it = todoList.begin(); it != todoList.end(); ++it ) {
      Todo *currEvent = *it;

      // Filter out the subitems.
      if ( currEvent->relatedTo() ) {
        continue;
      }

      QDate start = currEvent->dtStart().date();
      // if it is not to start yet, skip.
      if ( ( !start.isValid() ) && ( start >= mToDate ) ) {
        continue;
      }
      // priority
      int priority = currEvent->priority();
      // 6 is the lowest priority (the unspecified one)
      if ( ( priority != cprior ) ||
           ( ( cprior == 6 ) && ( priority == 0 ) ) ) {
        continue;
      }
      count++;
      int todoHeight = height - mCurrentLinePos;
      drawTodo( count, currEvent, p, mConnectSubTodos,
                mIncludeDescription, pospriority, possummary, posdue, 0,
                0, mCurrentLinePos, width, todoHeight, height );
    }
  }
}


#endif
