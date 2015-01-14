/*
  This file is part of KTnef.

  Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include "ktnefmain.h"
#include "attachpropertydialog.h"
#include "ktnefview.h"
#include "messagepropertydialog.h"

#include <KTNEF/KTNEFAttach>
#include <KTNEF/KTNEFMessage>
#include <KTNEF/KTNEFParser>
#include <KTNEF/KTNEFProperty>

#include <KFileItemActions>
#include <kservice.h>
#include <QAction>
#include <KActionCollection>
#include <QDebug>
#include <KEditToolBar>
#include <KLocalizedString>
#include <QMenu>
#include <KMessageBox>
#include <KRun>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QUrl>
#include <QIcon>

#include <KRecentFilesAction>
#include <KConfigGroup>

#include <QContextMenuEvent>
#include <QDir>
#include <QDrag>
#include <QMimeData>
#include <KSharedConfig>
#include <QMimeDatabase>
#include <QMimeType>
#include <QFileDialog>
#include <QStatusBar>

KTNEFMain::KTNEFMain(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    setupActions();
    setupStatusbar();

    setupTNEF();

    KConfigGroup config(KSharedConfig::openConfig(), "Settings");
    mDefaultDir = config.readPathEntry("defaultdir", QLatin1String("/tmp/"));

    config = KConfigGroup(KSharedConfig::openConfig(), "Recent Files");
    mOpenRecentFileAction->loadEntries(config);

    mLastDir = mDefaultDir;

    // create personal temp extract dir
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1String("/ktnef/"));

    resize(430, 350);

    setStandardToolBarMenuEnabled(true);

    createStandardStatusBarAction();

    setupGUI(Keys | Save | Create, QLatin1String("ktnefui.rc"));

    setAutoSaveSettings();
}

KTNEFMain::~KTNEFMain()
{
    delete mParser;
    cleanup();
}

void KTNEFMain::setupActions()
{
    KStandardAction::quit(this, SLOT(close()), actionCollection());

    QAction *action =
        KStandardAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "You will be presented with a dialog where you can configure "
              "the application-wide shortcuts."));

    KStandardAction::configureToolbars(this, SLOT(slotEditToolbars()), actionCollection());

    // File menu
    KStandardAction::open(this, SLOT(openFile()), actionCollection());

    mOpenRecentFileAction = KStandardAction::openRecent(this, SLOT(openRecentFile(QUrl)), actionCollection());

    // Action menu
    QAction *openAction = actionCollection()->addAction(QLatin1String("view_file"));
    openAction->setText(i18nc("@action:inmenu", "View"));
    openAction->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
    connect(openAction, &QAction::triggered, this, &KTNEFMain::viewFile);

    QAction *openAsAction = actionCollection()->addAction(QLatin1String("view_file_as"));
    openAsAction->setText(i18nc("@action:inmenu", "View With..."));
    connect(openAsAction, &QAction::triggered, this, &KTNEFMain::viewFileAs);

    QAction *extractAction = actionCollection()->addAction(QLatin1String("extract_file"));
    extractAction->setText(i18nc("@action:inmenu", "Extract"));
    connect(extractAction, &QAction::triggered, this, &KTNEFMain::extractFile);

    QAction *extractToAction = actionCollection()->addAction(QLatin1String("extract_file_to"));
    extractToAction->setText(i18nc("@action:inmenu", "Extract To..."));
    extractToAction->setIcon(QIcon::fromTheme(QLatin1String("archive-extract")));
    connect(extractToAction, &QAction::triggered, this, &KTNEFMain::extractFileTo);

    QAction *extractAllToAction = actionCollection()->addAction(QLatin1String("extract_all_files"));
    extractAllToAction->setText(i18nc("@action:inmenu", "Extract All To..."));
    extractAllToAction->setIcon(QIcon::fromTheme(QLatin1String("archive-extract")));
    connect(extractAllToAction, &QAction::triggered, this, &KTNEFMain::extractAllFiles);

    QAction *filePropsAction = actionCollection()->addAction(QLatin1String("properties_file"));
    filePropsAction->setText(i18nc("@action:inmenu", "Properties"));
    filePropsAction->setIcon(QIcon::fromTheme(QLatin1String("document-properties")));
    connect(filePropsAction, &QAction::triggered, this, &KTNEFMain::propertiesFile);

    QAction *messPropsAction = actionCollection()->addAction(QLatin1String("msg_properties"));
    messPropsAction->setText(i18nc("@action:inmenu", "Message Properties"));
    connect(messPropsAction, &QAction::triggered, this, &KTNEFMain::slotShowMessageProperties);

    QAction *messShowAction = actionCollection()->addAction(QLatin1String("msg_text"));
    messShowAction->setText(i18nc("@action:inmenu", "Show Message Text"));
    messShowAction->setIcon(QIcon::fromTheme(QLatin1String("document-preview-archive")));
    connect(messShowAction, &QAction::triggered, this, &KTNEFMain::slotShowMessageText);

    QAction *messSaveAction = actionCollection()->addAction(QLatin1String("msg_save"));
    messSaveAction->setText(i18nc("@action:inmenu", "Save Message Text As..."));
    messSaveAction->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    connect(messSaveAction, &QAction::triggered, this, &KTNEFMain::slotSaveMessageText);

    actionCollection()->action(QLatin1String("view_file"))->setEnabled(false);
    actionCollection()->action(QLatin1String("view_file_as"))->setEnabled(false);
    actionCollection()->action(QLatin1String("extract_file"))->setEnabled(false);
    actionCollection()->action(QLatin1String("extract_file_to"))->setEnabled(false);
    actionCollection()->action(QLatin1String("extract_all_files"))->setEnabled(false);
    actionCollection()->action(QLatin1String("properties_file"))->setEnabled(false);

    // Options menu
    QAction *defFolderAction = actionCollection()->addAction(QLatin1String("options_default_dir"));
    defFolderAction->setText(i18nc("@action:inmenu", "Default Folder..."));
    defFolderAction->setIcon(QIcon::fromTheme(QLatin1String("folder-open")));
    connect(defFolderAction, &QAction::triggered, this, &KTNEFMain::optionDefaultDir);

}

void KTNEFMain::slotConfigureKeys()
{
    KShortcutsDialog::configure(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this);
}

void KTNEFMain::setupStatusbar()
{
    statusBar()->showMessage(i18nc("@info:status", "No file loaded"));
}

void KTNEFMain::setupTNEF()
{
    mView = new KTNEFView(this);
    mView->setAllColumnsShowFocus(true);
    mParser = new KTNEFParser;

    setCentralWidget(mView);

    connect(mView, SIGNAL(itemSelectionChanged()),
            SLOT(viewSelectionChanged()));

    connect(mView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(viewDoubleClicked(QTreeWidgetItem*)));

//PORTME:  connect( mView, SIGNAL(dragRequested(QList<KTNEFAttach*>)),
//PORTME:           SLOT(viewDragRequested(QList<KTNEFAttach*>)) );
}

void KTNEFMain::loadFile(const QString &filename)
{
    mFilename = filename;
    setCaption(mFilename);
    if (!mParser->openFile(filename)) {
        KMessageBox::error(
            this,
            i18nc("@info",
                  "Unable to open file \"%1\".", filename));
        mView->setAttachments(QList<KTNEFAttach *>());
        enableExtractAll(false);
    } else {
        addRecentFile(QUrl::fromLocalFile(filename));
        QList<KTNEFAttach *> list = mParser->message()->attachmentList();
        QString msg;
        msg = i18ncp("@info:status",
                     "%1 attachment found", "%1 attachments found", list.count());
        statusBar()->showMessage(msg);
        mView->setAttachments(list);
        enableExtractAll((list.count() > 0));
        enableSingleAction(false);
    }
}

void KTNEFMain::openFile()
{
    QString filename = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Open TNEF File"),  QString(), QString());
    if (!filename.isEmpty()) {
        loadFile(filename);
    }
}

void KTNEFMain::openRecentFile(const QUrl &url)
{
    loadFile(url.path());
}

void KTNEFMain::addRecentFile(const QUrl &url)
{
    mOpenRecentFileAction->addUrl(url);
    KConfigGroup config(KSharedConfig::openConfig(), "Recent Files");
    mOpenRecentFileAction->saveEntries(config);
    config.sync();
}

void KTNEFMain::viewFile()
{
    if (!mView->getSelection().isEmpty()) {
        KTNEFAttach *attach = mView->getSelection().first();
        QUrl url = QUrl::fromLocalFile(extractTemp(attach));
        QString mimename(attach->mimeTag());

        if (mimename.isEmpty() || mimename == QLatin1String("application/octet-stream")) {
            qDebug() << "No mime type found in attachment object, trying to guess...";
            QMimeDatabase db;
            db.mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension).name();
            qDebug() << "Detected mime type: " << mimename;
        } else {
            qDebug() << "Mime type from attachment object: " << mimename;
        }

        KRun::runUrl(url, mimename, this, true);
    } else {
        KMessageBox::information(
            this,
            i18nc("@info",
                  "There is no file selected. Please select a file an try again."));
    }

}

QString KTNEFMain::extractTemp(KTNEFAttach *att)
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1String("/ktnef/");
    mParser->extractFileTo(att->name(), dir);
    dir.append(att->name());
    return dir;
}

void KTNEFMain::viewFileAs()
{
    if (!mView->getSelection().isEmpty()) {
        QList<QUrl> list;
        list.append(QUrl::fromLocalFile(extractTemp(mView->getSelection().first())));

        if (!list.isEmpty()) {
            KRun::displayOpenWithDialog(list, this);
        }
    } else {
        KMessageBox::information(
            this,
            i18nc("@info",
                  "There is no file selected. Please select a file an try again."));
    }
}

void KTNEFMain::extractFile()
{
    extractTo(mDefaultDir);
}

void KTNEFMain::extractFileTo()
{
    QString dir = QFileDialog::getExistingDirectory(this , QString(),  mLastDir);
    if (!dir.isEmpty()) {
        extractTo(dir);
        mLastDir = dir;
    }
}

void KTNEFMain::extractAllFiles()
{
    QString dir = QFileDialog::getExistingDirectory(this , QString(),  mLastDir);
    if (!dir.isEmpty()) {
        mLastDir = dir;
        dir.append(QLatin1String("/"));
        QList<KTNEFAttach *> list = mParser->message()->attachmentList();
        QList<KTNEFAttach *>::ConstIterator it;
        QList<KTNEFAttach *>::ConstIterator end(list.constEnd());
        for (it = list.constBegin(); it != end; ++it) {
            if (!mParser->extractFileTo((*it)->name(), dir)) {
                KMessageBox::error(
                    this,
                    i18nc("@info",
                          "Unable to extract file \"%1\".", (*it)->name()));
                return;
            }
        }
    }
}

void KTNEFMain::propertiesFile()
{
    KTNEFAttach *attach = mView->getSelection().first();
    AttachPropertyDialog dlg(this);
    dlg.setAttachment(attach);
    dlg.exec();
}

void KTNEFMain::optionDefaultDir()
{
    const QString dirname = QFileDialog::getExistingDirectory(this , QString(),  mDefaultDir);
    if (!dirname.isEmpty()) {
        mDefaultDir = dirname;

        KConfigGroup config(KSharedConfig::openConfig(), "Settings");
        config.writePathEntry("defaultdir", mDefaultDir);
    }
}

void KTNEFMain::viewSelectionChanged()
{
    const QList<KTNEFAttach *> list = mView->getSelection();
    const int nbItem = list.count();
    const bool on1 = (nbItem == 1);
    const bool on2 = (nbItem > 0);

    actionCollection()->action(QLatin1String("view_file"))->setEnabled(on1);
    actionCollection()->action(QLatin1String("view_file_as"))->setEnabled(on1);
    actionCollection()->action(QLatin1String("properties_file"))->setEnabled(on1);

    actionCollection()->action(QLatin1String("extract_file"))->setEnabled(on2);
    actionCollection()->action(QLatin1String("extract_file_to"))->setEnabled(on2);
}

void KTNEFMain::enableExtractAll(bool on)
{
    if (!on) {
        enableSingleAction(false);
    }

    actionCollection()->action(QLatin1String("extract_all_files"))->setEnabled(on);
}

void KTNEFMain::enableSingleAction(bool on)
{
    actionCollection()->action(QLatin1String("extract_file"))->setEnabled(on);
    actionCollection()->action(QLatin1String("extract_file_to"))->setEnabled(on);
    actionCollection()->action(QLatin1String("view_file"))->setEnabled(on);
    actionCollection()->action(QLatin1String("view_file_as"))->setEnabled(on);
    actionCollection()->action(QLatin1String("properties_file"))->setEnabled(on);
}

void KTNEFMain::cleanup()
{
    QDir d(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1String("/ktnef/"));
    d.removeRecursively();
}

void KTNEFMain::extractTo(const QString &dirname)
{
    QString dir = dirname;
    if (dir.right(1) != QLatin1String("/")) {
        dir.append(QLatin1String("/"));
    }
    QList<KTNEFAttach *>list = mView->getSelection();
    QList<KTNEFAttach *>::ConstIterator it;
    QList<KTNEFAttach *>::ConstIterator end(list.constEnd());
    for (it = list.constBegin(); it != end; ++it) {
        if (!mParser->extractFileTo((*it)->name(), dir)) {
            KMessageBox::error(
                this,
                i18nc("@info",
                      "Unable to extract file \"%1\".", (*it)->name()));
            return;
        }
    }
}

/* This breaks the saveMainWindowSettings stuff....
  void KTNEFMain::closeEvent(QCloseEvent *e)
{
  e->accept();
}*/

