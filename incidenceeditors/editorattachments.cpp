/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include <config-enterprise.h>
#include "editorattachments.h"

#include <KABC/VCardDrag>
#include <KCal/Attachment>
#include <KCal/Incidence>
#include <KMime/Message>

#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KFileDialog>
#include <KLineEdit>
#include <KLocale>
#include <KMenu>
#include <KMessageBox>
#include <KRun>
#include <KSeparator>
#include <KTemporaryFile>
#include <KUrlRequester>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KIO/NetAccess>

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDrag>
#include <QDragEnterEvent>
#include <QGridLayout>
#include <QLabel>
#include <KProtocolManager>
#include <QSpacerItem>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <qlistwidget.h>

class AttachmentIconItem : public QListWidgetItem
{
  public:
    AttachmentIconItem( KCal::Attachment *att, QListWidget *parent )
      : QListWidgetItem( parent )
    {
      if ( att ) {
        mAttachment = new KCal::Attachment( *att );
      } else {
        // for the enteprise, inline attachments are the default
#ifdef KDEPIM_ENTERPRISE_BUILD
        mAttachment = new KCal::Attachment( '\0' ); //use the non-uri constructor
                                                    // as we want inline by default
#else
        mAttachment = new KCal::Attachment( QString() );
#endif
      }
      readAttachment();
      setFlags( flags() | Qt::ItemIsDragEnabled );
    }
    ~AttachmentIconItem() { delete mAttachment; }
    KCal::Attachment *attachment() const
    {
      return mAttachment;
    }
    const QString uri() const
    {
      return mAttachment->uri();
    }
    void setUri( const QString &uri )
    {
      mAttachment->setUri( uri );
      readAttachment();
    }
    using QListWidgetItem::setData;
    void setData( const QByteArray &data )
    {
      mAttachment->setDecodedData( data );
      readAttachment();
    }
    const QString mimeType() const
    {
      return mAttachment->mimeType();
    }
    void setMimeType( const QString &mime )
    {
      mAttachment->setMimeType( mime );
      readAttachment();
    }
    const QString label() const
    {
      return mAttachment->label();
    }
    void setLabel( const QString &description )
    {
      if ( mAttachment->label() == description )
        return;
      mAttachment->setLabel( description );
      readAttachment();
    }
    bool isBinary() const
    {
      return mAttachment->isBinary();
    }
    QPixmap icon() const
    {
      return icon( KMimeType::mimeType( mAttachment->mimeType() ),
                   mAttachment->uri(), mAttachment->isBinary() );
    }
    static QPixmap icon( KMimeType::Ptr mimeType, const QString &uri,
                         bool binary = false )
    {
      QString iconStr = mimeType->iconName( uri );
      QStringList overlays;
      if ( !uri.isEmpty() && !binary ) {
        overlays << "emblem-link";
      }

      return KIconLoader::global()->loadIcon( iconStr, KIconLoader::Desktop, 0,
                                              KIconLoader::DefaultState,
                                              overlays );
    }

    void readAttachment()
    {
      if ( mAttachment->label().isEmpty() ) {
        if ( mAttachment->isUri() ) {
          setText( mAttachment->uri() );
        } else {
          setText( i18nc( "@label attachment contains binary data", "[Binary data]" ) );
        }
      } else {
        setText( mAttachment->label() );
      }

      setFlags( flags() | Qt::ItemIsEditable );

      if ( mAttachment->mimeType().isEmpty() ||
           !( KMimeType::mimeType( mAttachment->mimeType() ) ) ) {
        KMimeType::Ptr mimeType;
        if ( mAttachment->isUri() ) {
          mimeType = KMimeType::findByUrl( mAttachment->uri() );
        } else {
          mimeType = KMimeType::findByContent( mAttachment->decodedData() );
        }
        mAttachment->setMimeType( mimeType->name() );
      }

      setIcon( icon() );
    }

  private:
    KCal::Attachment *mAttachment;
};

