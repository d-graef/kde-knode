/*

  Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "backupjob.h"

#include <Libkdepim/BroadcastStatus>
#include "mailcommon_debug.h"
#include <CollectionDeleteJob>
#include <CollectionFetchJob>
#include <CollectionFetchScope>
#include <ItemFetchJob>
#include <ItemFetchScope>

#include <KMime/Message>

#include <KLocalizedString>
#include <KMessageBox>
#include <KTar>
#include <KZip>
#include <kio/global.h> //krazy:exclude=camelcase as there is no such

#include <QFileInfo>
#include <QTimer>

using namespace MailCommon;
static const mode_t archivePerms = S_IFREG | 0644;

BackupJob::BackupJob(QWidget *parent)
    : QObject(parent),
      mArchiveTime(QDateTime::currentDateTime()),
      mArchiveType(Zip),
      mRootFolder(0),
      mArchive(0),
      mParentWidget(parent),
      mArchivedMessages(0),
      mArchivedSize(0),
      mProgressItem(0),
      mAborted(false),
      mDeleteFoldersAfterCompletion(false),
      mRecursive(true),
      mCurrentFolder(Akonadi::Collection()),
      mCurrentJob(0),
      mDisplayMessageBox(true)
{
}

BackupJob::~BackupJob()
{
    mPendingFolders.clear();
    delete mArchive;
    mArchive = 0;
}

void BackupJob::setRootFolder(const Akonadi::Collection &rootFolder)
{
    mRootFolder = rootFolder;
}

void BackupJob::setRealPath(const QString &path)
{
    mRealPath = path;
}

void BackupJob::setSaveLocation(const QUrl &savePath)
{
    mMailArchivePath = savePath;
}

void BackupJob::setArchiveType(ArchiveType type)
{
    mArchiveType = type;
}

void BackupJob::setDeleteFoldersAfterCompletion(bool deleteThem)
{
    mDeleteFoldersAfterCompletion = deleteThem;
}

void BackupJob::setRecursive(bool recursive)
{
    mRecursive = recursive;
}

bool BackupJob::queueFolders(const Akonadi::Collection &root)
{
    mPendingFolders.append(root);
    if (mRecursive) {
        // FIXME: Get rid of the exec()
        // We could do a recursive CollectionFetchJob, but we only fetch the first level
        // and then recurse manually. This is needed because a recursive fetch doesn't
        // sort the collections the way we want. We need all first level children to be
        // in the mPendingFolders list before all second level children, so that the
        // directories for the first level are written before the directories in the
        // second level, in the archive file.
        Akonadi::CollectionFetchJob *job =
            new Akonadi::CollectionFetchJob(root, Akonadi::CollectionFetchJob::FirstLevel);
        job->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
        job->exec();
        if (job->error()) {
            qCWarning(MAILCOMMON_LOG) << job->errorString();
            abort(i18n("Unable to retrieve folder list."));
            return false;
        }

        foreach (const Akonadi::Collection &collection, job->collections()) {
            if (!queueFolders(collection)) {
                return false;
            }
        }
    }
    mAllFolders = mPendingFolders;
    return true;
}

bool BackupJob::hasChildren(const Akonadi::Collection &collection) const
{
    foreach (const Akonadi::Collection &curCol, mAllFolders) {
        if (collection == curCol.parentCollection()) {
            return true;
        }
    }
    return false;
}

void BackupJob::cancelJob()
{
    abort(i18n("The operation was canceled by the user."));
}

void BackupJob::abort(const QString &errorMessage)
{
    // We could be called this twice, since killing the current job below will
    // cause the job to fail, and that will call abort()
    if (mAborted) {
        return;
    }

    mAborted = true;
    if (mCurrentFolder.isValid()) {
        mCurrentFolder = Akonadi::Collection();
    }

    if (mArchive && mArchive->isOpen()) {
        mArchive->close();
    }

    if (mCurrentJob) {
        mCurrentJob->kill();
        mCurrentJob = 0;
    }

    if (mProgressItem) {
        mProgressItem->setComplete();
        mProgressItem = 0;
        // The progressmanager will delete it
    }
    QString text = i18n("Failed to archive the folder '%1'.", mRootFolder.name());
    text += QLatin1Char('\n') + errorMessage;
    Q_EMIT error(text);
    if (mDisplayMessageBox) {
        KMessageBox::sorry(mParentWidget, text, i18n("Archiving failed"));
    }
    deleteLater();
    // Clean up archive file here?
}

void BackupJob::finish()
{
    if (mArchive->isOpen()) {
        if (!mArchive->close()) {
            abort(i18n("Unable to finalize the archive file."));
            return;
        }
    }

    const QString archivingStr(i18n("Archiving finished"));
    KPIM::BroadcastStatus::instance()->setStatusMsg(archivingStr);

    if (mProgressItem) {
        mProgressItem->setStatus(archivingStr);
        mProgressItem->setComplete();
        mProgressItem = 0;
    }

    QFileInfo archiveFileInfo(mMailArchivePath.path());
    QString text = i18n("Archiving folder '%1' successfully completed. "
                        "The archive was written to the file '%2'.",
                        mRealPath.isEmpty() ? mRootFolder.name() : mRealPath, mMailArchivePath.path());
    text += QLatin1Char('\n') + i18np("1 message of size %2 was archived.",
                                      "%1 messages with the total size of %2 were archived.",
                                      mArchivedMessages, KIO::convertSize(mArchivedSize));
    text += QLatin1Char('\n') + i18n("The archive file has a size of %1.",
                                     KIO::convertSize(archiveFileInfo.size()));
    if (mDisplayMessageBox) {
        KMessageBox::information(mParentWidget, text, i18n("Archiving finished"));
    }

    if (mDeleteFoldersAfterCompletion) {
        // Some safety checks first...
        if (archiveFileInfo.exists() && (mArchivedSize > 0 || mArchivedMessages == 0)) {
            // Sorry for any data loss!
            new Akonadi::CollectionDeleteJob(mRootFolder);
        }
    }
    Q_EMIT backupDone(text);
    deleteLater();
}

void BackupJob::archiveNextMessage()
{
    if (mAborted) {
        return;
    }

    if (mPendingMessages.isEmpty()) {
        qCDebug(MAILCOMMON_LOG) << "===> All messages done in folder " << mCurrentFolder.name();
        archiveNextFolder();
        return;
    }

    Akonadi::Item item = mPendingMessages.front();
    mPendingMessages.pop_front();
    qCDebug(MAILCOMMON_LOG) << "Fetching item with ID" << item.id() << "for folder" << mCurrentFolder.name();

    mCurrentJob = new Akonadi::ItemFetchJob(item);
    mCurrentJob->fetchScope().fetchFullPayload(true);
    connect(mCurrentJob, &Akonadi::ItemFetchJob::result, this, &BackupJob::itemFetchJobResult);
}

void BackupJob::processMessage(const Akonadi::Item &item)
{
    if (mAborted) {
        return;
    }

    const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    qCDebug(MAILCOMMON_LOG) << "Processing message with subject " << message->subject(false);
    const QByteArray messageData = message->encodedContent();
    const qint64 messageSize = messageData.size();
    const QString messageName = QString::number(item.id());
    const QString fileName = pathForCollection(mCurrentFolder) + QLatin1String("/cur/") + messageName;

    // PORT ME: user and group!
    qCDebug(MAILCOMMON_LOG) << "AKONDI PORT: disabled code here!";
    if (!mArchive->writeFile(fileName, messageData, archivePerms, QStringLiteral("user"), QStringLiteral("group"), mArchiveTime, mArchiveTime, mArchiveTime)) {
        abort(i18n("Failed to write a message into the archive folder '%1'.",
                   mCurrentFolder.name()));
        return;
    }

    ++mArchivedMessages;
    mArchivedSize += messageSize;

    // Use a singleshot timer, otherwise the job started in archiveNextMessage()
    // will hang
    QTimer::singleShot(0, this, &BackupJob::archiveNextMessage);
}

void BackupJob::itemFetchJobResult(KJob *job)
{
    if (mAborted) {
        return;
    }

    Q_ASSERT(job == mCurrentJob);
    mCurrentJob = 0;

    if (job->error()) {
        Q_ASSERT(mCurrentFolder.isValid());
        qCWarning(MAILCOMMON_LOG) << job->errorString();
        abort(i18n("Downloading a message in folder '%1' failed.", mCurrentFolder.name()));
    } else {
        Akonadi::ItemFetchJob *fetchJob = dynamic_cast<Akonadi::ItemFetchJob *>(job);
        Q_ASSERT(fetchJob);
        Q_ASSERT(fetchJob->items().size() == 1);
        processMessage(fetchJob->items().first());
    }
}

bool BackupJob::writeDirHelper(const QString &directoryPath)
{
    // PORT ME: Correct user/group
    qCDebug(MAILCOMMON_LOG) << "AKONDI PORT: Disabled code here!";
    return mArchive->writeDir(directoryPath, QStringLiteral("user"), QStringLiteral("group"), 040755, mArchiveTime, mArchiveTime, mArchiveTime);
}

QString BackupJob::collectionName(const Akonadi::Collection &collection) const
{
    foreach (const Akonadi::Collection &curCol, mAllFolders) {
        if (curCol == collection) {
            return curCol.name();
        }
    }
    Q_ASSERT(false);
    return QString();
}

QString BackupJob::pathForCollection(const Akonadi::Collection &collection) const
{
    QString fullPath = collectionName(collection);
    Akonadi::Collection curCol = collection.parentCollection();
    if (collection != mRootFolder) {
        Q_ASSERT(curCol.isValid());
        while (curCol != mRootFolder) {
            fullPath.prepend(QLatin1Char('.') + collectionName(curCol) + QLatin1String(".directory/"));
            curCol = curCol.parentCollection();
        }
        Q_ASSERT(curCol == mRootFolder);
        fullPath.prepend(QLatin1Char('.') + collectionName(curCol) + QLatin1String(".directory/"));
    }
    return fullPath;
}

QString BackupJob::subdirPathForCollection(const Akonadi::Collection &collection) const
{
    QString path = pathForCollection(collection);
    const int parentDirEndIndex = path.lastIndexOf(collection.name());
    Q_ASSERT(parentDirEndIndex != -1);
    path = path.left(parentDirEndIndex);
    path.append(QLatin1Char('.') + collection.name() + QLatin1String(".directory"));
    return path;
}

void BackupJob::archiveNextFolder()
{
    if (mAborted) {
        return;
    }

    if (mPendingFolders.isEmpty()) {
        finish();
        return;
    }

    mCurrentFolder = mPendingFolders.takeAt(0);
    qCDebug(MAILCOMMON_LOG) << "===> Archiving next folder: " << mCurrentFolder.name();
    const QString archivingStr(i18n("Archiving folder %1", mCurrentFolder.name()));
    if (mProgressItem) {
        mProgressItem->setStatus(archivingStr);
    }
    KPIM::BroadcastStatus::instance()->setStatusMsg(archivingStr);

    const QString folderName = mCurrentFolder.name();
    bool success = true;
    if (hasChildren(mCurrentFolder)) {
        if (!writeDirHelper(subdirPathForCollection(mCurrentFolder))) {
            success = false;
        }
    }
    if (success) {
        if (!writeDirHelper(pathForCollection(mCurrentFolder))) {
            success = false;
        } else if (!writeDirHelper(pathForCollection(mCurrentFolder) + QLatin1String("/cur"))) {
            success = false;
        } else if (!writeDirHelper(pathForCollection(mCurrentFolder) + QLatin1String("/new"))) {
            success = false;
        } else if (!writeDirHelper(pathForCollection(mCurrentFolder) + QLatin1String("/tmp"))) {
            success = false;
        }
    }
    if (!success) {
        abort(i18n("Unable to create folder structure for folder '%1' within archive file.",
                   mCurrentFolder.name()));
        return;
    }
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(mCurrentFolder);
    job->setProperty("folderName", folderName);
    connect(job, &Akonadi::ItemFetchJob::result, this, &BackupJob::onArchiveNextFolderDone);
}

void BackupJob::onArchiveNextFolderDone(KJob *job)
{
    if (job->error()) {
        qCWarning(MAILCOMMON_LOG) << job->errorString();
        abort(i18n("Unable to get message list for folder %1.",
                   job->property("folderName").toString()));
        return;
    }

    Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
    mPendingMessages += fetchJob->items();
    archiveNextMessage();
}

void BackupJob::start()
{
    Q_ASSERT(!mMailArchivePath.isEmpty());
    Q_ASSERT(mRootFolder.isValid());

    if (!queueFolders(mRootFolder)) {
        return;
    }

    switch (mArchiveType) {
    case Zip: {
        KZip *zip = new KZip(mMailArchivePath.path());
        zip->setCompression(KZip::DeflateCompression);
        mArchive = zip;
        break;
    }
    case Tar: {
        mArchive = new KTar(mMailArchivePath.path(), QStringLiteral("application/x-tar"));
        break;
    }
    case TarGz: {
        mArchive = new KTar(mMailArchivePath.path(), QStringLiteral("application/x-gzip"));
        break;
    }
    case TarBz2: {
        mArchive = new KTar(mMailArchivePath.path(), QStringLiteral("application/x-bzip2"));
        break;
    }
    }

    qCDebug(MAILCOMMON_LOG) << "Starting backup.";
    if (!mArchive->open(QIODevice::WriteOnly)) {
        abort(i18n("Unable to open archive for writing."));
        return;
    }

    mProgressItem = KPIM::ProgressManager::createProgressItem(
                        QStringLiteral("BackupJob"),
                        i18n("Archiving"),
                        QString(),
                        true);
    mProgressItem->setUsesBusyIndicator(true);
    connect(mProgressItem.data(), &KPIM::ProgressItem::progressItemCanceled, this, &BackupJob::cancelJob);

    archiveNextFolder();
}

void BackupJob::setDisplayMessageBox(bool display)
{
    mDisplayMessageBox = display;
}