void KTNEFMain::contextMenuEvent(QContextMenuEvent *event)
{
    QList<KTNEFAttach *> list = mView->getSelection();
    if (!list.count()) {
        return;
    }

    QAction *prop = 0;
    QMenu *menu = new QMenu();
    if (list.count() == 1) {
        createOpenWithMenu(menu);
        menu->addSeparator();
    }
    QAction *extract = menu->addAction(i18nc("@action:inmenu", "Extract"));
    QAction *extractTo = menu->addAction(QIcon::fromTheme(QLatin1String("archive-extract")),
                                         i18nc("@action:inmenu", "Extract To..."));
    if (list.count() == 1) {
        menu->addSeparator();
        prop = menu->addAction(QIcon::fromTheme(QLatin1String("document-properties")),
                               i18nc("@action:inmenu", "Properties"));
    }

    QAction *a = menu->exec(event->globalPos(), 0);
    if (a) {
        if (a == extract) {
            extractFile();
        } else if (a == extractTo) {
            extractFileTo();
        } else if (a == prop) {
            propertiesFile();
        }
    }
    delete menu;
}

void KTNEFMain::viewDoubleClicked(QTreeWidgetItem *item)
{
    if (item && item->isSelected()) {
        viewFile();
    }
}

void KTNEFMain::viewDragRequested(const QList<KTNEFAttach *> &list)
{
    QList<QUrl> urlList;
    QList<KTNEFAttach *>::ConstIterator end(list.constEnd());
    for (QList<KTNEFAttach *>::ConstIterator it = list.constBegin();
            it != end; ++it) {
        urlList << QUrl::fromLocalFile(extractTemp(*it));
    }

    if (!list.isEmpty()) {
        QMimeData *mimeData = new QMimeData;
        mimeData->setUrls(urlList);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
    }
}

