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

#include "imagescaling_gui.h"
#include "MessageComposer/ImageScalingWidget"

#include <KLocalizedString>
#include <QHBoxLayout>

#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <QStandardPaths>

ImageScalingTestWidget::ImageScalingTestWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->addWidget(new MessageComposer::ImageScalingWidget);
    setLayout(lay);
}

ImageScalingTestWidget::~ImageScalingTestWidget()
{
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    KAboutData aboutData(QStringLiteral("imagescaling_gui"), i18n("ImageScalingTest_Gui"), QStringLiteral("1.0"));
    aboutData.setShortDescription(i18n("Test for imagescaling widget"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    ImageScalingTestWidget *w = new ImageScalingTestWidget();
    w->resize(800, 600);

    w->show();
    app.exec();
    delete w;
    return 0;
}

