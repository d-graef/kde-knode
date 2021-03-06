/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>

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

#ifndef AKREGATOR_ARTICLEVIEWER_H
#define AKREGATOR_ARTICLEVIEWER_H

#include "article.h"
#include "akregatorpart_export.h"

#include <khtml_part.h>

#include <QPointer>

#include <QSharedPointer>
#include <vector>
#include <QUrl>

class KJob;

namespace Akregator
{

namespace Filters
{
class AbstractMatcher;
}

class ArticleFormatter;
class ArticleListJob;
class OpenUrlRequest;
class TreeNode;

class ArticleViewerPart;

class AKREGATORPART_EXPORT ArticleViewer : public QWidget
{
    Q_OBJECT
public:
    explicit ArticleViewer(QWidget *parent);
    ~ArticleViewer();

    /** Repaints the view. */
    void reload();

    void displayAboutPage();

    KParts::ReadOnlyPart *part() const;

    void setNormalViewFormatter(const QSharedPointer<ArticleFormatter> &formatter);

    void setCombinedViewFormatter(const QSharedPointer<ArticleFormatter> &formatter);

    void showArticle(const Article &article);

    /** Shows the articles of the tree node @c node (combined view).
     * Changes in the node will update the view automatically.
     *
     *  @param node The node to observe */
    void showNode(Akregator::TreeNode *node);

    QSize sizeHint() const Q_DECL_OVERRIDE;

public Q_SLOTS:

    void slotZoomIn(int);
    void slotZoomOut(int);
    void slotSetZoomFactor(int percent);
    void slotPrint();

    /** Set filters which will be used if the viewer is in combined view mode
     */
    void setFilters(const std::vector< QSharedPointer<const Akregator::Filters::AbstractMatcher> > &filters);

    /** Update view if combined view mode is set. Has to be called when
     * the displayed node gets modified.
     */
    void slotUpdateCombinedView();

    /**
     * Clears the canvas and disconnects from the currently observed node
     * (if in combined view mode).
     */
    void slotClear();

    void slotShowSummary(Akregator::TreeNode *node);

    void slotPaletteOrFontChanged();

Q_SIGNALS:

    /** This gets emitted when url gets clicked */
    void signalOpenUrlRequest(Akregator::OpenUrlRequest &);

    void started(KIO::Job *);
    void selectionChanged();
    void completed();

protected: // methods
    int pointsToPixel(int points) const;

    bool openUrl(const QUrl &url);

protected Q_SLOTS:

    void slotOpenUrlRequestDelayed(const QUrl &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &);

    void slotCreateNewWindow(const QUrl &url,
                             const KParts::OpenUrlArguments &args,
                             const KParts::BrowserArguments &browserArgs,
                             const KParts::WindowArgs &windowArgs,
                             KParts::ReadOnlyPart **part);

    void slotPopupMenu(const QPoint &, const QUrl &, mode_t, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &, KParts::BrowserExtension::PopupFlags);

    /** Copies current link to clipboard. */
    void slotCopyLinkAddress();

    /** Copies currently selected text to clipboard */
    void slotCopy();

    /** Opens @c m_url inside this viewer */
    void slotOpenLinkInternal();

    /** Opens @c m_url in external viewer, eg. Konqueror */
    void slotOpenLinkInBrowser();

    /** Opens @c m_url in foreground tab */
    void slotOpenLinkInForegroundTab();

    /** Opens @c m_url in background tab */
    void slotOpenLinkInBackgroundTab();

    void slotSaveLinkAs();

    /** This changes cursor to wait cursor */
    void slotStarted(KIO::Job *);

    /** This reverts cursor back to normal one */
    void slotCompleted();

    void slotSelectionChanged();

    void slotArticlesListed(KJob *job);

    void slotArticlesUpdated(Akregator::TreeNode *node, const QVector<Akregator::Article> &list);
    void slotArticlesAdded(Akregator::TreeNode *node, const QVector<Akregator::Article> &list);
    void slotArticlesRemoved(Akregator::TreeNode *node, const QVector<Akregator::Article> &list);

    // from ArticleViewer
private:

    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

    /** renders @c body. Use this method whereever possible.
     *  @param body html to render, without header and footer */
    void renderContent(const QString &body);

    /** Resets the canvas and adds writes the HTML header to it.
        */
    void beginWriting();

    /** Finishes writing to the canvas and completes the HTML (by adding closing tags) */
    void endWriting();

    void updateCss();

    void connectToNode(TreeNode *node);
    void disconnectFromNode(TreeNode *node);

    void setArticleActionsEnabled(bool enabled);

private:
    QUrl m_url;
    QString m_normalModeCSS;
    QString m_combinedModeCSS;
    QString m_htmlFooter;
    QString m_currentText;
    QUrl m_imageDir;
    QPointer<TreeNode> m_node;
    QPointer<ArticleListJob> m_listJob;
    Article m_article;
    QVector<Article> m_articles;
    QUrl m_link;
    std::vector<QSharedPointer<const Filters::AbstractMatcher> > m_filters;
    enum ViewMode { NormalView, CombinedView, SummaryView };
    ViewMode m_viewMode;
    ArticleViewerPart *m_part;
    QSharedPointer<ArticleFormatter> m_normalViewFormatter;
    QSharedPointer<ArticleFormatter> m_combinedViewFormatter;
};

class ArticleViewerPart : public KHTMLPart
{
    Q_OBJECT

public:
    explicit ArticleViewerPart(QWidget *parent);

    bool closeUrl() Q_DECL_OVERRIDE;

    int button() const;

protected:

    /** reimplemented to get the mouse button */
    bool urlSelected(const QString &url, int button, int state, const QString &_target,
                     const KParts::OpenUrlArguments &args = KParts::OpenUrlArguments(),
                     const KParts::BrowserArguments &browserArgs = KParts::BrowserArguments()) Q_DECL_OVERRIDE;

private:

    int m_button;
};

} // namespace Akregator

#endif // AKREGATOR_ARTICLEVIEWER_H

