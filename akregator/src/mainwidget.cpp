/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "mainwidget.h"
#include "actionmanagerimpl.h"
#include "addfeeddialog.h"
#include "articlelistview.h"
#include "articleviewer.h"
#include "abstractselectioncontroller.h"
#include "articlejobs.h"
#include "articlematcher.h"
#include "akregatorconfig.h"
#include "akregator_part.h"
#include "browserframe.h"
#include "createfeedcommand.h"
#include "createfoldercommand.h"
#include "deletesubscriptioncommand.h"
#include "editsubscriptioncommand.h"
#include "expireitemscommand.h"
#include "importfeedlistcommand.h"
#include "feed.h"
#include "feedlist.h"
#include "feedpropertiesdialog.h"
#include "fetchqueue.h"
#include "folder.h"
#include "framemanager.h"
#include "kernel.h"
#include "notificationmanager.h"
#include "openurlrequest.h"
#include "progressmanager.h"
#include "searchbar.h"
#include "selectioncontroller.h"
#include "subscriptionlistjobs.h"
#include "subscriptionlistmodel.h"
#include "subscriptionlistview.h"
#include "tabwidget.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "types.h"
#include <solid/networking.h>

#include <QAction>
#include <kfileitem.h>
#include <kiconloader.h>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <krandom.h>
#include <kshell.h>

#include <ktoggleaction.h>
#include <QUrl>

#include <QClipboard>
#include <QNetworkConfigurationManager>
#include <QSplitter>
#include <QTextDocument>
#include <QDomDocument>
#include <QTimer>
#include <QDesktopServices>

#include <algorithm>
#include <memory>
#include <cassert>

using namespace Akregator;

Akregator::MainWidget::~MainWidget()
{
    // if m_shuttingDown is false, slotOnShutdown was not called. That
    // means that not the whole app is shutdown, only the part. So it
    // should be no risk to do the cleanups now
    if (!m_shuttingDown) {
        slotOnShutdown();
    }
}

