/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <q3popupmenu.h>
#include <QFont>
#include <QFontMetrics>
#include <qnamespace.h>
#include <q3hbox.h>
#include <q3vbox.h>
#include <QPushButton>
#include <QToolTip>
#include <QPainter>
#include <QCursor>
#include <q3listbox.h>
#include <QLayout>
#include <QLabel>
//Added by qt3to4:
#include <QGridLayout>
#include <QKeyEvent>
#include <QFrame>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QMouseEvent>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kwordwrap.h>

#include <kcalendarsystem.h>
#include <kcal/calfilter.h>
#include <kcal/calendar.h>
#include <kcal/incidenceformatter.h>
#include <kcal/calendarresources.h>

#include "koprefs.h"
#include "koglobals.h"
#include "koeventpopupmenu.h"
#include "kohelper.h"

#include "komonthview.h"
#include "komonthview.moc"

//--------------------------------------------------------------------------

#warning Port me!
#if 0
KOMonthCellToolTip::KOMonthCellToolTip( QWidget *parent,
                                        KNoScrollListBox *lv )
  : QToolTip( parent )
{
  eventlist = lv;
}

void KOMonthCellToolTip::maybeTip( const QPoint & pos )
{
  QRect r;
  Q3ListBoxItem *it = eventlist->itemAt( pos );
  MonthViewItem *i = static_cast<MonthViewItem*>( it );

  if( i && KOPrefs::instance()->mEnableToolTips ) {
    /* Calculate the rectangle. */
    r=eventlist->itemRect( it );
    /* Show the tip */
    QString tipText( IncidenceFormatter::toolTipString( i->incidence() ) );
    if ( !tipText.isEmpty() ) {
      tip( r, tipText );
    }
  }
}
#endif

KNoScrollListBox::KNoScrollListBox( QWidget *parent )
  : Q3ListBox( parent ), mSqueezing( false )
{
  QPalette pal = palette();
  pal.setColor( QPalette::Foreground, KOPrefs::instance()->agendaBgColor().dark( 150 ) );
  pal.setColor( QPalette::Base, KOPrefs::instance()->agendaBgColor() );
  setPalette( pal );
}

void KNoScrollListBox::setBackground( bool primary, bool workDay )
{
  QColor color;
  if ( workDay ) {
    color = KOPrefs::instance()->workingHoursColor();
  } else {
    color = KOPrefs::instance()->agendaBgColor();
  }

  QPalette pal = palette();
  if ( primary ) {
    pal.setColor( QPalette::Base, color );
  } else {
    pal.setColor( QPalette::Base, color.dark( 115 ) );
  }
  setPalette( pal );
}

void KNoScrollListBox::keyPressEvent( QKeyEvent *e )
{
  switch( e->key() ) {
    case Qt::Key_Right:
      scrollBy( 4, 0 );
      break;
    case Qt::Key_Left:
      scrollBy( -4, 0 );
      break;
    case Qt::Key_Up:
      if ( !count() ) break;
      setCurrentItem( ( currentItem() + count() - 1 ) % count() );
      if ( !itemVisible( currentItem() ) ) {
        if ( (unsigned int)currentItem() == ( count() - 1 ) ) {
          setTopItem( currentItem() - numItemsVisible() + 1 );
        } else {
          setTopItem( topItem() - 1 );
        }
      }
      break;
    case Qt::Key_Down:
      if ( !count() ) break;
      setCurrentItem( ( currentItem() + 1 ) % count() );
      if( !itemVisible( currentItem() ) ) {
        if( currentItem() == 0 ) {
          setTopItem( 0 );
        } else {
          setTopItem( topItem() + 1 );
        }
      }
    case Qt::Key_Shift:
      emit shiftDown();
      break;
    default:
      break;
  }
}

void KNoScrollListBox::keyReleaseEvent( QKeyEvent *e )
{
  switch( e->key() ) {
    case Qt::Key_Shift:
      emit shiftUp();
      break;
    default:
      break;
  }
}

void KNoScrollListBox::mousePressEvent( QMouseEvent *e )
{
  Q3ListBox::mousePressEvent( e );

  if ( e->button() == Qt::RightButton ) {
    emit rightClick();
  }
}

