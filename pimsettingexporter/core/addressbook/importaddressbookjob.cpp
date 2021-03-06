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

#include "importaddressbookjob.h"
#include "archivestorage.h"

#include "PimCommon/CreateResource"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KZip>
#include <KArchiveEntry>

#include <QTimer>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

namespace
{
inline const QString storeAddressbook()
{
    return QStringLiteral("backupaddressbook/");
}
}

ImportAddressbookJob::ImportAddressbookJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportAddressbookJob::~ImportAddressbookJob()
{

}

void ImportAddressbookJob::start()
{
    Q_EMIT title(i18n("Start import KAddressBook settings..."));
    mArchiveDirectory = archive()->directory();
    createProgressDialog(i18n("Import KAddressBook settings"));
    searchAllFiles(mArchiveDirectory, QString(), QStringLiteral("addressbook"));
    initializeListStep();
    QTimer::singleShot(0, this, SLOT(slotNextStep()));
}

void ImportAddressbookJob::slotNextStep()
{
    ++mIndex;
    if (mIndex < mListStep.count()) {
        const Utils::StoredType type = mListStep.at(mIndex);
        if (type == Utils::Resources) {
            restoreResources();
        } else if (type == Utils::Config) {
            restoreConfig();
        }
    } else {
        Q_EMIT jobFinished();
    }
}

