/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "knotesiconview.h"
#include "NoteShared/NotesAkonadiTreeModel"
#include "NoteShared/NoteDisplayAttribute"
#include "NoteShared/NoteLockAttribute"
#include "noteutils.h"
#include "knotes/notes/knotedisplaysettings.h"
#include "utils/knoteutils.h"

#include <KLocalizedString>

#include <AkonadiCore/ItemModifyJob>

#include <KMime/KMimeMessage>

#include <KIconEffect>
#include <KIconLoader>

#include <QColor>
#include <QPixmap>
#include <QMouseEvent>
#include <QListWidgetItem>
#include "knotes_kontact_plugin_debug.h"
#include <QToolTip>

//#define DEBUG_SAVE_NOTE 1

KNotesIconView::KNotesIconView(KNotesPart *part, QWidget *parent)
    : QListWidget(parent),
      m_part(part)
{
    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setWordWrap(true);
    setMouseTracking(true);
}

KNotesIconView::~KNotesIconView()
{

}

bool KNotesIconView::event(QEvent *e)
{
    if (e->type() != QEvent::ToolTip) {
        return QListWidget::event(e);
    }
    QHelpEvent *he = static_cast< QHelpEvent * >(e);

    QPoint pnt = viewport()->mapFromGlobal(mapToGlobal(he->pos()));

    if (pnt.y() < 0) {
        return true;
    }

    QListWidgetItem *item = itemAt(pnt);
    if (item) {
        KNotesIconViewItem *noteItem = static_cast<KNotesIconViewItem *>(item);
        NoteShared::NoteUtils noteUtils;
        const QString toolTip = noteUtils.createToolTip(noteItem->item());
        QToolTip::showText(he->globalPos(), toolTip, viewport(), visualItemRect(item));
    }
    return true;
}

void KNotesIconView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {
        QListView::mousePressEvent(e);
        m_part->popupRMB(currentItem(), e->pos(), e->globalPos());
    } else {
        QListView::mousePressEvent(e);
    }
}

void KNotesIconView::addNote(const Akonadi::Item &item)
{
    KNotesIconViewItem *iconView = new KNotesIconViewItem(item, this);
    mNoteList.insert(item.id(), iconView);
}

KNotesIconViewItem *KNotesIconView::iconView(Akonadi::Item::Id id) const
{
    if (mNoteList.contains(id)) {
        return mNoteList.value(id);
    }
    return Q_NULLPTR;
}

QHash<Akonadi::Item::Id, KNotesIconViewItem *> KNotesIconView::noteList() const
{
    return mNoteList;
}

KNotesIconViewItem::KNotesIconViewItem(const Akonadi::Item &item, QListWidget *parent)
    : QListWidgetItem(parent),
      mItem(item),
      mDisplayAttribute(new KNoteDisplaySettings),
      mReadOnly(false)
{
    if (mItem.hasAttribute<NoteShared::NoteDisplayAttribute>()) {
        mDisplayAttribute->setDisplayAttribute(mItem.attribute<NoteShared::NoteDisplayAttribute>());
    } else {
        setDisplayDefaultValue();
        //save default display value
    }
    prepare();
}

KNotesIconViewItem::~KNotesIconViewItem()
{
    delete mDisplayAttribute;
}

void KNotesIconViewItem::prepare()
{
    const KMime::Message::Ptr noteMessage = mItem.payload<KMime::Message::Ptr>();
    const KMime::Headers::Subject *const subject = noteMessage ? noteMessage->subject(false) : Q_NULLPTR;
    setText(subject ? subject->asUnicodeString() : QString());

    if (mItem.hasAttribute<NoteShared::NoteLockAttribute>()) {
        mReadOnly = true;
    } else {
        mReadOnly = false;
    }
    updateSettings();
}

bool KNotesIconViewItem::readOnly() const
{
    return mReadOnly;
}

void KNotesIconViewItem::setReadOnly(bool b, bool save)
{
    mReadOnly = b;
    if (mItem.hasAttribute<NoteShared::NoteLockAttribute>()) {
        if (!mReadOnly) {
            mItem.removeAttribute<NoteShared::NoteLockAttribute>();
        }
    } else {
        if (mReadOnly) {
            mItem.attribute<NoteShared::NoteLockAttribute>(Akonadi::Item::AddIfMissing);
        }
    }
    if (save) {
        Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mItem);
#ifdef DEBUG_SAVE_NOTE
        qCDebug(KNOTES_KONTACT_PLUGIN_LOG) << " KNotesIconViewItem::setReadOnly savenote";
#endif
        connect(job, &Akonadi::ItemModifyJob::result, this, &KNotesIconViewItem::slotNoteSaved);
    }
}

void KNotesIconViewItem::setDisplayDefaultValue()
{
    KNoteUtils::setDefaultValue(mItem);
    Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mItem);
    connect(job, &Akonadi::ItemModifyJob::result, this, &KNotesIconViewItem::slotNoteSaved);
}

void KNotesIconViewItem::slotNoteSaved(KJob *job)
{
    qCDebug(KNOTES_KONTACT_PLUGIN_LOG) << " void KNotesIconViewItem::slotNoteSaved(KJob *job)";
    if (job->error()) {
        qCDebug(KNOTES_KONTACT_PLUGIN_LOG) << " problem during save note:" << job->errorString();
    }
}

void KNotesIconViewItem::setChangeIconTextAndDescription(const QString &iconText, const QString &description, int position)
{
    setIconText(iconText, false);
    saveNoteContent(iconText, description, position);
}

