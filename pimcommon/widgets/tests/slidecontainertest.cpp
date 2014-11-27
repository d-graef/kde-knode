// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2011 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
// Self
#include "slidecontainertest.h"

// Local
#include "pimcommon/widgets/slidecontainer.h"

// KDE
#include <qtest_kde.h>
#include <KDebug>

// Qt
#include <QTextEdit>
#include <QVBoxLayout>



using namespace PimCommon;

struct TestWindow : public QWidget
{
    explicit TestWindow(QWidget* parent = 0)
        : QWidget(parent)
        , mContainer(new SlideContainer)
        , mContent(0) {
        createContent();

        mMainWidget = new QTextEdit();
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setMargin(0);
        layout->addWidget(mMainWidget);
        layout->addWidget(mContainer);
    }

    void createContent()
    {
        mContent = new QTextEdit;
        mContent->setFixedSize(100, 40);
        mContainer->setContent(mContent);
    }

    SlideContainer* mContainer;
    QWidget* mMainWidget;
    QWidget* mContent;
};

void SlideContainerAutoTest::testInit()
{
    // Even with content, a SlideContainer should be invisible until slideIn()
    // is called
    TestWindow window;
    window.show();

    QTest::qWait(500);
    QCOMPARE(window.mMainWidget->height(), window.height());
}

void SlideContainerAutoTest::testSlideIn()
{
    TestWindow window;
    QSignalSpy inSpy(window.mContainer, SIGNAL(slidedIn()));
    QSignalSpy outSpy(window.mContainer, SIGNAL(slidedOut()));
    window.show();

    window.mContainer->slideIn();
    while (window.mContainer->slideHeight() != window.mContent->height()) {
        QTest::qWait(100);
    }
    QCOMPARE(window.mContainer->height(), window.mContent->height());
    QCOMPARE(inSpy.count(), 1);
    QCOMPARE(outSpy.count(), 0);
}

void SlideContainerAutoTest::testSlideOut()
{
    TestWindow window;
    window.show();

    window.mContainer->slideIn();
    while (window.mContainer->slideHeight() != window.mContent->height()) {
        QTest::qWait(100);
    }

    QSignalSpy inSpy(window.mContainer, SIGNAL(slidedIn()));
    QSignalSpy outSpy(window.mContainer, SIGNAL(slidedOut()));
    window.mContainer->slideOut();
    while (window.mContainer->slideHeight() != 0) {
        QTest::qWait(100);
    }
    QCOMPARE(window.mContainer->height(), 0);
    QCOMPARE(inSpy.count(), 0);
    QCOMPARE(outSpy.count(), 1);
}

void SlideContainerAutoTest::testSlideInDeleteSlideOut()
{
    // If content is deleted while visible, slideOut() should still produce an
    // animation
    TestWindow window;
    window.show();

    window.mContainer->slideIn();
    while (window.mContainer->slideHeight() != window.mContent->height()) {
        QTest::qWait(100);
    }
    window.mContent->deleteLater();
    window.mContainer->slideOut();
    while (window.mContainer->slideHeight() != 0) {
        QTest::qWait(100);
    }
    QCOMPARE(window.mContainer->height(), 0);
}

void SlideContainerAutoTest::testHiddenContentResize()
{
    // Resizing content should not trigger a slide if it is not visible.
    TestWindow window;
    window.show();
    QTest::qWaitForWindowShown(&window);

    window.mContent->show();
    window.mContent->setFixedSize(150, 80);
    QTest::qWait(500);
    QCOMPARE(window.mContainer->height(), 0);
}

QTEST_KDEMAIN(SlideContainerAutoTest, GUI)