Akregator::MainWidget::MainWidget(Part *part, QWidget *parent, ActionManagerImpl *actionManager, const char *name)
    : QWidget(parent),
      m_feedList(),
      m_viewMode(NormalView),
      m_actionManager(actionManager),
      m_feedListManagementInterface(new FeedListManagementImpl),
      m_networkConfigManager(new QNetworkConfigurationManager(this))
{
    setObjectName(QLatin1String(name));

    FeedListManagementInterface::setInstance(m_feedListManagementInterface);

    m_actionManager->initMainWidget(this);
    m_actionManager->initFrameManager(Kernel::self()->frameManager());
    m_part = part;
    m_shuttingDown = false;
    m_displayingAboutPage = false;
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *lt = new QVBoxLayout(this);
    lt->setMargin(0);

    m_horizontalSplitter = new QSplitter(Qt::Horizontal, this);

    m_horizontalSplitter->setOpaqueResize(true);
    lt->addWidget(m_horizontalSplitter);

    connect(Kernel::self()->fetchQueue(), &FetchQueue::signalStarted,
            this, &MainWidget::slotFetchingStarted);
    connect(Kernel::self()->fetchQueue(), &FetchQueue::signalStopped,
            this, &MainWidget::slotFetchingStopped);

    m_feedListView = new SubscriptionListView(m_horizontalSplitter);
    m_feedListView->setObjectName(QStringLiteral("feedtree"));
    m_actionManager->initSubscriptionListView(m_feedListView);

    connect(m_feedListView, &SubscriptionListView::userActionTakingPlace,
            this, &MainWidget::ensureArticleTabVisible);

    // FIXME:
    // connect(m_feedListView, SIGNAL(signalDropped (QList<QUrl> &, Akregator::TreeNode*,
    //         Akregator::Folder*)),
    //         this, SLOT(slotFeedUrlDropped (QList<QUrl> &,
    //         Akregator::TreeNode*, Akregator::Folder*)));

    m_tabWidget = new TabWidget(m_horizontalSplitter);
    m_actionManager->initTabWidget(m_tabWidget);

    connect(m_part, &Part::signalSettingsChanged,
            m_tabWidget, &TabWidget::slotSettingsChanged);

    connect(m_tabWidget, &TabWidget::signalCurrentFrameChanged,
            Kernel::self()->frameManager(), &FrameManager::slotChangeFrame);

    connect(m_tabWidget, &TabWidget::signalRemoveFrameRequest,
            Kernel::self()->frameManager(), &FrameManager::slotRemoveFrame);

    connect(m_tabWidget, SIGNAL(signalOpenUrlRequest(Akregator::OpenUrlRequest&)),
            Kernel::self()->frameManager(), SLOT(slotOpenUrlRequest(Akregator::OpenUrlRequest&)));

    connect(Kernel::self()->frameManager(), &FrameManager::signalFrameAdded,
            m_tabWidget, &TabWidget::slotAddFrame);

    connect(Kernel::self()->frameManager(), &FrameManager::signalSelectFrame,
            m_tabWidget, &TabWidget::slotSelectFrame);

    connect(Kernel::self()->frameManager(), &FrameManager::signalFrameRemoved,
            m_tabWidget, &TabWidget::slotRemoveFrame);

    connect(Kernel::self()->frameManager(), &FrameManager::signalRequestNewFrame,
            this, &MainWidget::slotRequestNewFrame);

    connect(Kernel::self()->frameManager(), &FrameManager::signalFrameAdded,
            this, &MainWidget::slotFramesChanged);
    connect(Kernel::self()->frameManager(), &FrameManager::signalFrameRemoved,
            this, &MainWidget::slotFramesChanged);
    connect(m_networkConfigManager, &QNetworkConfigurationManager::onlineStateChanged,
            this, &MainWidget::slotNetworkStatusChanged);

    m_tabWidget->setWhatsThis(i18n("You can view multiple articles in several open tabs."));

    m_mainTab = new QWidget(this);
    m_mainTab->setObjectName(QStringLiteral("Article Tab"));
    m_mainTab->setWhatsThis(i18n("Articles list."));

    QVBoxLayout *mainTabLayout = new QVBoxLayout(m_mainTab);
    mainTabLayout->setMargin(0);

    m_searchBar = new SearchBar(m_mainTab);
    if (!Settings::showQuickFilter()) {
        m_searchBar->hide();
    }

    mainTabLayout->addWidget(m_searchBar);

    m_articleSplitter = new QSplitter(Qt::Vertical, m_mainTab);
    m_articleSplitter->setObjectName(QStringLiteral("panner2"));

    m_articleListView = new ArticleListView(m_articleSplitter);
    connect(m_articleListView, &ArticleListView::userActionTakingPlace,
            this, &MainWidget::ensureArticleTabVisible);

    m_selectionController = new SelectionController(this);
    m_selectionController->setArticleLister(m_articleListView);
    m_selectionController->setFeedSelector(m_feedListView);

    connect(m_searchBar, &SearchBar::signalSearch,
            m_selectionController, &AbstractSelectionController::setFilters);

    FolderExpansionHandler *expansionHandler = new FolderExpansionHandler(this);
    connect(m_feedListView, &QTreeView::expanded, expansionHandler, &FolderExpansionHandler::itemExpanded);
    connect(m_feedListView, &QTreeView::collapsed, expansionHandler, &FolderExpansionHandler::itemCollapsed);

    m_selectionController->setFolderExpansionHandler(expansionHandler);

    connect(m_selectionController, &AbstractSelectionController::currentSubscriptionChanged,
            this, &MainWidget::slotNodeSelected);

    connect(m_selectionController, &AbstractSelectionController::currentArticleChanged,
            this, &MainWidget::slotArticleSelected);

    connect(m_selectionController, &AbstractSelectionController::articleDoubleClicked,
            this, &MainWidget::slotOpenArticleInBrowser);

    m_actionManager->initArticleListView(m_articleListView);

    connect(m_articleListView, &ArticleListView::signalMouseButtonPressed,
            this, &MainWidget::slotMouseButtonPressed);

    /*
        connect( m_part, SIGNAL(signalSettingsChanged()),
                 m_articleListView, SLOT(slotPaletteOrFontChanged()));
    */

    m_articleViewer = new ArticleViewer(m_articleSplitter);
    m_actionManager->initArticleViewer(m_articleViewer);
    m_articleListView->setFocusProxy(m_articleViewer);
    setFocusProxy(m_articleViewer);

    connect(m_articleViewer, SIGNAL(signalOpenUrlRequest(Akregator::OpenUrlRequest&)),
            Kernel::self()->frameManager(), SLOT(slotOpenUrlRequest(Akregator::OpenUrlRequest&)));
    connect(m_articleViewer->part()->browserExtension(), &KParts::BrowserExtension::mouseOverInfo,
            this, &MainWidget::slotMouseOverInfo);
    connect(m_part, &Part::signalSettingsChanged,
            m_articleViewer, &ArticleViewer::slotPaletteOrFontChanged);
    connect(m_searchBar, &SearchBar::signalSearch,
            m_articleViewer, &ArticleViewer::setFilters);

    m_articleViewer->part()->widget()->setWhatsThis(i18n("Browsing area."));

    mainTabLayout->addWidget(m_articleSplitter);

    m_mainFrame = new MainFrame(this, m_part, m_mainTab);
    m_mainFrame->slotSetTitle(i18n("Articles"));
    Kernel::self()->frameManager()->slotAddFrame(m_mainFrame);

    const QList<int> sp1sizes = Settings::splitter1Sizes();
    if (sp1sizes.count() >= m_horizontalSplitter->count()) {
        m_horizontalSplitter->setSizes(sp1sizes);
    }
    const QList<int> sp2sizes = Settings::splitter2Sizes();
    if (sp2sizes.count() >= m_articleSplitter->count()) {
        m_articleSplitter->setSizes(sp2sizes);
    }

    KConfigGroup conf(Settings::self()->config(), "General");
    if (!conf.readEntry("Disable Introduction", false)) {
        m_articleListView->hide();
        m_searchBar->hide();
        m_articleViewer->displayAboutPage();
        m_mainFrame->slotSetTitle(i18n("About"));
        m_displayingAboutPage = true;
    }

    m_fetchTimer = new QTimer(this);
    connect(m_fetchTimer, &QTimer::timeout,
            this, &MainWidget::slotDoIntervalFetches);
    m_fetchTimer->start(1000 * 60);

    // delete expired articles once per hour
    m_expiryTimer = new QTimer(this);
    connect(m_expiryTimer, &QTimer::timeout,
            this, &MainWidget::slotDeleteExpiredArticles);
    m_expiryTimer->start(3600 * 1000);

    m_markReadTimer = new QTimer(this);
    m_markReadTimer->setSingleShot(true);
    connect(m_markReadTimer, &QTimer::timeout, this, &MainWidget::slotSetCurrentArticleReadDelayed);

    setFeedList(QSharedPointer<FeedList>(new FeedList(Kernel::self()->storage())));

    switch (Settings::viewMode()) {
    case CombinedView:
        slotCombinedView();
        break;
    case WidescreenView:
        slotWidescreenView();
        break;
    default:
        slotNormalView();
    }

    if (!Settings::resetQuickFilterOnNodeChange()) {
        m_searchBar->slotSetStatus(Settings::statusFilter());
        m_searchBar->slotSetText(Settings::textFilter());
    }
}

