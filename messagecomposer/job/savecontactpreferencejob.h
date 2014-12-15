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

#ifndef SAVECONTACTPREFERENCEJOB_H
#define SAVECONTACTPREFERENCEJOB_H

#include <KJob>
#include <KContacts/Addressee>
#include "keyresolver.h"

namespace MessageComposer
{
class SaveContactPreferenceJob : public QObject
{
    Q_OBJECT
public:
    explicit SaveContactPreferenceJob(const QString &email, const Kleo::KeyResolver::ContactPreferences &pref, QObject *parent = Q_NULLPTR);
    ~SaveContactPreferenceJob();
    void start();

private Q_SLOTS:
    void slotSearchContact(KJob *job);

    void slotModifyCreateItem(KJob *job);
private:
    void writeCustomContactProperties(KContacts::Addressee &contact, const Kleo::KeyResolver::ContactPreferences &pref) const;
    QString mEmail;
    Kleo::KeyResolver::ContactPreferences mPref;
};
}
#endif // SAVECONTACTPREFERENCEJOB_H
