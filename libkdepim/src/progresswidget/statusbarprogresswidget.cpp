/*
  statusbarprogresswidget.cpp

  (C) 2004 Till Adam <adam@kde.org>
           Don Sanders
           David Faure <dfaure@kde.org>

  Copyright 2004 David Faure <faure@kde.org>
  Includes StatusbarProgressWidget which is based on KIOLittleProgressDlg
  by Matt Koss <koss@miesto.sk>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2 or above,
  as published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "statusbarprogresswidget.h"
#include "progressdialog.h"
#include "ssllabel.h"
using KPIM::SSLLabel;
#include "progressmanager.h"
using KPIM::ProgressItem;
using KPIM::ProgressManager;

#include <KLocalizedString>
#include <KIconLoader>

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>

using namespace KPIM;

//-----------------------------------------------------------------------------
StatusbarProgressWidget::StatusbarProgressWidget(ProgressDialog *progressDialog, QWidget *parent, bool button)
    : QFrame(parent),
      mShowTypeProgressItem(0),
      m_bShowDetailedProgress(false),
      mCurrentItem(Q_NULLPTR),
      mProgressDialog(progressDialog),
      mDelayTimer(Q_NULLPTR),
      mBusyTimer(Q_NULLPTR),
      mCleanTimer(Q_NULLPTR)
{
    m_bShowButton = button;
    int w = fontMetrics().width(QStringLiteral(" 999.9 kB/s 00:00:01 ")) + 8;
    QHBoxLayout *box = new QHBoxLayout(this);
    box->setMargin(0);
    box->setSpacing(0);

    m_pButton = new QPushButton(this);
    m_pButton->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
                                         QSizePolicy::Minimum));
    m_pButton->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    box->addWidget(m_pButton);
    stack = new QStackedWidget(this);
    int maximumHeight = qMax(m_pButton->iconSize().height(), fontMetrics().height());
    stack->setMaximumHeight(maximumHeight);
    box->addWidget(stack);

    m_sslLabel = new SSLLabel(this);
    box->addWidget(m_sslLabel);

    m_pButton->setToolTip(i18n("Open detailed progress dialog"));

    m_pProgressBar = new QProgressBar(this);
    m_pProgressBar->installEventFilter(this);
    m_pProgressBar->setMinimumWidth(w);
    stack->insertWidget(1, m_pProgressBar);

    m_pLabel = new QLabel(QString(), this);
    m_pLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_pLabel->installEventFilter(this);
    m_pLabel->setMinimumWidth(w);
    stack->insertWidget(2, m_pLabel);
    m_pButton->setMaximumHeight(maximumHeight);
    setMinimumWidth(minimumSizeHint().width());

    mode = None;
    setMode();

    connect(m_pButton, &QAbstractButton::clicked,
            this, &StatusbarProgressWidget::slotProgressButtonClicked);

    connect(ProgressManager::instance(), &ProgressManager::progressItemAdded,
            this, &StatusbarProgressWidget::slotProgressItemAdded);
    connect(ProgressManager::instance(), &ProgressManager::progressItemCompleted,
            this, &StatusbarProgressWidget::slotProgressItemCompleted);
    connect(ProgressManager::instance(), &ProgressManager::progressItemUsesBusyIndicator,
            this, &StatusbarProgressWidget::updateBusyMode);

    connect(progressDialog, &ProgressDialog::visibilityChanged,
            this, &StatusbarProgressWidget::slotProgressDialogVisible);

    mDelayTimer = new QTimer(this);
    mDelayTimer->setSingleShot(true);
    connect(mDelayTimer, &QTimer::timeout,
            this, &StatusbarProgressWidget::slotShowItemDelayed);

    mCleanTimer = new QTimer(this);
    mCleanTimer->setSingleShot(true);
    connect(mCleanTimer, &QTimer::timeout,
            this, &StatusbarProgressWidget::slotClean);
}

void StatusbarProgressWidget::setShowTypeProgressItem(unsigned int type)
{
    mShowTypeProgressItem = type;
}

// There are three cases: no progressitem, one progressitem (connect to it directly),
// or many progressitems (display busy indicator). Let's call them 0,1,N.
// In slot..Added we can only end up in 1 or N.
// In slot..Removed we can end up in 0, 1, or we can stay in N if we were already.

void StatusbarProgressWidget::updateBusyMode(KPIM::ProgressItem *item)
{
    if (item->typeProgressItem() == mShowTypeProgressItem) {
        connectSingleItem(); // if going to 1 item
        if (mCurrentItem) {   // Exactly one item
            delete mBusyTimer;
            mBusyTimer = Q_NULLPTR;
            mDelayTimer->start(1000);
        } else { // N items
            if (!mBusyTimer) {
                mBusyTimer = new QTimer(this);
                connect(mBusyTimer, &QTimer::timeout,
                        this, &StatusbarProgressWidget::slotBusyIndicator);
                mDelayTimer->start(1000);
            }
        }
    }
}

void StatusbarProgressWidget::slotProgressItemAdded(ProgressItem *item)
{
    if (item->parent()) {
        return;    // we are only interested in top level items
    }

    updateBusyMode(item);
}

void StatusbarProgressWidget::slotProgressItemCompleted(ProgressItem *item)
{
    if (item->parent()) {
        item->deleteLater();
        item = Q_NULLPTR;
        return; // we are only interested in top level items
    }
    item->deleteLater();
    item = Q_NULLPTR;
    connectSingleItem(); // if going back to 1 item
    if (ProgressManager::instance()->isEmpty()) {   // No item
        // Done. In 5s the progress-widget will close, then we can clean up the statusbar
        mCleanTimer->start(5000);
    } else if (mCurrentItem) {   // Exactly one item
        delete mBusyTimer;
        mBusyTimer = Q_NULLPTR;
        activateSingleItemMode();
    }
}

void StatusbarProgressWidget::connectSingleItem()
{
    if (mCurrentItem) {
        disconnect(mCurrentItem, &ProgressItem::progressItemProgress,
                   this, &StatusbarProgressWidget::slotProgressItemProgress);
        mCurrentItem = Q_NULLPTR;
    }
    mCurrentItem = ProgressManager::instance()->singleItem();
    if (mCurrentItem) {
        connect(mCurrentItem, &ProgressItem::progressItemProgress,
                this, &StatusbarProgressWidget::slotProgressItemProgress);
    }
}

void StatusbarProgressWidget::activateSingleItemMode()
{
    m_pProgressBar->setMaximum(100);
    m_pProgressBar->setValue(mCurrentItem->progress());
    m_pProgressBar->setTextVisible(true);
}

void StatusbarProgressWidget::slotShowItemDelayed()
{
    bool noItems = ProgressManager::instance()->isEmpty();
    if (mCurrentItem) {
        activateSingleItemMode();
    } else if (!noItems) {   // N items
        m_pProgressBar->setMaximum(0);
        m_pProgressBar->setTextVisible(false);
        Q_ASSERT(mBusyTimer);
        if (mBusyTimer) {
            mBusyTimer->start(100);
        }
    }

    if (!noItems && mode == None) {
        mode = Progress;
        setMode();
    }
}

void StatusbarProgressWidget::slotBusyIndicator()
{
    const int p = m_pProgressBar->value();
    m_pProgressBar->setValue(p + 10);
}

void StatusbarProgressWidget::slotProgressItemProgress(ProgressItem *item, unsigned int value)
{
    Q_ASSERT(item == mCurrentItem);  // the only one we should be connected to
    Q_UNUSED(item);
    m_pProgressBar->setValue(value);
}

void StatusbarProgressWidget::setMode()
{
    switch (mode) {
    case None:
        if (m_bShowButton) {
            m_pButton->hide();
        }
        m_sslLabel->setState(SSLLabel::Done);
        // show the empty label in order to make the status bar look better
        stack->show();
        stack->setCurrentWidget(m_pLabel);
        break;

    case Progress:
        stack->show();
        stack->setCurrentWidget(m_pProgressBar);
        if (m_bShowButton) {
            m_pButton->show();
        }
        m_sslLabel->setState(m_sslLabel->lastState());
        break;
    }
}

void StatusbarProgressWidget::slotClean()
{
    // check if a new item showed up since we started the timer. If not, clear
    if (ProgressManager::instance()->isEmpty()) {
        m_pProgressBar->setValue(0);
        //m_pLabel->clear();
        mode = None;
        setMode();
    }
}

bool StatusbarProgressWidget::eventFilter(QObject *, QEvent *ev)
{
    if (ev->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = (QMouseEvent *)ev;

        if (e->button() == Qt::LeftButton && mode != None) {      // toggle view on left mouse button
            // Consensus seems to be that we should show/hide the fancy dialog when the user
            // clicks anywhere in the small one.
            slotProgressButtonClicked();
            return true;
        }
    }
    return false;
}

void StatusbarProgressWidget::slotProgressButtonClicked()
{
    if (m_bShowDetailedProgress) {
        m_bShowDetailedProgress = false;
        m_pButton->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
        m_pButton->setToolTip(i18n("Show detailed progress window"));
    } else {
        m_bShowDetailedProgress = true;
        m_pButton->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
        m_pButton->setToolTip(i18n("Hide detailed progress window"));
    }
    mProgressDialog->slotToggleVisibility();
}

void StatusbarProgressWidget::slotProgressDialogVisible(bool b)
{
    // Update the hide/show button when the detailed one is shown/hidden
    if (b) {
        setMode();
    }
}

