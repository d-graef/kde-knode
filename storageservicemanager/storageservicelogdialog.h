/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef STORAGESERVICELOGDIALOG_H
#define STORAGESERVICELOGDIALOG_H

#include <QDialog>
class QPushButton;
namespace KPIMTextEdit
{
class RichTextEditorWidget;
}
class StorageServiceLogDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StorageServiceLogDialog(QWidget *parent = Q_NULLPTR);
    ~StorageServiceLogDialog();

    void setLog(const QString &log);

Q_SIGNALS:
    void clearLog();

private Q_SLOTS:
    void slotSaveAs();
    void slotClearLog();
    void slotTextChanged();
private:
    void readConfig();
    void writeConfig();
    KPIMTextEdit::RichTextEditorWidget *mLog;
    QPushButton *mUser1Button;
    QPushButton *mUser2Button;
};

#endif // STORAGESERVICELOGDIALOG_H