void Akregator::MainWidget::slotOnShutdown()
{
    m_shuttingDown = true;

    // close all pageviewers in a controlled way
    // fixes bug 91660, at least when no part loading data
    while (m_tabWidget->count() > 1) {   // remove frames until only the main frame remains
        m_tabWidget->setCurrentIndex(m_tabWidget->count() - 1);   // select last page
        m_tabWidget->slotRemoveCurrentFrame();
    }

    Kernel::self()->fetchQueue()->slotAbort();
    setFeedList(QSharedPointer<FeedList>());

    delete m_feedListManagementInterface;
    delete m_feedListView; // call delete here, so that the header settings will get saved
    delete m_articleListView; // same for this one

    delete m_mainTab;
    delete m_mainFrame;
    m_mainFrame = 0;

    Settings::self()->save();
}

void Akregator::MainWidget::saveSettings()
{
    const QList<int> spl1 = m_horizontalSplitter->sizes();
    if (std::count(spl1.begin(), spl1.end(), 0) == 0) {
        Settings::setSplitter1Sizes(spl1);
    }
    const QList<int> spl2 = m_articleSplitter->sizes();
    if (std::count(spl2.begin(), spl2.end(), 0) == 0) {
        Settings::setSplitter2Sizes(spl2);
    }
    Settings::setViewMode(m_viewMode);
    Settings::self()->save();
}

void Akregator::MainWidget::slotRequestNewFrame(int &frameId)
{
    BrowserFrame *frame = new BrowserFrame(m_tabWidget);

    connect(m_part, &Part::signalSettingsChanged, frame, &BrowserFrame::slotPaletteOrFontChanged);
    connect(m_tabWidget, &TabWidget::signalZoomInFrame, frame, &BrowserFrame::slotZoomIn);
    connect(m_tabWidget, &TabWidget::signalZoomOutFrame, frame, &BrowserFrame::slotZoomOut);

    Kernel::self()->frameManager()->slotAddFrame(frame);

    frameId = frame->id();
}

void Akregator::MainWidget::sendArticle(bool attach)
{
    QByteArray text;
    QString title;

    Frame *frame = Kernel::self()->frameManager()->currentFrame();

    if (frame && frame->id() > 0) { // are we in some other tab than the articlelist?
        text = frame->url().toString().toLatin1();
        title = frame->title();
    } else { // nah, we're in articlelist..
        const Article article =  m_selectionController->currentArticle();
        if (!article.isNull()) {
            text = article.link().toDisplayString().toLatin1();
            title = article.title();
        }
    }

    if (text.isEmpty()) {
        return;
    }

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("subject"), title);
    query.addQueryItem(QStringLiteral("body"), QString::fromUtf8(text));
    if (attach) {
        query.addQueryItem(QStringLiteral("attach"), QString::fromUtf8(text));
    }
    QUrl url;
    url.setScheme(QStringLiteral("mailto"));
    url.setQuery(query);
    QDesktopServices::openUrl(url);
}

void MainWidget::importFeedList(const QDomDocument &doc)
{
    ImportFeedListCommand *cmd = new ImportFeedListCommand;
    cmd->setParentWidget(this);
    cmd->setFeedListDocument(doc);
    cmd->setTargetList(m_feedList);
    cmd->start();
}

