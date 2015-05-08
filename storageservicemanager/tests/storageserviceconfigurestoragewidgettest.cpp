/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "storageserviceconfigurestoragewidgettest.h"
#include "../storageserviceconfigurestoragewidget.h"
#include <QPushButton>
#include <qtest_kde.h>
#include <qtestmouse.h>
#include <QSignalSpy>
StorageServiceConfigureStorageWidgetTest::StorageServiceConfigureStorageWidgetTest(QObject *parent)
    : QObject(parent)
{

}

StorageServiceConfigureStorageWidgetTest::~StorageServiceConfigureStorageWidgetTest()
{

}

void StorageServiceConfigureStorageWidgetTest::shouldHaveDefaultValue()
{
    StorageServiceConfigureStorageWidget w;
    QPushButton *button = qFindChild<QPushButton*>(&w, QLatin1String("configure_button"));
    QVERIFY(button);
}

void StorageServiceConfigureStorageWidgetTest::shouldEmitSignalWhenClickOnButton()
{
    StorageServiceConfigureStorageWidget w;
    QPushButton *button = qFindChild<QPushButton*>(&w, QLatin1String("configure_button"));
    QSignalSpy spy(&w, SIGNAL(configureClicked()));
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

QTEST_KDEMAIN(StorageServiceConfigureStorageWidgetTest, GUI)