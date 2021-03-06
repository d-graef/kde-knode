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

#ifndef VACATIONPAGEWIDGET_H
#define VACATIONPAGEWIDGET_H

#include <QWidget>
#include <QUrl>
class QStackedWidget;
namespace KManageSieve
{
class SieveJob;
}

namespace KSieveUi
{
class VacationEditWidget;
class VacationWarningWidget;
class VacationCreateScriptJob;
class MultiImapVacationManager;
class ParseUserScriptJob;
class VacationPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VacationPageWidget(QWidget *parent = Q_NULLPTR);
    ~VacationPageWidget();

    void setServerUrl(const QUrl &url);
    void setServerName(const QString &serverName);
    KSieveUi::VacationCreateScriptJob *writeScript();
    void setDefault();
    void setVacationManager(MultiImapVacationManager *vacationManager);

private Q_SLOTS:
    void slotGetResult(const QString &serverName, const QStringList &sieveCapabilities, const QString &scriptName, const QString &script, bool active);

private:

    void fillWithDefaults();

    enum PageType {
        Script = 0,
        ScriptNotSupported = 1
    };

    QString mServerName;
    QUrl mUrl;
    QStackedWidget *mStackWidget;
    VacationEditWidget *mVacationEditWidget;
    VacationWarningWidget *mVacationWarningWidget;
    MultiImapVacationManager *mVacationManager;
    PageType mPageScript;
    bool mWasActive;
    bool mHasDateSupport;
};
}

#endif // VACATIONPAGEWIDGET_H