void Akregator::MainWidget::setFeedList(const QSharedPointer<FeedList> &list)
{
    if (list == m_feedList) {
        return;
    }
    const QSharedPointer<FeedList> oldList = m_feedList;

    m_feedList = list;
    if (m_feedList) {
        connect(m_feedList.data(), &FeedList::unreadCountChanged,
                this, &MainWidget::slotSetTotalUnread);
    }

    slotSetTotalUnread();

    m_feedListManagementInterface->setFeedList(m_feedList);
    Kernel::self()->setFeedList(m_feedList);
    ProgressManager::self()->setFeedList(m_feedList);
    m_selectionController->setFeedList(m_feedList);

    if (oldList) {
        oldList->disconnect(this);
    }

    slotDeleteExpiredArticles();
}

void Akregator::MainWidget::deleteExpiredArticles(const QSharedPointer<FeedList> &list)
{
    if (!list) {
        return;
    }
    ExpireItemsCommand *cmd = new ExpireItemsCommand(this);
    cmd->setParentWidget(this);
    cmd->setFeedList(list);
    cmd->setFeeds(list->feedIds());
    cmd->start();
}

void Akregator::MainWidget::slotDeleteExpiredArticles()
{
    deleteExpiredArticles(m_feedList);
}

QDomDocument Akregator::MainWidget::feedListToOPML()
{
    QDomDocument dom;
    if (m_feedList) {
        dom = m_feedList->toOpml();
    }
    return dom;
}

void Akregator::MainWidget::addFeedToGroup(const QString &url, const QString &groupName)
{
    // Locate the group.
    QList<TreeNode *> namedGroups = m_feedList->findByTitle(groupName);
    Folder *group = 0;
    foreach (TreeNode *const candidate, namedGroups) {
        if (candidate->isGroup()) {
            group =  static_cast<Folder *>(candidate);
            break;
        }
    }

    if (!group) {
        Folder *g = new Folder(groupName);
        m_feedList->allFeedsFolder()->appendChild(g);
        group = g;
    }

    // Invoke the Add Feed dialog with url filled in.
    addFeed(url, 0, group, true);
}

void Akregator::MainWidget::slotNormalView()
{
    if (m_viewMode == NormalView) {
        return;
    }

    if (m_viewMode == CombinedView) {
        m_articleListView->show();

        const Article article =  m_selectionController->currentArticle();

        if (!article.isNull()) {
            m_articleViewer->showArticle(article);
        } else {
            m_articleViewer->slotShowSummary(m_selectionController->selectedSubscription());
        }
    }

    m_articleSplitter->setOrientation(Qt::Vertical);
    m_viewMode = NormalView;

    Settings::setViewMode(m_viewMode);
}

void Akregator::MainWidget::slotWidescreenView()
{
    if (m_viewMode == WidescreenView) {
        return;
    }

    if (m_viewMode == CombinedView) {
        m_articleListView->show();

        Article article =  m_selectionController->currentArticle();

        if (!article.isNull()) {
            m_articleViewer->showArticle(article);
        } else {
            m_articleViewer->slotShowSummary(m_selectionController->selectedSubscription());
        }
    }

    m_articleSplitter->setOrientation(Qt::Horizontal);
    m_viewMode = WidescreenView;

    Settings::setViewMode(m_viewMode);
}

void Akregator::MainWidget::slotCombinedView()
{
    if (m_viewMode == CombinedView) {
        return;
    }

    m_articleListView->slotClear();
    m_articleListView->hide();
    m_viewMode = CombinedView;

    Settings::setViewMode(m_viewMode);
}

void Akregator::MainWidget::slotMoveCurrentNodeUp()
{
    TreeNode *current = m_selectionController->selectedSubscription();
    if (!current) {
        return;
    }
    TreeNode *prev = current->prevSibling();
    Folder *parent = current->parent();

    if (!prev || !parent) {
        return;
    }

    parent->removeChild(prev);
    parent->insertChild(prev, current);
    m_feedListView->ensureNodeVisible(current);
}

void Akregator::MainWidget::slotMoveCurrentNodeDown()
{
    TreeNode *current = m_selectionController->selectedSubscription();
    if (!current) {
        return;
    }
    TreeNode *next = current->nextSibling();
    Folder *parent = current->parent();

    if (!next || !parent) {
        return;
    }

    parent->removeChild(current);
    parent->insertChild(current, next);
    m_feedListView->ensureNodeVisible(current);
}

void Akregator::MainWidget::slotMoveCurrentNodeLeft()
{
    TreeNode *current = m_selectionController->selectedSubscription();
    if (!current || !current->parent() || !current->parent()->parent()) {
        return;
    }

    Folder *parent = current->parent();
    Folder *grandparent = current->parent()->parent();

    parent->removeChild(current);
    grandparent->insertChild(current, parent);
    m_feedListView->ensureNodeVisible(current);
}

