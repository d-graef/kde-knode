/***************************************************************************
                          knnntpaccount.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "utilities.h"
#include "knnntpaccount.h"


KNNntpAccount::KNNntpAccount() : KNCollection(0), KNServerInfo()
{
	u_nsentCount=0;
}



KNNntpAccount::~KNNntpAccount()
{
	
}



// trys to read information, returns false if it fails to do so
bool KNNntpAccount::readInfo(const QString &confPath)
{
  KSimpleConfig conf(confPath);

  n_ame = conf.readEntry("name");
  u_nsentCount = conf.readNumEntry("unsentCnt", 0);

  KNServerInfo::readConf(&conf);
  return true;
  if (n_ame.isEmpty() || s_erver.isEmpty() || i_d == -1)
    return false;
  else
    return true;
}


void KNNntpAccount::saveInfo()
{
	QString dir(path());
	if (dir == QString::null)
		return;
		
	KSimpleConfig conf(dir+"info");
	
	conf.writeEntry("name", n_ame);
	conf.writeEntry("unsentCnt", u_nsentCount);
	KNServerInfo::saveConf(&conf);      // save not KNNntpAccount specific settings
}



void KNNntpAccount::syncInfo()
{
	QString dir(path());
	if (dir == QString::null)
		return;
	KSimpleConfig conf(dir+"info");
	conf.writeEntry("unsentCnt", u_nsentCount);
}


		
QString KNNntpAccount::path()
{
	QString dir(KGlobal::dirs()->saveLocation("appdata",QString("nntp.%1/").arg(i_d)));
	if (dir==QString::null)
		displayInternalFileError();
  return (dir);
}