void KNoScrollListBox::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  Q3ListBox::contentsMouseDoubleClickEvent( e );
  Q3ListBoxItem *item = itemAt( e->pos() );
  if ( !item ) {
    emit doubleClicked( item );
  }
}

void KNoScrollListBox::resizeEvent( QResizeEvent *e )
{
  bool s = count() && ( maxItemWidth() > e->size().width() );
  if ( mSqueezing || s )
    triggerUpdate( false );

  mSqueezing = s;
  Q3ListBox::resizeEvent( e );
}

MonthViewItem::MonthViewItem( Incidence *incidence, const QDateTime &qd,
                              const QString & s ) : Q3ListBoxItem()
{
  setText( s );

  mIncidence = incidence;
  mDateTime = qd;

  mTodoPixmap      = KOGlobals::self()->smallIcon("todo");
  mTodoDonePixmap  = KOGlobals::self()->smallIcon("checkedbox");
  mAlarmPixmap     = KOGlobals::self()->smallIcon("bell");
  mRecurPixmap     = KOGlobals::self()->smallIcon("recur");
  mReplyPixmap     = KOGlobals::self()->smallIcon("mail_reply");

  mResourceColor = QColor();
  mTodo      = false;
  mTodoDone  = false;
  mRecur     = false;
  mAlarm     = false;
  mReply     = false;
}

void MonthViewItem::paint( QPainter *p )
{
  bool sel = isSelected();

  QColor bgColor = palette().color( QPalette::Normal,
            sel ? QPalette::Highlight : QPalette::Background );
  int offset=0;
  if ( KOPrefs::instance()->monthViewUsesResourceColor() &&
    mResourceColor.isValid() ) {
    p->setBackground( QBrush( mResourceColor ) );
    p->eraseRect( 0, 0, listBox()->maxItemWidth(), height( listBox() ) );
    offset=2;
  }
  if ( KOPrefs::instance()->monthViewUsesCategoryColor() ) {
    p->setBackground( QBrush( bgColor ) );
    p->eraseRect( offset, offset, listBox()->maxItemWidth()-2*offset, height( listBox() )-2*offset );
  }
  int x = 3;
  if ( mTodo ) {
    p->drawPixmap( x, 0, mTodoPixmap );
    x += mTodoPixmap.width() + 2;
  }
  if ( mTodoDone ) {
    p->drawPixmap( x, 0, mTodoDonePixmap );
    x += mTodoPixmap.width() + 2;
  }
  if ( mRecur ) {
    p->drawPixmap( x, 0, mRecurPixmap );
    x += mRecurPixmap.width() + 2;
  }
  if ( mAlarm ) {
    p->drawPixmap( x, 0, mAlarmPixmap );
    x += mAlarmPixmap.width() + 2;
  }
  if ( mReply ) {
    p->drawPixmap(x, 0, mReplyPixmap );
    x += mReplyPixmap.width() + 2;
  }
  QFontMetrics fm = p->fontMetrics();
  int yPos;
  int pmheight = qMax( mRecurPixmap.height(),
                       qMax( mAlarmPixmap.height(), mReplyPixmap.height() ) );
  if( pmheight < fm.height() )
    yPos = fm.ascent() + fm.leading()/2;
  else
    yPos = pmheight/2 - fm.height()/2  + fm.ascent();
  QColor textColor = getTextColor( bgColor );
  p->setPen( textColor );

  KWordWrap::drawFadeoutText( p, x, yPos, listBox()->width() - x, text() );
}

int MonthViewItem::height( const Q3ListBox *lb ) const
{
  return qMax( qMax( mRecurPixmap.height(), mReplyPixmap.height() ),
               qMax( mAlarmPixmap.height(), lb->fontMetrics().lineSpacing()+1) );
}

int MonthViewItem::width( const Q3ListBox *lb ) const
{
  int x = 3;
  if( mRecur ) {
    x += mRecurPixmap.width()+2;
  }
  if( mAlarm ) {
    x += mAlarmPixmap.width()+2;
  }
  if( mReply ) {
    x += mReplyPixmap.width()+2;
  }

  return( x + lb->fontMetrics().boundingRect( text() ).width() + 1 );
}


