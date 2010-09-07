/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Bertjan Broeksema, broeksema@kde.org

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#include "monthview.h"
#include "eventview_p.h"

namespace EventViews {

class MonthViewPrivate : public EventViewPrivate
{
  MonthView *q;
  
  public: /// Methods
    explicit MonthViewPrivate( MonthView *qq );
    virtual ~MonthViewPrivate();

    virtual void setUpModels();

    void addIncidence( const Akonadi::Item &incidence );
    void moveStartDate( int weeks, int months );
    void triggerDelayedReload();

  public:  /// Members
    CalendarSupport::CalendarSearch* calendarSearch;
};

} // EventViews
