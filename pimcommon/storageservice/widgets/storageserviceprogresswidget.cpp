/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "storageserviceprogresswidget.h"
#include "storageservice/storageserviceabstract.h"

#include <KLocalizedString>
#include <KLocale>
#include <KGlobal>
#include <KIcon>

#include <QProgressBar>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QToolButton>

using namespace PimCommon;

StorageServiceProgressWidget::StorageServiceProgressWidget(PimCommon::StorageServiceAbstract *service, QWidget *parent)
    : QFrame(parent),
      mCancel(0),
      mStorageService(service)
{
    QHBoxLayout *box = new QHBoxLayout( this );
    box->setMargin(0);
    box->setSpacing(0);
    mProgressInfo = new QLabel;
    box->addWidget(mProgressInfo);

    mProgressBar = new QProgressBar;
    mProgressBar->setMinimum(0);
    mProgressBar->setMaximum(100);
    box->addWidget(mProgressBar);

    if (service) {
        mCancel = new QToolButton;
        mCancel->setIcon(KIcon(QLatin1String("dialog-cancel")));
        connect(mCancel, SIGNAL(clicked()), this, SLOT(slotCancelTask()));
        box->addWidget(mCancel);
    }
}

StorageServiceProgressWidget::~StorageServiceProgressWidget()
{

}

void StorageServiceProgressWidget::slotCancelTask()
{
    mStorageService->cancelUploadDownloadFile();
}

void StorageServiceProgressWidget::reset()
{
    mProgressBar->setValue(0);
}

void StorageServiceProgressWidget::setBusyIndicator(bool busy)
{
    mProgressInfo->clear();
    if (busy) {
        mProgressBar->setMinimum(0);
        mProgressBar->setMaximum(0);
    } else {
        mProgressBar->setMinimum(0);
        mProgressBar->setMaximum(100);
    }
}

void StorageServiceProgressWidget::setProgressValue(qint64 done, qint64 total)
{
    mProgressInfo->setText(i18n("%1 on %2", KGlobal::locale()->formatByteSize(done), KGlobal::locale()->formatByteSize(total)));
    if (total > 0)
       mProgressBar->setValue((100*done)/total);
    else
       mProgressBar->setValue(100);
}

void StorageServiceProgressWidget::hideEvent(QHideEvent *e)
{
    if (!e->spontaneous()) {
        mProgressBar->reset();
    }
    QFrame::hideEvent(e);
}

#include "moc_storageserviceprogresswidget.cpp"