MonthViewCell::MonthViewCell( KOMonthView *parent)
  : QWidget( parent ),
    mMonthView( parent ), mPrimary( false ), mHoliday( false )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  mLabel = new QLabel( this );
  mLabel->setFrameStyle( QFrame::Panel | QFrame::Plain );
  mLabel->setLineWidth( 1 );
  mLabel->setAlignment( Qt::AlignCenter );

  mItemList = new KNoScrollListBox( this );
  mItemList->setMinimumSize( 10, 10 );
  mItemList->setFrameStyle( QFrame::Panel | QFrame::Plain );
  mItemList->setLineWidth( 1 );

#warning Port me!
#if 0
  new KOMonthCellToolTip( mItemList->viewport(),
                          static_cast<KNoScrollListBox *>( mItemList ) );
#endif

  topLayout->addWidget( mItemList );

  mLabel->raise();

  mStandardPalette = palette();

  enableScrollBars( false );

  updateConfig();

  connect( mItemList, SIGNAL( doubleClicked( Q3ListBoxItem *) ),
           SLOT( defaultAction( Q3ListBoxItem * ) ) );
  connect( mItemList, SIGNAL( rightButtonPressed( Q3ListBoxItem *,
                                                  const QPoint &) ),
           SLOT( contextMenu( Q3ListBoxItem * ) ) );
  connect( mItemList, SIGNAL( clicked( Q3ListBoxItem * ) ),
           SLOT( select() ) );
}

void MonthViewCell::setDate( const QDate &date )
{
//  kDebug(5850) << "MonthViewCell::setDate(): " << date.toString() << endl;

  mDate = date;

  setFrameWidth();

  QString text;
  if ( KOGlobals::self()->calendarSystem()->day( date ) == 1 ) {
    text = i18nc("'Month day' for month view cells", "%1 %2",
          KOGlobals::self()->calendarSystem()->monthName( date, true ) ,
          KOGlobals::self()->calendarSystem()->day(mDate) );
    QFontMetrics fm( mLabel->font() );
    mLabel->resize( mLabelSize + QSize( fm.width( text ), 0 ) );
  } else {
    mLabel->resize( mLabelSize );
    text = QString::number( KOGlobals::self()->calendarSystem()->day(mDate) );
  }
  mLabel->setText( text );

  resizeEvent( 0 );
}

QDate MonthViewCell::date() const
{
  return mDate;
}

void MonthViewCell::setFrameWidth()
{
  // show current day with a thicker frame
  if ( mDate == QDate::currentDate() )
    mItemList->setLineWidth( 3 );
  else
    mItemList->setLineWidth( 1 );
}

void MonthViewCell::setPrimary( bool primary )
{
  mPrimary = primary;

#warning Port me!
#if 0
  if ( mPrimary ) {
    mLabel->setBackgroundMode( PaletteBase );
  } else {
    mLabel->setBackgroundMode( PaletteBackground );
  }
#endif

  mItemList->setBackground( mPrimary, KOGlobals::self()->isWorkDay( mDate ) );
}

bool MonthViewCell::isPrimary() const
{
  return mPrimary;
}

void MonthViewCell::setHoliday( bool holiday )
{
  mHoliday = holiday;

  if ( holiday ) {
    setPalette( mHolidayPalette );
  } else {
    setPalette( mStandardPalette );
  }
}

void MonthViewCell::setHolidayString( const QString &holiday )
{
  mHolidayString = holiday;
}

void MonthViewCell::updateCell()
{
  setFrameWidth();

  if ( mDate == QDate::currentDate() ) {
    setPalette( mTodayPalette );

    QPalette pal = mItemList->palette();
    pal.setColor( QPalette::Foreground, KOPrefs::instance()->highlightColor() );
    mItemList->setPalette( pal );
  }
  else {
    if ( mHoliday )
      setPalette( mHolidayPalette );
    else
      setPalette( mStandardPalette );

    QPalette pal = mItemList->palette();
    pal.setColor( QPalette::Foreground, KOPrefs::instance()->agendaBgColor().dark( 150 ) );
    mItemList->setPalette( pal );
  }

  mItemList->clear();

  if ( !mHolidayString.isEmpty() ) {
    MonthViewItem *item = new MonthViewItem( 0, QDateTime( mDate ), mHolidayString );
    item->setPalette( mHolidayPalette );
    mItemList->insertItem( item );
  }
}

