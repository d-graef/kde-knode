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

#ifndef OPENATTACHMENTFOLDERWIDGET_H
#define OPENATTACHMENTFOLDERWIDGET_H

#include <KMessageWidget>
#include <QUrl>
class QTimer;
namespace MessageViewer
{
class OpenAttachmentFolderWidget : public KMessageWidget
{
    Q_OBJECT
public:
    explicit OpenAttachmentFolderWidget(QWidget *parent = Q_NULLPTR);
    ~OpenAttachmentFolderWidget();

    void setFolder(const QUrl &url);

public Q_SLOTS:
    void slotShowWarning();
    void slotHideWarning();

private Q_SLOTS:
    void slotOpenAttachmentFolder();
    void slotTimeOut();
    void slotExplicitlyClosed();
private:
    QUrl mUrl;
    QTimer *mTimer;
};
}

#endif // OPENATTACHMENTFOLDERWIDGET_H