AttachmentEditDialog::AttachmentEditDialog( AttachmentIconItem *item,
                                            QWidget *parent, bool modal )
  : KDialog ( parent ), mItem( item ), mURLRequester( 0 )
{
  // based loosely on KPropertiesDialog code
  QWidget *page = new QWidget(this);
  setMainWidget( page );
  setCaption( i18nc( "@title", "Properties for %1",
                     item->label().isEmpty() ? item->uri() : item->label() ) );
  setButtons( KDialog::Ok | KDialog::Cancel );
  setDefaultButton( KDialog::Ok );
  setModal( modal );
  QVBoxLayout *vbl = new QVBoxLayout( page );
  vbl->setSpacing( KDialog::spacingHint() );
  vbl->setMargin( 0 );
  QGridLayout *grid = new QGridLayout();
  grid->setColumnStretch( 0, 0 );
  grid->setColumnStretch( 1, 0 );
  grid->setColumnStretch( 2, 1 );
  grid->addItem( new QSpacerItem( KDialog::spacingHint(), 0 ), 0, 1 );
  vbl->addLayout( grid );

  mIcon = new QLabel( page );
  int bsize = 66 + 2 * mIcon->style()->pixelMetric( QStyle::PM_ButtonMargin );
  mIcon->setFixedSize( bsize, bsize );
  mIcon->setPixmap( item->icon() );
  grid->addWidget( mIcon, 0, 0, Qt::AlignLeft );

  mLabelEdit = new KLineEdit( page );
  mLabelEdit->setText( item->label().isEmpty() ? item->uri() : item->label() );
  mLabelEdit->setClickMessage( i18nc( "@label", "Attachment name" ) );
  mLabelEdit->setToolTip(
    i18nc( "@info:tooltip", "Give the attachment a name" ) );
  mLabelEdit->setWhatsThis(
    i18nc( "@info:whatsthis", "Type any string you desire here for the name of the attachment" ) );
  grid->addWidget( mLabelEdit, 0, 2 );

  KSeparator *sep = new KSeparator( Qt::Horizontal, page );
  grid->addWidget( sep, 1, 0, 1, 3 );

  QLabel *label = new QLabel( i18nc( "@label", "Type:" ), page );
  grid->addWidget( label, 2, 0 );
  QString typecomment = item->mimeType().isEmpty() ?
                        i18nc( "@label unknown mimetype", "Unknown" ) :
                        KMimeType::mimeType( item->mimeType() )->comment();
  mTypeLabel = new QLabel( typecomment, page );
  grid->addWidget( mTypeLabel, 2, 2 );
  mMimeType = KMimeType::mimeType( item->mimeType() );

  mInline = new QCheckBox( i18nc( "@option:check", "Store attachment inline" ), page );
  grid->addWidget( mInline, 3, 0, 1, 3 );
  mInline->setChecked( item->isBinary() );
  mInline->setToolTip(
    i18nc( "@info:tooltip", "Store the attachment file inside the calendar" ) );
  mInline->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Checking this option will cause the attachment to be stored inside "
           "your calendar, which can take a lot of space depending on the size "
           "of the attachment. If this option is not checked, then only a link "
           "pointing to the attachment will be stored.  Do not use a link for "
           "attachments that change often or may be moved (or removed) from "
           "their current location." ) );

  if ( item->attachment()->isUri() || !item->attachment()->data() ) {
      label = new QLabel( i18nc( "@label", "Location:" ), page );
    grid->addWidget( label, 4, 0 );
    mURLRequester = new KUrlRequester( item->uri(), page );
    mURLRequester->setToolTip(
      i18nc( "@info:tooltip", "Provide a location for the attachment file" ) );
    mURLRequester->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Enter the path to the attachment file or use the file browser "
             "by pressing the adjacent button" ) );
    grid->addWidget( mURLRequester, 4, 2 );
    connect( mURLRequester, SIGNAL(urlSelected(const KUrl &)),
             SLOT(urlChanged(const KUrl &)) );
    connect( mURLRequester, SIGNAL( textChanged( const QString& ) ),
             SLOT( urlChanged( const QString& ) ) );
    urlChanged( item->uri() );
  } else {
    grid->addWidget( new QLabel( i18nc( "@label", "Size:" ), page ), 4, 0 );
    grid->addWidget( new QLabel( QString::fromLatin1( "%1 (%2)" ).
                                 arg( KIO::convertSize( item->attachment()->size() ) ).
                                 arg( KGlobal::locale()->formatNumber(
                                        item->attachment()->size(), 0 ) ), page ), 4, 2 );
  }
  vbl->addStretch( 10 );
}

