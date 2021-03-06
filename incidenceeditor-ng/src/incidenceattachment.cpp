/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include "incidenceattachment.h"
#include "attachmenteditdialog.h"
#include "attachmenticonview.h"
#include "ui_dialogdesktop.h"

#include <KdepimDBusInterfaces/UriHandler>

#include <KContacts/VCardDrag>

#include <KMime/Message>

#include <QAction>
#include <KActionCollection>
#include <QFileDialog>
#include <QMenu>
#include <KMessageBox>
#include <KProtocolManager>
#include <KRun>
#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <QIcon>
#include <QUrl>

#include <QClipboard>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>

using namespace IncidenceEditorNG;

IncidenceAttachment::IncidenceAttachment(Ui::EventOrTodoDesktop *ui)
    : IncidenceEditor(0),
      mUi(ui),
      mPopupMenu(new QMenu)
{
    setupActions();
    setupAttachmentIconView();
    setObjectName(QStringLiteral("IncidenceAttachment"));

    connect(mUi->mAddButton, &QPushButton::clicked, this, &IncidenceAttachment::addAttachment);
    connect(mUi->mRemoveButton, &QPushButton::clicked, this, &IncidenceAttachment::removeSelectedAttachments);
}

IncidenceAttachment::~IncidenceAttachment()
{
    delete mPopupMenu;
}

void IncidenceAttachment::load(const KCalCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;
    mAttachmentView->clear();

    KCalCore::Attachment::List attachments = incidence->attachments();
    KCalCore::Attachment::List::ConstIterator it;
    for (it = attachments.constBegin(); it != attachments.constEnd(); ++it) {
        new AttachmentIconItem((*it), mAttachmentView);
    }

    mWasDirty = false;
}

void IncidenceAttachment::save(const KCalCore::Incidence::Ptr &incidence)
{
    incidence->clearAttachments();

    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        AttachmentIconItem *attitem = dynamic_cast<AttachmentIconItem *>(item);
        Q_ASSERT(item);
        incidence->addAttachment(
            KCalCore::Attachment::Ptr(new KCalCore::Attachment(*(attitem->attachment()))));
    }
}

bool IncidenceAttachment::isDirty() const
{
    if (mLoadedIncidence) {
        if (mAttachmentView->count() != mLoadedIncidence->attachments().count()) {
            return true;
        }

        KCalCore::Attachment::List origAttachments = mLoadedIncidence->attachments();
        for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
            QListWidgetItem *item = mAttachmentView->item(itemIndex);
            Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));

            const KCalCore::Attachment::Ptr listAttachment =
                static_cast<AttachmentIconItem *>(item)->attachment();

            for (int i = 0; i < origAttachments.count(); ++i) {
                const KCalCore::Attachment::Ptr attachment = origAttachments.at(i);

                if (*attachment == *listAttachment) {
                    origAttachments.remove(i);
                    break;
                }
            }
        }
        // All attachments are removed from the list, meaning, the items in mAttachmentView
        // are equal to the attachments set on mLoadedIncidence.
        return !origAttachments.isEmpty();

    } else {
        // No incidence loaded, so if the user added attachments we're dirty.
        return mAttachmentView->count() != 0;
    }

    return false;
}

int IncidenceAttachment::attachmentCount() const
{
    return mAttachmentView->count();
}

/// Private slots

void IncidenceAttachment::addAttachment()
{
    AttachmentIconItem *item = new AttachmentIconItem(KCalCore::Attachment::Ptr(), mAttachmentView);

    QWeakPointer<AttachmentEditDialog> dialog(new AttachmentEditDialog(item, mAttachmentView));
    dialog.data()->setWindowTitle(i18nc("@title", "Add Attachment"));
    if (dialog.data()->exec() == QDialog::Rejected) {
        delete item;
    } else {
        Q_EMIT attachmentCountChanged(mAttachmentView->count());
    }

    if (dialog.data()) {
        dialog.data()->deleteLater();
    }

    checkDirtyStatus();
}

void IncidenceAttachment::copyToClipboard()
{
#ifndef QT_NO_CLIPBOARD
    QApplication::clipboard()->setMimeData(mAttachmentView->mimeData(), QClipboard::Clipboard);
#endif
}

void IncidenceAttachment::openURL(const QUrl &url)
{
    QString uri = url.url();
    UriHandler::process(uri);
}

void IncidenceAttachment::pasteFromClipboard()
{
#ifndef QT_NO_CLIPBOARD
    handlePasteOrDrop(QApplication::clipboard()->mimeData());
#endif
}

