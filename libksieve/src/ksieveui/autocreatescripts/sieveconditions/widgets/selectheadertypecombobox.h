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

#ifndef SELECTHEADERTYPECOMBOBOX_H
#define SELECTHEADERTYPECOMBOBOX_H

#include <KComboBox>
#include <QDialog>

#include <QListWidget>

class KLineEdit;
class QPushButton;
namespace KSieveUi
{

class SelectHeadersWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit SelectHeadersWidget(QWidget *parent = Q_NULLPTR);
    ~SelectHeadersWidget();

    QString headers() const;
    void setListHeaders(const QMap<QString, QString> &lst, const QStringList &selectedHeaders);
    void addNewHeader(const QString &header);
private:
    enum HeaderEnum {
        HeaderId = Qt::UserRole + 1
    };

    void init();
};

class SelectHeadersDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectHeadersDialog(QWidget *parent = Q_NULLPTR);
    ~SelectHeadersDialog();

    QString headers() const;
    void setListHeaders(const QMap<QString, QString> &lst, const QStringList &selectedHeaders);

private Q_SLOTS:
    void slotNewHeaderTextChanged(const QString &text);
    void slotAddNewHeader();

private:
    void readConfig();
    void writeConfig();
    SelectHeadersWidget *mListWidget;
    KLineEdit *mNewHeader;
    QPushButton *mAddNewHeader;
};

class SelectHeaderTypeComboBox : public KComboBox
{
    Q_OBJECT
public:
    explicit SelectHeaderTypeComboBox(bool onlyEnvelopType = false, QWidget *parent = Q_NULLPTR);
    ~SelectHeaderTypeComboBox();

    QString code() const;
    void setCode(const QString &code);

Q_SIGNALS:
    void valueChanged();

private Q_SLOTS:
    void slotSelectItem(const QString &str);

private:
    void initialize(bool onlyEnvelopType);
    void headerMap(bool onlyEnvelopType);
    QMap<QString, QString> mHeaderMap;
    QString mCode;
};

}

#endif // SELECTHEADERTYPECOMBOBOX_H
