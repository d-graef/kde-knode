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

#include "unitcomboboxtest.h"
#include "../widgets/unitcombobox.h"
#include <qtest.h>

UnitComboBoxTest::UnitComboBoxTest(QObject *parent)
    : QObject(parent)
{

}

UnitComboBoxTest::~UnitComboBoxTest()
{

}

void UnitComboBoxTest::shouldHaveDefaultValue()
{
    UnitComboBox combo;
    QCOMPARE(combo.count(), 4);
}

void UnitComboBoxTest::changeCurrentItem_data()
{
    QTest::addColumn<int>("input");
    QTest::addColumn<int>("output");
    QTest::newRow("first") <<  0 << 0;
    QTest::newRow("second") <<  1 << 1;
    QTest::newRow("third") <<  2 << 2;
    QTest::newRow("fourth") <<  3 << 3;
    QTest::newRow("invalid") <<  5 << 0;
}

void UnitComboBoxTest::changeCurrentItem()
{
    QFETCH(int, input);
    QFETCH(int, output);
    UnitComboBox combo;
    combo.setUnit(static_cast<ArchiveMailInfo::ArchiveUnit>(input));
    QCOMPARE(combo.unit(), static_cast<ArchiveMailInfo::ArchiveUnit>(output));
}

QTEST_MAIN(UnitComboBoxTest)
