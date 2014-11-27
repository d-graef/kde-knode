/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "splittercollapsertest.h"
#include "widgets/splittercollapser.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <QTextEdit>
#include <qtest_kde.h>
#include <qtestmouse.h>

TestSplitter::TestSplitter(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    setLayout(lay);
    splitter = new QSplitter;
    lay->addWidget(splitter);
    splitter->setObjectName(QLatin1String("splitter"));
    edit1 = new QTextEdit;
    edit1->setObjectName(QLatin1String("edit1"));
    splitter->addWidget(edit1);

    edit2 = new QTextEdit;
    edit2->setObjectName(QLatin1String("edit2"));
    splitter->addWidget(edit2);
}

SplitterCollapserTest::SplitterCollapserTest(QObject *parent)
    : QObject(parent)
{

}

SplitterCollapserTest::~SplitterCollapserTest()
{

}

void SplitterCollapserTest::shouldHaveDefaultValue()
{
    TestSplitter testSplitter;

    new PimCommon::SplitterCollapser(testSplitter.edit2, testSplitter.splitter);

    testSplitter.show();
    QTest::qWaitForWindowShown(&testSplitter);
    QVERIFY(testSplitter.isVisible());


    PimCommon::SplitterCollapser *collapser = qFindChild<PimCommon::SplitterCollapser *>(&testSplitter, QLatin1String("splittercollapser"));
    QVERIFY(collapser);
    QVERIFY(!collapser->isCollapsed());

    QTextEdit *edit1 = qFindChild<QTextEdit *>(&testSplitter, QLatin1String("edit1"));
    QVERIFY(edit1);
    QTextEdit *edit2 = qFindChild<QTextEdit *>(&testSplitter, QLatin1String("edit2"));
    QVERIFY(edit2);

    QSplitter *splitter = qFindChild<QSplitter *>(&testSplitter, QLatin1String("splitter"));
    QVERIFY(splitter);
}

void SplitterCollapserTest::shouldCollapseWhenClickOnButton()
{
    TestSplitter testSplitter;
    PimCommon::SplitterCollapser *splitterCollapser = new PimCommon::SplitterCollapser(testSplitter.edit2, testSplitter.splitter);

    testSplitter.show();
    QTest::qWaitForWindowShown(&testSplitter);

    QVERIFY(!splitterCollapser->isCollapsed());
    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isCollapsed());

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isCollapsed());
}

void SplitterCollapserTest::shouldRestoreCorrectPosition()
{
    TestSplitter testSplitter;

    PimCommon::SplitterCollapser *splitterCollapser = new PimCommon::SplitterCollapser(testSplitter.edit2, testSplitter.splitter);

    testSplitter.show();
    QTest::qWaitForWindowShown(&testSplitter);
    QVERIFY(testSplitter.isVisible());

    QVERIFY(!splitterCollapser->isCollapsed());

    QTextEdit *edit2 = qFindChild<QTextEdit *>(&testSplitter, QLatin1String("edit2"));

    int size = edit2->width();

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isCollapsed());
    QCOMPARE(edit2->width(), 0);

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isCollapsed());
    QCOMPARE(edit2->width(), size);

}

void SplitterCollapserTest::shouldRestoreCorrectPositionForFirstWidget()
{
    TestSplitter testSplitter;

    PimCommon::SplitterCollapser *splitterCollapser = new PimCommon::SplitterCollapser(testSplitter.edit1, testSplitter.splitter);

    testSplitter.show();
    QTest::qWaitForWindowShown(&testSplitter);
    QVERIFY(testSplitter.isVisible());

    QVERIFY(!splitterCollapser->isCollapsed());

    QTextEdit *edit1 = qFindChild<QTextEdit *>(&testSplitter, QLatin1String("edit1"));

    int size = edit1->width();

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isCollapsed());
    QCOMPARE(edit1->width(), 0);

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isCollapsed());
    QCOMPARE(edit1->width(), size);
}

void SplitterCollapserTest::shouldTestVerticalSplitterFirstWidget()
{
    TestSplitter testSplitter;
    testSplitter.splitter->setOrientation(Qt::Vertical);
    PimCommon::SplitterCollapser *splitterCollapser = new PimCommon::SplitterCollapser(testSplitter.edit1, testSplitter.splitter);

    testSplitter.show();
    QTest::qWaitForWindowShown(&testSplitter);
    QVERIFY(testSplitter.isVisible());

    QVERIFY(!splitterCollapser->isCollapsed());

    QTextEdit *edit1 = qFindChild<QTextEdit *>(&testSplitter, QLatin1String("edit1"));

    int size = edit1->height();

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isCollapsed());
    QCOMPARE(edit1->height(), 0);

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isCollapsed());
    QCOMPARE(edit1->height(), size);

}

void SplitterCollapserTest::shouldTestVerticalSplitterSecondWidget()
{
    TestSplitter testSplitter;
    testSplitter.splitter->setOrientation(Qt::Vertical);
    PimCommon::SplitterCollapser *splitterCollapser = new PimCommon::SplitterCollapser(testSplitter.edit2, testSplitter.splitter);

    testSplitter.show();
    QTest::qWaitForWindowShown(&testSplitter);
    QVERIFY(testSplitter.isVisible());

    QVERIFY(!splitterCollapser->isCollapsed());

    QTextEdit *edit2 = qFindChild<QTextEdit *>(&testSplitter, QLatin1String("edit2"));

    int size = edit2->height();

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isCollapsed());
    QCOMPARE(edit2->height(), 0);

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isCollapsed());
    QCOMPARE(edit2->height(), size);

}




QTEST_KDEMAIN(SplitterCollapserTest, GUI)