void KNotesIconViewItem::setIconText(const QString &text, bool save)
{
    QString replaceText;
    if (text.count() > 50) {
        replaceText = text.left(50) + QLatin1String("...");
    } else {
        replaceText = text;
    }

    setText(replaceText);
    if (save) {
        saveNoteContent(text);
    }
}

QString KNotesIconViewItem::realName() const
{
    const KMime::Message::Ptr noteMessage = mItem.payload<KMime::Message::Ptr>();
    const KMime::Headers::Subject *const subject = noteMessage ? noteMessage->subject(false) : Q_NULLPTR;
    return subject ? subject->asUnicodeString() : QString();
}

int KNotesIconViewItem::tabSize() const
{
    return mDisplayAttribute->tabSize();
}

QColor KNotesIconViewItem::textBackgroundColor() const
{
    return mDisplayAttribute->backgroundColor();
}

QColor KNotesIconViewItem::textForegroundColor() const
{
    return mDisplayAttribute->foregroundColor();
}

bool KNotesIconViewItem::autoIndent() const
{
    return mDisplayAttribute->autoIndent();
}

QFont KNotesIconViewItem::textFont() const
{
    return mDisplayAttribute->font();
}

bool KNotesIconViewItem::isRichText() const
{
    const KMime::Message::Ptr noteMessage = mItem.payload<KMime::Message::Ptr>();
    return noteMessage->contentType()->isHTMLText();
}

QString KNotesIconViewItem::description() const
{
    const KMime::Message::Ptr noteMessage = mItem.payload<KMime::Message::Ptr>();
    return QString::fromUtf8(noteMessage->mainBodyPart()->decodedContent());
}

int KNotesIconViewItem::cursorPositionFromStart() const
{
    int pos = 0;
    const KMime::Message::Ptr noteMessage = mItem.payload<KMime::Message::Ptr>();
    if (noteMessage->headerByType("X-Cursor-Position")) {
        pos = noteMessage->headerByType("X-Cursor-Position")->asUnicodeString().toInt();
    }
    return pos;
}

void KNotesIconViewItem::setDescription(const QString &description)
{
    saveNoteContent(QString(), description);
}

KNoteDisplaySettings *KNotesIconViewItem::displayAttribute() const
{
    return mDisplayAttribute;
}

Akonadi::Item KNotesIconViewItem::item()
{
    return mItem;
}

void KNotesIconViewItem::saveNoteContent(const QString &subject, const QString &description, int position)
{
    KMime::Message::Ptr message = mItem.payload<KMime::Message::Ptr>();
    const QByteArray encoding("utf-8");
    if (!subject.isEmpty()) {
        message->subject(true)->fromUnicodeString(subject, encoding);
    }
    message->contentType(true)->setMimeType(isRichText() ? "text/html" : "text/plain");
    message->contentType()->setCharset(encoding);
    message->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEquPr);
    message->date(true)->setDateTime(QDateTime::currentDateTime());
    if (!description.isEmpty()) {
        message->mainBodyPart()->fromUnicodeString(description);
    } else if (message->mainBodyPart()->decodedText().isEmpty()) {
        message->mainBodyPart()->fromUnicodeString(QStringLiteral(" "));
    }

    if (position >= 0) {
        KMime::Headers::Generic *header = new KMime::Headers::Generic("X-Cursor-Position");
        header->fromUnicodeString(QString::number(position), "utf-8");
        message->setHeader(header);
    }

    message->assemble();

    mItem.setPayload(message);
    Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mItem);
#ifdef DEBUG_SAVE_NOTE
    qCDebug(KNOTES_KONTACT_PLUGIN_LOG) << " KNotesIconViewItem::saveNoteContent savenote";
#endif
    connect(job, &Akonadi::ItemModifyJob::result, this, &KNotesIconViewItem::slotNoteSaved);
}

void KNotesIconViewItem::setChangeItem(const Akonadi::Item &item, const QSet<QByteArray> &set)
{
    mItem = item;
    if (item.hasAttribute<NoteShared::NoteDisplayAttribute>()) {
        mDisplayAttribute->setDisplayAttribute(item.attribute<NoteShared::NoteDisplayAttribute>());
    }
    if (set.contains("ATR:KJotsLockAttribute")) {
        setReadOnly(item.hasAttribute<NoteShared::NoteLockAttribute>(), false);
    }
    if (set.contains("PLD:RFC822")) {
        const KMime::Message::Ptr noteMessage = item.payload<KMime::Message::Ptr>();
        const KMime::Headers::Subject *const subject = noteMessage ? noteMessage->subject(false) : Q_NULLPTR;
        setIconText(subject ? subject->asUnicodeString() : QString(), false);
    }
    if (set.contains("ATR:NoteDisplayAttribute")) {
        updateSettings();
    }

}
void KNotesIconViewItem::updateSettings()
{
    KIconEffect effect;
    const QColor color(mDisplayAttribute->backgroundColor());
    if (mDefaultPixmap.isNull()) {
        mDefaultPixmap = KIconLoader::global()->loadIcon(QStringLiteral("knotes"), KIconLoader::Desktop);
    }
    QPixmap icon = effect.apply(mDefaultPixmap, KIconEffect::Colorize, 1, color, false);
    setFont(mDisplayAttribute->titleFont());
    setIcon(icon);
}

#include "moc_knotesiconview.cpp"