void Akregator::MainWidget::slotMoveCurrentNodeRight()
{
    TreeNode *current = m_selectionController->selectedSubscription();
    if (!current || !current->parent()) {
        return;
    }
    TreeNode *prev = current->prevSibling();

    if (prev && prev->isGroup()) {
        Folder *fg = static_cast<Folder *>(prev);
        current->parent()->removeChild(current);
        fg->appendChild(current);
        m_feedListView->ensureNodeVisible(current);
    }
}

void Akregator::MainWidget::slotNodeSelected(TreeNode *node)
{
    m_markReadTimer->stop();

    if (m_displayingAboutPage) {
        m_mainFrame->slotSetTitle(i18n("Articles"));
        if (m_viewMode != CombinedView) {
            m_articleListView->show();
        }
        if (Settings::showQuickFilter()) {
            m_searchBar->show();
        }
        m_displayingAboutPage = false;
    }

    m_tabWidget->setCurrentWidget(m_mainFrame);
    if (Settings::resetQuickFilterOnNodeChange()) {
        m_searchBar->slotClearSearch();
    }

    if (m_viewMode == CombinedView) {
        m_articleViewer->showNode(node);
    } else {
        m_articleViewer->slotShowSummary(node);
    }

    if (node) {
        m_mainFrame->setWindowTitle(node->title());
    }

    m_actionManager->slotNodeSelected(node);
}

void Akregator::MainWidget::slotFeedAdd()
{
    Folder *group = 0;
    if (!m_selectionController->selectedSubscription()) {
        group = m_feedList->allFeedsFolder();
    } else {
        if (m_selectionController->selectedSubscription()->isGroup()) {
            group = static_cast<Folder *>(m_selectionController->selectedSubscription());
        } else {
            group = m_selectionController->selectedSubscription()->parent();
        }

    }

    TreeNode *const lastChild = !group->children().isEmpty() ? group->children().last() : 0;

    addFeed(QString(), lastChild, group, false);
}

void Akregator::MainWidget::addFeed(const QString &url, TreeNode *after, Folder *parent, bool autoExec)
{
    CreateFeedCommand *cmd(new CreateFeedCommand(this));
    cmd->setParentWidget(this);
    cmd->setPosition(parent, after);
    cmd->setRootFolder(m_feedList->allFeedsFolder());
    cmd->setAutoExecute(autoExec);
    cmd->setUrl(url);
    cmd->setSubscriptionListView(m_feedListView);
    cmd->start();
}

void Akregator::MainWidget::slotFeedAddGroup()
{
    CreateFolderCommand *cmd = new CreateFolderCommand(this);
    cmd->setParentWidget(this);
    cmd->setSelectedSubscription(m_selectionController->selectedSubscription());
    cmd->setRootFolder(m_feedList->allFeedsFolder());
    cmd->setSubscriptionListView(m_feedListView);
    cmd->start();
}

void Akregator::MainWidget::slotFeedRemove()
{
    TreeNode *selectedNode = m_selectionController->selectedSubscription();

    // don't delete root element! (safety valve)
    if (!selectedNode || selectedNode == m_feedList->allFeedsFolder()) {
        return;
    }

    DeleteSubscriptionCommand *cmd = new DeleteSubscriptionCommand(this);
    cmd->setParentWidget(this);
    cmd->setSubscription(m_feedList, selectedNode->id());
    cmd->start();
}

void Akregator::MainWidget::slotFeedModify()
{
    TreeNode *const node = m_selectionController->selectedSubscription();
    if (!node) {
        return;
    }
    EditSubscriptionCommand *cmd = new EditSubscriptionCommand(this);
    cmd->setParentWidget(this);
    cmd->setSubscription(m_feedList, node->id());
    cmd->setSubscriptionListView(m_feedListView);
    cmd->start();
}

void Akregator::MainWidget::slotNextUnreadArticle()
{
    ensureArticleTabVisible();
    if (m_viewMode == CombinedView) {
        m_feedListView->slotNextUnreadFeed();
        return;
    }
    TreeNode *sel = m_selectionController->selectedSubscription();
    if (sel && sel->unread() > 0) {
        m_articleListView->slotNextUnreadArticle();
    } else {
        m_feedListView->slotNextUnreadFeed();
    }
}

void Akregator::MainWidget::slotPrevUnreadArticle()
{
    ensureArticleTabVisible();
    if (m_viewMode == CombinedView) {
        m_feedListView->slotPrevUnreadFeed();
        return;
    }
    TreeNode *sel = m_selectionController->selectedSubscription();
    if (sel && sel->unread() > 0) {
        m_articleListView->slotPreviousUnreadArticle();
    } else {
        m_feedListView->slotPrevUnreadFeed();
    }
}