void KTNEFMain::slotEditToolbars()
{
    KConfigGroup grp = KSharedConfig::openConfig()->group("MainWindow");
    saveMainWindowSettings(grp);

    KEditToolBar dlg(factory());
    connect(&dlg, &KEditToolBar::newToolBarConfig, this, &KTNEFMain::slotNewToolbarConfig);
    dlg.exec();
}

void KTNEFMain::slotNewToolbarConfig()
{
    createGUI(QLatin1String("ktnefui.rc"));
    applyMainWindowSettings(KSharedConfig::openConfig()->group("MainWindow"));
}

void KTNEFMain::slotShowMessageProperties()
{
    MessagePropertyDialog dlg(this, mParser->message());
    dlg.exec();
}

void KTNEFMain::slotShowMessageText()
{
    if (!mParser->message()) {
        return;
    }

    QString rtf = mParser->message()->rtfString();
    if (!rtf.isEmpty()) {
        QTemporaryFile *tmpFile = new QTemporaryFile(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1String("/ktnef/") + QLatin1String("ktnef_XXXXXX.rtf"));
        tmpFile->open();
        tmpFile->setPermissions(QFile::ReadUser);
        tmpFile->write(rtf.toLocal8Bit());
        tmpFile->close();

        KRun::runUrl(QUrl::fromLocalFile(tmpFile->fileName()), QLatin1String("text/rtf"), this, true);
        delete tmpFile;
    } else {
        KMessageBox::error(
            this,
            i18nc("@info",
                  "The message does not contain any Rich Text data."));
    }
}