void ImportAddressbookJob::restoreResources()
{
    Q_EMIT info(i18n("Restore resources..."));
    setProgressDialogLabel(i18n("Restore resources..."));
    increaseProgressDialog();
    QStringList listResource;
    listResource << restoreResourceFile(QStringLiteral("akonadi_vcard_resource"), Utils::addressbookPath(), QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/share/kaddressbook/"));

    if (!mListResourceFile.isEmpty()) {
        QDir dir(mTempDirName);
        dir.mkdir(Utils::addressbookPath());
        const QString copyToDirName(mTempDirName + QLatin1Char('/') + Utils::addressbookPath());

        const int numberOfResourceFile = mListResourceFile.size();
        for (int i = 0; i < numberOfResourceFile; ++i) {
            resourceFiles value = mListResourceFile.at(i);
            QMap<QString, QVariant> settings;
            if (value.akonadiConfigFile.contains(QStringLiteral("akonadi_vcarddir_resource_")) ||
                    value.akonadiConfigFile.contains(QStringLiteral("akonadi_contacts_resource_"))) {
                const KArchiveEntry *fileResouceEntry = mArchiveDirectory->entry(value.akonadiConfigFile);
                if (fileResouceEntry && fileResouceEntry->isFile()) {
                    const KArchiveFile *file = static_cast<const KArchiveFile *>(fileResouceEntry);
                    file->copyTo(copyToDirName);
                    QString resourceName(file->name());

                    QString filename(file->name());
                    //TODO adapt filename otherwise it will use all the time the same filename.
                    KSharedConfig::Ptr resourceConfig = KSharedConfig::openConfig(copyToDirName + QLatin1Char('/') + resourceName);

                    //TODO fix default path
                    const QString newUrl = Utils::adaptResourcePath(resourceConfig, QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/share/contacts"));
                    QFileInfo newUrlInfo(newUrl);
                    const QString dataFile = value.akonadiResources;
                    const KArchiveEntry *dataResouceEntry = mArchiveDirectory->entry(dataFile);
                    if (dataResouceEntry->isFile()) {
                        const KArchiveFile *file = static_cast<const KArchiveFile *>(dataResouceEntry);
                        //TODO  adapt directory name too
                        extractZipFile(file, copyToDirName, newUrlInfo.path(), value.akonadiConfigFile.contains(QStringLiteral("akonadi_contacts_resource_")));
                    }
                    settings.insert(QStringLiteral("Path"), newUrl);

                    const QString agentConfigFile = value.akonadiAgentConfigFile;
                    if (!agentConfigFile.isEmpty()) {
                        const KArchiveEntry *akonadiAgentConfigEntry = mArchiveDirectory->entry(agentConfigFile);
                        if (akonadiAgentConfigEntry->isFile()) {
                            const KArchiveFile *file = static_cast<const KArchiveFile *>(akonadiAgentConfigEntry);
                            file->copyTo(copyToDirName);
                            resourceName = file->name();
                            filename = Utils::akonadiAgentName(copyToDirName + QLatin1Char('/') + resourceName);
                        }
                    }
                    QString instanceType;
                    if (value.akonadiConfigFile.contains(QStringLiteral("akonadi_vcarddir_resource_"))) {
                        instanceType = QStringLiteral("akonadi_vcarddir_resource");
                    } else if (value.akonadiConfigFile.contains(QStringLiteral("akonadi_contacts_resource_"))) {
                        instanceType = QStringLiteral("akonadi_contacts_resource");
                    } else {
                        qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " not supported" << value.akonadiConfigFile;
                    }

                    const QString newResource = mCreateResource->createResource(instanceType, filename, settings, true);
                    infoAboutNewResource(newResource);
                    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " newResource" << newResource;
                    listResource << newResource;
                }
            }
        }
    }

    Q_EMIT info(i18n("Resources restored."));
    //It's maildir support. Need to add support
    startSynchronizeResources(listResource);
}

void ImportAddressbookJob::addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings)
{
    if (resourceName == QLatin1String("akonadi_vcard_resource")) {
        KConfigGroup general = resourceConfig->group(QStringLiteral("General"));
        if (general.hasKey(QStringLiteral("DisplayName"))) {
            settings.insert(QStringLiteral("DisplayName"), general.readEntry(QStringLiteral("DisplayName")));
        }
        if (general.hasKey(QStringLiteral("ReadOnly"))) {
            settings.insert(QStringLiteral("ReadOnly"), general.readEntry(QStringLiteral("ReadOnly"), false));
        }
        if (general.hasKey(QStringLiteral("MonitorFile"))) {
            settings.insert(QStringLiteral("MonitorFile"), general.readEntry(QStringLiteral("MonitorFile"), true));
        }
    }
}

bool ImportAddressbookJob::isAConfigFile(const QString &name) const
{
    return name.endsWith(QLatin1String("rc")) && (name.contains(QStringLiteral("akonadi_vcarddir_resource_")) ||
            name.contains(QStringLiteral("akonadi_vcard_resource_")) ||
            name.contains(QStringLiteral("akonadi_contacts_resource_")));
}

void ImportAddressbookJob::restoreConfig()
{
    increaseProgressDialog();
    setProgressDialogLabel(i18n("Restore configs..."));
    const QString kaddressbookStr(QStringLiteral("kaddressbookrc"));
    const KArchiveEntry *kaddressbookrcentry  = mArchiveDirectory->entry(Utils::configsPath() + kaddressbookStr);
    if (kaddressbookrcentry && kaddressbookrcentry->isFile()) {
        const KArchiveFile *kaddressbookrcFile = static_cast<const KArchiveFile *>(kaddressbookrcentry);
        const QString kaddressbookrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + kaddressbookStr;
        if (QFile(kaddressbookrc).exists()) {
            if (overwriteConfigMessageBox(kaddressbookStr)) {
                importkaddressBookConfig(kaddressbookrcFile, kaddressbookrc, kaddressbookStr, Utils::configsPath());
            }
        } else {
            importkaddressBookConfig(kaddressbookrcFile, kaddressbookrc, kaddressbookStr, Utils::configsPath());
        }
    }
    restoreUiRcFile(QStringLiteral("kaddressbookui.rc"), QStringLiteral("kaddressbook"));
    Q_EMIT info(i18n("Config restored."));
    QTimer::singleShot(0, this, &ImportAddressbookJob::slotNextStep);
}

void ImportAddressbookJob::importkaddressBookConfig(const KArchiveFile *file, const QString &config, const QString &filename, const QString &prefix)
{
    copyToFile(file, config, filename, prefix);
    KSharedConfig::Ptr kaddressBookConfig = KSharedConfig::openConfig(config);

    const QString collectionViewCheckStateStr(QStringLiteral("CollectionViewCheckState"));
    if (kaddressBookConfig->hasGroup(collectionViewCheckStateStr)) {
        KConfigGroup group = kaddressBookConfig->group(collectionViewCheckStateStr);
        const QString selectionKey(QStringLiteral("Selection"));
        convertRealPathToCollectionList(group, selectionKey, true);
    }

    const QString collectionViewStateStr(QStringLiteral("CollectionViewState"));
    if (kaddressBookConfig->hasGroup(collectionViewStateStr)) {
        KConfigGroup group = kaddressBookConfig->group(collectionViewStateStr);
        QString currentKey(QStringLiteral("Current"));
        convertRealPathToCollection(group, currentKey, true);

        currentKey = QStringLiteral("Expansion");
        convertRealPathToCollection(group, currentKey, true);

        currentKey = QStringLiteral("Selection");
        convertRealPathToCollection(group, currentKey, true);
    }
    const QString cvsTemplateDirName = QStringLiteral("/kaddressbook/csv-templates/");
    const KArchiveEntry *csvtemplateEntry  = mArchiveDirectory->entry(Utils::dataPath() + cvsTemplateDirName);
    if (csvtemplateEntry && csvtemplateEntry->isDirectory()) {
        const KArchiveDirectory *csvTemplateDir = static_cast<const KArchiveDirectory *>(csvtemplateEntry);
        Q_FOREACH (const QString &entryName, csvTemplateDir->entries()) {
            const KArchiveEntry *entry = csvTemplateDir->entry(entryName);
            if (entry && entry->isFile()) {
                const KArchiveFile *csvTemplateFile = static_cast<const KArchiveFile *>(entry);
                const QString name = csvTemplateFile->name();
                QString autocorrectionPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + cvsTemplateDirName;
                if (QFile(autocorrectionPath).exists()) {
                    if (overwriteConfigMessageBox(name)) {
                        copyToFile(csvTemplateFile, autocorrectionPath + QLatin1Char('/') + name, name, Utils::dataPath() + cvsTemplateDirName);
                    }
                } else {
                    autocorrectionPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + cvsTemplateDirName;
                    copyToFile(csvTemplateFile, autocorrectionPath + QLatin1Char('/') + name, name, Utils::dataPath() + cvsTemplateDirName);
                }
            }
        }
    }

    importDataSubdirectory(QStringLiteral("/kaddressbook/viewertemplates/"));
    importDataSubdirectory(QStringLiteral("/kaddressbook/printing/"));
    kaddressBookConfig->sync();
}
