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

#ifdef __GNUG__
# pragma interface "RMM_Address.h"
#endif

#ifndef RMM_RADDRESS_H
#define RMM_RADDRESS_H

#include <qstring.h>
#include <qvaluelist.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>

namespace RMM {

class RGroup;
class RMailbox;

/**
 * An RAddress is schizophrenic. It's either an RGroup or an RMailbox. Don't
 * worry about it.
 */
class RAddress : public RHeaderBody {

#include "generated/RAddress_generated.h"
    
    public:

        typedef KSharedPtr<RAddress> Ptr;
        typedef QValueList<Ptr> List;
    
        RMailbox    * mailbox();
        RGroup      * group();

    private:
        
        RMailbox    * mailbox_;
        RGroup      * group_;
};

}

#endif //RADDRESS_H
// vim:ts=4:sw=4:tw=78
