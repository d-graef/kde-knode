/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "updatecontactjob.h"
#include <Akonadi/Contact/ContactSearchJob>
#include <AkonadiCore/itemmodifyjob.h>
#include <kcontacts/addressee.h>
#include <KLocalizedString>
#include <kmessagebox.h>

class Q_DECL_HIDDEN UpdateContactJob::Private
{
public:
    Private(UpdateContactJob *qq, const QString &email, const KContacts::Addressee &contact, QWidget *parentWidget)
        : q(qq), mEmail(email), mContact(contact), mParentWidget(parentWidget)
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

        if (contacts.isEmpty()) {
            const QString text = i18n("The vCard's primary email address is not in addressbook.");
            KMessageBox::information(mParentWidget, text);
            q->setError(UserDefinedError);
            q->emitResult();
            return;
        } else if (contacts.count() > 1) {
            const QString text = i18n("There are two or more contacts with same email stored in addressbook.");
            KMessageBox::information(mParentWidget, text);
            q->setError(UserDefinedError);
            q->emitResult();
            return;
        }
        Akonadi::Item item = searchJob->items().at(0);
        item.setPayload<KContacts::Addressee>(mContact);
        Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(item);
        q->connect(modifyJob, SIGNAL(result(KJob*)), SLOT(slotUpdateContactDone(KJob*)));
    }

    void slotUpdateContactDone(KJob *job)
    {
        if (job->error()) {
            q->setError(job->error());
            q->setErrorText(job->errorText());
            q->emitResult();
            return;
        }

        const QString text = i18n("The vCard was updated to your address book; "
                                  "you can add more information to this "
                                  "entry by opening the address book.");
        KMessageBox::information(mParentWidget, text, QString(), QStringLiteral("updatedtokabc"));

        q->emitResult();
    }

    UpdateContactJob *q;
    QString mEmail;
    KContacts::Addressee mContact;
    QWidget *mParentWidget;
};

UpdateContactJob::UpdateContactJob(const QString &email, const KContacts::Addressee &contact, QWidget *parentWidget, QObject *parent)
    : KJob(parent), d(new Private(this, email, contact, parentWidget))
{
}

UpdateContactJob::~UpdateContactJob()
{
    delete d;
}

void UpdateContactJob::start()
{
    if (d->mEmail.isEmpty()) {
        const QString text = i18n("Email not specified");
        KMessageBox::information(d->mParentWidget, text);
        setError(UserDefinedError);
        emitResult();
        return;
    }
    // first check whether a contact with the same email exists already
    Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob(this);
    searchJob->setLimit(1);
    searchJob->setQuery(Akonadi::ContactSearchJob::Email, d->mEmail.toLower(),
                        Akonadi::ContactSearchJob::ExactMatch);

    connect(searchJob, SIGNAL(result(KJob*)), SLOT(slotSearchDone(KJob*)));
}

#include "moc_updatecontactjob.cpp"
