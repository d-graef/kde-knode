/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "storageserviceprogressmanager.h"
#include "Libkdepim/ProgressManager"
#include "storageservice/storageserviceabstract.h"

namespace PimCommon
{
class StorageServiceProgressManagerPrivate
{
public:
    StorageServiceProgressManagerPrivate()
        : storageServiceProgressManager(new StorageServiceProgressManager)
    {
    }

    ~StorageServiceProgressManagerPrivate()
    {
        delete storageServiceProgressManager;
    }

    StorageServiceProgressManager *storageServiceProgressManager;
};

Q_GLOBAL_STATIC(StorageServiceProgressManagerPrivate, sInstance)

StorageServiceProgressManager::StorageServiceProgressManager(QObject *parent)
    : QObject(parent)
{
}

StorageServiceProgressManager::~StorageServiceProgressManager()
{

}

unsigned int StorageServiceProgressManager::progressTypeValue()
{
    return 1;
}

StorageServiceProgressManager *StorageServiceProgressManager::self()
{
    return sInstance->storageServiceProgressManager; //will create it
}

void StorageServiceProgressManager::addProgress(PimCommon::StorageServiceAbstract *storageService, ProgressType type)
{
    if (!mHashList.contains(storageService->storageServiceName())) {
        KPIM::ProgressItem *progressItem = KPIM::ProgressManager::createProgressItem(StorageServiceProgressManager::progressTypeValue(), storageService->storageServiceName());
        progressItem->setCryptoStatus(KPIM::ProgressItem::Unknown);
        progressItem->setCanBeCanceled(storageService->hasCancelSupport());
        ProgressJob *job = new ProgressJob(progressItem, type);
        job->setStorageService(storageService);

        mHashList.insert(storageService->storageServiceName(), job);
        connect(progressItem, &KPIM::ProgressItem::progressItemCanceled, this, &StorageServiceProgressManager::slotProgressItemCanceled);
        connect(storageService, &StorageServiceAbstract::uploadFileDone, this, &StorageServiceProgressManager::slotUploadFileDone, Qt::UniqueConnection);
        connect(storageService, &StorageServiceAbstract::uploadFileFailed, this, &StorageServiceProgressManager::slotUploadFileFailed, Qt::UniqueConnection);
        connect(storageService, &StorageServiceAbstract::downLoadFileDone, this, &StorageServiceProgressManager::slotDownloadFileDone, Qt::UniqueConnection);
        connect(storageService, &StorageServiceAbstract::downLoadFileFailed, this, &StorageServiceProgressManager::slotDownloadFileFailed, Qt::UniqueConnection);
        connect(storageService, &StorageServiceAbstract::actionFailed, this, &StorageServiceProgressManager::slotActionFailed, Qt::UniqueConnection);
        connect(storageService, &StorageServiceAbstract::uploadDownloadFileProgress, this, &StorageServiceProgressManager::slotDownloadFileProgress, Qt::UniqueConnection);
    }
}

void StorageServiceProgressManager::slotActionFailed(const QString &serviceName, const QString &)
{
    if (mHashList.contains(serviceName)) {
        ProgressJob *job = mHashList.value(serviceName);
        KPIM::ProgressItem *mProgressItem = job->item();
        if (mProgressItem) {
            mProgressItem->setComplete();
        }
        mHashList.remove(serviceName);
    }
}

void StorageServiceProgressManager::slotDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total)
{
    if (mHashList.contains(serviceName)) {
        ProgressJob *job = mHashList.value(serviceName);
        KPIM::ProgressItem *mProgressItem = job->item();
        if (mProgressItem) {
            if (total > 0) {
                mProgressItem->setProgress((100 * done) / total);
            } else {
                mProgressItem->setProgress(100);
            }
        }
    }
}

void StorageServiceProgressManager::slotDownloadFileDone(const QString &serviceName, const QString &)
{
    ProgressJob *job = mHashList.value(serviceName);
    if (job) {
        KPIM::ProgressItem *mProgressItem = job->item();
        if (mProgressItem) {
            mProgressItem->setComplete();
        }
        mHashList.remove(serviceName);
    }
}

void StorageServiceProgressManager::slotDownloadFileFailed(const QString &serviceName, const QString &)
{
    ProgressJob *job = mHashList.value(serviceName);
    if (job) {
        KPIM::ProgressItem *mProgressItem = job->item();
        if (mProgressItem) {
            mProgressItem->setComplete();
        }
        mHashList.remove(serviceName);
    }
}

void StorageServiceProgressManager::slotUploadFileDone(const QString &serviceName, const QString &)
{
    if (mHashList.contains(serviceName)) {
        ProgressJob *job = mHashList.value(serviceName);
        KPIM::ProgressItem *mProgressItem = job->item();
        if (mProgressItem) {
            mProgressItem->setComplete();
        }
        mHashList.remove(serviceName);
    }
}

void StorageServiceProgressManager::slotUploadFileFailed(const QString &serviceName, const QString &)
{
    if (mHashList.contains(serviceName)) {
        ProgressJob *job = mHashList.value(serviceName);
        KPIM::ProgressItem *mProgressItem = job->item();
        if (mProgressItem) {
            mProgressItem->setComplete();
        }
        mHashList.remove(serviceName);
    }
}

void StorageServiceProgressManager::slotProgressItemCanceled(KPIM::ProgressItem *item)
{
    QHashIterator<QString, ProgressJob *> i(mHashList);
    while (i.hasNext()) {
        i.next();
        if (i.value()->item() == item) {
            ProgressType type = i.value()->type();
            switch (type) {
            case DownLoad:
                i.value()->storageService()->cancelDownloadFile();
                break;
            case Upload:
                i.value()->storageService()->cancelUploadFile();
                break;
            }
        }
    }
}

ProgressJob::ProgressJob(KPIM::ProgressItem *item, StorageServiceProgressManager::ProgressType type)
    : mType(type),
      mStorageService(Q_NULLPTR),
      mProgressItem(item)
{
}

void ProgressJob::setStorageService(StorageServiceAbstract *storage)
{
    mStorageService = storage;
}

StorageServiceAbstract *ProgressJob::storageService() const
{
    return mStorageService;
}

StorageServiceProgressManager::ProgressType ProgressJob::type() const
{
    return mType;
}

KPIM::ProgressItem *ProgressJob::item() const
{
    return mProgressItem;
}

}