void Akregator::MainWidget::slotMarkAllFeedsRead()
{
    KJob *job = m_feedList->createMarkAsReadJob();
    connect(job, &KJob::finished, m_selectionController, &AbstractSelectionController::forceFilterUpdate);
    job->start();
}

void Akregator::MainWidget::slotMarkAllRead()
{
    if (!m_selectionController->selectedSubscription()) {
        return;
    }
    KJob *job = m_selectionController->selectedSubscription()->createMarkAsReadJob();
    connect(job, &KJob::finished, m_selectionController, &AbstractSelectionController::forceFilterUpdate);
    job->start();
}

void Akregator::MainWidget::slotSetTotalUnread()
{
    Q_EMIT signalUnreadCountChanged(m_feedList ? m_feedList->unread() : 0);
}

void Akregator::MainWidget::slotDoIntervalFetches()
{
    if (!m_feedList) {
        return;
    }
#if 0
    // the following solid check apparently doesn't work reliably and causes
    // interval fetching not working although the user is actually online (but solid reports he's not
    const Networking::Status status = Solid::Networking::status();
    if (status != Networking::Connected && status != Networking::Unknown) {
        return;
    }
#endif
    m_feedList->addToFetchQueue(Kernel::self()->fetchQueue(), true);
}

void Akregator::MainWidget::slotFetchCurrentFeed()
{
    if (!m_selectionController->selectedSubscription()) {
        return;
    }
    if (isNetworkAvailable()) {
        m_selectionController->selectedSubscription()->slotAddToFetchQueue(Kernel::self()->fetchQueue());
    } else {
        m_mainFrame->slotSetStatusText(i18n("Networking is not available."));
    }
}

void Akregator::MainWidget::slotFetchAllFeeds()
{
    if (m_feedList && isNetworkAvailable()) {
        m_feedList->addToFetchQueue(Kernel::self()->fetchQueue());
    } else if (m_feedList) {
        m_mainFrame->slotSetStatusText(i18n("Networking is not available."));
    }
}

void Akregator::MainWidget::slotFetchingStarted()
{
    m_mainFrame->slotSetState(Frame::Started);
    m_actionManager->action("feed_stop")->setEnabled(true);
    m_mainFrame->slotSetStatusText(i18n("Fetching Feeds..."));
}

void Akregator::MainWidget::slotFetchingStopped()
{
    m_mainFrame->slotSetState(Frame::Completed);
    m_actionManager->action("feed_stop")->setEnabled(false);
    m_mainFrame->slotSetStatusText(QString());
}

void Akregator::MainWidget::slotArticleSelected(const Akregator::Article &article)
{
    if (m_viewMode == CombinedView) {
        return;
    }

    m_markReadTimer->stop();

    Q_ASSERT(article.isNull() || article.feed());

    QVector<Article> articles = m_selectionController->selectedArticles();
    Q_EMIT signalArticlesSelected(articles);

    KToggleAction *const maai = qobject_cast<KToggleAction *>(m_actionManager->action("article_set_status_important"));
    Q_ASSERT(maai);
    maai->setChecked(article.keep());

    m_articleViewer->showArticle(article);
    if (m_selectionController->selectedArticles().count() == 0) {
        m_articleListView->setCurrentIndex(m_selectionController->currentArticleIndex());
    }

    if (article.isNull() || article.status() == Akregator::Read) {
        return;
    }

    if (!Settings::useMarkReadDelay()) {
        return;
    }

    const int delay = Settings::markReadDelay();

    if (delay > 0) {
        m_markReadTimer->start(delay * 1000);
    } else {
        Akregator::ArticleModifyJob *job = new Akregator::ArticleModifyJob;
        const Akregator::ArticleId aid = { article.feed()->xmlUrl(), article.guid() };
        job->setStatus(aid, Akregator::Read);
        job->start();
    }
}

void Akregator::MainWidget::slotMouseButtonPressed(int button, const QUrl &url)
{
    if (button != Qt::MidButton) {
        return;
    }

    if (!url.isValid()) {
        return;
    }

    OpenUrlRequest req(url);

    switch (Settings::mMBBehaviour()) {
    case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
        req.setOptions(OpenUrlRequest::ExternalBrowser);
        break;
    case Settings::EnumMMBBehaviour::OpenInBackground:
        req.setOptions(OpenUrlRequest::NewTab);
        req.setOpenInBackground(true);
        break;
    default:
        req.setOptions(OpenUrlRequest::NewTab);
        req.setOpenInBackground(false);
    }

    Kernel::self()->frameManager()->slotOpenUrlRequest(req);
}

void Akregator::MainWidget::slotOpenHomepage()
{
    Feed *feed = dynamic_cast<Feed *>(m_selectionController->selectedSubscription());

    if (!feed) {
        return;
    }

    QUrl url(feed->htmlUrl());

    if (url.isValid()) {
        OpenUrlRequest req(feed->htmlUrl());
        req.setOptions(OpenUrlRequest::ExternalBrowser);
        Kernel::self()->frameManager()->slotOpenUrlRequest(req);
    }
}

