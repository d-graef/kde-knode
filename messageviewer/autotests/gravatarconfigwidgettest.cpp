/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "gravatarconfigwidgettest.h"
#include "../widgets/gravatarconfigwidget.h"
#include <QCheckBox>
#include <qtest.h>
#include <QPushButton>

GravatarConfigWidgetTest::GravatarConfigWidgetTest(QObject *parent)
    : QObject(parent)
{

}

GravatarConfigWidgetTest::~GravatarConfigWidgetTest()
{

}

void GravatarConfigWidgetTest::shouldHaveDefaultValue()
{
    MessageViewer::GravatarConfigWidget w;
    QCheckBox *checkBox = w.findChild<QCheckBox *>(QStringLiteral("gravatarcheckbox"));
    QVERIFY(checkBox);

    QCheckBox *useDefaultImage = w.findChild<QCheckBox *>(QLatin1String("usedefaultimage"));
    QVERIFY(useDefaultImage);

    QPushButton *clearGravatarCache = qFindChild<QPushButton *>(&w, QLatin1String("cleargravatarcachebutton"));
    QVERIFY(clearGravatarCache);
}

QTEST_MAIN(GravatarConfigWidgetTest)