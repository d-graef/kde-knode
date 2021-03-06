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

#include "mergecontactshowresulttabwidget.h"
#include <KContacts/Addressee>
#include "merge/widgets/mergecontactinfowidget.h"

#include <QTabBar>
#include "kaddressbook_debug.h"

using namespace KABMergeContacts;

MergeContactShowResultTabWidget::MergeContactShowResultTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    updateTabWidget();
}

MergeContactShowResultTabWidget::~MergeContactShowResultTabWidget()
{
}

void MergeContactShowResultTabWidget::updateTabWidget()
{
    tabBar()->setVisible(count() > 1);
}

bool MergeContactShowResultTabWidget::tabBarVisible() const
{
    return tabBar()->isVisible();
}

void MergeContactShowResultTabWidget::showMergedContacts(const Akonadi::Item::List &lstItem)
{
    clear();
    Q_FOREACH (const Akonadi::Item &item, lstItem) {
        addMergedContact(item, false);
    }
    updateTabWidget();
}

void MergeContactShowResultTabWidget::addMergedContact(const Akonadi::Item &item, bool updateTab)
{
    if (item.hasPayload<KContacts::Addressee>()) {
        const KContacts::Addressee address = item.payload<KContacts::Addressee>();
        MergeContactInfoWidget *infoWidget = new MergeContactInfoWidget;
        infoWidget->setContact(item);
        addTab(infoWidget, address.name());
        if (updateTab) {
            updateTabWidget();
        }
    } else {
        qCDebug(KADDRESSBOOK_LOG) << " don't have address";
    }
}

