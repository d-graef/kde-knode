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

#ifndef MESSAGECOMPOSER_ATTACHMENTJOB_H
#define MESSAGECOMPOSER_ATTACHMENTJOB_H

#include "contentjobbase.h"
#include "messagecomposer_export.h"

#include <MessageCore/AttachmentPart>

namespace MessageComposer
{

class AttachmentJobPrivate;

/**
*/
class MESSAGECOMPOSER_EXPORT AttachmentJob : public ContentJobBase
{
    Q_OBJECT

public:
    explicit AttachmentJob(MessageCore::AttachmentPart::Ptr part, QObject *parent = Q_NULLPTR);
    virtual ~AttachmentJob();

    MessageCore::AttachmentPart::Ptr attachmentPart() const;
    void setAttachmentPart(MessageCore::AttachmentPart::Ptr part);

protected Q_SLOTS:
    void doStart() Q_DECL_OVERRIDE;
    void process() Q_DECL_OVERRIDE;

private:
    Q_DECLARE_PRIVATE(AttachmentJob)
};

}

#endif
