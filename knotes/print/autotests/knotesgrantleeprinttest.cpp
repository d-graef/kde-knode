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


#include "knotesgrantleeprinttest.h"
#include "../knotegrantleeprint.h"
#include <qtest_kde.h>
#include <KMime/Message>
#include <Akonadi/Item>
#include <knotes/print/knoteprintobject.h>
#include "akonadi_next/note.h"

KNotesGrantleePrintTest::KNotesGrantleePrintTest(QObject *parent)
    : QObject(parent)
{

}

KNotesGrantleePrintTest::~KNotesGrantleePrintTest()
{

}

void KNotesGrantleePrintTest::shouldDefaultValue()
{
    KNoteGrantleePrint *grantleePrint = new KNoteGrantleePrint;
    QVERIFY(grantleePrint);
    grantleePrint->deleteLater();
    grantleePrint = 0;
}

void KNotesGrantleePrintTest::shouldReturnEmptyStringWhenNotContentAndNoNotes()
{
    KNoteGrantleePrint *grantleePrint = new KNoteGrantleePrint;
    QList<KNotePrintObject *> lst;
    const QString result = grantleePrint->notesToHtml(lst);
    QVERIFY(result.isEmpty());
    grantleePrint->deleteLater();
    grantleePrint = 0;
}

void KNotesGrantleePrintTest::shouldReturnEmptyStringWhenAddContentWithoutNotes()
{
    KNoteGrantleePrint *grantleePrint = new KNoteGrantleePrint;
    grantleePrint->setContent(QLatin1String("foo"));
    QList<KNotePrintObject *> lst;
    const QString result = grantleePrint->notesToHtml(lst);
    QVERIFY(result.isEmpty());
    grantleePrint->deleteLater();
    grantleePrint = 0;
}

void KNotesGrantleePrintTest::shouldDisplayNoteInfo_data()
{
    QTest::addColumn<QString>("variable");
    QTest::addColumn<QString>("result");
    QTest::newRow("name") << QString(QLatin1String("name")) << QString(QLatin1String("Test Note"));
    QTest::newRow("description") << QString(QLatin1String("description")) << QString(QLatin1String("notes test"));
}

void KNotesGrantleePrintTest::shouldDisplayNoteInfo()
{
    QFETCH( QString, variable );
    QFETCH( QString, result );
    Akonadi::Item note(42);

    KNoteGrantleePrint *grantleePrint = new KNoteGrantleePrint;
    grantleePrint->setContent(QString::fromLatin1("{% if notes %}{% for note in notes %}{{ note.%1 }}{% endfor %}{% endif %}").arg(variable));

    KMime::Message::Ptr msg(new KMime::Message);
    note.setMimeType( Akonotes::Note::mimeType() );
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    msg->contentType( true )->setMimeType( "text/plain" );
    msg->contentType()->setCharset("utf-8");
    msg->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEquPr);
    msg->date( true )->setDateTime( KDateTime::currentLocalDateTime() );
    msg->mainBodyPart()->fromUnicodeString( QLatin1String("notes test") );
    note.setPayload( msg );
    msg->assemble();

    QList<KNotePrintObject *> lst;
    KNotePrintObject *obj = new KNotePrintObject(note);
    lst << obj;
    const QString html = grantleePrint->notesToHtml(lst);
    QCOMPARE(html, result);
    grantleePrint->deleteLater();
    grantleePrint = 0;
}


QTEST_KDEMAIN(KNotesGrantleePrintTest, NoGUI)