void AttachmentEditDialog::slotApply()
{
  if ( mLabelEdit->text().isEmpty() ) {
    if ( mURLRequester->url().isLocalFile() ) {
      mItem->setLabel( mURLRequester->url().fileName() );
    } else {
      mItem->setLabel( mURLRequester->url().url() );
    }
  } else {
    mItem->setLabel( mLabelEdit->text() );
  }
  if ( mItem->label().isEmpty() ) {
    mItem->setLabel( i18nc( "@label", "New attachment" ) );
  }
  mItem->setMimeType( mMimeType->name() );
  if ( mURLRequester ) {
    if ( mInline->isChecked() ) {
      QString tmpFile;
      if ( KIO::NetAccess::download( mURLRequester->url(), tmpFile, this ) ) {
        QFile f( tmpFile );
        if ( !f.open( QIODevice::ReadOnly ) ) {
          return;
        }
        QByteArray data = f.readAll();
        f.close();
        mItem->setData( data );
      }
      KIO::NetAccess::removeTempFile( tmpFile );
    } else {
      mItem->setUri( mURLRequester->url().url() );
    }
  }
}

void AttachmentEditDialog::accept()
{
  slotApply();
  KDialog::accept();
}

void AttachmentEditDialog::urlChanged( const QString &url )
{
  enableButtonOk( !url.isEmpty() );
}

void AttachmentEditDialog::urlChanged( const KUrl &url )
{
  mMimeType = KMimeType::findByUrl( url );
  mTypeLabel->setText( mMimeType->comment() );
  mIcon->setPixmap( AttachmentIconItem::icon( mMimeType, url.path() ) );
}

class AttachmentIconView : public QListWidget
{
  friend class EditorAttachments;
  public:
    AttachmentIconView( QWidget *parent ) : QListWidget( parent )
    {
      setMovement( Static );
      setAcceptDrops( true );
      setSelectionMode( ExtendedSelection );
      setSelectionRectVisible( false );
      setIconSize( QSize( KIconLoader::SizeLarge, KIconLoader::SizeLarge ) );
      setFlow( LeftToRight );
      setDragDropMode( DragDrop );
      setDragEnabled( true );
      setEditTriggers( EditKeyPressed );
      setContextMenuPolicy( Qt::CustomContextMenu );
    }

    KUrl tempFileForAttachment( KCal::Attachment *attachment ) const
    {
      if ( mTempFiles.contains( attachment ) ) {
        return mTempFiles.value( attachment );
      }
      KTemporaryFile *file = new KTemporaryFile();
      file->setParent( const_cast<AttachmentIconView*>( this ) );

      QStringList patterns = KMimeType::mimeType( attachment->mimeType() )->patterns();

      if ( !patterns.empty() ) {
        file->setSuffix( QString( patterns.first() ).remove( '*' ) );
      }
      file->setAutoRemove( true );
      file->open();
      // read-only not to give the idea that it could be written to
      file->setPermissions( QFile::ReadUser );
      file->write( QByteArray::fromBase64( attachment->data() ) );
      mTempFiles.insert( attachment, file->fileName() );
      file->close();
      return mTempFiles.value( attachment );
    }

  protected:

