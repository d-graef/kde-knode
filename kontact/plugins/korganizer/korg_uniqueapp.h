/*
   This file is part of KDE Kontact.

   Copyright (c) 2003 David Faure <faure@kde.org>

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

#ifndef KORG_UNIQUEAPP_H
#define KORG_UNIQUEAPP_H

#include <uniqueapphandler.h>

class KOrganizerUniqueAppHandler : public Kontact::UniqueAppHandler
{
public:
    KOrganizerUniqueAppHandler( Kontact::Plugin* plugin ) : Kontact::UniqueAppHandler( plugin ) {}
    virtual ~KOrganizerUniqueAppHandler() {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};


#endif /* KORG_UNIQUEAPP_H */