void IncidenceAttachment::removeSelectedAttachments()
{
    QList<QListWidgetItem *> selected;
    QStringList labels;

    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *it = mAttachmentView->item(itemIndex);
        if (it->isSelected()) {
            AttachmentIconItem *attitem = static_cast<AttachmentIconItem *>(it);
            if (attitem) {
                KCalCore::Attachment::Ptr att = attitem->attachment();
                labels << att->label();
                selected << it;
            }
        }
    }

    if (selected.isEmpty()) {
        return;
    }

    QString labelsStr = labels.join(QStringLiteral("<nl/>"));

    if (KMessageBox::questionYesNo(
                0,
                xi18nc("@info",
                       "Do you really want to remove these attachments?<nl/>%1", labelsStr),
                i18nc("@title:window", "Remove Attachments?"),
                KStandardGuiItem::yes(), KStandardGuiItem::no(),
                QStringLiteral("calendarRemoveAttachments")) != KMessageBox::Yes) {
        return;
    }

    for (QList<QListWidgetItem *>::iterator it(selected.begin()), end(selected.end());
            it != end; ++it) {
        int row = mAttachmentView->row(*it);
        QListWidgetItem *next = mAttachmentView->item(++row);
        QListWidgetItem *prev = mAttachmentView->item(--row);
        if (next) {
            next->setSelected(true);
        } else if (prev) {
            prev->setSelected(true);
        }
        delete *it;
    }

    mAttachmentView->update();
    Q_EMIT attachmentCountChanged(mAttachmentView->count());
    checkDirtyStatus();
}

void IncidenceAttachment::saveAttachment(QListWidgetItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));

    AttachmentIconItem *attitem = static_cast<AttachmentIconItem *>(item);
    if (!attitem->attachment()) {
        return;
    }

    KCalCore::Attachment::Ptr att = attitem->attachment();

    // get the saveas file name
    QString saveAsFile = QFileDialog::getSaveFileName(0, i18nc("@title", "Save Attachment"),
                         att->label());

    if (saveAsFile.isEmpty() ||
            (QFile(saveAsFile).exists() &&
             (KMessageBox::warningYesNo(
                  0,
                  i18nc("@info", "%1 already exists. Do you want to overwrite it?",
                        saveAsFile)) == KMessageBox::No))) {
        return;
    }

    QUrl sourceUrl;
    if (att->isUri()) {
        sourceUrl = att->uri();
    } else {
        sourceUrl = mAttachmentView->tempFileForAttachment(att);
    }
    // save the attachment url
    auto job = KIO::file_copy(sourceUrl, QUrl::fromLocalFile(saveAsFile));
    if (!job->exec() && job->error()) {
        KMessageBox::error(0, job->errorString());
    }
}

void IncidenceAttachment::saveSelectedAttachments()
{
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            saveAttachment(item);
        }
    }
}

void IncidenceAttachment::showAttachment(QListWidgetItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));
    AttachmentIconItem *attitem = static_cast<AttachmentIconItem *>(item);
    if (!attitem->attachment()) {
        return;
    }

    KCalCore::Attachment::Ptr att = attitem->attachment();
    if (att->isUri()) {
        Q_EMIT openURL(att->uri());
    } else {
        KRun::runUrl(mAttachmentView->tempFileForAttachment(att), att->mimeType(), 0, true);
    }
}

void IncidenceAttachment::showContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = mAttachmentView->itemAt(pos);
    const bool enable = item != 0;

    int numSelected = 0;
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            numSelected++;
        }
    }

    mOpenAction->setEnabled(enable);
    //TODO: support saving multiple attachments into a directory
    mSaveAsAction->setEnabled(enable && numSelected == 1);
#ifndef QT_NO_CLIPBOARD
    mCopyAction->setEnabled(enable && numSelected == 1);
    mCutAction->setEnabled(enable && numSelected == 1);
#endif
    mDeleteAction->setEnabled(enable);
    mEditAction->setEnabled(enable);
    mPopupMenu->exec(mAttachmentView->mapToGlobal(pos));
}

void IncidenceAttachment::showSelectedAttachments()
{
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            showAttachment(item);
        }
    }
}

void IncidenceAttachment::cutToClipboard()
{
#ifndef QT_NO_CLIPBOARD
    copyToClipboard();
    removeSelectedAttachments();
#endif
}

void IncidenceAttachment::editSelectedAttachments()
{
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));

            AttachmentIconItem *attitem = static_cast<AttachmentIconItem *>(item);
            if (!attitem->attachment()) {
                return;
            }

            QPointer<AttachmentEditDialog> dialog(
                new AttachmentEditDialog(attitem, mAttachmentView, false));
            dialog->setModal(false);
            connect(dialog.data(), SIGNAL(hidden()), dialog.data(), SLOT(delayedDestruct()));
            dialog->show();
        }
    }
}

void IncidenceAttachment::slotItemRenamed(QListWidgetItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));
    static_cast<AttachmentIconItem *>(item)->setLabel(item->text());
    checkDirtyStatus();
}

