/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "task.h"

#include <libkcal/todo.h>
#include <kdebug.h>

using namespace Kolab;

// Kolab Storage Specification:
//    "The priority can be a number between 1 and 5, with 1 being the highest priority."
// iCalendar (RFC 2445):
//    "The priority is specified as an integer in the range
//     zero to nine. A value of zero specifies an
//     undefined priority. A value of one is the
//     highest priority. A value of nine is the lowest
//     priority."

static int kcalPriorityToKolab( const int kcalPriority )
{
  if ( kcalPriority >= 0 && kcalPriority <= 9 ) {
    // We'll map undefined (0) to 3 (default)
    //                                   0  1  2  3  4  5  6  7  8  9
    static const int priorityMap[10] = { 3, 1, 1, 2, 2, 3, 3, 4, 4, 5 };
    return priorityMap[kcalPriority];
  }
  else {
    kdWarning() << "kcalPriorityToKolab(): Got invalid priority " << kcalPriority << endl;
    return 3;
  }
}

static int kolabPrioritytoKCal( const int kolabPriority )
{
  if ( kolabPriority >= 1 && kolabPriority <= 5 ) {
    //                                  1  2  3  4  5
    static const int priorityMap[5] = { 1, 3, 5, 7, 9 };
    return priorityMap[kolabPriority - 1];
  }
  else {
    kdWarning() << "kolabPrioritytoKCal(): Got invalid priority " << kolabPriority << endl;
    return 5;
  }
}

KCal::Todo* Task::xmlToTask( const QString& xml, const QString& tz, KCal::ResourceKolab *res,
                             const QString& subResource, Q_UINT32 sernum )
{
  Task task( res, subResource, sernum, tz );
  task.load( xml );
  KCal::Todo* todo = new KCal::Todo();
  task.saveTo( todo );
  return todo;
}

QString Task::taskToXML( KCal::Todo* todo, const QString& tz )
{
  Task task( 0, QString::null, 0, tz, todo );
  return task.saveXML();
}

Task::Task( KCal::ResourceKolab *res, const QString &subResource, Q_UINT32 sernum,
            const QString& tz, KCal::Todo* task )
  : Incidence( res, subResource, sernum, tz ),
    mPriority( 5 ), mPercentCompleted( 0 ),
    mStatus( KCal::Incidence::StatusNone ),
    mHasStartDate( false ), mHasDueDate( false ),
    mHasCompletedDate( false )
{
  if ( task )
    setFields( task );
}

Task::~Task()
{
}

void Task::setPriority( int priority )
{
  mPriority = priority;
}

int Task::priority() const
{
  return mPriority;
}

void Task::setPercentCompleted( int percent )
{
  mPercentCompleted = percent;
}

int Task::percentCompleted() const
{
  return mPercentCompleted;
}

void Task::setStatus( KCal::Incidence::Status status )
{
  mStatus = status;
}

KCal::Incidence::Status Task::status() const
{
  return mStatus;
}

void Task::setParent( const QString& parentUid )
{
  mParent = parentUid;
}

QString Task::parent() const
{
  return mParent;
}

void Task::setDueDate( const QDateTime& date )
{
  mDueDate = date;
  mHasDueDate = true;
}

QDateTime Task::dueDate() const
{
  return mDueDate;
}

void Task::setHasStartDate( bool v )
{
  mHasStartDate = v;
}

bool Task::hasStartDate() const
{
  return mHasStartDate;
}

bool Task::hasDueDate() const
{
  return mHasDueDate;
}

void Task::setCompletedDate( const QDateTime& date )
{
  mCompletedDate = date;
  mHasCompletedDate = true;
}

QDateTime Task::completedDate() const
{
  return mCompletedDate;
}

bool Task::hasCompletedDate() const
{
  return mHasCompletedDate;
}

