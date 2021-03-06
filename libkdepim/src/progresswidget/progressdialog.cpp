/** -*- c++ -*-
 * progressdialog.cpp
 *
 *  Copyright (c) 2004 Till Adam <adam@kde.org>,
 *                     David Faure <faure@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#include "progressdialog.h"
#include "ssllabel.h"

#include <QHBoxLayout>
#include <KLocalizedString>
#include <KIconLoader>

#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QTimer>
#include <QLayout>
#include <QVBoxLayout>

namespace KPIM
{

static const int MAX_LABEL_WIDTH = 650;

class TransactionItem;

TransactionItemView::TransactionItemView(QWidget *parent, const char *name)
    : QScrollArea(parent)
{
    setObjectName(QLatin1String(name));
    setFrameStyle(NoFrame);
    mBigBox = new QWidget(this);
    QVBoxLayout *mBigBoxVBoxLayout = new QVBoxLayout(mBigBox);
    mBigBoxVBoxLayout->setMargin(0);
    setWidget(mBigBox);
    setWidgetResizable(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

TransactionItem *TransactionItemView::addTransactionItem(ProgressItem *item, bool first)
{
    TransactionItem *ti = new TransactionItem(mBigBox, item, first);
    mBigBox->layout()->addWidget(ti);

    resize(mBigBox->width(), mBigBox->height());

    return ti;
}

void TransactionItemView::resizeEvent(QResizeEvent *event)
{
    // Tell the layout in the parent (progressdialog) that our size changed
    updateGeometry();

    QSize sz = parentWidget()->sizeHint();
    int currentWidth = parentWidget()->width();

    // Don't resize to sz.width() every time when it only reduces a little bit
    if (currentWidth < sz.width() || currentWidth > sz.width() + 100) {
        currentWidth = sz.width();
    }
    parentWidget()->resize(currentWidth, sz.height());

    QScrollArea::resizeEvent(event);
}

QSize TransactionItemView::sizeHint() const
{
    return minimumSizeHint();
}

QSize TransactionItemView::minimumSizeHint() const
{
    int f = 2 * frameWidth();
    // Make room for a vertical scrollbar in all cases, to avoid a horizontal one
    int vsbExt = verticalScrollBar()->sizeHint().width();
    int minw = topLevelWidget()->width() / 3;
    int maxh = topLevelWidget()->height() / 2;
    QSize sz(mBigBox->minimumSizeHint());
    sz.setWidth(qMax(sz.width(), minw) + f + vsbExt);
    sz.setHeight(qMin(sz.height(), maxh) + f);
    return sz;
}

void TransactionItemView::slotLayoutFirstItem()
{
    //This slot is called whenever a TransactionItem is deleted, so this is a
    //good place to call updateGeometry(), so our parent takes the new size
    //into account and resizes.
    updateGeometry();

    /*
     The below relies on some details in Qt's behaviour regarding deleting
     objects. This slot is called from the destroyed signal of an item just
     going away. That item is at that point still in the  list of chilren, but
     since the vtable is already gone, it will have type QObject. The first
     one with both the right name and the right class therefor is what will
     be the first item very shortly. That's the one we want to remove the
     hline for.
    */
    TransactionItem *ti = mBigBox->findChild<KPIM::TransactionItem *>(QStringLiteral("TransactionItem"));
    if (ti) {
        ti->hideHLine();
    }
}

// ----------------------------------------------------------------------------

TransactionItem::TransactionItem(QWidget *parent,
                                 ProgressItem *item, bool first)
    : QWidget(parent), mCancelButton(Q_NULLPTR), mItem(item)

