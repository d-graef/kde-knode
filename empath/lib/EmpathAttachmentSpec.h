/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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

#ifndef EMPATH_ATTACHMENT_SPEC_H
#define EMPATH_ATTACHMENT_SPEC_H

// Qt includes
#include <qstring.h>
#include <qvaluelist.h>

// Local includes
#include "EmpathDefines.h"

/**
 * Attachment information.
 * @author Rikkus
 */
class EmpathAttachmentSpec
{
    public:

        enum EncodingType {
            EncodingType7bit,
            EncodingType8bit,
            EncodingTypeBinary,
            EncodingTypeQuotedPrintable,
            EncodingTypeBase64,
            EncodingTypeXtension
        };

        /**
         * Default ctor. Not much use until you fill in the fields.
         */
        EmpathAttachmentSpec()
        {
        }

        /**
         * Construct with all the fields filled in.
         */
        EmpathAttachmentSpec(
                const QString & filename,
                const QString & description,
                EncodingType    encoding,
                const QString & type,
                const QString & subType,
                const QString & charset)
            :
                filename_       (filename),
                description_    (description),
                encoding_       (encoding),
                type_           (type),
                subType_        (subType),
                charset_        (charset)
        {
        }
                
        EmpathAttachmentSpec(const EmpathAttachmentSpec & a)
            :
                filename_       (a.filename_),
                description_    (a.description_),
                encoding_       (a.encoding_),
                type_           (a.type_),
                subType_        (a.subType_),
                charset_        (a.charset_)
        {
        }
    
            EmpathAttachmentSpec &
        operator = (const EmpathAttachmentSpec & a)
        {    
            if (this == &a) return *this;
            
            filename_       = a.filename_;
            description_    = a.description_;
            encoding_       = a.encoding_;
            type_           = a.type_;
            subType_        = a.subType_;
            charset_        = a.charset_;
            
            return *this;
        }

        virtual ~EmpathAttachmentSpec()
        {
        }

        bool operator == (const EmpathAttachmentSpec & other) const
        {
            return (
                filename_   == other.filename_  &&
                encoding_   == other.encoding_  &&
                type_       == other.type_      &&
                subType_    == other.subType_   &&
                charset_    == other.charset_);
        } 
        
        QString filename()      const { return filename_;       }
        QString description()   const { return description_;    }
        EncodingType encoding() const { return encoding_;       }
        QString type()          const { return type_;           }
        QString subType()       const { return subType_;        }
        QString charset()       const { return charset_;        }
        
        void setFilename    (const QString & s) { filename_     = s; }
        void setDescription (const QString & s) { description_  = s; }
        void setEncoding    (EncodingType t)    { encoding_     = t; }
        void setType        (const QString & s) { type_         = s; }
        void setSubType     (const QString & s) { subType_      = s; }
        void setCharset     (const QString & s) { charset_      = s; }
        
        /**
         * @internal
         */
        const char * className() const { return "EmpathAttachmentSpec"; }
        
    private:
        
        QString filename_;
        QString description_;
        EncodingType encoding_;
        QString type_;
        QString subType_;
        QString charset_;
};

typedef QValueList<EmpathAttachmentSpec> EmpathAttachmentSpecList;

#endif

// vim:ts=4:sw=4:tw=78
