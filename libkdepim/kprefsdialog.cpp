/*
    This file is part of libkdepim.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qfont.h>
#include <qspinbox.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <kfontdialog.h>
#include <kmessagebox.h>

#include "kprefs.h"

#include "kprefsdialog.h"
#include "kprefsdialog.moc"

namespace KPrefsWidFactory {

KPrefsWid *create( KPrefsItem *item, QWidget *parent )
{
  KPrefsItemBool *boolItem = dynamic_cast<KPrefsItemBool *>( item );
  if ( boolItem ) {
    return new KPrefsWidBool( boolItem->name(), boolItem->value(), parent );
  }

  KPrefsItemString *stringItem = dynamic_cast<KPrefsItemString *>( item );
  if ( stringItem ) {
    return new KPrefsWidString( stringItem->name(), stringItem->value(),
                                parent );
  }
  
  return 0;
}

}


QValueList<QWidget *> KPrefsWid::widgets() const
{
  return QValueList<QWidget *>();
}


KPrefsWidBool::KPrefsWidBool(const QString &text,bool &reference,
                             QWidget *parent, const QString &whatsThis)
  : mReference( reference )
{
  mCheck = new QCheckBox(text,parent);
  if (!whatsThis.isNull()) {
    QWhatsThis::add(mCheck, whatsThis);
  }
}

void KPrefsWidBool::readConfig()
{
  mCheck->setChecked(mReference);
}

void KPrefsWidBool::writeConfig()
{
  mReference = mCheck->isChecked();
}

QCheckBox *KPrefsWidBool::checkBox()
{
  return mCheck;
}

QValueList<QWidget *> KPrefsWidBool::widgets() const
{
  QValueList<QWidget *> widgets;
  widgets.append( mCheck );
  return widgets;
}


KPrefsWidColor::KPrefsWidColor(const QString &text,QColor &reference,
                               QWidget *parent, const QString &whatsThis)
  : mReference( reference )
{
  mButton = new KColorButton(parent);
  mLabel = new QLabel(mButton, text, parent);
  if (!whatsThis.isNull()) {
    QWhatsThis::add(mButton, whatsThis);
  }
}

KPrefsWidColor::~KPrefsWidColor()
{
//  kdDebug(5300) << "KPrefsWidColor::~KPrefsWidColor()" << endl;
}

void KPrefsWidColor::readConfig()
{
  mButton->setColor(mReference);
}

void KPrefsWidColor::writeConfig()
{
  mReference = mButton->color();
}

QLabel *KPrefsWidColor::label()
{
  return mLabel;
}

KColorButton *KPrefsWidColor::button()
{
  return mButton;
}

KPrefsWidFont::KPrefsWidFont(const QString &sampleText,const QString &labelText,
                             QFont &reference,QWidget *parent,
			     const QString &whatsThis)
  : mReference( reference )
{
  mLabel = new QLabel(labelText, parent);

  mPreview = new QLabel(sampleText,parent);
  mPreview->setFrameStyle(QFrame::Panel|QFrame::Sunken);

  mButton = new QPushButton(i18n("Choose..."), parent);
  connect(mButton,SIGNAL(clicked()),SLOT(selectFont()));
  if (!whatsThis.isNull()) {
    QWhatsThis::add(mPreview, whatsThis);
    QWhatsThis::add(mButton, whatsThis);
  }
}

KPrefsWidFont::~KPrefsWidFont()
{
}

void KPrefsWidFont::readConfig()
{
  mPreview->setFont(mReference);
}

void KPrefsWidFont::writeConfig()
{
  mReference = mPreview->font();
}

QLabel *KPrefsWidFont::label()
{
  return mLabel;
}

QFrame *KPrefsWidFont::preview()
{
  return mPreview;
}

QPushButton *KPrefsWidFont::button()
{
  return mButton;
}

void KPrefsWidFont::selectFont()
{
  QFont myFont(mPreview->font());
  int result = KFontDialog::getFont(myFont);
  if (result == KFontDialog::Accepted) {
    mPreview->setFont(myFont);
  }
}


KPrefsWidTime::KPrefsWidTime(const QString &text,int &reference,
                             QWidget *parent, const QString &whatsThis)
  : mReference( reference )
{
  mLabel = new QLabel(text,parent);
  mSpin = new QSpinBox(0,23,1,parent);
  mSpin->setSuffix(":00");
  if (!whatsThis.isNull()) {
    QWhatsThis::add(mSpin, whatsThis);
  }
}

void KPrefsWidTime::readConfig()
{
  mSpin->setValue(mReference);
}

void KPrefsWidTime::writeConfig()
{
  mReference = mSpin->value();
}

QLabel *KPrefsWidTime::label()
{
  return mLabel;
}

QSpinBox *KPrefsWidTime::spinBox()
{
  return mSpin;
}


KPrefsWidRadios::KPrefsWidRadios(const QString &text,int &reference,
                                 QWidget *parent)
  : mReference( reference )
{
  mBox = new QButtonGroup(1,Qt::Horizontal,text,parent);
  connect( mBox, SIGNAL( clicked( int ) ), SIGNAL( changed() ) );
}

KPrefsWidRadios::~KPrefsWidRadios()
{
}

void KPrefsWidRadios::addRadio(const QString &text, const QString &whatsThis)
{
  QRadioButton *r = new QRadioButton(text,mBox);
  if (!whatsThis.isNull()) {
    QWhatsThis::add(r, whatsThis);
  }
}

QButtonGroup *KPrefsWidRadios::groupBox()
{
  return mBox;
}

void KPrefsWidRadios::readConfig()
{
  mBox->setButton(mReference);
}

void KPrefsWidRadios::writeConfig()
{
  mReference = mBox->id(mBox->selected());
}


KPrefsWidString::KPrefsWidString(const QString &text,QString &reference,
                                 QWidget *parent, QLineEdit::EchoMode echomode,
				 const QString &whatsThis)
  : mReference( reference )
{
  mLabel = new QLabel(text,parent);
  mEdit = new QLineEdit(parent);
  mEdit->setEchoMode( echomode );
  if (!whatsThis.isNull()) {
    QWhatsThis::add(mEdit, whatsThis);
  }
}

KPrefsWidString::~KPrefsWidString()
{
}

void KPrefsWidString::readConfig()
{
  mEdit->setText(mReference);
}

void KPrefsWidString::writeConfig()
{
  mReference = mEdit->text();
}

QLabel *KPrefsWidString::label()
{
  return mLabel;
}

QLineEdit *KPrefsWidString::lineEdit()
{
  return mEdit;
}

QValueList<QWidget *> KPrefsWidString::widgets() const
{
  QValueList<QWidget *> widgets;
  widgets.append( mLabel );
  widgets.append( mEdit );
  return widgets;
}

KPrefsWidManager::KPrefsWidManager( KPrefs *prefs )
  : mPrefs( prefs )
{
}

KPrefsWidManager::~KPrefsWidManager()
{
}

void KPrefsWidManager::addWid(KPrefsWid *wid)
{
  mPrefsWids.append(wid);
}

KPrefsWidBool *KPrefsWidManager::addWidBool(const QString &text,bool &reference,QWidget *parent,
                                            const QString &whatsThis)
{
  KPrefsWidBool *w = new KPrefsWidBool(text,reference,parent, whatsThis);
  addWid(w);
  return w;
}

KPrefsWidTime *KPrefsWidManager::addWidTime(const QString &text,int &reference,QWidget *parent,
                                            const QString &whatsThis)
{
  KPrefsWidTime *w = new KPrefsWidTime(text,reference,parent, whatsThis);
  addWid(w);
  return w;
}

KPrefsWidColor *KPrefsWidManager::addWidColor(const QString &text,QColor &reference,
                                              QWidget *parent, const QString &whatsThis)
{
  KPrefsWidColor *w = new KPrefsWidColor(text,reference,parent, whatsThis);
  addWid(w);
  return w;
}

KPrefsWidRadios *KPrefsWidManager::addWidRadios(const QString &text,int &reference,QWidget *parent)
{
  KPrefsWidRadios *w = new KPrefsWidRadios(text,reference,parent);
  addWid(w);
  return w;
}

KPrefsWidString *KPrefsWidManager::addWidString(const QString &text,QString &reference,
                                                QWidget *parent, const QString &whatsThis)
{
  KPrefsWidString *w = new KPrefsWidString(text,reference,parent,
                                           QLineEdit::Normal, whatsThis);
  addWid(w);
  return w;
}

KPrefsWidString *KPrefsWidManager::addWidPassword(const QString &text,QString &reference,
                                                  QWidget *parent, const QString &whatsThis)
{
  KPrefsWidString *w = new KPrefsWidString(text,reference,parent,QLineEdit::Password,
                                           whatsThis);
  addWid(w);
  return w;
}

KPrefsWidFont *KPrefsWidManager::addWidFont(const QString &sampleText,const QString &buttonText,
                                            QFont &reference,QWidget *parent,
					    const QString &whatsThis)
{
  KPrefsWidFont *w = new KPrefsWidFont(sampleText,buttonText,reference,parent,
                                       whatsThis);
  addWid(w);
  return w;
}

void KPrefsWidManager::setWidDefaults()
{
  mPrefs->setDefaults();

  readWidConfig();
}

void KPrefsWidManager::readWidConfig()
{
  KPrefsWid *wid;
  for(wid = mPrefsWids.first();wid;wid=mPrefsWids.next()) {
    wid->readConfig();
  }
}

void KPrefsWidManager::writeWidConfig()
{
  KPrefsWid *wid;
  for(wid = mPrefsWids.first();wid;wid=mPrefsWids.next()) {
    wid->writeConfig();
  }

  mPrefs->writeConfig();
}


KPrefsDialog::KPrefsDialog(KPrefs *prefs,QWidget *parent,char *name,bool modal)
  : KDialogBase(IconList,i18n("Preferences"),Ok|Apply|Cancel|Default,Ok,parent,
                name,modal,true),
    KPrefsWidManager( prefs )
{
// TODO: This seems to cause a crash on exit. Investigate later.
//  mPrefsWids.setAutoDelete(true);

  connect(this,SIGNAL(defaultClicked()),SLOT(setDefaults()));
  connect(this,SIGNAL(cancelClicked()),SLOT(reject()));
}

KPrefsDialog::~KPrefsDialog()
{
}

void KPrefsDialog::autoCreate()
{
  KPrefsItem::List items = prefs()->items();
  
  QMap<QString,QWidget *> mGroupPages;
  QMap<QString,QGridLayout *> mGroupLayouts;
  QMap<QString,int> mCurrentRows;
  
  KPrefsItem::List::ConstIterator it;
  for( it = items.begin(); it != items.end(); ++it ) {
    QString group = (*it)->group();
    QString name = (*it)->name();

    kdDebug() << "ITEMS: " << (*it)->name() << endl;

    QWidget *page;
    QGridLayout *layout;
    int currentRow;
    if ( !mGroupPages.contains( group ) ) {
      page = addPage( group );
      layout = new QGridLayout( page );
      mGroupPages.insert( group, page );
      mGroupLayouts.insert( group, layout );
      currentRow = 0;
      mCurrentRows.insert( group, currentRow );
    } else {
      page = mGroupPages[ group ];
      layout = mGroupLayouts[ group ];
      currentRow = mCurrentRows[ group ];
    }

    KPrefsWid *wid = KPrefsWidFactory::create( *it, page );
    
    if ( wid ) {
      QValueList<QWidget *> widgets = wid->widgets();
      if ( widgets.count() == 1 ) {
        layout->addMultiCellWidget( widgets[ 0 ],
                                    currentRow, currentRow, 0, 1 );
      } else if ( widgets.count() == 2 ) {
        layout->addWidget( widgets[ 0 ], currentRow, 0 );
        layout->addWidget( widgets[ 1 ], currentRow, 1 );      
      } else {
        kdError() << "More widgets than expected: " << widgets.count() << endl;
      }
  
      addWid( wid );
  
      mCurrentRows.replace( group, ++currentRow );
    }
  }
  
  readConfig();
}

void KPrefsDialog::setDefaults()
{
  setWidDefaults();

  readConfig();
}

void KPrefsDialog::readConfig()
{
  readWidConfig();

  usrReadConfig();
}

void KPrefsDialog::writeConfig()
{
  writeWidConfig();

  usrWriteConfig();

  readConfig();
}


void KPrefsDialog::slotApply()
{
  writeConfig();
  emit configChanged();
}

void KPrefsDialog::slotOk()
{
  slotApply();
  accept();
}

void KPrefsDialog::slotDefault()
{
  if (KMessageBox::warningContinueCancel(this,
      i18n("You are about to set all preferences to default values. All "
      "custom modifications will be lost."),i18n("Setting Default Preferences"),
      i18n("Continue"))
    == KMessageBox::Continue) setDefaults();
}


KPrefsModule::KPrefsModule( KPrefs *prefs, QWidget *parent, const char *name )
  : KCModule( parent, name ),
    KPrefsWidManager( prefs )
{
}

void KPrefsModule::addWid( KPrefsWid *wid )
{
  KPrefsWidManager::addWid( wid );

  connect( wid, SIGNAL( changed() ), SLOT( slotWidChanged() ) );
}

void KPrefsModule::slotWidChanged()
{
  emit changed( true );
}

void KPrefsModule::load()
{
  readWidConfig();

  usrReadConfig();
}

void KPrefsModule::save()
{
  writeWidConfig();

  usrWriteConfig();

  load();
}

void KPrefsModule::defaults()
{
  setWidDefaults();

  load();
}
