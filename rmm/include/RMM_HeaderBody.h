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

#ifndef RMM_FIELD_BODY_H
#define RMM_FIELD_BODY_H

#include <qstring.h>

#include <RMM_MessageComponent.h>
#include <RMM_Defines.h>

namespace RMM {

/**
 * Base class for RAddress, RContentType, RMailboxList etc.
 */
class RHeaderBody : public RMessageComponent {

#include "generated/RHeaderBody_generated.h"
        
};

};

#endif
// vim:ts=4:sw=4:tw=78
