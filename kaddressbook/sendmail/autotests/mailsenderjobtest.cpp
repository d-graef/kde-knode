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

#include "mailsenderjobtest.h"
#include "mailsenderjob.h"

#include <KContacts/Addressee>

#include <qtest.h>
#include <QSignalSpy>

MailSenderJobTest::MailSenderJobTest()
{
}

void MailSenderJobTest::shouldNotSendSignalWhenWeDontSelectItem()
{
    Akonadi::Item::List lst;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderJobTest::shouldNotSendSignalWhenNoValidAddressItem()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    lst << item << item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderJobTest::shouldNotSendSignalWhenNoEmails()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    item.setPayload<KContacts::Addressee>(address);
    lst << item << item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderJobTest::shouldSendSignalWhenOneEmail()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    address.insertEmail(QStringLiteral("foo@kde.org"), true);
    item.setPayload<KContacts::Addressee>(address);
    lst << item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 1);
    const QStringList resultLst = spy.at(0).at(0).value<QStringList>();
    QCOMPARE(resultLst.count(), 1);
}

void MailSenderJobTest::shouldNotSendTwiceEmails()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    address.insertEmail(QStringLiteral("foo@kde.org"), true);
    item.setPayload<KContacts::Addressee>(address);
    lst << item << item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    const QStringList resultLst = spy.at(0).at(0).value<QStringList>();
    QCOMPARE(resultLst.count(), 1);
}

void MailSenderJobTest::shouldNotAddInvalidEmail()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    //Invalid email
    address.insertEmail(QStringLiteral("foo2"), true);
    item.setPayload<KContacts::Addressee>(address);
    lst << item << item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderJobTest::shouldEmitSignalIfThereIsAValidEmail()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    //Invalid email
    address.insertEmail(QStringLiteral("foo2"), true);
    item.setPayload<KContacts::Addressee>(address);
    lst << item;

    Akonadi::Item item2;
    KContacts::Addressee address2;
    address2.setName(QStringLiteral("foo2"));
    address2.insertEmail(QStringLiteral("foo2@kde.org"), true);
    item2.setPayload<KContacts::Addressee>(address2);
    lst << item2;

    Akonadi::Item item3;
    KContacts::Addressee address3;
    address3.setName(QStringLiteral("foo3"));
    address3.insertEmail(QStringLiteral("foo3@"), true);
    item3.setPayload<KContacts::Addressee>(address3);
    lst << item3;

    Akonadi::Item item4;
    KContacts::Addressee address4;
    address4.setName(QStringLiteral("foo4"));
    address4.insertEmail(QStringLiteral("foo4@kde.org"), true);
    item4.setPayload<KContacts::Addressee>(address4);
    lst << item4;

    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 1);
    const QStringList resultLst = spy.at(0).at(0).value<QStringList>();
    QCOMPARE(resultLst.count(), 2);
}

//TODO Create unittest for ContactGroup too

QTEST_MAIN(MailSenderJobTest)
