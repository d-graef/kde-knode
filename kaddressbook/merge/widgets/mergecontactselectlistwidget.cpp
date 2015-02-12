/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "mergecontactselectlistwidget.h"
#include <KABC/Addressee>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
using namespace KABMergeContacts;
using namespace KABC;

MergeContactSelectListWidget::MergeContactSelectListWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    mTitle = new QLabel;
    mTitle->setObjectName(QLatin1String("title"));
    vbox->addWidget(mTitle);
    mSelectListWidget = new QListWidget;
    mSelectListWidget->setObjectName(QLatin1String("listwidget"));
    vbox->addWidget(mSelectListWidget);
}

MergeContactSelectListWidget::~MergeContactSelectListWidget()
{

}

void MergeContactSelectListWidget::setContacts(MergeContacts::ConflictInformation conflictType, const Akonadi::Item::List &lst)
{
    mConflictType = conflictType;
    if (lst.isEmpty() || ( conflictType == MergeContacts::None) ) {
        return;
    }
    updateTitle();
    fillList(lst);
}

void MergeContactSelectListWidget::updateTitle()
{
    QString title;
    //TODO
    switch(mConflictType) {
    case MergeContacts::None:
        break;
    case MergeContacts::Birthday:
        break;
    case MergeContacts::Geo:
        break;
    case MergeContacts::Photo:
        break;
    case MergeContacts::Logo:
        break;
    }
    mTitle->setText(title);
}

void MergeContactSelectListWidget::fillList(const Akonadi::Item::List &lst)
{
    switch(mConflictType) {
    case MergeContacts::None:
        break;
    case MergeContacts::Birthday:
        break;
    case MergeContacts::Geo:
        break;
    case MergeContacts::Photo:
        break;
    case MergeContacts::Logo:
        break;
    }
}

void MergeContactSelectListWidget::createContact(KABC::Addressee &addr)
{
    //TODO
    switch(mConflictType) {
    case MergeContacts::None:
        break;
    case MergeContacts::Birthday:
        break;
    case MergeContacts::Geo:
        break;
    case MergeContacts::Photo:
        break;
    case MergeContacts::Logo:
        break;
    }
}