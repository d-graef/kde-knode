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

#include "sievetemplatewidget.h"
#include "sievetemplateeditdialog.h"

#include <KMenu>
#include <KLocale>

#include <QListWidget>
#include <QHBoxLayout>
#include <QPointer>

using namespace KSieveUi;

SieveTemplateListWidget::SieveTemplateListWidget(QWidget *parent)
    : QListWidget(parent), mDirty(false)
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             SLOT(slotContextMenu(QPoint)) );
    loadTemplates();
}

SieveTemplateListWidget::~SieveTemplateListWidget()
{
    saveTemplates();
}

void SieveTemplateListWidget::slotContextMenu(const QPoint &pos)
{
    QList<QListWidgetItem *> lstSelectedItems = selectedItems();
    const bool hasItemsSelected = !lstSelectedItems.isEmpty();
    KMenu *menu = new KMenu( this );
    menu->addAction( i18n("Add..."), this, SLOT(slotAdd()));
    if (lstSelectedItems.count() == 1) {
        menu->addAction( i18n("Modify..."), this, SLOT(slotModify()));
    }
    if (hasItemsSelected) {
        menu->addAction( i18n("Remove"), this, SLOT(slotRemove()));
    }
    menu->exec( mapToGlobal( pos ) );
    delete menu;
}

void SieveTemplateListWidget::slotRemove()
{
    QList<QListWidgetItem *> lstSelectedItems = selectedItems();
    Q_FOREACH (QListWidgetItem *item, lstSelectedItems) {
        delete item;
    }
    mDirty = true;
}

void SieveTemplateListWidget::slotAdd()
{
    QPointer<SieveTemplateEditDialog> dlg = new SieveTemplateEditDialog(this);
    if (dlg->exec()) {
        const QString templateName = dlg->templateName();
        const QString templateScript = dlg->script();
        //TODO create item.
        mDirty = true;
    }
    delete dlg;
}

void SieveTemplateListWidget::slotModify()
{
    if(currentItem()) {
        QPointer<SieveTemplateEditDialog> dlg = new SieveTemplateEditDialog(this);
        if (dlg->exec()) {
            const QString templateName = dlg->templateName();
            const QString templateScript = dlg->script();
            //TODO create item.
            mDirty = true;
        }
        delete dlg;
    }
}


void SieveTemplateListWidget::loadTemplates()
{
    clear();
    KSharedConfig::Ptr config = KSharedConfig::openConfig( "sievetemplaterc", KConfig::NoGlobals );
    KConfigGroup group = config->group( "template" );
    if (group.hasKey(QLatin1String("templateCount"))) {
        const int numberTemplate = group.readEntry( "templateCount", 0 );
        for (int i = 0; i < numberTemplate; ++i) {
            KConfigGroup group = config->group( QString::fromLatin1( "templateDefine_%1" ).arg ( i ) );
            const QString name = group.readEntry( "Name", QString() );
            const QString text = group.readEntry( "Text", QString() );

            QListWidgetItem *item = new QListWidgetItem(name, this);
            item->setData(SieveTemplateListWidget::SieveText, text);
        }
    } else {
        //Load default template
    }
    mDirty = false;
}

void SieveTemplateListWidget::saveTemplates()
{
    if (!mDirty)
        return;

    KSharedConfig::Ptr config = KSharedConfig::openConfig( "sievetemplaterc", KConfig::NoGlobals );
    // clear everything
    foreach ( const QString &group, config->groupList() ) {
        config->deleteGroup( group );
    }

    KConfigGroup group = config->group( "template" );
    group.writeEntry( "templateCount", count() );

    for ( int i = 0; i < count(); ++i ) {
        KConfigGroup group = config->group( QString::fromLatin1( "templateDefine_%1" ).arg ( i ) );
        group.writeEntry( "Name", item(i)->text() );
        group.writeEntry( "Text", item(i)->data(SieveTemplateListWidget::SieveText) );
    }
    config->sync();
    mDirty = false;
}


SieveTemplateWidget::SieveTemplateWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mListTemplate = new SieveTemplateListWidget;
    lay->addWidget(mListTemplate);
    setLayout(lay);
}


SieveTemplateWidget::~SieveTemplateWidget()
{
}

#include "sievetemplatewidget.moc"
