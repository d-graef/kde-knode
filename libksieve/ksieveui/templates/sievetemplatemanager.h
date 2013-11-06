/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef SIEVETEMPLATEMANAGER_H
#define SIEVETEMPLATEMANAGER_H

#include <QObject>
#include <QStringList>

class KDirWatch;
namespace KSieveUi {
class SieveTemplateWidget;

struct TemplateInfo {
    QString name;
    QString script;
    bool isValid() const {
        return (!name.isEmpty() && !script.isEmpty());
    }
};

class SieveTemplateManager : public QObject
{
    Q_OBJECT
public:
    explicit SieveTemplateManager(SieveTemplateWidget *sieveTemplateWidget);
    ~SieveTemplateManager();

private Q_SLOTS:
    void slotDirectoryChanged();

private:
    void setTemplatePath();
    void initTemplatesDirectories(const QString &templatesRelativePath);
    TemplateInfo loadTemplate(const QString &themePath, const QString &defaultDesktopFileName);

    QStringList mTemplatesDirectories;

    SieveTemplateWidget *mSieveTemplateWidget;
    KDirWatch *mDirWatch;
};
}

#endif // SIEVETEMPLATEMANAGER_H
