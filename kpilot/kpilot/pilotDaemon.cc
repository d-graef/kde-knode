/* pilotDaemon.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the KPilot Daemon, which does the actual communication with
** the Pilot and with the conduits.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
static const char *id="$Id$";

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <time.h>

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#ifndef QDIR_H
#include <qdir.h>
#endif
#ifndef QLIST_H
#include <qlist.h>
#endif
#ifndef QCURSOR_H
#include <qcursor.h>
#endif
#ifndef QDRAGOBJECT_H
#include <qdragobject.h>
#endif

#ifndef _KUNIQUEAPP_H
#include <kuniqueapp.h>
#endif
#ifndef _KABOUTDATA_H
#include <kaboutdata.h>
#endif
#ifndef _KABOUTAPPLICATION_H
#include <kaboutapplication.h>
#endif
#ifndef _KCMDLINEARGS_H
#include <kcmdlineargs.h>
#endif
#ifndef _KWIN_H
#include <kwin.h>
#endif
#ifndef _KSIMPLECONFIG_H
#include <ksimpleconfig.h>
#endif
#ifndef _KURL_H
#include <kurl.h>
#endif
#ifndef _KSOCK_H
#include <ksock.h>
#endif
#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif
#ifndef _KSTDDIRS_H
#include <kstddirs.h>
#endif
#ifndef _KPOPUPMENU_H
#include <kpopupmenu.h>
#endif
#ifndef _KICONLOADER_H
#include <kiconloader.h>
#endif
#ifndef _KIO_NETACCESS_H
#include <kio/netaccess.h>
#endif
#ifndef _KDEBUG_H
#include <kdebug.h>
#endif
#ifndef _KPROCESS_H
#include <kprocess.h>
#endif
#ifndef _DCOPCLIENT_H
#include <dcopclient.h>
#endif

// These two are XPMs disguised as .h files
#include "hotsync.h"
#include "busysync.h"


#ifndef _KPILOT_STATUSMESSAGES_H
#include "statusMessages.h"
#endif
#ifndef _KPILOT_FILEINSTALLER_H
#include "fileInstaller.h"
#endif
#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif
#ifndef _KPILOT_PILOTRECORD_H
#include "pilotRecord.h"
#endif

#include "hotSync.h"

#include "kpilotDCOP_stub.h"

#include "pilotDaemon.moc"


PilotDaemonTray::PilotDaemonTray(PilotDaemon *p) :
	KSystemTray(0,"pilotDaemon"), 
	daemon(p),
	kap(0L),
	fInstaller(0L)
{
	FUNCTIONSETUP;
	setupWidget();
	setAcceptDrops(true);

	fInstaller = new FileInstaller;
	connect(fInstaller,SIGNAL(filesChanged()),
		p,SLOT(slotFilesChanged()));

	/* NOTREACHED */
	(void) id;
}

/* virtual */ void PilotDaemonTray::dragEnterEvent(QDragEnterEvent *e)
{
	e->accept(QUriDrag::canDecode(e));
}

/* virtual */ void PilotDaemonTray::dropEvent(QDropEvent *e)
{
	FUNCTIONSETUP;

	QStrList list;
	QUriDrag::decode(e, list);

	fInstaller->addFiles(list);
}

/* virtual */ void PilotDaemonTray::mousePressEvent(QMouseEvent *e)
{
	FUNCTIONSETUP;

	if( e->button() == RightButton)
	{
		KPopupMenu *menu = contextMenu();
		contextMenuAboutToShow( menu );
		menu->popup( e->globalPos() );
	}
	else
	{
		KSystemTray::mousePressEvent(e);
	}
}

/* virtual */ void PilotDaemonTray::closeEvent(QCloseEvent *)
{
	FUNCTIONSETUP;
	daemon->quitImmediately();
}

void
PilotDaemonTray::setupWidget()
{
	FUNCTIONSETUP;

	KGlobal::iconLoader()->addAppDir("kpilot");
	icon = KGlobal::iconLoader()->loadIcon("hotsync",KIcon::Toolbar,
		0,KIcon::DefaultState,0,
		true);
	busyicon = KGlobal::iconLoader()->loadIcon("busysync",KIcon::Toolbar,
		0,KIcon::DefaultState,0,true);
	if (icon.isNull())
	{
		DEBUGDAEMON << fname 
			<< ": HotSync icon not found."
			<< endl;
		icon=QPixmap(hotsync_icon);
	}
	if (busyicon.isNull())
	{
		DEBUGDAEMON << fname 
			<< ": HotSync-Busy icon not found."
			<< endl;
		busyicon=QPixmap(busysync_icon);
	}

	changeIcon(Normal);

	KPopupMenu* menu = contextMenu();
	menu->insertItem(i18n("&About"), this, SLOT(slotShowAbout()));
	menuKPilotItem=menu->insertItem(i18n("Start &KPilot"),daemon,
		SLOT(slotRunKPilot()));

	DEBUGDAEMON << fname
		<< ": Finished getting icons"
		<< endl;
}