    QMimeData* mimeData(const QList< QListWidgetItem* > items) const
    {
      // create a list of the URL:s that we want to drag
      KUrl::List urls;
      QStringList labels;
      foreach( QListWidgetItem *it, items ) {
        if ( it->isSelected() ) {
          AttachmentIconItem *item = static_cast<AttachmentIconItem *>( it );
          if ( item->isBinary() ) {
            urls.append( tempFileForAttachment( item->attachment() ) );
          } else {
            urls.append( item->uri() );
          }
          labels.append( KUrl::toPercentEncoding( item->label() ) );
        }
      }
      if ( selectionMode() == NoSelection ) {
        AttachmentIconItem *item = static_cast<AttachmentIconItem *>( currentItem() );
        if ( item ) {
          urls.append( item->uri() );
          labels.append( KUrl::toPercentEncoding( item->label() ) );
        }
      }

      QMap<QString, QString> metadata;
      metadata["labels"] = labels.join( ":" );

      QMimeData *mimeData = new QMimeData;
      urls.populateMimeData( mimeData, metadata );
      return mimeData;
    }

    QMimeData* mimeData() const
    {
      return mimeData( selectedItems() );
    }

    void startDrag(Qt::DropActions supportedActions)
    {
      Q_UNUSED( supportedActions );
      QPixmap pixmap;
      if ( selectedItems().size() > 1 ) {
        pixmap = KIconLoader::global()->loadIcon( "mail-attachment", KIconLoader::Desktop );
      }
      if ( pixmap.isNull() ) {
        pixmap = static_cast<AttachmentIconItem *>( currentItem() )->icon();
      }

      const QPoint hotspot( pixmap.width() / 2, pixmap.height() / 2 );

      QDrag *drag = new QDrag( this );
      drag->setMimeData( mimeData() );

      drag->setPixmap( pixmap );
      drag->setHotSpot( hotspot );
      drag->exec( Qt::CopyAction );
    }

    void keyPressEvent(QKeyEvent* event)
    {
      if ( ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter )
        && currentItem() && state() != EditingState ) {
        emit itemDoubleClicked( currentItem() ); // ugly, but itemActivated() also includes single click
        return;
      }
      QListWidget::keyPressEvent( event );
    }

  private:
    mutable QHash<KCal::Attachment *, KUrl> mTempFiles;
};

