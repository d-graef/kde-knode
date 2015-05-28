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

#include "migrateapplicationfilestest.h"
#include "../migrateapplicationfiles.h"
#include <qtest.h>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>

using namespace PimCommon;
MigrateApplicationFilesTest::MigrateApplicationFilesTest(QObject *parent)
    : QObject(parent)
{

}

MigrateApplicationFilesTest::~MigrateApplicationFilesTest()
{

}

void MigrateApplicationFilesTest::initTestCase()
{
    qDebug() << " QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)" <<QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QStandardPaths::setTestModeEnabled(true);
    const QString applicationHome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    qDebug() << "application data" << applicationHome;
    QDir(applicationHome).removeRecursively();
}

void MigrateApplicationFilesTest::shouldHaveDefaultValue()
{
    MigrateApplicationFiles migrate;
    QVERIFY(!migrate.start());
    QVERIFY(migrate.configFileName().isEmpty());
    QVERIFY(migrate.applicationName().isEmpty());
}

void MigrateApplicationFilesTest::shouldVerifyIfCheckIsNecessary()
{
    MigrateApplicationFiles migrate;
    //Invalid before config file is not set.
    QVERIFY(!migrate.checkIfNecessary());
    migrate.setConfigFileName(QStringLiteral("foorc"));
    // If config file doesn't exist we need to check migrate
    QVERIFY(migrate.checkIfNecessary());
}

void MigrateApplicationFilesTest::shouldNotMigrateIfKdehomeDoNotExist()
{
    qputenv("KDEHOME", "");
    MigrateApplicationFiles migrate;
    migrate.setConfigFileName(QStringLiteral("foorc"));

    QCOMPARE(migrate.start(), false);
}

void MigrateApplicationFilesTest::shouldMigrateIfKde4HomeDirExist()
{
    QTemporaryDir kdehomeDir;
    QVERIFY(kdehomeDir.isValid());
    const QString kdehome = kdehomeDir.path();
    qputenv("KDEHOME", QFile::encodeName(kdehome));
    MigrateApplicationFiles migrate;
    QCOMPARE(migrate.start(), false);

    migrate.setConfigFileName(QStringLiteral("foorc"));

    MigrateFileInfo info;
    info.setPath(QStringLiteral("foo/foo"));
    info.setType(QStringLiteral("apps"));

    migrate.insertMigrateInfo(info);
    QCOMPARE(migrate.start(), true);

}



QTEST_MAIN(MigrateApplicationFilesTest)