void PilotDaemonTray::slotShowAbout()
{
	FUNCTIONSETUP;
  
	if (!kap)
	{
		kap=new KAboutApplication(0,"kpdab",false);
	}

	kap->show();
}


void PilotDaemonTray::enableRunKPilot(bool b)
{
	contextMenu()->setItemEnabled(menuKPilotItem,b);
}


void PilotDaemonTray::changeIcon(IconShape i)
{
	FUNCTIONSETUP;

	switch(i)
	{
	case Normal:
		if (icon.isNull())
		{
			kdWarning() << __FUNCTION__
				<< ": Regular icon is NULL!"
				<< endl;
		}
		setPixmap(icon);
		break;
	case Busy:
		if (busyicon.isNull())
		{
			kdWarning() << __FUNCTION__
				<< ": Busy icon is NULL!"
				<< endl;
		}
		setPixmap(busyicon);
		break;
	default :
		kdWarning() << __FUNCTION__ 
			<< ": Bad icon number "
			<< (int)i
			<< endl;
	}
}









PilotDaemon::PilotDaemon() : 
	DCOPObject("KPilotDaemonIface"),
	fStatus(INIT),
	fQuit(false), 
	fPilotDevice(QString::null),
	fPilotLink(0L),
	tray(0L),
	fKPilotStub(new KPilotDCOP_stub("kpilot","KPilotIface"))
{
	FUNCTIONSETUP;

	setupPilotLink();
	reloadSettings();

	if (fStatus == ERROR)
	{
		kdWarning() << __FUNCTION__
			<< ": Connecting to device failed."
			<< endl;
		return;
	}


	DEBUGDAEMON << fname
		<< ": The daemon is ready with status "
		<< (int)fStatus
		<< endl;
}

PilotDaemon::~PilotDaemon()
{
	FUNCTIONSETUP;

	delete fPilotLink;
}

int PilotDaemon::getPilotSpeed(KConfig& config)
{
	FUNCTIONSETUP;

	int speed = config.readNumEntry("PilotSpeed", 0);

	// Translate the speed entry in the
	// config file to something we can
	// put in the environment (for who?)
	//
	//
	const char *speedname=0L;

	switch(speed)
	{
	case 0:
		speedname="PILOTRATE=9600";
		break;
	case 1:
		speedname="PILOTRATE=19200";
		break;
	case 2:
		speedname="PILOTRATE=38400";
		break;
	case 3:
		speedname="PILOTRATE=57600";
		break;
	case 4:
		speedname="PILOTRATE=115200";
		break;
	default :
		speedname="PILOTRATE=9600";
	}

	DEBUGDAEMON << fname
		<< ": Speed set to "
		<< speedname << " ("
		<< speed << ")"
		<< endl;

	putenv((char *)speedname);

	return speed;
}


void PilotDaemon::showTray()
{
	FUNCTIONSETUP;

	if (!tray) 
	{
		DEBUGDAEMON << fname
			<< ": No tray icon to display!"
			<< endl;

		return;
	}

	// Copied from Klipper
	KWin::setSystemTrayWindowFor( tray->winId(), 0 );
	tray->setGeometry(-100, -100, 42, 42 );
	tray->show();

	DEBUGDAEMON << fname
		<< ": Tray icon displayed."
		<< endl;
}

/* DCOP ASYNC */ void
PilotDaemon::reloadSettings()
{
	FUNCTIONSETUP;

	KConfig& config = KPilotConfig::getConfig();
  
	getPilotSpeed(config);

	fPilotDevice = config.readEntry("PilotDevice");
	fPilotType = (KPilotDeviceLink::DeviceType) 
		config.readNumEntry("PilotType", KPilotDeviceLink::None);

	if (fPilotLink)
	{
		fPilotLink->reset(fPilotType,fPilotDevice);
	}

	if (config.readBoolEntry("DockDaemon",false))
	{
		if (!tray)
		{
			tray = new PilotDaemonTray(this);
			tray->show();
		}
	}
	else
	{
		if (tray)
		{
			tray->hide();
			delete tray;
			tray=0L;
		}
	}
}

/* DCOP */ QString
PilotDaemon::statusString()
{
	FUNCTIONSETUP;

	QString s("PilotDaemon=");

	switch(status())
	{
	case INIT : s.append(QString("Initializing")); break;
	case READY : s.append(QString("Found device")); break;
	case ERROR : s.append(QString("Error")); break;
	case FILE_INSTALL_REQ : s.append(QString("Installing File")); break;
	case HOTSYNC_END : s.append(QString("End of Hotsync")); break;
	case HOTSYNC_START : s.append(QString("Syncing")); break;
	}

	s.append(" (");
	if (fPilotLink)
	{
		s.append(fPilotLink->statusString());
	}
	s.append(")");

	return s;
}