{
    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->setSpacing(2);
    vboxLayout->setMargin(2);
    setLayout(vboxLayout);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    mFrame = new QFrame(this);
    mFrame->setFrameShape(QFrame::HLine);
    mFrame->setFrameShadow(QFrame::Raised);
    mFrame->show();
    layout()->addWidget(mFrame);

    QWidget *h = new QWidget(this);
    QHBoxLayout *hHBoxLayout = new QHBoxLayout(h);
    hHBoxLayout->setMargin(0);
    hHBoxLayout->setSpacing(5);
    layout()->addWidget(h);

    mItemLabel =
        new QLabel(fontMetrics().elidedText(item->label(), Qt::ElideRight, MAX_LABEL_WIDTH), h);
    h->layout()->addWidget(mItemLabel);
    h->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    mProgress = new QProgressBar(h);
    hHBoxLayout->addWidget(mProgress);
    mProgress->setMaximum(100);
    mProgress->setValue(item->progress());
    h->layout()->addWidget(mProgress);

    if (item->canBeCanceled()) {
        mCancelButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-cancel")), QString(), h);
        hHBoxLayout->addWidget(mCancelButton);
        mCancelButton->setToolTip(i18n("Cancel this operation."));
        connect(mCancelButton, &QAbstractButton::clicked,
                this, &TransactionItem::slotItemCanceled);
        h->layout()->addWidget(mCancelButton);
    }

    h = new QWidget(this);
    hHBoxLayout = new QHBoxLayout(h);
    hHBoxLayout->setMargin(0);
    hHBoxLayout->setSpacing(5);
    h->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    layout()->addWidget(h);
    mSSLLabel = new SSLLabel(h);
    hHBoxLayout->addWidget(mSSLLabel);
    mSSLLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    h->layout()->addWidget(mSSLLabel);
    mItemStatus = new QLabel(h);
    hHBoxLayout->addWidget(mItemStatus);
    mItemStatus->setTextFormat(Qt::RichText);
    mItemStatus->setText(
        fontMetrics().elidedText(item->status(), Qt::ElideRight, MAX_LABEL_WIDTH));
    h->layout()->addWidget(mItemStatus);
    setCryptoStatus(item->cryptoStatus());
    if (first) {
        hideHLine();
    }
}

TransactionItem::~TransactionItem()
{
}

void TransactionItem::hideHLine()
{
    mFrame->hide();
}

void TransactionItem::setProgress(int progress)
{
    mProgress->setValue(progress);
}

void TransactionItem::setLabel(const QString &label)
{
    mItemLabel->setText(fontMetrics().elidedText(label, Qt::ElideRight, MAX_LABEL_WIDTH));
}

void TransactionItem::setStatus(const QString &status)
{
    mItemStatus->setText(fontMetrics().elidedText(status, Qt::ElideRight, MAX_LABEL_WIDTH));
}

void TransactionItem::setCryptoStatus(KPIM::ProgressItem::CryptoStatus status)
{
    switch (status) {
    case KPIM::ProgressItem::Encrypted:
        mSSLLabel->setEncrypted(SSLLabel::Encrypted);
        break;
    case KPIM::ProgressItem::Unencrypted:
        mSSLLabel->setEncrypted(SSLLabel::Unencrypted);
        break;
    case KPIM::ProgressItem::Unknown:
        mSSLLabel->setEncrypted(SSLLabel::Unknown);
        break;
    }
    mSSLLabel->setState(mSSLLabel->lastState());
}

void TransactionItem::setTotalSteps(int totalSteps)
{
    mProgress->setMaximum(totalSteps);
}

void TransactionItem::slotItemCanceled()
{
    if (mItem) {
        mItem->cancel();
    }
}

void TransactionItem::addSubTransaction(ProgressItem *item)
{
    Q_UNUSED(item);
}

// ---------------------------------------------------------------------------

ProgressDialog::ProgressDialog(QWidget *alignWidget, QWidget *parent)
    : OverlayWidget(alignWidget, parent),
      mShowTypeProgressItem(0),
      mWasLastShown(false)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);   // QFrame

    setAutoFillBackground(true);

    mScrollView = new TransactionItemView(this, "ProgressScrollView");
    layout()->addWidget(mScrollView);
    /*
    * Get the singleton ProgressManager item which will inform us of
    * appearing and vanishing items.
    */
    ProgressManager *pm = ProgressManager::instance();
    connect(pm, &ProgressManager::progressItemAdded,
            this, &ProgressDialog::slotTransactionAdded);
    connect(pm, &ProgressManager::progressItemCompleted,
            this, &ProgressDialog::slotTransactionCompleted);
    connect(pm, &ProgressManager::progressItemProgress,
            this, &ProgressDialog::slotTransactionProgress);
    connect(pm, &ProgressManager::progressItemStatus,
            this, &ProgressDialog::slotTransactionStatus);
    connect(pm, &ProgressManager::progressItemLabel,
            this, &ProgressDialog::slotTransactionLabel);
    connect(pm, &ProgressManager::progressItemCryptoStatus,
            this, &ProgressDialog::slotTransactionCryptoStatus);
    connect(pm, &ProgressManager::progressItemUsesBusyIndicator,
            this, &ProgressDialog::slotTransactionUsesBusyIndicator);
    connect(pm, &ProgressManager::showProgressDialog,
            this, &ProgressDialog::slotShow);
}

