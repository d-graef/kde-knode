#ifndef _KPILOT_KPILOTCONFIGDIALOG_H
#define _KPILOT_KPILOTCONFIGDIALOG_H
/* kpilotConfigDialog.h                 KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines a specialization of KPilotDeviceLink
** that can actually handle some HotSync tasks, like backup
** and restore. It does NOT do conduit stuff.
*/
 
/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "uiDialog.h"

class KPilotConfigWidget;

class KPilotConfigDialog : public UIDialog 
{
Q_OBJECT
public:
	KPilotConfigDialog(QWidget *,const char *,bool);
	virtual ~KPilotConfigDialog();

protected:
	virtual bool validate();
	virtual void commitChanges();

protected slots:
	void changePortType(int);

private:
	KPilotConfigWidget *fConfigWidget;

	void readConfig();
	void disableUnusedOptions();
	int getAddressDisplay() const;
	void setAddressDisplay(int);
} ;

#endif