bool PilotDaemon::setupPilotLink()
{
	FUNCTIONSETUP;

	if (fPilotLink)
	{
		delete fPilotLink;
		fPilotLink=0;
	}

	fPilotLink = KPilotHotSyncLink::getLink();
	if (!fPilotLink)
	{
		kdWarning() << __FUNCTION__ 
			<< ": Can't get pilot link."
			<< endl;
		return false;
	}

	connect(fPilotLink, SIGNAL(deviceReady()),
		this, SLOT(startHotSync()));
	connect(fPilotLink, SIGNAL(databaseSyncComplete()),
		this, SLOT(slotDBBackupFinished()));
	return true;
}


void PilotDaemon::quitImmediately()
{
	FUNCTIONSETUP;
	quit(true);
	kapp->quit();
} 
/* DCOP ASYNC */ void
PilotDaemon::quitNow()
{
	// Using switch to make sure we cover all the cases.
	//
	//
	switch(fStatus)
	{
	case INIT :
	case READY :
	case HOTSYNC_END :
	case ERROR : 
		quitImmediately();
		break;
	case HOTSYNC_START :
	case FILE_INSTALL_REQ :
		quit(true);
		break;
	}
}
 
/* DCOP ASYNC */ void
PilotDaemon::requestRegularSyncNext()
{
	// TODO: do something sensible here.
	startHotSync(1);
}

/* DCOP ASYNC */ void
PilotDaemon::requestFastSyncNext()
{
	// TODO: do something sensible here.
	startHotSync(2);
}


/* DCOP ASYNC */ void
PilotDaemon::startHotSync(int mode)
{
	FUNCTIONSETUP;

	switch(mode)
	{
	case 1:
		DEBUGDAEMON << fname
			<< ": Starting a normal HotSync"
			<< endl;
		break;
	case 2: 
		DEBUGDAEMON << fname
			<< ": Starting a FastSync"
			<< endl;
		break;
	case 3:
		DEBUGDAEMON << fname
			<< ": Starting a full Backup"
			<< endl;
		break;
	default :
		kdWarning() << __FUNCTION__
			<< ": Unknown mode "
			<< mode
			<< " passed to startHotSync()"
			<< endl;
		return;
	}

	kdWarning() << __FUNCTION__
		<< ": Unimplemented right now."
		<< endl;
}

void
PilotDaemon::startHotSync()
{
	FUNCTIONSETUP;


	if (tray) 
	{
		DEBUGKPILOT << fname
			<< ": Changing tray icon."
			<< endl;

		tray->changeIcon(PilotDaemonTray::Busy);
	}

	// FIXME
	// getPilotLink().startHotSync();
	getKPilot().daemonStatus(
		i18n("Starting HotSync ..."));
}

void
PilotDaemon::slotDBBackupFinished()
{
	FUNCTIONSETUP;

	DEBUGDAEMON << fname << ": DB Syncing finished." << endl;

	KConfig& config = KPilotConfig::getConfig();
	if(config.readBoolEntry("SyncFiles"))
	{
		FileInstaller fi;

		fStatus = FILE_INSTALL_REQ;

		getPilotLink().installFiles(fi.fileNames());
	}

	emit(endHotSync());
}

void
PilotDaemon::slotEndHotSync()
{
	FUNCTIONSETUP;

	if (tray) { tray -> changeIcon(PilotDaemonTray::Normal); } 

	getKPilot().daemonStatus(
		i18n("HotSync Completed."));

	if(!quit())
	{
		fStatus = HOTSYNC_END;
	}
	else
	{
		kapp->quit();
	}
}


void 
PilotDaemon::slotSyncingDatabase(char *dbName)
{
	FUNCTIONSETUP;

	getKPilot().daemonStatus(
		i18n("Syncing database %1 ...").arg(dbName));

	fStatus = HOTSYNC_START;
}


void PilotDaemon::slotFilesChanged()
{
	FUNCTIONSETUP;

	getKPilot().filesChanged();
}

void PilotDaemon::slotRunKPilot()
{
	FUNCTIONSETUP;

	QString kpilotError;
	QCString kpilotDCOP;
	int kpilotPID;

	if (KApplication::startServiceByDesktopPath(
		"Utilities/kpilot",
		QString::null,
		&kpilotError,
		&kpilotDCOP,
		&kpilotPID
#if (KDE_VERSION >= 220)
		// Startup notification added in 2.2
		,
		""
#endif
		))
	{
		kdWarning() << __FUNCTION__
			<< ": Couldn't start KPilot! "
			<< kpilotError
			<< endl;
	}
	else
	{
		DEBUGDAEMON << fname
			<< ": Started KPilot with DCOP name "
			<< kpilotDCOP
			<< " (pid "
			<< kpilotPID
			<< ")"
			<< endl;
	}
}

