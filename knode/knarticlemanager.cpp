/***************************************************************************
                          knarticlemanager.cpp  -  description
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

#include <mimelib/string.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kio/netaccess.h>
#include <ktempfile.h>

#include "knarticlewidget.h"
#include "knarticle.h"
#include "knglobals.h"
#include "utilities.h"
#include "knarticlemanager.h"


KNSaveHelper::KNSaveHelper(QString saveName)
  : s_aveName(saveName), file(0), tmpFile(0)
{
}


KNSaveHelper::~KNSaveHelper()
{
  if (file) {       // local filesystem, just close the file
    delete file;
  } else
    if (tmpFile) {      // network location, initiate transaction
      tmpFile->close();
      if (KIO::NetAccess::upload(tmpFile->name(),url) == false)
        displayRemoteFileError();
      tmpFile->unlink();   // delete temp file
      delete tmpFile;
    }
}


QFile* KNSaveHelper::getFile()
{
  url = KFileDialog::getSaveURL(s_aveName,"*");

  if (url.isEmpty())
    return 0;

  if (url.isLocalFile()) {
    file = new QFile(url.url());
		if(!file->open(IO_WriteOnly)) {
      displayExternalFileError();
      delete file;
      file = 0;
    }
    return file;
  } else {
    tmpFile = new KTempFile();
    if (tmpFile->status()!=0) {
      displayTempFileError();
      delete tmpFile;
      tmpFile = 0;
      return 0;
    }
    return tmpFile->file();
  }
}


//===============================================================================


QList<KTempFile> KNArticleManager::tempFiles;


KNArticleManager::KNArticleManager(KNListView *v)
{
	view=v;
	mainArtWidget=KNArticleWidget::mainWidget();
}



KNArticleManager::~KNArticleManager()
{
}



void KNArticleManager::deleteTempFiles()
{
  KTempFile *file;

  while ((file = tempFiles.first())) {
    file->unlink();                 // deletes file
    tempFiles.removeFirst();
    delete file;
  }
}



void KNArticleManager::saveContentToFile(KNMimeContent *c)
{
  KNSaveHelper helper(c->ctName().data());

  QFile *file = helper.getFile();

  if (file) {
		DwString data=c->decodedData();
		file->writeBlock(data.data(), data.size());
	}
}



void KNArticleManager::saveArticleToFile(KNArticle *a)
{
  KNSaveHelper helper(a->subject().data());

  QFile *file = helper.getFile();
	
	if (file) {
    DwString tmp = "";
	  for(char *line=a->firstHeaderLine(); line; line=a->nextHeaderLine()) {
  		tmp+=line;
  		tmp+="\n";
  	}
  	tmp+="\n";
  	tmp+=a->mainContent()->decodedData();
		file->writeBlock(tmp.data(), tmp.size());
  }
}



QString KNArticleManager::saveContentToTemp(KNMimeContent *c)
{
  QString path;

	QCString tmp=c->headerLine("X-KNode-Tempfile");       // check for existing temp file
	if(!tmp.isEmpty()) {
	  path=QString(tmp);
	  return path;
	}

  KTempFile* tmpFile = new KTempFile(QString::null,c->ctName());    // prefix null, real filename as suffix
  if (tmpFile->status()!=0) {
    displayTempFileError();
    delete tmpFile;
    return QString::null;
  }

  tempFiles.append(tmpFile);
  QFile *f = tmpFile->file();
	DwString data=c->decodedData();
 	f->writeBlock(data.data(), data.size());
 	tmpFile->close();
  path = tmpFile->name();
  c->setHeader(KNArticleBase::HTxkntempfile, path.local8Bit());

  return path;
}



void KNArticleManager::openContent(KNMimeContent *c)
{
  QString path=saveContentToTemp(c);
  KURL url;
  KRun *run;
  if(path.isNull()) return;
  url.setPath(KURL::encode_string(path));
  run=new KRun(url, 0, true, true); //auto-deletion enabled
}



void KNArticleManager::showArticle(KNArticle *a, bool force)
{
	KNArticleWidget *aw;
	if(!force) KNArticleWidget::showArticle(a);
	else {
		aw=KNArticleWidget::find(a);
		if(aw) aw->updateContents();
	}
}



void KNArticleManager::showError(KNArticle *a, const QString &error)
{
	KNArticleWidget *aw=KNArticleWidget::find(a);
	if(aw) aw->showErrorMessage(error);
}

