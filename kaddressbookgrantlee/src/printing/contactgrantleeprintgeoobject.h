/*
  Copyright (c) 2014-2014 Montel Laurent <montel@kde.org>

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

#ifndef CONTACTGRANTLEEPRINTGEOOBJECT_H
#define CONTACTGRANTLEEPRINTGEOOBJECT_H

#include <QObject>
#include <kcontacts/geo.h>

namespace KAddressBookGrantlee
{

class ContactGrantleePrintGeoObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float latitude READ latitude)
    Q_PROPERTY(float longitude READ longitude)
    Q_PROPERTY(QString toString READ toString)
    Q_PROPERTY(bool isValid READ isValid)

public:
    explicit ContactGrantleePrintGeoObject(const KContacts::Geo &geo, QObject *parent = Q_NULLPTR);
    ~ContactGrantleePrintGeoObject();

    float latitude() const;
    float longitude() const;
    QString toString() const;
    bool isValid() const;

private:
    KContacts::Geo mGeo;
};
}
#endif // CONTACTGRANTLEEPRINTGEOOBJECT_H
