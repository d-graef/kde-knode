//
/***************************************************************************
                          kmsystemtray.cpp  -  description
                             ------------------
    begin                : Fri Aug 31 22:38:44 EDT 2001
    copyright            : (C) 2001 by Ryan Breen
    email                : ryan@porivo.com

    Copyright (c) 2010-2015 Montel Laurent <montel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmsystemtray.h"
#include "kmmainwidget.h"
#include "settings/kmailsettings.h"
#include "mailcommon/mailutil.h"
#include "MailCommon/MailKernel"
#include "MailCommon/FolderTreeView"
#include <AkonadiCore/NewMailNotifierAttribute>

#include <kiconloader.h>
#include <kcolorscheme.h>
#include <kwindowsystem.h>
#include "kmail_debug.h"
#include <QMenu>
#include <KLocalizedString>
#include <QAction>
#include <KActionMenu>
#include "widgets/kactionmenutransport.h"
#include <KActionCollection>
#include <QPainter>

#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/EntityTreeModel>
#include <QFontDatabase>
#include <AkonadiCore/EntityMimeTypeFilterModel>

using namespace MailCommon;

/**
 * Construct a KSystemTray icon to be displayed when new mail
 * has arrived in a non-system folder.  The KMSystemTray listens
 * for updateNewMessageNotification events from each non-system
 * KMFolder and maintains a store of all folders with unread
 * messages.
 *
 * The KMSystemTray also provides a popup menu listing each folder
 * with its count of unread messages, allowing the user to jump
 * to the first unread message in each folder.
 */
