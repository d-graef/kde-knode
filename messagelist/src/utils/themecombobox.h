/* Copyright 2009 James Bendig <james@imptalk.com>

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
#ifndef __MESSAGELIST_UTILS_THEMECOMBOBOX_H__
#define __MESSAGELIST_UTILS_THEMECOMBOBOX_H__

#include <messagelist_export.h>
#include <KComboBox>
#include <collection.h>
namespace MessageList
{

namespace Core
{

class StorageModel;
class Theme;

} // namespace Core

namespace Utils
{

class ThemeComboBoxPrivate;

/**
 * A specialized KComboBox that lists all message list themes.
 */
class MESSAGELIST_EXPORT ThemeComboBox : public KComboBox
{
    Q_OBJECT

public:
    explicit ThemeComboBox(QWidget *parent);
    ~ThemeComboBox();

    QString currentTheme() const;

    void writeDefaultConfig() const;

    void writeStorageModelConfig(MessageList::Core::StorageModel *storageModel, bool isPrivateSetting) const;
    void writeStorageModelConfig(const Akonadi::Collection &col, bool isPrivateSetting) const;
    void writeStorageModelConfig(const QString &id, bool isPrivateSetting)const;

    void readStorageModelConfig(const Akonadi::Collection &col, bool &isPrivateSetting);
    void readStorageModelConfig(MessageList::Core::StorageModel *storageModel, bool &isPrivateSetting);

public Q_SLOTS:
    void selectDefault();

private:
    Q_PRIVATE_SLOT(d, void slotLoadThemes())
    ThemeComboBoxPrivate *const d;
};

} // namespace Utils

} // namespace MessageList

#endif //!__MESSAGELIST_UTILS_THEMECOMBOBOX_H__