class MonthViewCell::CreateItemVisitor :
      public IncidenceBase::Visitor
{
  public:
    CreateItemVisitor() : mItem(0) {}

    bool act( IncidenceBase *incidence, const QDate &date, QPalette stdPal, int multiDay )
    {
      mItem = 0;
      mDate = date;
      mStandardPalette = stdPal;
      mMultiDay = multiDay;
      return incidence->accept( *this );
    }
    MonthViewItem *item() const { return mItem; }

  protected:
    bool visit( Event *event ) {
      QString text;
      QDateTime dt( mDate );
      // take the time 0:00 into account, which is non-inclusive
      QDate dtEnd = event->dtEnd().addSecs( event->doesFloat() ? 0 : -1).date();
      int length = event->dtStart().date().daysTo( dtEnd );
      if ( event->isMultiDay() ) {
        if ( mDate == event->dtStart().date() ||
           ( mMultiDay == 0 && event->recursOn( mDate ) ) ) {
          text = "(-- " + event->summary();
          dt = event->dtStart();
        } else if ( !event->doesRecur() && mDate == dtEnd ||
                    // last day of a recurring multi-day event?
                  ( mMultiDay == length && event->recursOn( mDate.addDays( -length ) ) ) ) {
          text = event->summary() + " --)";
        } else if (!(event->dtStart().date().daysTo(mDate) % 7) && length > 7 ) {
          text = "-- " + event->summary() + " --";
        } else {
          text = "----------------";
          dt = QDateTime( mDate );
        }
      } else {
        if (event->doesFloat())
          text = event->summary();
        else {
          text = KGlobal::locale()->formatTime(event->dtStart().time());
          dt.setTime( event->dtStart().time() );
          text += ' ' + event->summary();
        }
      }

      mItem = new MonthViewItem( event, dt, text );
      if (KOPrefs::instance()->monthViewUsesCategoryColor()) {
        QStringList categories = event->categories();
        QString cat = categories.first();
        if (cat.isEmpty()) {
          mItem->setPalette(QPalette(KOPrefs::instance()->mEventColor, KOPrefs::instance()->mEventColor));
        } else {
          mItem->setPalette( QPalette( KOPrefs::instance()->categoryColor(cat), KOPrefs::instance()->categoryColor(cat) ) );
        }
      } else {
        mItem->setPalette( mStandardPalette );
      }

      Attendee *me = event->attendeeByMails( KOPrefs::instance()->allEmails() );
      if ( me != 0 ) {
        mItem->setReply( me->status() == Attendee::NeedsAction && me->RSVP() );
      } else
        mItem->setReply(false);
      return true;
    }
    bool visit( Todo *todo ) {
      QString text;
      if ( !KOPrefs::instance()->showAllDayTodo() )
        return false;
      QDateTime dt( mDate );
      if ( todo->hasDueDate() && !todo->doesFloat() ) {
        text += KGlobal::locale()->formatTime( todo->dtDue().time() );
        text += ' ';
        dt.setTime( todo->dtDue().time() );
      }
      text += todo->summary();

      mItem = new MonthViewItem( todo, dt, text );
      if ( todo->doesRecur() ) {
        mDate < todo->dtDue().date() ?
        mItem->setTodoDone( true ) : mItem->setTodo( true );
      }
      else
        todo->isCompleted() ? mItem->setTodoDone( true ) : mItem->setTodo( true );
      mItem->setPalette( mStandardPalette );
      return true;
    }
  protected:
    MonthViewItem *mItem;
    QDate mDate;
    QPalette mStandardPalette;
    int mMultiDay;
};


