/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <kdebug.h>

#include <kglobal.h>
#include <klocale.h>

#include <libkcal/event.h>
//#include <libkcal/imipscheduler.h>
#include <libkcal/dummyscheduler.h>

#ifndef KORG_NOMAIL
#include "mailscheduler.h"
#endif

#include "koprefs.h"
#include "outgoingdialog.h"
#include "koeventviewerdialog.h"

ScheduleItemOut::ScheduleItemOut(QListView *parent,Event *ev,
                                 Scheduler::Method method,
                                 const QString &recipients)
  : QListViewItem(parent)
{
  mEvent = ev;
  mMethod = method;
  mRecipients = recipients;

  setText(0,ev->summary());
  setText(1,ev->dtStartDateStr());
  if (ev->doesFloat()) {
    setText(2,"no time");
    setText(4,"no time");
  }
  else {
    setText(2,ev->dtStartTimeStr());
    if (ev->hasDuration()) {
      setText(4,ev->dtEndTimeStr());
    }
    else {
      setText(4,"no time");
    }
  }
  if (ev->hasEndDate()) {
    setText(3,ev->dtEndDateStr());
  }
  else {
    setText(3,"");
  }
  setText(5,Scheduler::methodName(mMethod));
//  kdDebug() << "OutgoingItem rescipients: " << mRecipients << endl;
//  if (mMethod == Scheduler::Publish) {
//    if (!recipients.isEmpty())
//    setText(6,mRecipients);
//  }
}

OutgoingDialog::OutgoingDialog(Calendar *calendar,QWidget* parent,
                               const char* name,bool modal,
                               WFlags fl)
    : OutgoingDialog_base(parent,name,modal,fl)
{
  mCalendar = calendar;

  if (KOPrefs::instance()->mIMIPScheduler == KOPrefs::IMIPDummy ) {
    kdDebug() << "--- Dummy" << endl;
    mScheduler = new DummyScheduler(mCalendar);
  } else {
#ifndef KORG_NOMAIL
    kdDebug() << "--- Mailer" << endl;
    mScheduler = new MailScheduler(mCalendar);
#else
    mScheduler = new DummyScheduler(mCalendar);
#endif
  }
  mMessageListView->setColumnAlignment(1,AlignHCenter);
  mMessageListView->setColumnAlignment(2,AlignHCenter);
  mMessageListView->setColumnAlignment(3,AlignHCenter);
  mMessageListView->setColumnAlignment(4,AlignHCenter);
  QObject::connect(mMessageListView,SIGNAL(doubleClicked(QListViewItem *)),
                   this,SLOT(showEvent(QListViewItem *)));
}

OutgoingDialog::~OutgoingDialog()
{
}

bool OutgoingDialog::addMessage(Event *incidence,Scheduler::Method method)
{
  if (method == Scheduler::Publish) return false;

  if (KOPrefs::instance()->mIMIPSend == KOPrefs::IMIPOutbox) {
    new ScheduleItemOut(mMessageListView,incidence,method);
    emit numMessagesChanged(mMessageListView->childCount());
  }
  else {
    mScheduler->performTransaction(incidence,method);
  }
  return true;
}

bool OutgoingDialog::addMessage(Event *incidence,Scheduler::Method method,
                                const QString &recipients)
{
  if (method != Scheduler::Publish) return false;

  if (KOPrefs::instance()->mIMIPSend == KOPrefs::IMIPOutbox) {
    new ScheduleItemOut(mMessageListView,incidence,method,recipients);
    emit numMessagesChanged(mMessageListView->childCount());
  }
  else {
    mScheduler->publish(incidence,recipients);
  }
  return true;
}

void OutgoingDialog::send()
{
  ScheduleItemOut *item = (ScheduleItemOut *)(mMessageListView->firstChild());
  while(item) {
    bool success;
    if (item->method() == Scheduler::Publish) {
      success = mScheduler->publish(item->event(),item->recipients());
    } else {
      success = mScheduler->performTransaction(item->event(),item->method());
    }
    ScheduleItemOut *oldItem = item;
    item = (ScheduleItemOut *)(item->nextSibling());
    if (success) delete oldItem;
  }

  emit numMessagesChanged(mMessageListView->childCount());
}

void OutgoingDialog::deleteItem()
{
  ScheduleItemOut *item = (ScheduleItemOut *)(mMessageListView->selectedItem());
  mMessageListView->takeItem(item);
  emit numMessagesChanged(mMessageListView->childCount());
}

void OutgoingDialog::showEvent(QListViewItem *item)
{
  Event *event = ((ScheduleItemOut *)item)->event();
  //ScheduleItemOut *so = (ScheduleItemOut *)item;
  //Event *event;
  if (event) {
    KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
    eventViewer->setEvent(event);
    eventViewer->show();
  }
}

#include "outgoingdialog.moc"
