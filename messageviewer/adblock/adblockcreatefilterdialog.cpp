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

#include "adblockcreatefilterdialog.h"
#include "ui_adblockcreatefilterwidget.h"

#include <QDebug>

using namespace MessageViewer;
AdBlockCreateFilterDialog::AdBlockCreateFilterDialog(QWidget *parent)
    : KDialog(parent),
      mCurrentType(AdBlockBlockableItemsWidget::None)
{
    QWidget *w = new QWidget;
    mUi = new Ui::AdBlockCreateFilterWidget;
    mUi->setupUi(w);
    setMainWidget(w);
    connect(mUi->blockingFilter, SIGNAL(toggled(bool)), SLOT(slotUpdateFilter()));
    connect(mUi->exceptionFilter, SIGNAL(toggled(bool)), SLOT(slotUpdateFilter()));
    connect(mUi->atTheBeginning, SIGNAL(toggled(bool)), SLOT(slotUpdateFilter()));
    connect(mUi->atTheEnd, SIGNAL(toggled(bool)), SLOT(slotUpdateFilter()));
    connect(mUi->restrictToDomain, SIGNAL(toggled(bool)), SLOT(slotUpdateFilter()));
    connect(mUi->restrictToDomainStr, SIGNAL(textChanged(QString)), SLOT(slotUpdateFilter()));
    connect(mUi->firstPartOnly, SIGNAL(toggled(bool)), SLOT(slotUpdateFilter()));
    connect(mUi->matchCase, SIGNAL(toggled(bool)), SLOT(slotUpdateFilter()));
    connect(mUi->applyListElement, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(slotUpdateFilter()));
    readConfig();
}

AdBlockCreateFilterDialog::~AdBlockCreateFilterDialog()
{
    writeConfig();
    delete mUi;
    mUi = 0;
}

void AdBlockCreateFilterDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "AdBlockCreateFilterDialog" );
    group.writeEntry( "Size", size() );
}

void AdBlockCreateFilterDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "AdBlockCreateFilterDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void AdBlockCreateFilterDialog::setPattern(AdBlockBlockableItemsWidget::TypeElement type, const QString &pattern)
{
    if (mPattern != pattern) {
        mPattern = pattern;
        mCurrentType = type;
        initialize();
    }
}

void AdBlockCreateFilterDialog::initialize()
{
    mUi->applyListElement->clear();
    for (int i = 0; i < AdBlockBlockableItemsWidget::MaxTypeElement; ++i) {
        if (i == (int)mCurrentType)
            continue;
        QListWidgetItem *item = new QListWidgetItem(AdBlockBlockableItemsWidget::elementTypeToI18n(static_cast<AdBlockBlockableItemsWidget::TypeElement>(i)), mUi->applyListElement);
        item->setData(ElementValue, static_cast<AdBlockBlockableItemsWidget::TypeElement>(i));
        item->setCheckState(Qt::Unchecked);
    }

    mUi->blockingFilter->setChecked(true);
    mUi->filterName->setText(mPattern);
    slotUpdateFilter();
}

QString AdBlockCreateFilterDialog::filter() const
{
    return mUi->filterName->text();
}

void AdBlockCreateFilterDialog::slotUpdateFilter()
{
    QString pattern = mPattern;
    if (mUi->atTheBeginning->isChecked()) {
        pattern = QLatin1String("|") + pattern;
    }
    if (mUi->atTheEnd->isChecked()) {
        pattern += QLatin1String("|");
    }

    pattern += QLatin1Char('$') + AdBlockBlockableItemsWidget::elementType(mCurrentType);


    if (mUi->exceptionFilter->isChecked()) {
        pattern = QLatin1String("@@") + pattern;
    }
    const int numberOfElement(mUi->applyListElement->count());
    for (int i = 0; i < numberOfElement; ++i) {
        if (mUi->applyListElement->item(i)->checkState() == Qt::Checked) {

        }
    }


    if (mUi->restrictToDomain->isChecked()) {
        if (!mUi->restrictToDomainStr->text().isEmpty()) {
            pattern += QLatin1String(",domain=") + mUi->restrictToDomainStr->text();
        }
    }
    if (mUi->matchCase->isChecked()) {
        pattern += QLatin1String(",match-case");
    }
    if (mUi->firstPartOnly->isChecked()) {
        pattern += QLatin1String(",~third-party");
    }
    mUi->filterName->setText(pattern);
}

#include "adblockcreatefilterdialog.moc"