EditorAttachments::EditorAttachments( int spacing, QWidget *parent )
  : QWidget( parent )
{
  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( spacing );

  QLabel *label = new QLabel( i18nc( "@label", "Attachments:" ), this );
  topLayout->addWidget( label );

  mAttachments = new AttachmentIconView( this );
  mAttachments->setWhatsThis( i18nc( "@info:whatsthis",
                                     "Displays items (files, mail, etc.) that "
                                     "have been associated with this event or to-do." ) );
  topLayout->addWidget( mAttachments );
  connect( mAttachments, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
           SLOT(showAttachment(QListWidgetItem*)) );
  connect( mAttachments, SIGNAL(itemChanged(QListWidgetItem*)),
           SLOT(slotItemRenamed(QListWidgetItem*)) );
  connect( mAttachments, SIGNAL(itemSelectionChanged()),
           SLOT(selectionChanged()) );
  connect( mAttachments, SIGNAL(customContextMenuRequested(QPoint)),
           SLOT(contextMenu(QPoint)) );

  QPushButton *addButton = new QPushButton( this );
  addButton->setIcon( KIcon( "list-add" ) );
  addButton->setToolTip( i18nc( "@info:tooltip", "Add an attachment" ) );
  addButton->setWhatsThis( i18nc( "@info:whatsthis",
                                  "Shows a dialog used to select an attachment "
                                  "to add to this event or to-do as link or as "
                                  "inline data." ) );
  topLayout->addWidget( addButton );
  connect( addButton, SIGNAL(clicked()), SLOT(slotAdd()) );

  mRemoveBtn = new QPushButton( this );
  mRemoveBtn->setIcon( KIcon( "list-remove" ) );
  mRemoveBtn->setToolTip( i18nc( "@info:tooltip", "Remove the selected attachment" ) );
  mRemoveBtn->setWhatsThis( i18nc( "@info:whatsthis",
                                   "Removes the attachment selected in the "
                                   "list above from this event or to-do." ) );
  topLayout->addWidget( mRemoveBtn );
  connect( mRemoveBtn, SIGNAL(clicked()), SLOT(slotRemove()) );

  KActionCollection *ac = new KActionCollection( this );
  ac->addAssociatedWidget( this );

  mPopupMenu = new KMenu( this );

  mOpenAction = new KAction( i18nc( "@action:inmenu open the attachment in a viewer",
                                    "&Open" ), this );
  connect( mOpenAction, SIGNAL(triggered(bool)), this, SLOT(slotShow()) );
  ac->addAction( "view", mOpenAction );
  mPopupMenu->addAction( mOpenAction );

  mSaveAsAction = new KAction( i18nc( "@action:inmenu save the attachment to a file",
                                      "Save As..." ), this );
  connect( mSaveAsAction, SIGNAL(triggered(bool)), this, SLOT(slotSaveAs()) );
  mPopupMenu->addAction( mSaveAsAction );

  mPopupMenu->addSeparator();

  mCopyAction = KStandardAction::copy( this, SLOT(slotCopy()), ac );
  mPopupMenu->addAction( mCopyAction );
  mCutAction = KStandardAction::cut( this, SLOT(slotCut()), ac );
  mPopupMenu->addAction( mCutAction );
  KAction *action = KStandardAction::paste( this, SLOT(slotPaste()), ac );
  mPopupMenu->addAction( action );
  mPopupMenu->addSeparator();

  mDeleteAction = new KAction( i18nc( "@action:inmenu remove the attachment",
                                      "&Remove" ), this );
  connect( mDeleteAction, SIGNAL(triggered(bool)), this, SLOT(slotRemove()) );
  ac->addAction( "remove", mDeleteAction );
  mPopupMenu->addAction( mDeleteAction );
  mPopupMenu->addSeparator();

  mEditAction = new KAction( i18nc( "@action:inmenu show a dialog used to edit the attachment",
                                    "&Properties..." ), this );
  connect( mEditAction, SIGNAL(triggered(bool)), this, SLOT(slotEdit()) );
  ac->addAction( "edit", mEditAction );
  mPopupMenu->addAction( mEditAction );

  selectionChanged();
  setAcceptDrops( true );
}

EditorAttachments::~EditorAttachments()
{
}

bool EditorAttachments::hasAttachments()
{
  return mAttachments->count() > 0;
}

void EditorAttachments::dragEnterEvent( QDragEnterEvent *event )
{
  const QMimeData *md = event->mimeData();
  event->setAccepted( KUrl::List::canDecode( md ) || md->hasText() );
}