void MonthViewCell::addIncidence( Incidence *incidence, int multiDay )
{
  CreateItemVisitor v;

  if ( v.act( incidence, mDate, mStandardPalette, multiDay ) ) {
    MonthViewItem *item = v.item();
    if ( item ) {
      item->setAlarm( incidence->isAlarmEnabled() );
      item->setRecur( incidence->recurrenceType() );

      QColor resourceColor = KOHelper::resourceColor( mCalendar, incidence );
      if ( !resourceColor.isValid() )
        resourceColor = KOPrefs::instance()->mEventColor;
      item->setResourceColor( resourceColor );

      // FIXME: Find the correct position (time-wise) to insert the item.
      //        Currently, the items are displayed in "random" order instead of
      //        chronologically sorted.
      uint i = 0;
      int pos = -1;
      QDateTime dt( item->incidenceDateTime() );

      while ( i < mItemList->count() && pos<0 ) {
        Q3ListBoxItem *item = mItemList->item( i );
        MonthViewItem *mvitem = dynamic_cast<MonthViewItem*>( item );
        if ( mvitem && mvitem->incidenceDateTime()>dt ) {
          pos = i;
        }
        ++i;
      }
      mItemList->insertItem( item, pos );
    }
  }
}

void MonthViewCell::removeIncidence( Incidence *incidence )
{
  for ( uint i = 0; i < mItemList->count(); ++i ) {
    MonthViewItem *item = static_cast<MonthViewItem *>(mItemList->item( i ) );
    if ( item && item->incidence() &&
         item->incidence()->uid() == incidence->uid() ) {
      mItemList->removeItem( i );
      --i;
    }
  }
}

void MonthViewCell::updateConfig()
{
  setFont( KOPrefs::instance()->mMonthViewFont );

  QFontMetrics fm( font() );
  mLabelSize = fm.size( 0, "30" ) +
               QSize( mLabel->frameWidth() * 2, mLabel->frameWidth() * 2 ) +
               QSize( 2, 2 );
//  mStandardPalette = mOriginalPalette;
  QColor bg = mStandardPalette.color( QPalette::Active, QPalette::Background );
  int h,s,v;
  bg.getHsv( &h, &s, &v );
  if ( date().month() %2 == 0 ) {
    if ( v < 128 ) {
      bg = bg.light( 125 );
    } else {
      bg = bg.dark( 125 );
    }
  }
  QPalette pal;
  pal.setColor( backgroundRole(), bg );
  setPalette( pal );
//  mStandardPalette.setColor( QPalette::Background, bg);*/

  mHolidayPalette = mStandardPalette;
  mHolidayPalette.setColor( QPalette::Foreground,
                            KOPrefs::instance()->holidayColor() );
  mHolidayPalette.setColor( QPalette::Text,
                            KOPrefs::instance()->holidayColor() );
  mTodayPalette = mStandardPalette;
  mTodayPalette.setColor( QPalette::Foreground,
                          KOPrefs::instance()->highlightColor() );
  mTodayPalette.setColor( QPalette::Text,
                          KOPrefs::instance()->highlightColor() );
  updateCell();

  mItemList->setBackground( mPrimary, KOGlobals::self()->isWorkDay( mDate ) );
}

void MonthViewCell::enableScrollBars( bool enabled )
{
  if ( enabled ) {
    mItemList->setVScrollBarMode( Q3ScrollView::Auto );
    mItemList->setHScrollBarMode( Q3ScrollView::Auto );
  } else {
    mItemList->setVScrollBarMode( Q3ScrollView::AlwaysOff );
    mItemList->setHScrollBarMode( Q3ScrollView::AlwaysOff );
  }
}

Incidence *MonthViewCell::selectedIncidence()
{
  int index = mItemList->currentItem();
  if ( index < 0 ) return 0;

  MonthViewItem *item =
      static_cast<MonthViewItem *>( mItemList->item( index ) );

  if ( !item ) return 0;

  return item->incidence();
}

QDate MonthViewCell::selectedIncidenceDate()
{
  QDate qd;
  int index = mItemList->currentItem();
  if ( index < 0 ) return qd;

  MonthViewItem *item =
      static_cast<MonthViewItem *>( mItemList->item( index ) );

  if ( !item ) return qd;

  return item->incidenceDateTime().date();
}

void MonthViewCell::select()
{
  // setSelectedCell will deselect currently selected cells
  mMonthView->setSelectedCell( this );

  if( KOPrefs::instance()->enableMonthScroll() )
    enableScrollBars( true );

  // don't mess up the cell when it represents today
  if( mDate != QDate::currentDate() ) {
    mItemList->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    mItemList->setLineWidth( 3 );
  }
}