void Akregator::MainWidget::slotOpenSelectedArticlesInBrowser()
{
    const QVector<Article> articles = m_selectionController->selectedArticles();

    Q_FOREACH (const Akregator::Article &article, articles) {
        slotOpenArticleInBrowser(article);
    }
}

void Akregator::MainWidget::slotOpenArticleInBrowser(const Akregator::Article &article)
{
    if (!article.isNull() && article.link().isValid()) {
        OpenUrlRequest req(article.link());
        req.setOptions(OpenUrlRequest::ExternalBrowser);
        Kernel::self()->frameManager()->slotOpenUrlRequest(req);
    }
}

void Akregator::MainWidget::openSelectedArticles(bool openInBackground)
{
    const QVector<Article> articles = m_selectionController->selectedArticles();

    Q_FOREACH (const Akregator::Article &article, articles) {
        const QUrl url = article.link();
        if (!url.isValid()) {
            continue;
        }

        OpenUrlRequest req(url);
        req.setOptions(OpenUrlRequest::NewTab);
        if (openInBackground) {
            req.setOpenInBackground(true);
            Kernel::self()->frameManager()->slotOpenUrlRequest(req, false /*don't use settings for open in background*/);
        } else {
            Kernel::self()->frameManager()->slotOpenUrlRequest(req);
        }
    }

}

void Akregator::MainWidget::currentArticleInfo(QString &link, QString &title)
{
    const Article article =  m_selectionController->currentArticle();

    if (article.isNull()) {
        return;
    }
    if (article.link().isValid()) {
        link = article.link().url();
        title = article.title();
    }
}

void Akregator::MainWidget::slotCopyLinkAddress()
{
    const Article article =  m_selectionController->currentArticle();

    if (article.isNull()) {
        return;
    }

    QString link;
    if (article.link().isValid()) {
        link = article.link().url();
        QClipboard *cb = QApplication::clipboard();
        cb->setText(link, QClipboard::Clipboard);
        // don't set url to selection as it's a no-no according to a fd.o spec
        //cb->setText(link, QClipboard::Selection);
    }
}

void Akregator::MainWidget::slotFeedUrlDropped(QList<QUrl> &urls, TreeNode *after, Folder *parent)
{
    Q_FOREACH (const QUrl &i, urls) {
        addFeed(i.toDisplayString(), after, parent, false);
    }
}

void Akregator::MainWidget::slotToggleShowQuickFilter()
{
    if (Settings::showQuickFilter()) {
        Settings::setShowQuickFilter(false);
        m_searchBar->slotClearSearch();
        m_searchBar->hide();
    } else {
        Settings::setShowQuickFilter(true);
        if (!m_displayingAboutPage) {
            m_searchBar->show();
        }
    }

}

void Akregator::MainWidget::slotArticleDelete()
{

    if (m_viewMode == CombinedView) {
        return;
    }

    const QVector<Article> articles = m_selectionController->selectedArticles();

    QString msg;
    switch (articles.count()) {
    case 0:
        return;
    case 1:
        msg = i18n("<qt>Are you sure you want to delete article <b>%1</b>?</qt>", articles.first().title());
        break;
    default:
        msg = i18np("<qt>Are you sure you want to delete the selected article?</qt>", "<qt>Are you sure you want to delete the %1 selected articles?</qt>", articles.count());
    }

    if (KMessageBox::warningContinueCancel(this,
                                           msg, i18n("Delete Article"),
                                           KStandardGuiItem::del(),
                                           KStandardGuiItem::cancel(),
                                           QStringLiteral("Disable delete article confirmation")) != KMessageBox::Continue) {
        return;
    }

    TreeNode *const selected = m_selectionController->selectedSubscription();

    if (selected) {
        selected->setNotificationMode(false);
    }

    Akregator::ArticleDeleteJob *job = new Akregator::ArticleDeleteJob;
    Q_FOREACH (const Akregator::Article &i, articles) {
        Feed *const feed = i.feed();
        if (!feed) {
            continue;
        }
        const Akregator::ArticleId aid = { feed->xmlUrl(), i.guid() };
        job->appendArticleId(aid);
    }

    job->start();

    if (selected) {
        selected->setNotificationMode(true);
    }
}

void Akregator::MainWidget::slotFramesChanged()
{
    // We need to wait till the frame is fully loaded
    QMetaObject::invokeMethod(m_part, "slotAutoSave", Qt::QueuedConnection);
}

