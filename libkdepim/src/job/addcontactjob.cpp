/*
  Copyright 2010 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "addcontactjob.h"
#include <Collection>
#include "akonadi/contact/selectaddressbookdialog.h"

#include <Akonadi/Contact/ContactSearchJob>
#include <item.h>
#include <itemcreatejob.h>
#include <kcontacts/addressee.h>
#include <KLocalizedString>
#include <kmessagebox.h>

#include <QPointer>

using namespace KPIM;

class Q_DECL_HIDDEN AddContactJob::Private
{
public:
    Private(AddContactJob *qq, const KContacts::Addressee &contact, QWidget *parentWidget)
        : q(qq), mContact(contact), mParentWidget(parentWidget), mShowMessageBox(true)
    {
    }

    Private(AddContactJob *qq, const KContacts::Addressee &contact, const Akonadi::Collection &collection)
        : q(qq), mContact(contact), mParentWidget(Q_NULLPTR), mCollection(collection), mShowMessageBox(true)
    {
    }

    void slotSearchDone(KJob *job)
    {
        if (job->error()) {
            q->setError(job->error());
            q->setErrorText(job->errorText());
            q->emitResult();
            return;
        }

        const Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob *>(job);

        const KContacts::Addressee::List contacts = searchJob->contacts();

        if (!contacts.isEmpty()) {   // contact is already part of the address book...
            if (mShowMessageBox) {
                const QString text =
                    i18nc("@info",
                          "The vCard's primary email address is already in "
                          "your address book; however, you may save the vCard into "
                          "a file and import it into the address book manually.");
                KMessageBox::information(mParentWidget, text);
            }
            q->setError(UserDefinedError);
            q->emitResult();
            return;
        }

        if (!mCollection.isValid()) {
            // ask user in which address book the new contact shall be stored
            QPointer<Akonadi::SelectAddressBookDialog> dlg = new Akonadi::SelectAddressBookDialog(mParentWidget);

            bool gotIt = true;
            if (!dlg->exec()) {
                q->setError(UserDefinedError);
                q->emitResult();
                gotIt = false;
            } else {
                mCollection = dlg->selectedCollection();
            }
            delete dlg;
            if (!gotIt) {
                return;
            }
        }

        if (mCollection.isValid()) {
            // create the new item
            Akonadi::Item item;
            item.setMimeType(KContacts::Addressee::mimeType());
            item.setPayload<KContacts::Addressee>(mContact);

            // save the new item in akonadi storage
            Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, mCollection);
            q->connect(job, SIGNAL(result(KJob*)), SLOT(slotAddContactDone(KJob*)));
        } else {
            q->setError(UserDefinedError);
            q->emitResult();
        }
    }

    void slotAddContactDone(KJob *job)
    {
        if (job->error()) {
            q->setError(job->error());
            q->setErrorText(job->errorText());
            q->emitResult();
            return;
        }

        if (mShowMessageBox) {
            const QString text =
                i18nc("@info",
                      "The vCard was added to your address book; "
                      "you can add more information to this "
                      "entry by opening the address book.");
            KMessageBox::information(
                mParentWidget,
                text,
                QString(),
                QStringLiteral("addedtokabc"));
        }
        q->emitResult();
    }

    AddContactJob *q;
    KContacts::Addressee mContact;
    QWidget *mParentWidget;
    Akonadi::Collection mCollection;
    bool mShowMessageBox;
};

AddContactJob::AddContactJob(const KContacts::Addressee &contact, QWidget *parentWidget, QObject *parent)
    : KJob(parent), d(new Private(this, contact, parentWidget))
{
}

AddContactJob::AddContactJob(const KContacts::Addressee &contact, const Akonadi::Collection &collection, QObject *parent)
    : KJob(parent), d(new Private(this, contact, collection))
{
}

AddContactJob::~AddContactJob()
{
    delete d;
}

void AddContactJob::showMessageBox(bool b)
{
    d->mShowMessageBox = b;
}

void AddContactJob::start()
{
    // first check whether a contact with the same email exists already
    Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob(this);
    searchJob->setLimit(1);
    searchJob->setQuery(Akonadi::ContactSearchJob::Email, d->mContact.preferredEmail().toLower(),
                        Akonadi::ContactSearchJob::ExactMatch);

    connect(searchJob, SIGNAL(result(KJob*)), SLOT(slotSearchDone(KJob*)));
}

#include "moc_addcontactjob.cpp"