void KTNEFMain::slotSaveMessageText()
{
    if (!mParser->message()) {
        return;
    }

    QString rtf = mParser->message()->rtfString();
    QString filename = QFileDialog::getSaveFileName(this , QString(), QString(), QString());
    if (!filename.isEmpty()) {
        QFile f(filename);
        if (f.open(QIODevice::WriteOnly)) {
            QTextStream t(&f);
            t << rtf;
        } else {
            KMessageBox::error(
                this,
                i18nc("@info",
                      "Unable to open file \"%1\" for writing, check file permissions.", filename));
        }
    }
}

void KTNEFMain::openWith(KService::Ptr offer)
{
    if ( !mView->getSelection().isEmpty() ) {
        KTNEFAttach *attach = mView->getSelection().first();
        QUrl url = QUrl::fromLocalFile( QLatin1String("file:") + extractTemp( attach ) );
        QList<QUrl> lst;
        lst.append( url );
        bool result = false;
        if(offer) {
            result = KRun::run( *offer, lst, this, false );
        } else {
            result = KRun::displayOpenWithDialog( lst, this, false );
        }
    }
}

QAction* KTNEFMain::createAppAction(const KService::Ptr& service, bool singleOffer, QActionGroup *actionGroup, QObject *parent )
{
    QString actionName(service->name().replace(QLatin1Char('&'), QLatin1String("&&")));
    if (singleOffer) {
        actionName = i18n("Open &with %1", actionName);
    } else {
        actionName = i18nc("@item:inmenu Open With, %1 is application name", "%1", actionName);
    }

    QAction *act = new QAction(parent);
    act->setIcon(QIcon::fromTheme(service->icon()));
    act->setText(actionName);
    actionGroup->addAction( act );
    act->setData(QVariant::fromValue(service));
    return act;
}

