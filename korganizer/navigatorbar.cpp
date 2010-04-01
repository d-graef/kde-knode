/*
    This file is part of KOrganizer.

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qstring.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpopupmenu.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "koglobals.h"
#include "koprefs.h"

#include <kcalendarsystem.h>

#include "navigatorbar.h"

ActiveLabel::ActiveLabel( QWidget *parent, const char *name )
  : QLabel( parent, name )
{
}

void ActiveLabel::mouseReleaseEvent( QMouseEvent * )
{
  emit clicked();
}


NavigatorBar::NavigatorBar( QWidget *parent, const char *name )
  : QWidget( parent, name ), mHasMinWidth( false )
{
  QFont tfont = font();
  tfont.setPointSize( 10 );
  tfont.setBold( false );

  bool isRTL = KOGlobals::self()->reverseLayout();

  QPixmap pix;
  // Create backward navigation buttons
  pix = KOGlobals::self()->smallIcon( isRTL ? "2rightarrow" : "2leftarrow" );
  mPrevYear = new QPushButton( this );
  mPrevYear->setPixmap( pix );
  mPrevYear->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  QToolTip::add( mPrevYear, i18n( "Previous year" ) );

  pix = KOGlobals::self()->smallIcon( isRTL ? "1rightarrow" : "1leftarrow");
  mPrevMonth = new QPushButton( this );
  mPrevMonth->setPixmap( pix );
  mPrevMonth->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  QToolTip::add( mPrevMonth, i18n( "Previous month" ) );

  // Create forward navigation buttons
  pix = KOGlobals::self()->smallIcon( isRTL ? "1leftarrow" : "1rightarrow");
  mNextMonth = new QPushButton( this );
  mNextMonth->setPixmap( pix );
  mNextMonth->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  QToolTip::add( mNextMonth, i18n( "Next month" ) );

  pix = KOGlobals::self()->smallIcon( isRTL ? "2leftarrow" : "2rightarrow");
  mNextYear = new QPushButton( this );
  mNextYear->setPixmap( pix );
  mNextYear->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  QToolTip::add( mNextYear, i18n( "Next year" ) );

  // Create month name button
  mMonth = new ActiveLabel( this );
  mMonth->setFont( tfont );
  mMonth->setAlignment( AlignCenter );
  mMonth->setMinimumHeight( mPrevYear->sizeHint().height() );
  QToolTip::add( mMonth, i18n( "Select a month" ) );

  // Create year button
  mYear = new ActiveLabel( this );
  mYear->setFont( tfont );
  mYear->setAlignment( AlignCenter );
  mYear->setMinimumHeight( mPrevYear->sizeHint().height() );
  QToolTip::add( mYear, i18n( "Select a year" ) );

  // set up control frame layout
  QBoxLayout *ctrlLayout = new QHBoxLayout( this, 0, 4 );
  ctrlLayout->addWidget( mPrevYear );
  ctrlLayout->addWidget( mPrevMonth );
  QBoxLayout *dLayout = new QHBoxLayout( this );
  dLayout->insertStretch( 0, 50 );
  dLayout->addWidget( mMonth );
  dLayout->addWidget( mYear );
  dLayout->addStretch( 50 );
  ctrlLayout->addLayout( dLayout );
  ctrlLayout->addWidget( mNextMonth );
  ctrlLayout->addWidget( mNextYear );

  connect( mPrevYear, SIGNAL( clicked() ), SIGNAL( goPrevYear() ) );
  connect( mPrevMonth, SIGNAL( clicked() ), SIGNAL( goPrevMonth() ) );
  connect( mNextMonth, SIGNAL( clicked() ), SIGNAL( goNextMonth() ) );
  connect( mNextYear, SIGNAL( clicked() ), SIGNAL( goNextYear() ) );
  connect( mMonth, SIGNAL( clicked() ), SLOT( selectMonthFromMenu() ) );
  connect( mYear, SIGNAL( clicked() ), SLOT( selectYearFromMenu() ) );
}

NavigatorBar::~NavigatorBar()
{
}

void NavigatorBar::showButtons( bool left, bool right )
{
  if ( left ) {
    mPrevYear->show();
    mPrevMonth->show();
  } else {
    mPrevYear->hide();
    mPrevMonth->hide();
  }

  if ( right ) {
    mNextYear->show();
    mNextMonth->show();
  } else {
    mNextYear->hide();
    mNextMonth->hide();
  }

}

void NavigatorBar::selectDates( const KCal::DateList &dateList )
{
  if ( dateList.count() > 0 ) {
    mDate = dateList.first();

    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    // Set minimum width to width of widest month name label
    int i;
    int maxwidth = 0;

    for( i = 1; i <= calSys->monthsInYear( mDate ); ++i ) {
      int w = QFontMetrics( mMonth->font() ).
              width( QString( "%1" ).
                     arg( calSys->monthName( i, calSys->year( mDate ) ) ) );
      if ( w > maxwidth ) {
        maxwidth = w;
      }
    }
    mMonth->setMinimumWidth( maxwidth );

    mHasMinWidth = true;

    // set the label text at the top of the navigator
    mMonth->setText( i18n( "monthname", "%1" ).arg( calSys->monthName( mDate ) ) );
    mYear->setText( i18n( "4 digit year", "%1" ).arg( calSys->yearString( mDate, false ) ) );
  }
}

void NavigatorBar::selectMonthFromMenu()
{
  // every year can have different month names (in some calendar systems)
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  int i, month, months = calSys->monthsInYear( mDate );

  QPopupMenu *popup = new QPopupMenu( mMonth );

  for ( i = 1; i <= months; i++ )
    popup->insertItem( calSys->monthName( i, calSys->year( mDate ) ), i );

  popup->setActiveItem( calSys->month( mDate ) - 1 );
  popup->setMinimumWidth( mMonth->width() );

  if ( ( month = popup->exec( mMonth->mapToGlobal( QPoint( 0, 0 ) ),
                              calSys->month( mDate ) - 1 ) ) == -1 ) {
    delete popup;
    return;  // canceled
  }

  emit goMonth( month );

  delete popup;
}

void NavigatorBar::selectYearFromMenu()
{
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  int year = calSys->year( mDate );
  int years = 11;  // odd number (show a few years ago -> a few years from now)
  int minYear = year - ( years / 3 );

  QPopupMenu *popup = new QPopupMenu( mYear );

  QString yearStr;
  int y = minYear;
  for ( int i=0; i < years; i++ ) {
    popup->insertItem( yearStr.setNum( y ), i );
    y++;
  }
  popup->setActiveItem( year - minYear );

  if ( ( year = popup->exec( mYear->mapToGlobal( QPoint( 0, 0 ) ),
                             year - minYear ) ) == -1 ) {
    delete popup;
    return;  // canceled
  }

  emit goYear( year + minYear );

  delete popup;
}

#include "navigatorbar.moc"