bool Task::loadAttribute( QDomElement& element )
{
  QString tagName = element.tagName();

  if ( tagName == "priority" ) {
    bool ok;
    mKolabPriorityFromDom = element.text().toInt( &ok );
    if ( !ok || mKolabPriorityFromDom < 1 || mKolabPriorityFromDom > 5 ) {
      kdWarning() << "loadAttribute(): Invalid \"priority\" value: " << element.text() << endl;
      mKolabPriorityFromDom = -1;
    }
  } else if ( tagName == "x-kcal-priority" ) {
    bool ok;
    mKCalPriorityFromDom = element.text().toInt( &ok );
    if ( !ok || mKCalPriorityFromDom < 0 || mKCalPriorityFromDom > 9 ) {
      kdWarning() << "loadAttribute(): Invalid \"x-kcal-priority\" value: " << element.text() << endl;
      mKCalPriorityFromDom = -1;
    }
  } else if ( tagName == "completed" ) {
    bool ok;
    int percent = element.text().toInt( &ok );
    if ( !ok || percent < 0 || percent > 100 )
      percent = 0;
    setPercentCompleted( percent );
  } else if ( tagName == "status" ) {
    if ( element.text() == "in-progress" )
      setStatus( KCal::Incidence::StatusInProcess );
    else if ( element.text() == "completed" )
      setStatus( KCal::Incidence::StatusCompleted );
    else if ( element.text() == "waiting-on-someone-else" )
      setStatus( KCal::Incidence::StatusNeedsAction );
    else if ( element.text() == "deferred" )
      // Guessing a status here
      setStatus( KCal::Incidence::StatusCanceled );
    else
      // Default
      setStatus( KCal::Incidence::StatusNone );
  } else if ( tagName == "due-date" )
    setDueDate( stringToDateTime( element.text() ) );
  else if ( tagName == "parent" )
    setParent( element.text() );
  else if ( tagName == "x-completed-date" )
    setCompletedDate( stringToDateTime( element.text() ) );
  else if ( tagName == "start-date" ) {
    setHasStartDate( true );
    setStartDate( element.text() );
  } else
    return Incidence::loadAttribute( element );

  // We handled this
  return true;
}

bool Task::saveAttributes( QDomElement& element ) const
{
  // Save the base class elements
  Incidence::saveAttributes( element );

  // We need to save x-kcal-priority as well, since the Kolab priority can only save values from
  // 1 to 5, but we have values from 0 to 9, and do not want to loose them
  writeString( element, "priority", QString::number( kcalPriorityToKolab( priority() ) ) );
  writeString( element, "x-kcal-priority", QString::number( priority() ) );

  writeString( element, "completed", QString::number( percentCompleted() ) );

  switch( status() ) {
  case KCal::Incidence::StatusInProcess:
    writeString( element, "status", "in-progress" );
    break;
  case KCal::Incidence::StatusCompleted:
    writeString( element, "status", "completed" );
    break;
  case KCal::Incidence::StatusNeedsAction:
    writeString( element, "status", "waiting-on-someone-else" );
    break;
  case KCal::Incidence::StatusCanceled:
    writeString( element, "status", "deferred" );
    break;
  case KCal::Incidence::StatusNone:
    writeString( element, "status", "not-started" );
    break;
  case KCal::Incidence::StatusTentative:
  case KCal::Incidence::StatusConfirmed:
  case KCal::Incidence::StatusDraft:
  case KCal::Incidence::StatusFinal:
  case KCal::Incidence::StatusX:
    // All of these are saved as StatusNone.
    writeString( element, "status", "not-started" );
    break;
  }

  if ( hasDueDate() )
    writeString( element, "due-date", dateTimeToString( dueDate() ) );

  if ( !parent().isNull() )
    writeString( element, "parent", parent() );

  if ( hasCompletedDate() && percentCompleted() == 100)
    writeString( element, "x-completed-date", dateTimeToString( completedDate() ) );

  return true;
}


