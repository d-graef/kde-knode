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
# pragma implementation "EmpathEnvelopeWidget.h"
#endif

// KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathEnvelopeWidget.h"
#include "EmpathAddressSelectionWidget.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathComposer.h"
#include <RMM_DateTime.h>
#include <RMM_Address.h>

EmpathEnvelopeWidget::EmpathEnvelopeWidget(
        RMM::REnvelope &    headers,
        QWidget *           parent,
        const char *        name)
    :
        QVBox(parent, name),
        headers_(headers)
{
    empathDebug("");
    setSpacing(2);
    maxSizeColOne_ = 0;
    
    RMM::RHeaderList headerList = headers_.headerList();

    RMM::RHeaderListIterator it(headerList);
    for (; it.current(); ++it) 
        _addHeader(*it.current());

    empathDebug("");

    _lineUpHeaders();

    empathDebug("");
    // headerSpecList_.getFirst()->setFocus();
}

EmpathEnvelopeWidget::~EmpathEnvelopeWidget()
{
    empathDebug("dtor");
}

    RMM::REnvelope &
EmpathEnvelopeWidget::headers()
{
    // FIXME
        
    RMM::REnvelope envelope;

    QListIterator<EmpathHeaderSpecWidget> hit(headerSpecList_);
    
    for (; hit.current(); ++hit)
        envelope.addHeader(hit.current()->header());
    
    headers_ = envelope;
    
    return headers_;
}

   bool
EmpathEnvelopeWidget::haveTo()
{
    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);

    for (; it.current(); ++it)
        if (stricmp(it.current()->headerName().ascii(), "To:") == 0 &&
            !it.current()->headerBody().isEmpty())
            return true;
    
    return false;
}
    
    bool
EmpathEnvelopeWidget::haveSubject()
{
    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);

    for (; it.current(); ++it)
        if (stricmp(it.current()->headerName().ascii(), "Subject:") == 0 &&
            !it.current()->headerBody().isEmpty())
            return true;
    
    return false;
}

    void
EmpathEnvelopeWidget::_addHeader(RMM::RHeader & header) 
{
    empathDebug("adding header: " + header.asString());

    EmpathHeaderSpecWidget * newHsw = new EmpathHeaderSpecWidget(header, this);
    newHsw->show();
    
    if (!headerSpecList_.isEmpty()) {
        QObject::connect(
            newHsw, SIGNAL(lineUp()),
            headerSpecList_.getLast(), SLOT(s_setFocus()));
        QObject::connect(
            headerSpecList_.getLast(), SIGNAL(lineDown()),
            newHsw, SLOT(s_setFocus()));
    }
    
    headerSpecList_.append(newHsw); 
    maxSizeColOne_ = QMAX(newHsw->sizeOfColumnOne(), maxSizeColOne_);
}
 
    void
EmpathEnvelopeWidget::_lineUpHeaders()
{
    QListIterator<EmpathHeaderSpecWidget> hit(headerSpecList_);
    
    for (; hit.current(); ++hit)
        hit.current()->setColumnOneSize(maxSizeColOne_);
}

// vim:ts=4:sw=4:tw=78
