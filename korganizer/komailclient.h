/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Barry D Benowitz
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
*/
#ifndef KOMAILCLIENT_H
#define KOMAILCLIENT_H
// $Id$

#include <qstring.h>

#include <libkcal/incidence.h>

class KURL;

using namespace KCal;

class KOMailClient
{
  public:
    KOMailClient();
    virtual ~KOMailClient();
    
    bool mailAttendees(Incidence *,const QString &attachment=QString::null);
    bool mailTo(Incidence *,QString recipients,const QString &attachment=QString::null);

  protected:
    /** Send mail with specified from, to and subject field and body as text. If
     * bcc is set, send a blind carbon copy to the sender from */
    bool send(const QString &from,const QString &to,const QString &subject,
              const QString &body,bool bcc=false,
              const QString &attachment=QString::null);

    QString createBody(Incidence *incidence);

    int kMailOpenComposer(const QString& to, const QString& cc,
                          const QString& bcc, const QString& subject,
                          const QString& body, int hidden,
                          const QString& attachName, const QCString& attachCte,
                          const QCString& attachData,
                          const QCString& attachType,
                          const QCString& attachSubType,
                          const QCString& attachParamAttr,
                          const QString& attachParamValue,
                          const QCString& attachContDisp );
    int kMailOpenComposer(const QString& arg0,const QString& arg1,
                          const QString& arg2,const QString& arg3,
                          const QString& arg4,int arg5,const KURL& arg6);
};

#endif