void MonthViewCell::deselect()
{
  mItemList->clearSelection();
  mItemList->setFrameStyle( QFrame::Plain | QFrame::Panel );
  setFrameWidth();

  enableScrollBars( false );
}

void MonthViewCell::resizeEvent ( QResizeEvent * )
{
  mLabel->move( width() - mLabel->width(), height() - mLabel->height() );
}

void MonthViewCell::defaultAction( Q3ListBoxItem *item )
{
  select();

  if ( !item ) {
    emit newEventSignal( date() );
  } else {
    MonthViewItem *eventItem = static_cast<MonthViewItem *>( item );
    Incidence *incidence = eventItem->incidence();
    if ( incidence ) mMonthView->defaultAction( incidence );
  }
}

void MonthViewCell::contextMenu( Q3ListBoxItem *item )
{
  select();

  if ( item ) {
    MonthViewItem *eventItem = static_cast<MonthViewItem *>( item );
    Incidence *incidence = eventItem->incidence();
    if ( incidence ) mMonthView->showEventContextMenu( incidence, date() );
  }
  else {
    mMonthView->showGeneralContextMenu();
  }
}


KOMonthView::KOMonthView( Calendar *calendar, QWidget *parent )
    : KOEventView( calendar, parent ),
      mDaysPerWeek( 7 ), mNumWeeks( 6 ), mNumCells( mDaysPerWeek * mNumWeeks ),
      mShortDayLabels( false ), mWidthLongDayLabel( 0 ), mSelectedCell( 0 )
{
  QGridLayout *dayLayout = new QGridLayout( this );

  QFont bfont = font();
  bfont.setBold( true );

  QFont mfont = bfont;
  mfont.setPointSize( 20 );

  // month name on top
  mLabel = new QLabel( this );
  mLabel->setFont( mfont );
  mLabel->setAlignment( Qt::AlignCenter );
  mLabel->setLineWidth( 0 );
  mLabel->setFrameStyle( QFrame::Plain );

  dayLayout->addWidget( mLabel, 0, 0, 1, mDaysPerWeek );

  // create the day of the week labels (Sun, Mon, etc) and add them to
  // the layout.
  mDayLabels.resize( mDaysPerWeek );
  int i;
  for( i = 0; i < mDaysPerWeek; i++ ) {
    QLabel *label = new QLabel( this );
    label->setFont( bfont );
    label->setFrameStyle( QFrame::Panel | QFrame::Raised );
    label->setLineWidth( 1 );
    label->setAlignment( Qt::AlignCenter );

    mDayLabels.insert( i, label );

    dayLayout->addWidget( label, 1, i );
    dayLayout->addItem( new QSpacerItem( 10, 0 ), 0, i );
    dayLayout->setColumnStretch( i, 1 );
  }

  int row, col;

  mCells.resize( mNumCells );
  for( row = 0; row < mNumWeeks; ++row ) {
    for( col = 0; col < mDaysPerWeek; ++col ) {
      MonthViewCell *cell = new MonthViewCell( this );
      cell->setCalendar(calendar);
      mCells.insert( row * mDaysPerWeek + col, cell );
      dayLayout->addWidget( cell, row + 2, col );

      connect( cell, SIGNAL( defaultAction( Incidence * ) ),
               SLOT( defaultAction( Incidence * ) ) );
      connect( cell, SIGNAL( newEventSignal( const QDate & ) ),
               SIGNAL( newEventSignal( const QDate & ) ) );
    }
    dayLayout->setRowStretch( row + 2, 1 );
  }

  mEventContextMenu = eventPopup();

  updateConfig();

  emit incidenceSelected( 0 );
}

KOMonthView::~KOMonthView()
{
  qDeleteAll( mCells );
  mCells.clear();
  delete mEventContextMenu;
}

int KOMonthView::maxDatesHint()
{
  return mNumCells;
}

int KOMonthView::currentDateCount()
{
  return mNumCells;
}

