/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "searchandmergecontactduplicatecontactdialogtest.h"

#include "../searchduplicate/searchandmergecontactduplicatecontactdialog.h"

#include <qtest.h>
#include <QStandardPaths>

#include <QStackedWidget>
using namespace KABMergeContacts;

SearchAndMergeContactDuplicateContactDialogTest::SearchAndMergeContactDuplicateContactDialogTest(QObject *parent)
    : QObject(parent)
{
}

void SearchAndMergeContactDuplicateContactDialogTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void SearchAndMergeContactDuplicateContactDialogTest::shouldHaveDefaultValueOnCreation()
{
    SearchAndMergeContactDuplicateContactDialog dlg;
    dlg.show();
    QStackedWidget *stackedWidget = dlg.findChild<QStackedWidget *>(QStringLiteral("stackedwidget"));
    QVERIFY(stackedWidget);
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QStringLiteral("nocontactselected"));

    for (int i = 0; i < stackedWidget->count(); ++i) {
        QWidget *w = stackedWidget->widget(i);
        const QString objName = w->objectName();
        const bool hasGoodNamePage = (objName == QLatin1String("mergecontact") ||
                                      objName == QLatin1String("nocontactselected") ||
                                      objName == QLatin1String("nocontactduplicatesfound") ||
                                      objName == QLatin1String("noenoughcontactselected") ||
                                      objName == QLatin1String("mergecontactresult") ||
                                      objName == QLatin1String("selectioninformation"));
        QVERIFY(hasGoodNamePage);
    }

}

void SearchAndMergeContactDuplicateContactDialogTest::shouldShowNoEnoughPageWhenSelectOneContact()
{
    SearchAndMergeContactDuplicateContactDialog dlg;
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    dlg.searchPotentialDuplicateContacts(lst);
    dlg.show();
    QStackedWidget *stackedWidget = dlg.findChild<QStackedWidget *>(QStringLiteral("stackedwidget"));
    QVERIFY(stackedWidget);
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QStringLiteral("noenoughcontactselected"));
}

void SearchAndMergeContactDuplicateContactDialogTest::shouldShowNoContactWhenListIsEmpty()
{
    SearchAndMergeContactDuplicateContactDialog dlg;
    Akonadi::Item::List lst;
    dlg.searchPotentialDuplicateContacts(lst);
    dlg.show();
    QStackedWidget *stackedWidget = dlg.findChild<QStackedWidget *>(QStringLiteral("stackedwidget"));
    QVERIFY(stackedWidget);
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QStringLiteral("nocontactselected"));
}

QTEST_MAIN(SearchAndMergeContactDuplicateContactDialogTest)
