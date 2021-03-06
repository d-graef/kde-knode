/*
  Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "categoryselectdialog.h"
#include "categoryhierarchyreader.h"
#include "ui_categoryselectdialog_base.h"

#include <CalendarSupport/CategoryConfig>

#include <QIcon>
#include <KLocalizedString>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;
using namespace CalendarSupport;

class CategorySelectWidgetBase : public QWidget, public Ui::CategorySelectDialog_base
{
public:
    CategorySelectWidgetBase(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);

        mButtonClear->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-locationbar-rtl")));
        mButtonEdit->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
    }
};

CategorySelectWidget::CategorySelectWidget(CategoryConfig *cc, QWidget *parent)
    : QWidget(parent), mCategoryConfig(cc)
{
    QHBoxLayout *topL = new QHBoxLayout(this);
    topL->setMargin(0);
    mWidgets = new CategorySelectWidgetBase(this);
    topL->addWidget(mWidgets);
    connect(mWidgets->mButtonEdit, &QAbstractButton::clicked,
            this, &CategorySelectWidget::editCategories);
    connect(mWidgets->mButtonClear, &QAbstractButton::clicked,
            this, &CategorySelectWidget::clear);
}

CategorySelectWidget::~CategorySelectWidget()
{
}

AutoCheckTreeWidget *CategorySelectWidget::listView() const
{
    return mWidgets->mCategories;
}

void CategorySelectWidget::hideButton()
{
    mWidgets->mButtonEdit->hide();
    mWidgets->mButtonClear->hide();
}

void CategorySelectWidget::setCategories(const QStringList &categoryList)
{
    mWidgets->mCategories->clear();
    mCategoryList.clear();

    QStringList::ConstIterator it;
    QStringList cats = mCategoryConfig->customCategories();
    for (it = categoryList.begin(); it != categoryList.end(); ++it) {
        if (!cats.contains(*it)) {
            cats.append(*it);
        }
    }
    mCategoryConfig->setCustomCategories(cats);
    CategoryHierarchyReaderQTreeWidget(mWidgets->mCategories).read(cats);
}

void CategorySelectWidget::setSelected(const QStringList &selList)
{
    clear();
    QStringList::ConstIterator it;

    bool remAutoCheckChildren = mWidgets->mCategories->autoCheckChildren();
    mWidgets->mCategories->setAutoCheckChildren(false);
    for (it = selList.begin(); it != selList.end(); ++it) {
        QStringList path = CategoryHierarchyReader::path(*it);
        QTreeWidgetItem *item = mWidgets->mCategories->itemByPath(path);
        if (item) {
            item->setCheckState(0, Qt::Checked);
        }
    }
    mWidgets->mCategories->setAutoCheckChildren(remAutoCheckChildren);
}

static QStringList getSelectedCategoriesFromCategoriesView(AutoCheckTreeWidget *categoriesView)
{
    QStringList categories;

    QTreeWidgetItemIterator it(categoriesView, QTreeWidgetItemIterator::Checked);
    while (*it) {
        QStringList path = categoriesView->pathByItem(*it++);
        if (path.count()) {
            path.replaceInStrings(CategoryConfig::categorySeparator, QStringLiteral("\\") +
                                  CategoryConfig::categorySeparator);
            categories.append(path.join(CategoryConfig::categorySeparator));
        }
    }

    return categories;
}

void CategorySelectWidget::clear()
{
    bool remAutoCheckChildren = mWidgets->mCategories->autoCheckChildren();
    mWidgets->mCategories->setAutoCheckChildren(false);

    QTreeWidgetItemIterator it(mWidgets->mCategories);
    while (*it) {
        (*it++)->setCheckState(0, Qt::Unchecked);
    }

    mWidgets->mCategories->setAutoCheckChildren(remAutoCheckChildren);
}

void CategorySelectWidget::setAutoselectChildren(bool autoselectChildren)
{
    mWidgets->mCategories->setAutoCheckChildren(autoselectChildren);
}

void CategorySelectWidget::hideHeader()
{
    mWidgets->mCategories->header()->hide();
}

QStringList CategorySelectWidget::selectedCategories(QString &categoriesStr)
{
    mCategoryList = getSelectedCategoriesFromCategoriesView(listView());
    categoriesStr = mCategoryList.join(QStringLiteral(", "));
    return mCategoryList;
}

QStringList CategorySelectWidget::selectedCategories() const
{
    return mCategoryList;
}

void CategorySelectWidget::setCategoryList(const QStringList &categories)
{
    mCategoryList = categories;
}

CategorySelectDialog::CategorySelectDialog(CategoryConfig *cc, QWidget *parent)
    : QDialog(parent), d(0)
{
    setWindowTitle(i18n("Select Categories"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QDialogButtonBox *buttonBox = 0;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel /*| QDialogButtonBox::Help*/ | QDialogButtonBox::Apply);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CategorySelectDialog::reject);

    QWidget *page = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setMargin(0);

    mWidgets = new CategorySelectWidget(cc, this);
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);
    mWidgets->setObjectName(QStringLiteral("CategorySelection"));
    mWidgets->hideHeader();
    lay->addWidget(mWidgets);

    mWidgets->setCategories();
    mWidgets->listView()->setFocus();

    connect(mWidgets, &CategorySelectWidget::editCategories, this, &CategorySelectDialog::editCategories);

    connect(okButton, &QPushButton::clicked, this, &CategorySelectDialog::slotOk);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &CategorySelectDialog::slotApply);
}

CategorySelectDialog::~CategorySelectDialog()
{
    delete mWidgets;
}

QStringList CategorySelectDialog::selectedCategories() const
{
    return mWidgets->selectedCategories();
}

void CategorySelectDialog::slotApply()
{
    QString categoriesStr;
    QStringList categories = mWidgets->selectedCategories(categoriesStr);
    Q_EMIT categoriesSelected(categories);
    Q_EMIT categoriesSelected(categoriesStr);
}

void CategorySelectDialog::slotOk()
{
    slotApply();
    accept();
}

void CategorySelectDialog::updateCategoryConfig()
{
    QString tmp;
    QStringList selected = mWidgets->selectedCategories(tmp);
    mWidgets->setCategories();
    mWidgets->setSelected(selected);
}

void CategorySelectDialog::setAutoselectChildren(bool autoselectChildren)
{
    mWidgets->setAutoselectChildren(autoselectChildren);
}

void CategorySelectDialog::setCategoryList(const QStringList &categories)
{
    mWidgets->setCategoryList(categories);
}

void CategorySelectDialog::setSelected(const QStringList &selList)
{
    mWidgets->setSelected(selList);
}

