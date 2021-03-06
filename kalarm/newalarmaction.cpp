/*
 *  newalarmaction.cpp  -  menu action to select a new alarm type
 *  Program:  kalarm
 *  Copyright © 2007-2009,2011 by David Jarvie <djarvie@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kalarm.h"
#include "newalarmaction.h"

#include "akonadimodel.h"
#include "collectionmodel.h"
#include "itemlistmodel.h"
#include "functions.h"
#include "shellprocess.h"
#include "templatemenuaction.h"

#include <kactionmenu.h>
#include <KLocalizedString>
#include <kstandardshortcut.h>
#include <QMenu>
#include "kalarm_debug.h"

using namespace KAlarmCal;

#define DISP_ICON     QStringLiteral("window-new")
#define CMD_ICON      QStringLiteral("new-command-alarm")
#define MAIL_ICON     QStringLiteral("mail-message-new")
#define AUDIO_ICON    QStringLiteral("new-audio-alarm")
#define TEMPLATE_ICON QStringLiteral("document-new-from-template")
#define DISP_KEY      QKeySequence(Qt::CTRL + Qt::Key_D)
#define CMD_KEY       QKeySequence(Qt::CTRL + Qt::Key_C)
#define MAIL_KEY      QKeySequence(Qt::CTRL + Qt::Key_M)
#define AUDIO_KEY     QKeySequence(Qt::CTRL + Qt::Key_U)


NewAlarmAction::NewAlarmAction(bool templates, const QString& label, QObject* parent)
    : KActionMenu(QIcon::fromTheme(QStringLiteral("document-new")), label, parent),
      mTemplateAction(Q_NULLPTR)
{
    mDisplayAction = new QAction(QIcon::fromTheme(DISP_ICON), (templates ? i18nc("@item:inmenu", "&Display Alarm Template") : i18nc("@action", "New Display Alarm")), parent);
    menu()->addAction(mDisplayAction);
    mTypes[mDisplayAction] = EditAlarmDlg::DISPLAY;
    mCommandAction = new QAction(QIcon::fromTheme(CMD_ICON), (templates ? i18nc("@item:inmenu", "&Command Alarm Template") : i18nc("@action", "New Command Alarm")), parent);
    menu()->addAction(mCommandAction);
    mTypes[mCommandAction] = EditAlarmDlg::COMMAND;
    mEmailAction = new QAction(QIcon::fromTheme(MAIL_ICON), (templates ? i18nc("@item:inmenu", "&Email Alarm Template") : i18nc("@action", "New Email Alarm")), parent);
    menu()->addAction(mEmailAction);
    mTypes[mEmailAction] = EditAlarmDlg::EMAIL;
    mAudioAction = new QAction(QIcon::fromTheme(AUDIO_ICON), (templates ? i18nc("@item:inmenu", "&Audio Alarm Template") : i18nc("@action", "New Audio Alarm")), parent);
    menu()->addAction(mAudioAction);
    mTypes[mAudioAction] = EditAlarmDlg::AUDIO;
    if (!templates)
    {
        mDisplayAction->setShortcut(DISP_KEY);
        mCommandAction->setShortcut(CMD_KEY);
        mEmailAction->setShortcut(MAIL_KEY);
        mAudioAction->setShortcut(AUDIO_KEY);

        // Include New From Template only in non-template menu
        mTemplateAction = new TemplateMenuAction(QIcon::fromTheme(TEMPLATE_ICON), i18nc("@action", "New Alarm From &Template"), parent);
        menu()->addAction(mTemplateAction);
        connect(AkonadiModel::instance(), &AkonadiModel::collectionStatusChanged, this, &NewAlarmAction::slotCalendarStatusChanged);
        connect(TemplateListModel::all(), &ItemListModel::haveEventsStatus, this, &NewAlarmAction::slotCalendarStatusChanged);
        slotCalendarStatusChanged();   // initialise action states
    }
    setDelayed(false);
    connect(menu(), &QMenu::aboutToShow, this, &NewAlarmAction::slotInitMenu);
    connect(menu(), &QMenu::triggered, this, &NewAlarmAction::slotSelected);
}

/******************************************************************************
* Called when the action is clicked.
*/
void NewAlarmAction::slotInitMenu()
{
    // Don't allow shell commands in kiosk mode
    mCommandAction->setEnabled(ShellProcess::authorised());
}

/******************************************************************************
* Called when an alarm type is selected from the New popup menu.
*/
void NewAlarmAction::slotSelected(QAction* action)
{
    QMap<QAction*, EditAlarmDlg::Type>::ConstIterator it = mTypes.constFind(action);
    if (it != mTypes.constEnd())
        Q_EMIT selected(it.value());
}

/******************************************************************************
* Called when the status of a calendar has changed.
* Enable or disable the New From Template action appropriately.
*/
void NewAlarmAction::slotCalendarStatusChanged()
{
    // Find whether there are any writable active alarm calendars
    bool active = !CollectionControlModel::enabledCollections(CalEvent::ACTIVE, true).isEmpty();
    bool haveEvents = TemplateListModel::all()->haveEvents();
    mTemplateAction->setEnabled(active && haveEvents);
    setEnabled(active);
}

// vim: et sw=4:
