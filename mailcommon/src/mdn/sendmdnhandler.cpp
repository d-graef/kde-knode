/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net,
     Author Tobias Koenig <tokoe@kdab.com>

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

#include "sendmdnhandler.h"

#include "mailcommon/mailinterfaces.h"
#include "kernel/mailkernel.h"
#include "util/mailutil.h"
#include "filter/mdnadvicedialog.h"
#include "mailcommon_debug.h"
#include <MessageComposer/MessageFactory>
#include <MessageComposer/MessageSender>
#include <messagecore/messagehelpers.h>
#include <messageviewer/messageviewersettings.h>

#include <Collection>
#include <Item>
#include <Akonadi/KMime/MessageFlags>

#include <QQueue>
#include <QTimer>

using namespace MailCommon;

class Q_DECL_HIDDEN SendMdnHandler::Private
{
public:
    Private(SendMdnHandler *qq, IKernel *kernel)
        : q(qq), mKernel(kernel)
    {
    }

    void handleMessages();

    SendMdnHandler *q;
    IKernel *mKernel;
    QQueue<Akonadi::Item> mItemQueue;
    QTimer mTimer;
};

void SendMdnHandler::Private::handleMessages()
{
    while (!mItemQueue.isEmpty()) {
        Akonadi::Item item = mItemQueue.dequeue();

#if 0
        // should we send an MDN?
        if (MessageViewer::MessageViewerSettings::notSendWhenEncrypted() &&
                message()->encryptionState() != KMMsgNotEncrypted &&
                message()->encryptionState() != KMMsgEncryptionStateUnknown) {
            return;
        }
#else
        qCDebug(MAILCOMMON_LOG) << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif

        const Akonadi::Collection collection = item.parentCollection();
        if (collection.isValid() &&
                (CommonKernel->folderIsSentMailFolder(collection) ||
                 CommonKernel->folderIsTrash(collection) ||
                 CommonKernel->folderIsDraftOrOutbox(collection) ||
                 CommonKernel->folderIsTemplates(collection))) {
            continue;
        }

        const KMime::Message::Ptr message = MessageCore::Util::message(item);
        if (!message) {
            continue;
        }

        const QPair<bool, KMime::MDN::SendingMode> mdnSend =
            MDNAdviceHelper::instance()->checkAndSetMDNInfo(item, KMime::MDN::Displayed);
        if (mdnSend.first) {
            const int quote =  MessageViewer::MessageViewerSettings::self()->quoteMessage();

            MessageComposer::MessageFactory factory(message, Akonadi::Item().id());
            factory.setIdentityManager(mKernel->identityManager());
            factory.setFolderIdentity(MailCommon::Util::folderIdentity(item));

            const KMime::Message::Ptr mdn =
                factory.createMDN(KMime::MDN::ManualAction, KMime::MDN::Displayed, mdnSend.second, quote);
            if (mdn) {
                if (!mKernel->msgSender()->send(mdn)) {
                    qCDebug(MAILCOMMON_LOG) << "Sending failed.";
                }
            }
        }
    }
}

SendMdnHandler::SendMdnHandler(IKernel *kernel, QObject *parent)
    : QObject(parent), d(new Private(this, kernel))
{
    d->mTimer.setSingleShot(true);
    connect(&d->mTimer, SIGNAL(timeout()), this, SLOT(handleMessages()));
}

SendMdnHandler::~SendMdnHandler()
{
    delete d;
}

void SendMdnHandler::setItem(const Akonadi::Item &item)
{
    if (item.hasFlag(Akonadi::MessageFlags::Seen)) {
        return;
    }

    d->mTimer.stop();

    d->mItemQueue.enqueue(item);

    if (MessageViewer::MessageViewerSettings::self()->delayedMarkAsRead() &&
            MessageViewer::MessageViewerSettings::self()->delayedMarkTime() != 0) {
        d->mTimer.start(MessageViewer::MessageViewerSettings::self()->delayedMarkTime() * 1000);
        return;
    }

    d->handleMessages();
}

#include "moc_sendmdnhandler.cpp"
