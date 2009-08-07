/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef KDEPIM_ATTACHMENTLOADJOB_H
#define KDEPIM_ATTACHMENTLOADJOB_H

#include "attachmentpart.h"
#include "kdepim_export.h"

#include <KDE/KJob>

namespace KPIM {

/**
*/
class KDEPIM_EXPORT AttachmentLoadJob : public KJob
{
  Q_OBJECT

  public:
    explicit AttachmentLoadJob( QObject *parent = 0 );
    virtual ~AttachmentLoadJob();

    virtual void start();

    AttachmentPart::Ptr attachmentPart() const;

  protected:
    /// for subclasses to set the resulting loaded part.
    void setAttachmentPart( AttachmentPart::Ptr part );

  protected slots:
    virtual void doStart() = 0;

  private:
    class Private;
    Private *const d;
};

} // namespace KPIM

#endif