Incidence::List KOMonthView::selectedIncidences()
{
  Incidence::List selected;

  if ( mSelectedCell ) {
    Incidence *incidence = mSelectedCell->selectedIncidence();
    if ( incidence ) selected.append( incidence );
  }

  return selected;
}

DateList KOMonthView::selectedDates()
{
  DateList selected;

  if ( mSelectedCell ) {
    QDate qd = mSelectedCell->selectedIncidenceDate();
    if ( qd.isValid() ) selected.append( qd );
  }

  return selected;
}

bool KOMonthView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  if ( mSelectedCell ) {
    startDt.setDate( mSelectedCell->date() );
    endDt.setDate( mSelectedCell->date() );
    allDay = true;
    return true;
  }
  return false;
}

void KOMonthView::updateConfig()
{
  mWeekStartDay = KGlobal::locale()->weekStartDay();

  QFontMetrics fontmetric( mDayLabels[0]->font() );
  mWidthLongDayLabel = 0;

  for ( int i = 0; i < 7; ++i ) {
    int width =
        fontmetric.width( KOGlobals::self()->calendarSystem()->weekDayName( i + 1 ) );
    if ( width > mWidthLongDayLabel ) mWidthLongDayLabel = width;
  }

  updateDayLabels();

  for ( int i = 0; i < mCells.count(); ++i ) {
    mCells[i]->updateConfig();
  }
}

void KOMonthView::updateDayLabels()
{
  kDebug(5850) << "KOMonthView::updateDayLabels()" << endl;

  const KCalendarSystem*calsys=KOGlobals::self()->calendarSystem();
  int currDay;
  for ( int i = 0; i < 7; i++ ) {
    currDay = i+mWeekStartDay;
    if ( currDay > 7 ) currDay -= 7;
    mDayLabels[i]->setText( calsys->weekDayName( currDay, mShortDayLabels ) );
  }
}

void KOMonthView::showDates( const QDate &start, const QDate & )
{
//  kDebug(5850) << "KOMonthView::showDates(): " << start.toString() << endl;

  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  mDateToCell.clear();

  // show first day of month on top for readability issues
  mStartDate = start.addDays( -start.day() + 1 );
  // correct begin of week
  int weekdayCol=( mStartDate.dayOfWeek() + 7 - mWeekStartDay ) % 7;
  mStartDate = mStartDate.addDays( -weekdayCol );

  mLabel->setText( i18nc( "monthname year", "%1 %2" ,
                     calSys->monthName( start ) ,
                     calSys->year( start ) ) );

  bool primary = false;
  int i;
  for( i = 0; i < mCells.size(); ++i ) {
    QDate date = mStartDate.addDays( i );
    if ( calSys->day( date ) == 1 ) {
      primary = !primary;
    }

    mCells[i]->setDate( date );
    mDateToCell[date] = mCells[i];
    if( date == start )
      mCells[i]->select();

    mCells[i]->setPrimary( primary );

    bool isHoliday = calSys->dayOfWeek( date ) == calSys->weekDayOfPray()
                  || !KOGlobals::self()->isWorkDay( date );
    mCells[i]->setHoliday( isHoliday );

    // add holiday, if present
    QStringList holidays( KOGlobals::self()->holiday( date ) );
    mCells[i]->setHolidayString( holidays.join( i18nc("delimiter for joining holiday names", "," ) ) );
  }

  updateView();
}

void KOMonthView::showIncidences( const Incidence::List & )
{
  kDebug(5850) << "KOMonthView::showIncidences( const Incidence::List & ) is not implemented yet." << endl;
}

class KOMonthView::GetDateVisitor : public IncidenceBase::Visitor
{
  public:
    GetDateVisitor() {}

    bool act( IncidenceBase *incidence )
    {
      return incidence->accept( *this );
    }
    QDateTime startDate() const { return mStartDate; }
    QDateTime endDate() const { return mEndDate; }

  protected:
    bool visit( Event *event ) {
      mStartDate = event->dtStart();
      mEndDate = event->dtEnd();

      return true;
    }
    bool visit( Todo *todo ) {
      if ( todo->hasDueDate() ) {
        mStartDate = todo->dtDue();
        mEndDate = todo->dtDue();

        return true;
      } else
        return false;
    }
    bool visit( Journal *journal ) {
      mStartDate = journal->dtStart();
      mEndDate = journal->dtStart();
      return true;
    }
  protected:
    QDateTime mStartDate;
    QDateTime mEndDate;
};