void IncidenceAttachment::slotSelectionChanged()
{
    bool selected = false;
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            selected = true;
            break;
        }
    }
    mUi->mRemoveButton->setEnabled(selected);
}

/// Private functions

void IncidenceAttachment::handlePasteOrDrop(const QMimeData *mimeData)
{
    QList<QUrl> urls;
    bool probablyWeHaveUris = false;
    QStringList labels;

    if (KContacts::VCardDrag::canDecode(mimeData)) {
        KContacts::Addressee::List addressees;
        KContacts::VCardDrag::fromMimeData(mimeData, addressees);
        urls.reserve(addressees.count());
        labels.reserve(addressees.count());
        for (KContacts::Addressee::List::ConstIterator it = addressees.constBegin();
                it != addressees.constEnd(); ++it) {
            urls.append(QString(QLatin1String("uid:") + (*it).uid()));
            // there is some weirdness about realName(), hence fromUtf8
            labels.append(QString::fromUtf8((*it).realName().toLatin1()));
        }
        probablyWeHaveUris = true;
    } else if (mimeData->hasUrls()) {
        QMap<QString, QString> metadata;

        //QT5
        //urls = QList<QUrl>::fromMimeData( mimeData, &metadata );
        probablyWeHaveUris = true;
        labels = metadata[QStringLiteral("labels")].split(':', QString::SkipEmptyParts);
        for (QStringList::Iterator it = labels.begin(); it != labels.end(); ++it) {
            *it = QUrl::fromPercentEncoding((*it).toLatin1());
        }
    } else if (mimeData->hasText()) {
        QString text = mimeData->text();
        QStringList lst = text.split('\n', QString::SkipEmptyParts);
        urls.reserve(lst.count());
        for (QStringList::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it) {
            urls.append(*it);
        }
        probablyWeHaveUris = true;
    }
    QMenu menu;
    QAction *linkAction = 0, *cancelAction;
    if (probablyWeHaveUris) {
        linkAction = menu.addAction(QIcon::fromTheme(QStringLiteral("insert-link")), i18nc("@action:inmenu", "&Link here"));
        // we need to check if we can reasonably expect to copy the objects
        bool weCanCopy = true;
        for (QList<QUrl>::ConstIterator it = urls.constBegin(); it != urls.constEnd(); ++it) {
            if (!(weCanCopy = KProtocolManager::supportsReading(*it))) {
                break; // either we can copy them all, or no copying at all
            }
        }
        if (weCanCopy) {
            menu.addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu", "&Copy here"));
        }
    } else {
        menu.addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu", "&Copy here"));
    }

    menu.addSeparator();
    cancelAction = menu.addAction(QIcon::fromTheme(QStringLiteral("process-stop")), i18nc("@action:inmenu", "C&ancel"));

    QByteArray data;
    QString mimeType;
    QString label;

    if (!mimeData->formats().isEmpty() && !probablyWeHaveUris) {
        mimeType = mimeData->formats().first();
        data = mimeData->data(mimeType);
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForName(mimeType);
        if (mime.isValid()) {
            label = mime.comment();
        }
    }

    QAction *ret = menu.exec(QCursor::pos());
    if (linkAction == ret) {
        QStringList::ConstIterator jt = labels.constBegin();
        for (QList<QUrl>::ConstIterator it = urls.constBegin();
                it != urls.constEnd(); ++it) {
            addUriAttachment((*it).url(), QString(), (jt == labels.constEnd() ?
                             QString() : * (jt++)), true);
        }
    } else if (cancelAction != ret) {
        if (probablyWeHaveUris) {
            for (QList<QUrl>::ConstIterator it = urls.constBegin();
                    it != urls.constEnd(); ++it) {
                KIO::Job *job = KIO::storedGet(*it);
                //TODO verify if slot exist !
                connect(job, &KIO::Job::result, this, &IncidenceAttachment::downloadComplete);
            }
        } else { // we take anything
            addDataAttachment(data, mimeType, label);
        }
    }
}

void IncidenceAttachment::downloadComplete(KJob *)
{
    //TODO
}