static KCmdLineOptions kpilotoptions[] =
{
#ifdef DEBUG
	{ "debug <level>", I18N_NOOP("Set debugging level"),"0" },
#endif
	{ 0,0,0 }
} ;


int main(int argc, char* argv[])
{
	FUNCTIONSETUP;

        KAboutData about("kpilotDaemon", 
			I18N_NOOP("KPilot"),
			 KPILOT_VERSION,
                         "KPilot - Hot-sync software for unix\n\n",
                         KAboutData::License_GPL,
                         "(c) 1998-2001, Dan Pilone");
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com",
		"http://www.slac.com/pilone/kpilot_home/");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"adridg@cs.kun.nl",
		"http://www.cs.kun.nl/~adridg/kpilot/");

        KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(kpilotoptions);
	KUniqueApplication::addCmdLineOptions();
#ifdef DEBUG
	KCmdLineArgs *p=KCmdLineArgs::parsedArgs();

	debug_level=atoi(p->getOption("debug"));
	if (debug_level)
	{
		DEBUGDAEMON << fname
			<< ": Debug level set to "
			<< debug_level
			<< endl;
	}
#else
	(void) KCmdLineArgs::parsedArgs();
#endif

	if (!KUniqueApplication::start())
	{
		return 0;
	}
	KUniqueApplication a(true,true);

	// A block just to keep variables local.
	//
	//
	{
	KConfig& c=KPilotConfig::getConfig();
	c.setReadOnly(true);
	int v = c.readNumEntry("Configured",0);

	if (v < KPilotConfig::ConfigurationVersion)
	{
		kdError() << __FUNCTION__ 
			<< ": Is still not configured for use."
			<< endl;
		return 1;
	}

	DEBUGDAEMON << fname
		<< ": Configuration version "
		<< v
		<< endl;
	}


	PilotDaemon *gPilotDaemon = new PilotDaemon();

	if (gPilotDaemon->status()==PilotDaemon::ERROR)
	{
		delete gPilotDaemon;
		gPilotDaemon = 0;
		kdError() << __FUNCTION__ 
			<< ": **\n"
			   ": Failed to start up daemon\n"
			   ": due to errors constructing it.\n"
			   ": **"
			<< endl;
		return 2;
	}
	
	gPilotDaemon->showTray();

	return a.exec();

	/* NOTREACHED */
	(void) id;
}



// $Log$
// Revision 1.42  2001/08/27 22:54:27  adridg
// Decruftifying; improve DCOP link between daemon & viewer
//
// Revision 1.41  2001/08/19 19:25:57  adridg
// Removed kpilotlink dependency from kpilot; added DCOP interfaces to make that possible. Also fixed a connect() type mismatch that was harmless but annoying.
//
// Revision 1.40  2001/08/01 20:20:57  adridg
// Fix for bug #29764
//
// Revision 1.39  2001/06/11 07:36:10  adridg
// Cleanup char constant in <<
//
// Revision 1.38  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.37  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.36  2001/04/01 17:32:52  adridg
// I really don't remember
//
// Revision 1.35  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.34  2001/03/05 23:44:39  adridg
// KPILOT_VERSION added. Fixed double-sync (maybe). Extra monitor debugging.
//
// Revision 1.33  2001/03/04 21:22:00  adridg
// Added drag 'n drop file install to daemon
//
// Revision 1.32  2001/03/04 11:23:04  adridg
// Changed for bug 21392
//
// Revision 1.31  2001/02/26 22:09:49  adridg
// Fixed some exit() calls; extra listener process debugging
//
// Revision 1.30  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.29  2001/02/08 13:17:19  adridg
// Fixed crash when conduits run during a backup and exit after the
// end of that backup (because the event loop is blocked by the backup
// itself). Added better debugging error exit message (no i18n needed).
//
// Revision 1.28  2001/02/05 21:01:07  adridg
// Fixed copyright headers for source releases. No code changed
//
// Revision 1.27  2001/02/05 19:16:32  adridg
// Removing calls to exit() from internal functions
//
// Revision 1.26  2001/01/06 13:20:23  adridg
// Cleaned up DCOP; changed version number
//
// Revision 1.25  2001/01/04 22:19:37  adridg
// Stuff for Chris and Bug 18072
//
// Revision 1.24  2001/01/04 11:33:20  bero
// Fix build
//
// Revision 1.23  2001/01/03 00:02:45  adridg
// Added Heiko's FastSync
//
// Revision 1.22  2001/01/02 15:02:59  bero
// Fix build
//
// Revision 1.21  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
