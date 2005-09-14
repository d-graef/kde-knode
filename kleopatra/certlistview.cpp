#include <config.h>
#include "certlistview.h"
#include <kurldrag.h>
#include <kdebug.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

CertKeyListView::CertKeyListView( const ColumnStrategy * strategy,
                                  const DisplayStrategy * display,
                                  QWidget * parent, const char * name, Qt::WFlags f )
  : Kleo::KeyListView( strategy, display, parent, name, f )
{
  viewport()->setAcceptDrops( true );
}

void CertKeyListView::contentsDragEnterEvent( QDragEnterEvent * event )
{
  //const char* fmt;
  //for (int i=0; (fmt = event->format(i)); i++)
  //  kdDebug() << fmt << endl;

  // We only accept URL drops. We'll check the mimetype later on.
  event->accept( Q3UriDrag::canDecode( event ) );
}

void CertKeyListView::contentsDragMoveEvent( QDragMoveEvent * event )
{
  event->accept( Q3UriDrag::canDecode( event ) );
}


void CertKeyListView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    // Don't let QListView do its stuff
}

void CertKeyListView::contentsDropEvent( QDropEvent * event )
{
  KURL::List lst;
  if ( KURLDrag::decode( event, lst ) ) {
    event->accept();
    emit dropped( lst );
  }
}

#include "certlistview.moc"