void EditorAttachments::handlePasteOrDrop( const QMimeData *mimeData )
{
  KUrl::List urls;
  bool probablyWeHaveUris = false;
  bool weCanCopy = true;
  QStringList labels;

  if ( KABC::VCardDrag::canDecode( mimeData ) ) {
    KABC::Addressee::List addressees;
    KABC::VCardDrag::fromMimeData( mimeData, addressees );
    for ( KABC::Addressee::List::ConstIterator it = addressees.constBegin();
          it != addressees.constEnd(); ++it ) {
      urls.append( QLatin1String( "uid:" ) + ( *it ).uid() );
      // there is some weirdness about realName(), hence fromUtf8
      labels.append( QString::fromUtf8( ( *it ).realName().toLatin1() ) );
    }
    probablyWeHaveUris = true;
  } else if ( KUrl::List::canDecode( mimeData ) ) {
    QMap<QString,QString> metadata;

    urls = KUrl::List::fromMimeData( mimeData, &metadata );
    probablyWeHaveUris = true;
    labels = metadata["labels"].split( ':', QString::SkipEmptyParts );
    for ( QStringList::Iterator it = labels.begin(); it != labels.end(); ++it ) {
      *it = KUrl::fromPercentEncoding( (*it).toLatin1() );
    }
  } else if ( mimeData->hasText() ) {
    QString text = mimeData->text();
    QStringList lst = text.split( '\n', QString::SkipEmptyParts );
    for ( QStringList::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
      urls.append( *it );
    }
    probablyWeHaveUris = true;
  }
  KMenu menu( this );
  QAction *linkAction = 0, *cancelAction;
  if ( probablyWeHaveUris ) {
    linkAction = menu.addAction( KIcon( "insert-link" ), i18nc( "@action:inmenu", "&Link here" ) );
    // we need to check if we can reasonably expect to copy the objects
    for ( KUrl::List::ConstIterator it = urls.constBegin(); it != urls.constEnd(); ++it ) {
      if ( !( weCanCopy = KProtocolManager::supportsReading( *it ) ) ) {
        break; // either we can copy them all, or no copying at all
      }
    }
    if ( weCanCopy ) {
      menu.addAction( KIcon( "edit-copy" ), i18nc( "@action:inmenu", "&Copy here" ) );
    }
  } else {
    menu.addAction( KIcon( "edit-copy" ), i18nc( "@action:inmenu", "&Copy here" ) );
  }

  menu.addSeparator();
  cancelAction = menu.addAction( KIcon( "process-stop" ) , i18nc( "@action:inmenu", "C&ancel" ) );

  QByteArray data;
  QString mimeType;
  QString label;

  if(!mimeData->formats().isEmpty() && !probablyWeHaveUris) {
    data=mimeData->data( mimeData->formats().first() );
    mimeType = mimeData->formats().first();
    if( KMimeType::mimeType( mimeData->formats().first() ) )
       label = KMimeType::mimeType( mimeData->formats().first() )->name();

  }

  QAction *ret = menu.exec( QCursor::pos() );
  if ( linkAction == ret ) {
    QStringList::ConstIterator jt = labels.constBegin();
    for ( KUrl::List::ConstIterator it = urls.constBegin();
          it != urls.constEnd(); ++it ) {
      addUriAttachment( (*it).url(), QString(), ( jt == labels.constEnd() ?
                                                  QString() : *( jt++ ) ), true );
    }
  } else if ( cancelAction != ret ) {
    if ( probablyWeHaveUris ) {
      for ( KUrl::List::ConstIterator it = urls.constBegin();
            it != urls.constEnd(); ++it ) {
        KIO::Job *job = KIO::storedGet( *it );
        connect( job, SIGNAL(result(KJob *)), SLOT(downloadComplete(KJob *)) );
      }
    } else { // we take anything
      addDataAttachment( data,
                         mimeType,
                         label );
    }
  }
}

void EditorAttachments::dropEvent( QDropEvent *event )
{
  handlePasteOrDrop( event->mimeData() );
}

void EditorAttachments::downloadComplete( KJob *job )
{
  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->ui()->setWindow( this );
    static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
  } else {
    addDataAttachment( static_cast<KIO::StoredTransferJob *>( job )->data(),
                       QString(),
                       static_cast<KIO::SimpleJob *>( job )->url().fileName() );
  }
}

void EditorAttachments::showAttachment( QListWidgetItem *item )
{
  AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>( item );
  if ( !attitem || !attitem->attachment() ) {
    return;
  }

  KCal::Attachment *att = attitem->attachment();
  if ( att->isUri() ) {
    emit openURL( att->uri() );
  } else {
    KRun::runUrl( mAttachments->tempFileForAttachment( att ), att->mimeType(), 0, true );
  }
}