namespace KMail
{
KMSystemTray::KMSystemTray(QObject *parent)
    : KStatusNotifierItem(parent),
      mIcon(QIcon::fromTheme(QStringLiteral("mail-unread-new"))),
      mDesktopOfMainWin(0),
      mMode(KMailSettings::EnumSystemTrayPolicy::ShowOnUnread),
      mCount(0),
      mShowUnreadMailCount(true),
      mIconNotificationsEnabled(true),
      mNewMessagesPopup(Q_NULLPTR),
      mSendQueued(Q_NULLPTR)
{
    qCDebug(KMAIL_LOG) << "Initting systray";
    setToolTipTitle(i18n("KMail"));
    setToolTipIconByName(QStringLiteral("kmail"));
    setIconByName(QStringLiteral("kmail"));

    KMMainWidget *mainWidget = kmkernel->getKMMainWidget();
    if (mainWidget) {
        QWidget *mainWin = mainWidget->window();
        if (mainWin) {
            mDesktopOfMainWin = KWindowInfo(mainWin->winId(), NET::WMDesktop).desktop();
        }
    }

    connect(this, &KMSystemTray::activateRequested, this, &KMSystemTray::slotActivated);
    connect(contextMenu(), &QMenu::aboutToShow,
            this, &KMSystemTray::slotContextMenuAboutToShow);

    connect(kmkernel->folderCollectionMonitor(), &Akonadi::Monitor::collectionStatisticsChanged, this, &KMSystemTray::slotCollectionStatisticsChanged);

    connect(kmkernel->folderCollectionMonitor(), &Akonadi::Monitor::collectionAdded, this, &KMSystemTray::initListOfCollection);
    connect(kmkernel->folderCollectionMonitor(), &Akonadi::Monitor::collectionRemoved, this, &KMSystemTray::initListOfCollection);
    connect(kmkernel->folderCollectionMonitor(), &Akonadi::Monitor::collectionSubscribed, this, &KMSystemTray::initListOfCollection);
    connect(kmkernel->folderCollectionMonitor(), &Akonadi::Monitor::collectionUnsubscribed, this, &KMSystemTray::initListOfCollection);

    initListOfCollection();

}

bool KMSystemTray::buildPopupMenu()
{
    KMMainWidget *mainWidget = kmkernel->getKMMainWidget();
    if (!mainWidget || kmkernel->shuttingDown()) {
        return false;
    }

    if (!contextMenu()) {
        setContextMenu(new QMenu());
    }

    contextMenu()->clear();
    contextMenu()->setIcon(qApp->windowIcon());
    contextMenu()->setTitle(i18n("KMail"));
    QAction *action;
    if ((action = mainWidget->action(QStringLiteral("check_mail")))) {
        contextMenu()->addAction(action);
    }
    if ((action = mainWidget->action(QStringLiteral("check_mail_in")))) {
        contextMenu()->addAction(action);
    }

    mSendQueued = mainWidget->sendQueuedAction();
    contextMenu()->addAction(mSendQueued);
    contextMenu()->addAction(mainWidget->sendQueueViaMenu());

    contextMenu()->addSeparator();
    if ((action = mainWidget->action(QStringLiteral("new_message")))) {
        contextMenu()->addAction(action);
    }
    if ((action = mainWidget->action(QStringLiteral("kmail_configure_kmail")))) {
        contextMenu()->addAction(action);
    }
    contextMenu()->addSeparator();
    if ((action = mainWidget->action(QStringLiteral("akonadi_work_offline")))) {
        contextMenu()->addAction(action);
    }
    contextMenu()->addSeparator();

    if ((action = mainWidget->action(QStringLiteral("file_quit")))) {
        contextMenu()->addAction(action);
    }
    return true;
}

KMSystemTray::~KMSystemTray()
{
}

void KMSystemTray::setShowUnreadCount(bool showUnreadCount)
{
    if (mShowUnreadMailCount == showUnreadCount) {
        return;
    }
    mShowUnreadMailCount = showUnreadCount;
    updateSystemTray();
}

void KMSystemTray::setMode(int newMode)
{
    if (newMode == mMode) {
        return;
    }

    qCDebug(KMAIL_LOG) << "Setting systray mMode to" << newMode;
    mMode = newMode;

    switch (mMode) {
    case KMailSettings::EnumSystemTrayPolicy::ShowAlways:
        setStatus(KStatusNotifierItem::Active);
        break;
    case KMailSettings::EnumSystemTrayPolicy::ShowOnUnread:
        setStatus(mCount > 0 ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive);
        break;
    default:
        qCDebug(KMAIL_LOG) << "Unknown systray mode" << mMode;
    }
}

int KMSystemTray::mode() const
{
    return mMode;
}

void KMSystemTray::slotGeneralFontChanged()
{
    updateSystemTray();
}

void KMSystemTray::slotGeneralPaletteChanged()
{
    const KColorScheme scheme(QPalette::Active, KColorScheme::View);
    mTextColor = scheme.foreground(KColorScheme::LinkText).color();
    updateSystemTray();
}

/**
 * Update the count of unread messages.  If there are unread messages,
 * overlay the count on top of a transparent version of the KMail icon.
 * If there is no unread mail, restore the normal KMail icon.
 */
void KMSystemTray::updateCount()
{
    if (mCount == 0 || !mIconNotificationsEnabled) {
        setIconByName(QStringLiteral("kmail"));
        return;
    }
    if (mShowUnreadMailCount) {
        const int overlaySize = IconSize(KIconLoader::Panel);

        const QString countString = QString::number(mCount);
        QFont countFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        countFont.setBold(true);

        // decrease the size of the font for the number of unread messages if the
        // number doesn't fit into the available space
        float countFontSize = countFont.pointSizeF();
        QFontMetrics qfm(countFont);
        const int width = qfm.width(countString);
        if (width > (overlaySize - 2)) {
            countFontSize *= float(overlaySize - 2) / float(width);
            countFont.setPointSizeF(countFontSize);
        }

        // Paint the number in a pixmap
        QPixmap overlayPixmap(overlaySize, overlaySize);
        overlayPixmap.fill(Qt::transparent);

        QPainter p(&overlayPixmap);
        p.setFont(countFont);
        if (!mTextColor.isValid()) {
            slotGeneralPaletteChanged();
        }

        p.setBrush(Qt::NoBrush);
        p.setPen(mTextColor);
        p.setOpacity(1.0);
        p.drawText(overlayPixmap.rect(), Qt::AlignCenter, countString);
        p.end();

        QPixmap iconPixmap = mIcon.pixmap(overlaySize, overlaySize);

        QPainter pp(&iconPixmap);
        pp.drawPixmap(0, 0, overlayPixmap);
        pp.end();

        setIconByPixmap(iconPixmap);
    } else {
        setIconByPixmap(mIcon);
    }
}

void KMSystemTray::setSystrayIconNotificationsEnabled(bool enabled)
{
    if (enabled != mIconNotificationsEnabled) {
        mIconNotificationsEnabled = enabled;
        updateSystemTray();
    }
}

/**
 * On left mouse click, switch focus to the first KMMainWidget.  On right
 * click, bring up a list of all folders with a count of unread messages.
 */
void KMSystemTray::slotActivated()
{
    KMMainWidget *mainWidget = kmkernel->getKMMainWidget();
    if (!mainWidget) {
        return;
    }

    QWidget *mainWin = mainWidget->window();
    if (!mainWin) {
        return;
    }

    KWindowInfo cur = KWindowInfo(mainWin->winId(), NET::WMDesktop);

    const int currentDesktop = KWindowSystem::currentDesktop();
    const bool wasMinimized = cur.isMinimized();

    if (cur.valid()) {
        mDesktopOfMainWin = cur.desktop();
    }

    if (wasMinimized  && (currentDesktop != mDesktopOfMainWin) && (mDesktopOfMainWin == NET::OnAllDesktops)) {
        KWindowSystem::setOnDesktop(mainWin->winId(), currentDesktop);
    }

    if (mDesktopOfMainWin == NET::OnAllDesktops) {
        KWindowSystem::setOnAllDesktops(mainWin->winId(), true);
    }

    KWindowSystem::activateWindow(mainWin->winId());

    if (wasMinimized) {
        kmkernel->raise();
    }
}

void KMSystemTray::slotContextMenuAboutToShow()
{
    // Rebuild popup menu before show to minimize race condition if
    // the base KMainWidget is closed.
    if (!buildPopupMenu()) {
        return;
    }

    if (mNewMessagesPopup != Q_NULLPTR) {
        contextMenu()->removeAction(mNewMessagesPopup->menuAction());
        delete mNewMessagesPopup;
        mNewMessagesPopup = Q_NULLPTR;
    }
    mNewMessagesPopup = new QMenu();
    fillFoldersMenu(mNewMessagesPopup, kmkernel->treeviewModelSelection());

    connect(mNewMessagesPopup, &QMenu::triggered, this, &KMSystemTray::slotSelectCollection);

    if (mCount > 0) {
        mNewMessagesPopup->setTitle(i18n("New Messages In"));
        contextMenu()->insertAction(mSendQueued, mNewMessagesPopup->menuAction());
    }
}

void KMSystemTray::fillFoldersMenu(QMenu *menu, const QAbstractItemModel *model, const QString &parentName, const QModelIndex &parentIndex)
{
    const int rowCount = model->rowCount(parentIndex);
    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex index = model->index(row, 0, parentIndex);
        const Akonadi::Collection collection = model->data(index, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        qint64 count = 0;
        if (!excludeFolder(collection)) {
            Akonadi::CollectionStatistics statistics = collection.statistics();
            count = qMax(0LL, statistics.unreadCount());
            if (count > 0) {
                if (ignoreNewMailInFolder(collection)) {
                    count = 0;
                } else {
                    mCount += count;
                }
            }
        }
        QString label = parentName.isEmpty() ? QString() : QString(parentName + QLatin1String("->"));
        label += model->data(index).toString();
        label.replace(QLatin1Char('&'), QStringLiteral("&&"));
        if (count > 0) {
            // insert an item
            QAction *action = menu->addAction(label);
            action->setData(collection.id());
        }
        if (model->rowCount(index) > 0) {
            fillFoldersMenu(menu, model, label, index);
        }
    }
}

void KMSystemTray::hideKMail()
{
    KMMainWidget *mainWidget = kmkernel->getKMMainWidget();
    if (!mainWidget) {
        return;
    }
    QWidget *mainWin = mainWidget->window();
    Q_ASSERT(mainWin);
    if (mainWin) {
        mDesktopOfMainWin = KWindowInfo(mainWin->winId(), NET::WMDesktop).desktop();
        // iconifying is unnecessary, but it looks cooler
        KWindowSystem::minimizeWindow(mainWin->winId());
        mainWin->hide();
    }
}

void KMSystemTray::initListOfCollection()
{
    mCount = 0;
    const QAbstractItemModel *model = kmkernel->collectionModel();
    if (model->rowCount() == 0) {
        QTimer::singleShot(1000, this, &KMSystemTray::initListOfCollection);
        return;
    }
    unreadMail(model);

    if (mMode == KMailSettings::EnumSystemTrayPolicy::ShowOnUnread) {
        if (status() == KStatusNotifierItem::Passive && (mCount > 0)) {
            setStatus(KStatusNotifierItem::Active);
        } else if (status() == KStatusNotifierItem::Active && (mCount == 0)) {
            setStatus(KStatusNotifierItem::Passive);
        }
    }

    //qCDebug(KMAIL_LOG)<<" mCount :"<<mCount;
    updateCount();
}

void KMSystemTray::unreadMail(const QAbstractItemModel *model, const QModelIndex &parentIndex)
{
    const int rowCount = model->rowCount(parentIndex);
    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex index = model->index(row, 0, parentIndex);
        const Akonadi::Collection collection = model->data(index, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

        if (!excludeFolder(collection)) {

            const Akonadi::CollectionStatistics statistics = collection.statistics();
            const qint64 count = qMax(0LL, statistics.unreadCount());

            if (count > 0) {
                if (!ignoreNewMailInFolder(collection)) {
                    mCount += count;
                }
            }
        }
        if (model->rowCount(index) > 0) {
            unreadMail(model, index);
        }
    }
    // Update tooltip to reflect count of unread messages
    setToolTipSubTitle(mCount == 0 ? i18n("There are no unread messages")
                       : i18np("1 unread message",
                               "%1 unread messages",
                               mCount));
}

bool KMSystemTray::hasUnreadMail() const
{
    return (mCount != 0);
}

void KMSystemTray::slotSelectCollection(QAction *act)
{
    const Akonadi::Collection::Id id = act->data().value<Akonadi::Collection::Id>();
    kmkernel->selectCollectionFromId(id);
    KMMainWidget *mainWidget = kmkernel->getKMMainWidget();
    if (!mainWidget) {
        return;
    }
    QWidget *mainWin = mainWidget->window();
    if (mainWin && !mainWin->isVisible()) {
        activate();
    }
}

void KMSystemTray::updateSystemTray()
{
    initListOfCollection();
}

void KMSystemTray::slotCollectionStatisticsChanged(Akonadi::Collection::Id id, const Akonadi::CollectionStatistics &)
{
    //Exclude sent mail folder

    if (CommonKernel->outboxCollectionFolder().id() == id ||
            CommonKernel->sentCollectionFolder().id() == id ||
            CommonKernel->templatesCollectionFolder().id() == id ||
            CommonKernel->trashCollectionFolder().id() == id ||
            CommonKernel->draftsCollectionFolder().id() == id) {
        return;
    }
    initListOfCollection();
}

bool KMSystemTray::excludeFolder(const Akonadi::Collection &collection) const
{
    if (!collection.isValid()) {
        return true;
    }
    if (!collection.contentMimeTypes().contains(KMime::Message::mimeType())) {
        return true;
    }
    if (CommonKernel->outboxCollectionFolder() == collection ||
            CommonKernel->sentCollectionFolder() == collection ||
            CommonKernel->templatesCollectionFolder() == collection ||
            CommonKernel->trashCollectionFolder() == collection ||
            CommonKernel->draftsCollectionFolder() == collection) {
        return true;
    }

    if (MailCommon::Util::isVirtualCollection(collection)) {
        return true;
    }
    return false;
}

bool KMSystemTray::ignoreNewMailInFolder(const Akonadi::Collection &collection)
{
    if (collection.hasAttribute<Akonadi::NewMailNotifierAttribute>()) {
        if (collection.attribute<Akonadi::NewMailNotifierAttribute>()->ignoreNewMail()) {
            return true;
        }
    }
    return false;
}

}