void KOMonthView::changeIncidenceDisplayAdded( Incidence *incidence )
{
  GetDateVisitor gdv;

  if ( !gdv.act( incidence ) ) {
    kDebug(5850) << "Visiting GetDateVisitor failed." << endl;
    return;
  }

  bool floats = incidence->doesFloat();

  if ( incidence->doesRecur() ) {
    for ( int i = 0; i < mCells.count(); ++i ) {
      if ( incidence->recursOn( mCells[i]->date() ) ) {

        // handle multiday events
        int length = gdv.startDate().date().daysTo( gdv.endDate().addSecs( floats ? 0 : -1 ).date() );
        for ( int j = 0; j <= length && i+j < mCells.count(); ++j ) {
          mCells[i+j]->addIncidence( incidence, j );
        }
      }
    }
  } else {
    // addSecs(-1) is added to handle 0:00 cases (because it's non-inclusive according to rfc)
    QDate endDate = gdv.endDate().addSecs( floats ? 0 : -1).date();
    for ( QDate date = gdv.startDate().date(); date <= endDate; date = date.addDays( 1 ) ) {
      MonthViewCell *mvc = mDateToCell[ date ];
      if ( mvc ) mvc->addIncidence( incidence );
    }
  }
}

void KOMonthView::changeIncidenceDisplay( Incidence *incidence, int action )
{
  switch ( action ) {
    case KOGlobals::INCIDENCEADDED:
      changeIncidenceDisplayAdded( incidence );
      break;
    case KOGlobals::INCIDENCEEDITED:
      for( int i = 0; i < mCells.count(); i++ )
        mCells[i]->removeIncidence( incidence );
      changeIncidenceDisplayAdded( incidence );
      break;
    case KOGlobals::INCIDENCEDELETED:
      for( int i = 0; i < mCells.count(); i++ )
        mCells[i]->removeIncidence( incidence );
      break;
    default:
      return;
  }
}

void KOMonthView::updateView()
{
  for( int i = 0; i < mCells.count(); ++i ) {
    mCells[i]->updateCell();
  }

  Incidence::List incidences = calendar()->incidences();
  Incidence::List::ConstIterator it;

  for ( it = incidences.begin(); it != incidences.end(); ++it )
    changeIncidenceDisplayAdded( *it );

  processSelectionChange();
}

void KOMonthView::resizeEvent( QResizeEvent * )
{
  // select the appropriate heading string size. E.g. "Wednesday" or "Wed".
  // note this only changes the text if the requested size crosses the
  // threshold between big enough to support the full name and not big
  // enough.
  if( mDayLabels[0]->width() < mWidthLongDayLabel ) {
    if ( !mShortDayLabels ) {
      mShortDayLabels = true;
      updateDayLabels();
    }
  } else {
    if ( mShortDayLabels ) {
      mShortDayLabels = false;
      updateDayLabels();
    }
  }
}

void KOMonthView::showEventContextMenu( Incidence *incidence, const QDate &qd )
{
  mEventContextMenu->showIncidencePopup( incidence, qd );
}

void KOMonthView::showGeneralContextMenu()
{
  showNewEventPopup();
}

void KOMonthView::setSelectedCell( MonthViewCell *cell )
{
  if ( mSelectedCell && cell != mSelectedCell )
    mSelectedCell->deselect();

  mSelectedCell = cell;

  if ( !mSelectedCell )
    emit incidenceSelected( 0 );
  else
    emit incidenceSelected( mSelectedCell->selectedIncidence() );
}

void KOMonthView::processSelectionChange()
{
  Incidence::List incidences = selectedIncidences();
  if (incidences.count() > 0) {
    emit incidenceSelected( incidences.first() );
  } else {
    emit incidenceSelected( 0 );
  }
}

void KOMonthView::clearSelection()
{
  if ( mSelectedCell ) {
    mSelectedCell->deselect();
    mSelectedCell = 0;
  }
}