void EditorAttachments::saveAttachment( QListWidgetItem *item )
{
  AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>( item );
  if ( !attitem || !attitem->attachment() ) {
    return;
  }

  KCal::Attachment *att = attitem->attachment();

  // get the saveas file name
  QString saveAsFile =  KFileDialog::getSaveFileName(
    att->label(),
    QString(), 0,
    i18nc( "@title", "Save  Attachment" ) );

  if ( saveAsFile.isEmpty() ||
       ( QFile( saveAsFile ).exists() &&
         ( KMessageBox::warningYesNo(
           0,
           i18nc( "@info", "%1 already exists. Do you want to overwrite it?",
                  saveAsFile ) ) == KMessageBox::No ) ) ) {
    return;
  }

  KUrl sourceUrl;
  if ( att->isUri() ) {
    sourceUrl = att->uri();
  } else {
    sourceUrl = mAttachments->tempFileForAttachment( att );
  }
  // save the attachment url
  if ( !KIO::NetAccess::file_copy( sourceUrl, KUrl( saveAsFile ) ) &&
       KIO::NetAccess::lastError() ) {
    KMessageBox::error( this, KIO::NetAccess::lastErrorString() );
  }
}

void EditorAttachments::slotAdd()
{
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachments );

  QPointer<AttachmentEditDialog> dlg = new AttachmentEditDialog( item, mAttachments );
  dlg->setCaption( i18nc( "@title", "Add Attachment" ) );
  if ( dlg->exec() == KDialog::Rejected ) {
    delete item;
  }
  delete dlg;
}

void EditorAttachments::slotEdit()
{
  for ( int itemIndex = 0; itemIndex < mAttachments->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachments->item( itemIndex );
    if ( item->isSelected() ) {
      AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>( item );
      if ( !attitem || !attitem->attachment() ) {
        return;
      }

      AttachmentEditDialog *dialog = new AttachmentEditDialog( attitem, mAttachments, false );
      dialog->setModal( false );
      connect( dialog, SIGNAL(hidden()), dialog, SLOT(delayedDestruct()) );
      dialog->show();
    }
  }
}

void EditorAttachments::slotRemove()
{
  QList<QListWidgetItem *> toDelete;
  for ( int itemIndex = 0; itemIndex < mAttachments->count(); ++itemIndex ) {
    QListWidgetItem *it = mAttachments->item( itemIndex );
    if ( it->isSelected() ) {
      AttachmentIconItem *item = static_cast<AttachmentIconItem *>( it );

      if ( !item ) {
        continue;
      }

      if ( KMessageBox::questionYesNo(
             this,
             i18nc( "@info",
                    "Do you really want to remove the attachment labeled \"%1\"?", item->label() ),
             i18nc( "@title:window", "Remove Attachment?" ) ) == KMessageBox::Yes ) {
        toDelete.append( it );
      }
    }
  }

  qDeleteAll( toDelete );
}

void EditorAttachments::slotShow()
{
  for ( int itemIndex = 0; itemIndex < mAttachments->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachments->item( itemIndex );
    if ( item->isSelected() ) {
      showAttachment( item );
    }
  }
}

void EditorAttachments::slotSaveAs()
{
  for ( int itemIndex = 0; itemIndex < mAttachments->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachments->item( itemIndex );
    if ( item->isSelected() ) {
      saveAttachment( item );
    }
  }
}

void EditorAttachments::setDefaults()
{
  mAttachments->clear();
}

void EditorAttachments::addUriAttachment( const QString &uri,
                                            const QString &mimeType,
                                            const QString &label,
                                            bool inLine )
{
  if ( !inLine ) {
    AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachments );
    item->setUri( uri );
    item->setLabel( label );
    if ( mimeType.isEmpty() ) {
      if ( uri.startsWith( QLatin1String( "uid:" ) ) ) {
        item->setMimeType( "text/directory" );
      } else if ( uri.startsWith( QLatin1String( "kmail:" ) ) ) {
        item->setMimeType( "message/rfc822" );
      } else if ( uri.startsWith( QLatin1String( "urn:x-ical" ) ) ) {
        item->setMimeType( "text/calendar" );
      } else if ( uri.startsWith( QLatin1String( "news:" ) ) ) {
        item->setMimeType( "message/news" );
      } else {
        item->setMimeType( KMimeType::findByUrl( uri )->name() );
      }
    }
  } else {
    QString tmpFile;
    if ( KIO::NetAccess::download( uri, tmpFile, this ) ) {
      QFile f( tmpFile );
      if ( !f.open( QIODevice::ReadOnly ) ) {
        return;
      }
      const QByteArray data = f.readAll();
      f.close();
      addDataAttachment( data, mimeType, label );
    }
    KIO::NetAccess::removeTempFile( tmpFile );
  }
}

