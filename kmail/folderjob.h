/*  -*- mode: C++; c-file-style: "gnu" -*-
 *
 *  This file is part of KMail, the KDE mail client.
 *  Copyright (c) 2003 Zack Rusin <zack@kde.org>
 *
 *  KMail is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  KMail is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef FOLDERJOB_H
#define FOLDERJOB_H

#include <qobject.h>
#include <qobjectlist.h>
#include <qstring.h>

#include "kmmessage.h"

class KMFolder;

namespace KMail {

class FolderJob : public QObject
{
  Q_OBJECT

public:
  enum JobType { tListDirectory, tGetFolder, tCreateFolder, tExpungeFolder,
		 tDeleteMessage, tGetMessage, tPutMessage, tAddSubfolders,
		 tDeleteFolders, tCheckUidValidity, tRenameFolder,
                 tCopyMessage, tExpireMessages, tMoveMessage };
  /**
   * Constructs a new job, operating on the message msg, of type
   * @p jt and with a parent folder @p folder.
   */
  FolderJob( KMMessage *msg, JobType jt = tGetMessage, KMFolder *folder = 0  );

  /**
   * Constructs a new job, operating on a message list @p msgList,
   * set @sets, JobType @p jt and with the parent folder @p folder.
   *
   */
  FolderJob( const QPtrList<KMMessage>& msgList, const QString& sets,
	     JobType jt = tGetMessage, KMFolder *folder = 0 );
  /**
   * This one should ONLY be used in derived folders, when a job
   * does something internal and needs to construct an empty parent
   * FolderJob
   */
  FolderJob( JobType jt );
  virtual ~FolderJob();

  QPtrList<KMMessage> msgList() const;
  void start();

signals:
  void messageRetrieved( KMMessage * );
  void messageStored( KMMessage * );
  void messageCopied( KMMessage * );
  void messageCopied( QPtrList<KMMessage> );
  void finished();
protected:
  virtual void execute()=0;
  virtual void expireMessages()=0;

  QPtrList<KMMessage> mMsgList;
  JobType             mType;
  QString             mSets;
  KMFolder*           mDestFolder;
  //finished() won't be emitted when this is set
  bool                mPassiveDestructor;
};

}

#endif