void Akregator::MainWidget::slotArticleToggleKeepFlag(bool)
{
    const QVector<Article> articles = m_selectionController->selectedArticles();

    if (articles.isEmpty()) {
        return;
    }

    bool allFlagsSet = true;
    Q_FOREACH (const Akregator::Article &i, articles) {
        allFlagsSet = allFlagsSet && i.keep();
        if (!allFlagsSet) {
            break;
        }
    }

    Akregator::ArticleModifyJob *job = new Akregator::ArticleModifyJob;
    Q_FOREACH (const Akregator::Article &i, articles) {
        const Akregator::ArticleId aid = { i.feed()->xmlUrl(), i.guid() };
        job->setKeep(aid, !allFlagsSet);
    }
    job->start();
}

namespace
{

void setSelectedArticleStatus(const Akregator::AbstractSelectionController *controller, int status)
{
    const QVector<Akregator::Article> articles = controller->selectedArticles();

    if (articles.isEmpty()) {
        return;
    }

    Akregator::ArticleModifyJob *job = new Akregator::ArticleModifyJob;
    Q_FOREACH (const Akregator::Article &i, articles) {
        const Akregator::ArticleId aid = { i.feed()->xmlUrl(), i.guid() };
        job->setStatus(aid, status);
    }
    job->start();
}

}

void Akregator::MainWidget::slotSetSelectedArticleRead()
{
    ::setSelectedArticleStatus(m_selectionController, Akregator::Read);
}

QVector<Akregator::Article> Akregator::MainWidget::speakSelectedArticles()
{
    QVector<Akregator::Article> lst;
    if (Kernel::self()->frameManager()->currentFrame() == m_mainFrame) {
        if (m_viewMode != CombinedView) {
            lst = m_selectionController->selectedArticles();
        } else {
            if (m_selectionController->selectedSubscription()) {
                //TODO: read articles in current node, respecting quick filter!
            }
        }
    } else {
        // TODO: read selected page viewer
    }
    return lst;
}

void Akregator::MainWidget::slotSetSelectedArticleUnread()
{
    ::setSelectedArticleStatus(m_selectionController, Akregator::Unread);
}

void Akregator::MainWidget::slotSetSelectedArticleNew()
{
    ::setSelectedArticleStatus(m_selectionController, Akregator::New);
}

void Akregator::MainWidget::slotSetCurrentArticleReadDelayed()
{
    const Article article =  m_selectionController->currentArticle();

    if (article.isNull()) {
        return;
    }

    Akregator::ArticleModifyJob *const job = new Akregator::ArticleModifyJob;
    const Akregator::ArticleId aid = { article.feed()->xmlUrl(), article.guid() };
    job->setStatus(aid, Akregator::Read);
    job->start();
}

void Akregator::MainWidget::slotMouseOverInfo(const KFileItem &kifi)
{
    m_mainFrame->slotSetStatusText(kifi.isNull() ? QString() : kifi.url().toDisplayString());
}

void Akregator::MainWidget::readProperties(const KConfigGroup &config)
{
    if (!Settings::resetQuickFilterOnNodeChange()) {
        // read filter settings
        m_searchBar->slotSetText(config.readEntry("searchLine"));
        m_searchBar->slotSetStatus(config.readEntry("searchCombo").toInt());
    }
    // Reopen tabs
    QStringList childList = config.readEntry(QStringLiteral("Children"),
                            QStringList());
    Q_FOREACH (const QString &framePrefix, childList) {
        BrowserFrame *const frame = new BrowserFrame(m_tabWidget);
        frame->loadConfig(config, framePrefix + QLatin1Char('_'));

        connect(m_part, &Part::signalSettingsChanged, frame, &BrowserFrame::slotPaletteOrFontChanged);
        connect(m_tabWidget, &TabWidget::signalZoomInFrame, frame, &BrowserFrame::slotZoomIn);
        connect(m_tabWidget, &TabWidget::signalZoomOutFrame, frame, &BrowserFrame::slotZoomOut);

        Kernel::self()->frameManager()->slotAddFrame(frame);

    }
}

void Akregator::MainWidget::saveProperties(KConfigGroup &config)
{
    // save filter settings
    config.writeEntry("searchLine", m_searchBar->text());
    config.writeEntry("searchCombo", m_searchBar->status());

    Kernel::self()->frameManager()->saveProperties(config);
}

void Akregator::MainWidget::ensureArticleTabVisible()
{
    m_tabWidget->setCurrentWidget(m_mainFrame);
}

void MainWidget::slotReloadAllTabs()
{
    this->m_tabWidget->slotReloadAllTabs();
}

bool MainWidget::isNetworkAvailable()
{
    return m_networkConfigManager->isOnline();
}

void MainWidget::slotNetworkStatusChanged(bool status)
{
    if (status) {
        m_mainFrame->slotSetStatusText(i18n("Networking is available now."));
        this->slotFetchAllFeeds();
    } else {
        m_mainFrame->slotSetStatusText(i18n("Networking is not available."));
    }
}

void Akregator::MainWidget::slotOpenSelectedArticles()
{
    openSelectedArticles(false);
}

void Akregator::MainWidget::slotOpenSelectedArticlesInBackground()
{
    openSelectedArticles(true);
}