void EditorAttachments::addDataAttachment( const QByteArray &data,
                                             const QString &mimeType,
                                             const QString &label )
{
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachments );

  QString nlabel = label;
  if ( mimeType == "message/rfc822" ) {
    // mail message. try to set the label from the mail Subject:
    KMime::Message msg;
    msg.setContent( data );
    msg.parse();
    nlabel = msg.subject()->asUnicodeString();
  }

  item->setData( data );
  item->setLabel( nlabel );
  if ( mimeType.isEmpty() ) {
    item->setMimeType( KMimeType::findByContent( data )->name() );
  } else {
    item->setMimeType( mimeType );
  }
}

void EditorAttachments::addAttachment( KCal::Attachment *attachment )
{
  new AttachmentIconItem( attachment, mAttachments );
}

void EditorAttachments::readIncidence( KCal::Incidence *i )
{
  mAttachments->clear();

  KCal::Attachment::List attachments = i->attachments();
  KCal::Attachment::List::ConstIterator it;
  for ( it = attachments.constBegin(); it != attachments.constEnd(); ++it ) {
    addAttachment( (*it) );
  }

  mUid = i->uid();
}

void EditorAttachments::fillIncidence( KCal::Incidence *i )
{
  i->clearAttachments();

  for ( int itemIndex = 0; itemIndex < mAttachments->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachments->item( itemIndex );
    AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>(item);
    if ( attitem ) {
      i->addAttachment( new KCal::Attachment( *( attitem->attachment() ) ) );
    }
  }
}

void EditorAttachments::slotItemRenamed ( QListWidgetItem *item )
{
  static_cast<AttachmentIconItem *>( item )->setLabel( item->text() );
}

void EditorAttachments::applyChanges()
{
}

void EditorAttachments::slotCopy()
{
  QApplication::clipboard()->setMimeData( mAttachments->mimeData(), QClipboard::Clipboard );
}

void EditorAttachments::slotCut()
{
  slotCopy();
  slotRemove();
}

void EditorAttachments::slotPaste()
{
  handlePasteOrDrop( QApplication::clipboard()->mimeData() );
}

void EditorAttachments::selectionChanged()
{
  bool selected = false;
  for ( int itemIndex = 0; itemIndex < mAttachments->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachments->item( itemIndex );
    if ( item->isSelected() ) {
      selected = true;
      break;
    }
  }
  mRemoveBtn->setEnabled( selected );
}

void EditorAttachments::contextMenu( const QPoint &pos )
{
  QListWidgetItem *item = mAttachments->itemAt( pos );
  const bool enable = item != 0;

  int numSelected = 0;
  for ( int itemIndex = 0; itemIndex < mAttachments->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachments->item( itemIndex );
    if ( item->isSelected() ) {
      numSelected++;
    }
  }

  mOpenAction->setEnabled( enable );
  //TODO: support saving multiple attachments into a directory
  mSaveAsAction->setEnabled( enable && numSelected == 1 );
  mCopyAction->setEnabled( enable && numSelected == 1 );
  mCutAction->setEnabled( enable && numSelected == 1 );
  mDeleteAction->setEnabled( enable );
  mEditAction->setEnabled( enable );
  mPopupMenu->exec( mAttachments->mapToGlobal( pos ) );
}

#include "editorattachments.moc"
