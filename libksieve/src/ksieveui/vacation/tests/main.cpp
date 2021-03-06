/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <KLocalizedString>
#include <qapplication.h>
#include <QCommandLineParser>
#include <kaboutdata.h>
#include <QStandardPaths>

#include "vacation/multiimapvacationdialog.h"
#include "vacation/multiimapvacationmanager.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    KAboutData aboutData(QStringLiteral("vacationmultiscripttest"),
                         i18n("VacationMultiScriptTest_Gui"),
                         QStringLiteral("1.0"));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    app.setQuitOnLastWindowClosed(true);

    KSieveUi::MultiImapVacationManager manager;
    KSieveUi::MultiImapVacationDialog dlg(&manager);

    dlg.show();
    app.exec();
    return 0;
}
