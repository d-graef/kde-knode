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

#ifndef PARSEUSERSCRIPTJOB_H
#define PARSEUSERSCRIPTJOB_H

#include <QObject>
#include <QUrl>
#include "ksieveui_export.h"
class QDomDocument;
class QDomElement;
namespace KManageSieve
{
class SieveJob;
}

namespace KSieveUi
{
class KSIEVEUI_EXPORT ParseUserScriptJob : public QObject
{
    Q_OBJECT
public:
    explicit ParseUserScriptJob(QObject *parent = Q_NULLPTR);
    ~ParseUserScriptJob();

    void start();

    void scriptUrl(const QUrl &url);
    static QStringList parsescript(const QString &script, bool &result);

private Q_SLOTS:
    void slotGetResult(KManageSieve::SieveJob *, bool, const QString &, bool);

Q_SIGNALS:
    void success(const QStringList &activeScriptList);
    void error(const QString &msgError);

private:
    static QString loadInclude(const QDomElement &element);
    static QStringList extractActiveScript(const QDomDocument &doc);
    QUrl mCurrentUrl;
    KManageSieve::SieveJob *mSieveJob;
};
}

#endif // PARSEUSERSCRIPTJOB_H