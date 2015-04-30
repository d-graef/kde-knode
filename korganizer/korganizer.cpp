/*
  This file is part of KOrganizer.

  Copyright (c) 1997, 1998, 1999  Preston Brown <preston.brown@yale.edu>
  Fester Zigterman <F.J.F.ZigtermanRustenburg@student.utwente.nl>
  Ian Dawes <iadawes@globalserve.net>
  Laszlo Boloni <boloni@cs.purdue.edu>

  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "korganizer.h"
#include "actionmanager.h"
#include "calendarview.h"
#include "kocore.h"
#include "koglobals.h"
#include "impl/korganizerifaceimpl.h"

#include "libkdepim/progresswidget/progressstatusbarwidget.h"
#include "libkdepim/progresswidget/statusbarprogresswidget.h"

#include <KActionCollection>
#include "korganizer_debug.h"
#include <KShortcutsDialog>
#include <KStandardAction>
#include <QLabel>
#include <QStatusBar>

KOrganizer::KOrganizer() : KParts::MainWindow(), KOrg::MainWindow()
{
    // Set this to be the group leader for all subdialogs - this means
    // modal subdialogs will only affect this dialog, not the other windows
    setAttribute(Qt::WA_GroupLeader);

    qCDebug(KORGANIZER_LOG);
    KOCore::self()->addXMLGUIClient(this, this);
//  setMinimumSize(600,400);  // make sure we don't get resized too small...

    mCalendarView = new CalendarView(this);
    mCalendarView->setObjectName(QLatin1String("KOrganizer::CalendarView"));
    setCentralWidget(mCalendarView);

    mActionManager = new ActionManager(this, mCalendarView, this, this, false, menuBar());
    (void)new KOrganizerIfaceImpl(mActionManager, this, "IfaceImpl");
}

KOrganizer::~KOrganizer()
{
    delete mActionManager;

    KOCore::self()->removeXMLGUIClient(this);
}

void KOrganizer::init(bool document)
{
    setHasDocument(document);

    setComponentData(KComponentData::mainComponent());

    // Create calendar object, which manages all calendar information associated
    // with this calendar view window.
    mActionManager->createCalendarAkonadi();

    mActionManager->init();
    /*connect( mActionManager, SIGNAL(actionNewMainWindow(QUrl)),
             SLOT(newMainWindow(QUrl)) );*/

    mActionManager->loadParts();

    initActions();
    readSettings();

    QStatusBar *bar = statusBar();

    bar->addWidget(new QLabel(this));

    KPIM::ProgressStatusBarWidget *progressBar = new KPIM::ProgressStatusBarWidget(statusBar(), this);

    bar->addPermanentWidget(progressBar->littleProgress());

    connect(mActionManager->view(), &CalendarView::statusMessage,
            this, &KOrganizer::showStatusMessage);

    setStandardToolBarMenuEnabled(true);
    setTitle();
}

void KOrganizer::newMainWindow(const QUrl &url)
{
    KOrganizer *korg = new KOrganizer();
    if (url.isValid() || url.isEmpty()) {
        korg->init(true);
        if (mActionManager->importURL(url, false) || url.isEmpty()) {
            korg->show();
        } else {
            delete korg;
        }
    } else {
        korg->init(false);
        korg->show();
    }
}

void KOrganizer::readSettings()
{
    // read settings from the KConfig, supplying reasonable
    // defaults where none are to be found

    KConfig *config = KOGlobals::self()->config();

    mActionManager->readSettings();

    config->sync();
}

void KOrganizer::writeSettings()
{
    qCDebug(KORGANIZER_LOG);

    KConfig *config = KOGlobals::self()->config();

    mActionManager->writeSettings();
    config->sync();
}

void KOrganizer::initActions()
{
    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();

    KStandardAction::keyBindings(this, SLOT(slotEditKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());
    KStandardAction::quit(this, SLOT(close()), actionCollection());

    setXMLFile(QLatin1String("korganizerui.rc"), true);
    createGUI(Q_NULLPTR);

    setAutoSaveSettings();
}

void KOrganizer::slotEditKeys()
{
    KShortcutsDialog::configure(actionCollection(),
                                KShortcutsEditor::LetterShortcutsAllowed);
}

bool KOrganizer::queryClose()
{
    qCDebug(KORGANIZER_LOG);

    bool close = mActionManager->queryClose();

    // Write configuration. I don't know if it really makes sense doing it this
    // way, when having opened multiple calendars in different CalendarViews.
    if (close) {
        writeSettings();
    }

    return close;
}

void KOrganizer::showStatusMessage(const QString &message)
{
    statusBar()->showMessage(message, 2000);
}

bool KOrganizer::openURL(const QUrl &url, bool merge)
{
    return mActionManager->importURL(url, merge);
}

bool KOrganizer::saveURL()
{
    return mActionManager->saveURL();
}

bool KOrganizer::saveAsURL(const QUrl &kurl)
{
    return mActionManager->saveAsURL(kurl)  ;
}

QUrl KOrganizer::getCurrentURL() const
{
    return mActionManager->url();
}

void KOrganizer::saveProperties(KConfigGroup &config)
{
    return mActionManager->saveProperties(config);
}

void KOrganizer::readProperties(const KConfigGroup &config)
{
    return mActionManager->readProperties(config);
}

KOrg::CalendarViewBase *KOrganizer::view() const
{
    return mActionManager->view();
}

void KOrganizer::setTitle()
{
    QString title;
    if (!hasDocument()) {
        title = i18n("Calendar");
    } else {
        QUrl url = mActionManager->url();

        if (!url.isEmpty()) {
            if (url.isLocalFile()) {
                title = url.fileName();
            } else {
                title = url.toDisplayString();
            }
        } else {
            title = i18n("New Calendar");
        }

        if (mCalendarView->isReadOnly()) {
            title += QLatin1String(" [") + i18nc("the calendar is read-only", "read-only") + QLatin1Char(']');
        }
    }
    if (mCalendarView->isFiltered()) {
        title += QLatin1String(" - <") + mCalendarView->currentFilterName() + QLatin1String("> ");
    }

    setCaption(title, false);
}

