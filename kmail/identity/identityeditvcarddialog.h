/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#ifndef IDENTITYEDITVCARDDIALOG_H
#define IDENTITYEDITVCARDDIALOG_H

#include <QDialog>

namespace Akonadi
{
class ContactEditor;
}

class IdentityEditVcardDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IdentityEditVcardDialog(const QString &fileName, QWidget *parent = Q_NULLPTR);
    ~IdentityEditVcardDialog();
    /**
    * @brief loadVcard load vcard in a contact editor
    * @param vcardFileName
    */
    void loadVcard(const QString &vcardFileName);
    /**
    * @brief saveVcard
    * @return The file path for current vcard.
    */
    QString saveVcard() const;

Q_SIGNALS:
    void vcardRemoved();

private Q_SLOTS:
    void slotDeleteCurrentVCard();

private:
    void deleteCurrentVcard(bool deleteOnDisk);
    QString mVcardFileName;
    Akonadi::ContactEditor *mContactEditor;
};

#endif // IDENTITYEDITVCARDDIALOG_H