void IncidenceAttachment::setupActions()
{
    KActionCollection *ac = new KActionCollection(this);
//  ac->addAssociatedWidget( this );

    mOpenAction = new QAction(i18nc("@action:inmenu open the attachment in a viewer",
                                    "&Open"), this);
    connect(mOpenAction, &QAction::triggered, this, &IncidenceAttachment::showSelectedAttachments);
    ac->addAction(QStringLiteral("view"), mOpenAction);
    mPopupMenu->addAction(mOpenAction);

    mSaveAsAction = new QAction(i18nc("@action:inmenu save the attachment to a file",
                                      "Save As..."), this);
    connect(mSaveAsAction, &QAction::triggered, this, &IncidenceAttachment::saveSelectedAttachments);
    mPopupMenu->addAction(mSaveAsAction);
    mPopupMenu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    mCopyAction = KStandardAction::copy(this, SLOT(copyToClipboard()), ac);
    mPopupMenu->addAction(mCopyAction);

    mCutAction = KStandardAction::cut(this, SLOT(cutToClipboard()), ac);
    mPopupMenu->addAction(mCutAction);

    QAction *action = KStandardAction::paste(this, SLOT(pasteFromClipboard()), ac);
    mPopupMenu->addAction(action);
    mPopupMenu->addSeparator();
#endif

    mDeleteAction = new QAction(i18nc("@action:inmenu remove the attachment",
                                      "&Remove"), this);
    connect(mDeleteAction, &QAction::triggered, this, &IncidenceAttachment::removeSelectedAttachments);
    ac->addAction(QStringLiteral("remove"), mDeleteAction);
    mDeleteAction->setShortcut(Qt::Key_Delete);
    mPopupMenu->addAction(mDeleteAction);
    mPopupMenu->addSeparator();

    mEditAction = new QAction(i18nc("@action:inmenu show a dialog used to edit the attachment",
                                    "&Properties..."), this);
    connect(mEditAction, &QAction::triggered, this, &IncidenceAttachment::editSelectedAttachments);
    ac->addAction(QStringLiteral("edit"), mEditAction);
    mPopupMenu->addAction(mEditAction);
}

void IncidenceAttachment::setupAttachmentIconView()
{
    mAttachmentView = new AttachmentIconView;
    mAttachmentView->setWhatsThis(i18nc("@info:whatsthis",
                                        "Displays items (files, mail, etc.) that "
                                        "have been associated with this event or to-do."));

    connect(mAttachmentView, &AttachmentIconView::itemDoubleClicked, this, &IncidenceAttachment::showAttachment);
    connect(mAttachmentView, &AttachmentIconView::itemChanged, this, &IncidenceAttachment::slotItemRenamed);
    connect(mAttachmentView, &AttachmentIconView::itemSelectionChanged, this, &IncidenceAttachment::slotSelectionChanged);
    connect(mAttachmentView, &AttachmentIconView::customContextMenuRequested, this, &IncidenceAttachment::showContextMenu);

    QGridLayout *layout = new QGridLayout(mUi->mAttachmentViewPlaceHolder);
    layout->addWidget(mAttachmentView);
}

// void IncidenceAttachmentEditor::addAttachment( KCalCore::Attachment *attachment )
// {
//   new AttachmentIconItem( attachment, mAttachmentView );
// }

void IncidenceAttachment::addDataAttachment(const QByteArray &data,
        const QString &mimeType,
        const QString &label)
{
    AttachmentIconItem *item = new AttachmentIconItem(KCalCore::Attachment::Ptr(), mAttachmentView);

    QString nlabel = label;
    if (mimeType == QLatin1String("message/rfc822")) {
        // mail message. try to set the label from the mail Subject:
        KMime::Message msg;
        msg.setContent(data);
        msg.parse();
        nlabel = msg.subject()->asUnicodeString();
    }

    item->setData(data);
    item->setLabel(nlabel);
    if (mimeType.isEmpty()) {
        QMimeDatabase db;
        item->setMimeType(db.mimeTypeForData(data).name());
    } else {
        item->setMimeType(mimeType);
    }

    checkDirtyStatus();
}

void IncidenceAttachment::addUriAttachment(const QString &uri,
        const QString &mimeType,
        const QString &label,
        bool inLine)
{
    if (!inLine) {
        AttachmentIconItem *item =
            new AttachmentIconItem(KCalCore::Attachment::Ptr(), mAttachmentView);
        item->setUri(uri);
        item->setLabel(label);
        if (mimeType.isEmpty()) {
            if (uri.startsWith(QStringLiteral("uid:"))) {
                item->setMimeType(QStringLiteral("text/directory"));
            } else if (uri.startsWith(QStringLiteral("kmail:"))) {
                item->setMimeType(QStringLiteral("message/rfc822"));
            } else if (uri.startsWith(QStringLiteral("urn:x-ical"))) {
                item->setMimeType(QStringLiteral("text/calendar"));
            } else if (uri.startsWith(QStringLiteral("news:"))) {
                item->setMimeType(QStringLiteral("message/news"));
            } else {
                QMimeDatabase db;
                item->setMimeType(db.mimeTypeForUrl(uri).name());
            }
        }
    } else {
        auto job = KIO::storedGet(uri);
        KJobWidgets::setWindow(job, 0);
        if (job->exec()) {
            const QByteArray data = job->data();
            addDataAttachment(data, mimeType, label);
        }
    }
}

