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

#include "contactgrantleeprintphoneobject.h"

using namespace KAddressBookGrantlee;
ContactGrantleePrintPhoneObject::ContactGrantleePrintPhoneObject(const KContacts::PhoneNumber &phone, QObject *parent)
    : QObject(parent),
      mPhoneNumber(phone)
{
}

ContactGrantleePrintPhoneObject::~ContactGrantleePrintPhoneObject()
{
}

QString ContactGrantleePrintPhoneObject::type() const
{
    return mPhoneNumber.typeLabel();
}

QString ContactGrantleePrintPhoneObject::number() const
{
    return mPhoneNumber.number();
}
