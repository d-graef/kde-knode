/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef MERGECONTACTSJOB_H
#define MERGECONTACTSJOB_H
#include <QObject>

#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include <KContacts/Addressee>
namespace KABMergeContacts
{
class MergeContactsJob : public QObject
{
    Q_OBJECT
public:
    explicit MergeContactsJob(QObject *parent = Q_NULLPTR);
    ~MergeContactsJob();

    void start();

    void setListItem(const Akonadi::Item::List &lstItem);

    void setDestination(const Akonadi::Collection &collection);

    bool canStart();

    void setNewContact(const KContacts::Addressee &addr);

Q_SIGNALS:
    void finished(const Akonadi::Item &item);

private Q_SLOTS:
    void slotCreateMergedContactFinished(KJob *job);
    void slotDeleteContactsFinished(KJob *job);

private:
    void generateMergedContact();
    void createMergedContact(const KContacts::Addressee &addressee);

    Akonadi::Collection mCollection;
    Akonadi::Item::List mListItem;
    Akonadi::Item mCreatedContact;
    KContacts::Addressee mNewContact;
};
}
#endif // MERGECONTACTSJOB_H