void KTNEFMain::createOpenWithMenu( QMenu *topMenu )
{
    if (mView->getSelection().isEmpty())
        return;
    KTNEFAttach *attach = mView->getSelection().first();
    QString mimename( attach->mimeTag() );

    const KService::List offers = KFileItemActions::associatedApplications(QStringList()<<mimename, QString() );
    if (!offers.isEmpty()) {
        QMenu* menu = topMenu;
        QActionGroup *actionGroup = new QActionGroup( menu );
        connect( actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotOpenWithAction(QAction*)) );

        if (offers.count() > 1) { // submenu 'open with'
            menu = new QMenu(i18nc("@title:menu", "&Open With"), topMenu);
            menu->menuAction()->setObjectName(QLatin1String("openWith_submenu")); // for the unittest
            topMenu->addMenu(menu);
        }
        //kDebug() << offers.count() << "offers" << topMenu << menu;

        KService::List::ConstIterator it = offers.constBegin();
        KService::List::ConstIterator end = offers.constEnd();
        for(; it != end; ++it) {
            QAction* act = createAppAction(*it,
                                           // no submenu -> prefix single offer
                                           menu == topMenu, actionGroup,menu);
            menu->addAction(act);
        }

        QString openWithActionName;
        if (menu != topMenu) { // submenu
            menu->addSeparator();
            openWithActionName = i18nc("@action:inmenu Open With", "&Other...");
        } else {
            openWithActionName = i18nc("@title:menu", "&Open With...");
        }
        QAction *openWithAct = new QAction(menu);
        openWithAct->setText(openWithActionName);
        connect(openWithAct, SIGNAL(triggered()), this, SLOT(viewFileAs()));
        menu->addAction(openWithAct);
    }
    else { // no app offers -> Open With...
        QAction *act = new QAction(topMenu);
        act->setText(i18nc("@title:menu", "&Open With..."));
        connect(act, SIGNAL(triggered()), this, SLOT(viewFileAs()));
        topMenu->addAction(act);
    }
}

void KTNEFMain::slotOpenWithAction(QAction*act)
{
    KService::Ptr app = act->data().value<KService::Ptr>();

    openWith(app);
}