void ProgressDialog::closeEvent(QCloseEvent *e)
{
    e->accept();
    hide();
}

/*
 *  Destructor
 */
ProgressDialog::~ProgressDialog()
{
    // no need to delete child widgets.
}

void ProgressDialog::setShowTypeProgressItem(unsigned int type)
{
    mShowTypeProgressItem = type;
}

void ProgressDialog::slotTransactionAdded(ProgressItem *item)
{
    if (item->typeProgressItem() == mShowTypeProgressItem) {
        if (item->parent()) {
            if (mTransactionsToListviewItems.contains(item->parent())) {
                TransactionItem *parent = mTransactionsToListviewItems[ item->parent() ];
                parent->addSubTransaction(item);
            }
        } else {
            const bool first = mTransactionsToListviewItems.empty();
            TransactionItem *ti = mScrollView->addTransactionItem(item, first);
            if (ti) {
                mTransactionsToListviewItems.insert(item, ti);
            }
            if (first && mWasLastShown) {
                QTimer::singleShot(1000, this, &ProgressDialog::slotShow);
            }
        }
    }
}

void ProgressDialog::slotTransactionCompleted(ProgressItem *item)
{
    if (mTransactionsToListviewItems.contains(item)) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        mTransactionsToListviewItems.remove(item);
        ti->setItemComplete();
        QTimer::singleShot(3000, ti, &QObject::deleteLater);
        // see the slot for comments as to why that works
        connect(ti, &QObject::destroyed,
                mScrollView, &TransactionItemView::slotLayoutFirstItem);
    }
    // This was the last item, hide.
    if (mTransactionsToListviewItems.empty()) {
        QTimer::singleShot(3000, this, &ProgressDialog::slotHide);
    }
}

void ProgressDialog::slotTransactionCanceled(ProgressItem *)
{
}

void ProgressDialog::slotTransactionProgress(ProgressItem *item,
        unsigned int progress)
{
    if (mTransactionsToListviewItems.contains(item)) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        ti->setProgress(progress);
    }
}

void ProgressDialog::slotTransactionStatus(ProgressItem *item,
        const QString &status)
{
    if (mTransactionsToListviewItems.contains(item)) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        ti->setStatus(status);
    }
}

void ProgressDialog::slotTransactionLabel(ProgressItem *item,
        const QString &label)
{
    if (mTransactionsToListviewItems.contains(item)) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        ti->setLabel(label);
    }
}

void ProgressDialog::slotTransactionCryptoStatus(ProgressItem *item,
        KPIM::ProgressItem::CryptoStatus value)
{
    if (mTransactionsToListviewItems.contains(item)) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        ti->setCryptoStatus(value);
    }
}

void ProgressDialog::slotTransactionUsesBusyIndicator(KPIM::ProgressItem *item, bool value)
{
    if (mTransactionsToListviewItems.contains(item)) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        if (value) {
            ti->setTotalSteps(0);
        } else {
            ti->setTotalSteps(100);
        }
    }
}

void ProgressDialog::slotShow()
{
    setVisible(true);
}

void ProgressDialog::slotHide()
{
    // check if a new item showed up since we started the timer. If not, hide
    if (mTransactionsToListviewItems.isEmpty()) {
        setVisible(false);
    }
}

void ProgressDialog::slotClose()
{
    mWasLastShown = false;
    setVisible(false);
}

void ProgressDialog::setVisible(bool b)
{
    OverlayWidget::setVisible(b);
    Q_EMIT visibilityChanged(b);
}

void ProgressDialog::slotToggleVisibility()
{
    /* Since we are only hiding with a timeout, there is a short period of
    * time where the last item is still visible, but clicking on it in
    * the statusbarwidget should not display the dialog, because there
    * are no items to be shown anymore. Guard against that.
    */
    mWasLastShown = isHidden();
    if (!isHidden() || !mTransactionsToListviewItems.isEmpty()) {
        setVisible(isHidden());
    }
}

}

