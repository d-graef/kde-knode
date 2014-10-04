/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include <qdebug.h>

#include <QDialog>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include "messageviewer/adblock/adblocksettingwidget.h"

int main(int argc, char **argv)
{
    KAboutData aboutData(QLatin1String("adblocksettings_gui"), i18n("adblocksettingstest_Gui"), QLatin1String("1.0"));
    aboutData.setShortDescription(i18n("Test for adblocksettings"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    MessageViewer::AdBlockSettingWidget *w = new MessageViewer::AdBlockSettingWidget;
    QDialog dlg;
    QVBoxLayout *layout = new QVBoxLayout;
    dlg.setLayout(layout);
    layout->addWidget(w);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttonBox);

    w->doLoadFromGlobalSettings();
    if (dlg.exec()) {
        w->save();
    }
    return 0;
}

