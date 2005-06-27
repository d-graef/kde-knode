/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#include <qstringlist.h>

#include <dcopref.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kuniqueapplication.h>

#include "aboutdata.h"
#include "mainwindow.h"
#include "akregator_options.h"

class AkregatorApp : public KUniqueApplication {
  public:
    AkregatorApp() : mMainWindow( 0 ) {}
    ~AkregatorApp() {}

    int newInstance();

  private:
    Akregator::MainWindow *mMainWindow;
};

int AkregatorApp::newInstance()
{
  DCOPRef akr("akregator", "AkregatorIface");

  if ( isRestored() ) {
    if ( KMainWindow::canBeRestored( 1 ) ) {
      mMainWindow = new Akregator::MainWindow();
      setMainWidget( mMainWindow );
      mMainWindow->show();
      mMainWindow->restore( 1 );
    }
  } else {
    if ( !mMainWindow ) {
      mMainWindow = new Akregator::MainWindow();
      setMainWidget( mMainWindow );
      mMainWindow->loadPart();
      mMainWindow->setupProgressWidgets();
      mMainWindow->show();
      akr.send("openStandardFeedList");
    }

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QString addFeedGroup = !args->getOption("group").isEmpty() ? args->getOption("group") : i18n("Imported Folder");

    QCStringList feeds = args->getOptionList("addfeed");
    QStringList feedsToAdd;
    for (QCStringList::ConstIterator it = feeds.begin(); it != feeds.end(); ++it)
        feedsToAdd.append(*it);

    if (!feedsToAdd.isEmpty())
        akr.send("addFeedsToGroup", feedsToAdd, addFeedGroup );

    args->clear();
  }

  return KUniqueApplication::newInstance();
} 

int main(int argc, char **argv)
{
    Akregator::AboutData about;
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( Akregator::akregator_options );
    KUniqueApplication::addCmdLineOptions();

    // start knotifyclient if not already started. makes it work for people who doesn't use full kde, according to kmail devels
    KNotifyClient::startDaemon();

    if(!AkregatorApp::start())
      return 0;

    AkregatorApp app;

    bool ret = app.exec();
    while ( KMainWindow::memberList->first() )
        delete KMainWindow::memberList->first();

    return ret;
}