bool Task::loadXML( const QDomDocument& document )
{
  mKolabPriorityFromDom = -1;
  mKCalPriorityFromDom = -1;

  QDomElement top = document.documentElement();

  if ( top.tagName() != "task" ) {
    qWarning( "XML error: Top tag was %s instead of the expected task",
              top.tagName().ascii() );
    return false;
  }
  setHasStartDate( false ); // todo's don't necessarily have one

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      if ( !loadAttribute( e ) )
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  loadAttachments();
  decideAndSetPriority();
  return true;
}

QString Task::saveXML() const
{
  QDomDocument document = domTree();
  QDomElement element = document.createElement( "task" );
  element.setAttribute( "version", "1.0" );
  saveAttributes( element );
  if ( !hasStartDate() && startDate().isValid() ) {
    // events and journals always have a start date, but tasks don't.
    // Remove the entry done by the inherited save above, because we
    // don't have one.
    QDomNodeList l = element.elementsByTagName( "start-date" );
    Q_ASSERT( l.count() == 1 );
    element.removeChild( l.item( 0 ) );
  }
  document.appendChild( element );
  return document.toString();
}

void Task::setFields( const KCal::Todo* task )
{
  Incidence::setFields( task );

  setPriority( task->priority() );
  setPercentCompleted( task->percentComplete() );
  setStatus( task->status() );
  setHasStartDate( task->hasStartDate() );

  if ( task->hasDueDate() )
    setDueDate( localToUTC( task->dtDue() ) );
  else
    mHasDueDate = false;
  if ( task->relatedTo() )
    setParent( task->relatedTo()->uid() );
  else if ( !task->relatedToUid().isEmpty() )
    setParent( task->relatedToUid( ) );
  else
    setParent( QString::null );

  if ( task->hasCompletedDate() && task->percentComplete() == 100 )
    setCompletedDate( localToUTC( task->completed() ) );
  else
    mHasCompletedDate = false;
}

void Task::decideAndSetPriority()
{
  // If we have both Kolab and KCal values in the XML, we prefer the KCal value, but only if the
  // values are still in sync
  if  ( mKolabPriorityFromDom != -1 && mKCalPriorityFromDom != -1 ) {
    const bool inSync = ( kcalPriorityToKolab( mKCalPriorityFromDom ) == mKolabPriorityFromDom );
    if ( inSync ) {
      setPriority( mKCalPriorityFromDom );
    }
    else {
      // Out of sync, some other client changed the Kolab priority, so we have to ignore our
      // KCal priority
      setPriority( kolabPrioritytoKCal( mKolabPriorityFromDom ) );
    }
  }

  // Only KCal priority set, use that.
  else if ( mKolabPriorityFromDom == -1 && mKCalPriorityFromDom != -1 ) {
    kdWarning() << "decideAndSetPriority(): No Kolab priority found, only the KCal priority!" << endl;
    setPriority( mKCalPriorityFromDom );
  }

  // Only Kolab priority set, use that
  else if ( mKolabPriorityFromDom != -1 && mKCalPriorityFromDom == -1 ) {
    setPriority( kolabPrioritytoKCal( mKolabPriorityFromDom ) );
  }

  // No priority set, use the default
  else {
    // According the RFC 2445, we should use 0 here, for undefined priority, but AFAIK KOrganizer
    // doesn't support that, so we'll use 5.
    setPriority( 5 );
  }
}

void Task::saveTo( KCal::Todo* task )
{
  Incidence::saveTo( task );

  task->setPriority( priority() );
  task->setPercentComplete( percentCompleted() );
  task->setStatus( status() );
  task->setHasStartDate( hasStartDate() );
  task->setHasDueDate( hasDueDate() );
  if ( hasDueDate() )
    task->setDtDue( utcToLocal( dueDate() ) );

  if ( !parent().isNull() )
    task->setRelatedToUid( parent() );

  if ( hasCompletedDate() && task->percentComplete() == 100 )
    task->setCompleted( utcToLocal( mCompletedDate ) );
}
