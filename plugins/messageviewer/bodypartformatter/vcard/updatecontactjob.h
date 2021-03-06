/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#ifndef UPDATECONTACTJOB_H
#define UPDATECONTACTJOB_H

#include <kjob.h>

namespace KContacts
{
class Addressee;
}

class UpdateContactJob : public KJob
{
    Q_OBJECT
public:
    explicit UpdateContactJob(const QString &email, const KContacts::Addressee &contact, QWidget *parentWidget, QObject *parent = Q_NULLPTR);
    ~UpdateContactJob();

    void start() Q_DECL_OVERRIDE;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void slotSearchDone(KJob *))
    Q_PRIVATE_SLOT(d, void slotUpdateContactDone(KJob *))
};

#endif // UPDATECONTACTJOB_H
