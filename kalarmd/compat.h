/*
    KDE2 compatibility for KDE Alarm Daemon and KDE Alarm Daemon GUI.

    This file is part of the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AD_COMPAT_H
#define AD_COMPAT_H

// Compatibility with KDE 2.2
#include <qglobal.h>
#if QT_VERSION < 300
	#define QPtrList         QList
	#define QPtrListIterator QListIterator
#endif
#include <kapplication.h>
#if KDE_VERSION < 290
   #define Alarm KOAlarm
   #define uid   VUID
#endif

#endif
