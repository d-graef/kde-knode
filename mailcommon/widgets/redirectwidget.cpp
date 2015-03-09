/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "redirectwidget.h"
#include "kernel/mailkernel.h"

#include <QTreeView>

#include <messagecomposer/composer/composerlineedit.h>

#include <messageviewer/utils/autoqpointer.h>

#include <Akonadi/Contact/EmailAddressSelectionDialog>

#include <KIcon>
#include <QHBoxLayout>
#include <QPushButton>
#include <KLocalizedString>

using namespace MailCommon;

RedirectWidget::RedirectWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setSpacing( 0 );
    hbox->setMargin( 0 );
    hbox->setAlignment(Qt::AlignRight);
    setLayout(hbox);

    mEdit = new MessageComposer::ComposerLineEdit( true );
    mEdit->setRecentAddressConfig( KernelIf->config().data() );
    mEdit->setMinimumWidth( 300 );
    mEdit->setClearButtonShown( true );
    hbox->addWidget(mEdit);

    QPushButton *BtnTo = new QPushButton( QString() );
    BtnTo->setIcon( KIcon( QLatin1String("help-contents") ) );
    BtnTo->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    BtnTo->setMinimumSize( BtnTo->sizeHint() * 1.2 );
    BtnTo->setToolTip( i18n( "Use the Address-Selection Dialog" ) );
    BtnTo->setWhatsThis( i18n( "This button opens a separate dialog "
                               "where you can select recipients out "
                               "of all available addresses." ) );
    hbox->addWidget(BtnTo);
    connect( BtnTo, SIGNAL(clicked()), SLOT(slotAddressSelection()) );

    connect( mEdit, SIGNAL(textChanged(QString)), SIGNAL(addressChanged(QString)) );
}

RedirectWidget::~RedirectWidget()
{

}

QString RedirectWidget::resend()
{
    mResendStr = mEdit->text();
    return mResendStr;
}

void RedirectWidget::setFocus()
{
    mEdit->setFocus();
}

void RedirectWidget::slotAddressSelection()
{
    MessageViewer::AutoQPointer<Akonadi::EmailAddressSelectionDialog> dlg(
                new Akonadi::EmailAddressSelectionDialog( this ) );

    dlg->view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );

    mResendStr = mEdit->text();

    if ( dlg->exec() != KDialog::Rejected && dlg ) {
        QStringList addresses;
        foreach ( const Akonadi::EmailAddressSelection &selection, dlg->selectedAddresses() ) {
            addresses << selection.quotedEmail();
        }

        if ( !mResendStr.isEmpty() ) {
            addresses.prepend( mResendStr );
        }

        mEdit->setText( addresses.join( QLatin1String(", ") ) );
        mEdit->setModified( true );
    }
}