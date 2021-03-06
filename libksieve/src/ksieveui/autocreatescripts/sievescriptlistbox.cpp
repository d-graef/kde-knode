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
#include "sievescriptlistbox.h"
#include "sievescriptdescriptiondialog.h"
#include "sieveglobalvariablewidget.h"
#include "sieveforeverypartwidget.h"
#include "sievescriptpage.h"
#include "sieveincludewidget.h"

#include <QHBoxLayout>
#include <KMessageBox>
#include <KLocalizedString>
#include <QInputDialog>
#include <QPushButton>
#include <KIconLoader>
#include <QIcon>

#include <QVBoxLayout>
#include <QListWidget>
#include <QPointer>
#include <QDomElement>
#include "libksieve_debug.h"

namespace
{
inline const QString defaultScriptName()
{
    return QStringLiteral("SCRIPTNAME: ");
}
}

using namespace KSieveUi;

SieveScriptListItem::SieveScriptListItem(const QString &text, QListWidget *parent)
    : QListWidgetItem(text, parent),
      mScriptPage(Q_NULLPTR)
{
}

SieveScriptListItem::~SieveScriptListItem()
{
}

void SieveScriptListItem::setDescription(const QString &desc)
{
    mDescription = desc;
}

QString SieveScriptListItem::description() const
{
    return mDescription;
}

SieveScriptPage *SieveScriptListItem::scriptPage() const
{
    return mScriptPage;
}

void SieveScriptListItem::setScriptPage(SieveScriptPage *page)
{
    mScriptPage = page;
}

QString SieveScriptListItem::generatedScript(QStringList &requires) const
{
    QString script;
    if (!mDescription.isEmpty()) {
        script = QLatin1Char('#') + mDescription;
        script.replace(QLatin1Char('\n'), QStringLiteral("\n#"));
        script += QLatin1Char('\n');
    }
    if (mScriptPage) {
        mScriptPage->generatedScript(script, requires);
    }
    return script;
}

SieveScriptListBox::SieveScriptListBox(const QString &title, QWidget *parent)
    : QGroupBox(title, parent),
      mScriptNumber(0)
{
    QVBoxLayout *layout = new QVBoxLayout();
    mSieveListScript = new QListWidget;
    layout->addWidget(mSieveListScript);

    //----------- the first row of buttons
    QWidget *hb = new QWidget(this);
    QHBoxLayout *hbHBoxLayout = new QHBoxLayout(hb);
    hbHBoxLayout->setMargin(0);
    hbHBoxLayout->setSpacing(4);

    mBtnTop = new QPushButton(QString(), hb);
    hbHBoxLayout->addWidget(mBtnTop);
    mBtnTop->setIcon(QIcon::fromTheme(QStringLiteral("go-top")));
    mBtnTop->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    mBtnTop->setMinimumSize(mBtnTop->sizeHint() * 1.2);

    mBtnUp = new QPushButton(QString(), hb);
    hbHBoxLayout->addWidget(mBtnUp);
    mBtnUp->setAutoRepeat(true);
    mBtnUp->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    mBtnUp->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    mBtnUp->setMinimumSize(mBtnUp->sizeHint() * 1.2);
    mBtnDown = new QPushButton(QString(), hb);
    hbHBoxLayout->addWidget(mBtnDown);
    mBtnDown->setAutoRepeat(true);
    mBtnDown->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    mBtnDown->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    mBtnDown->setMinimumSize(mBtnDown->sizeHint() * 1.2);

    mBtnBottom = new QPushButton(QString(), hb);
    hbHBoxLayout->addWidget(mBtnBottom);
    mBtnBottom->setIcon(QIcon::fromTheme(QStringLiteral("go-bottom")));
    mBtnBottom->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    mBtnBottom->setMinimumSize(mBtnBottom->sizeHint() * 1.2);

    mBtnUp->setToolTip(i18nc("Move selected filter up.", "Up"));
    mBtnDown->setToolTip(i18nc("Move selected filter down.", "Down"));
    mBtnTop->setToolTip(i18nc("Move selected filter to the top.", "Top"));
    mBtnBottom->setToolTip(i18nc("Move selected filter to the bottom.", "Bottom"));

    layout->addWidget(hb);

    hb = new QWidget(this);
    hbHBoxLayout = new QHBoxLayout(hb);
    hbHBoxLayout->setMargin(0);
    hbHBoxLayout->setSpacing(4);

    mBtnNew = new QPushButton(QString(), hb);
    hbHBoxLayout->addWidget(mBtnNew);
    mBtnNew->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    mBtnNew->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    mBtnNew->setToolTip(i18n("New Script"));
    mBtnNew->setMinimumSize(mBtnNew->sizeHint() * 1.2);

    mBtnDelete = new QPushButton(QString(), hb);
    hbHBoxLayout->addWidget(mBtnDelete);
    mBtnDelete->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    mBtnDelete->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    mBtnDelete->setToolTip(i18n("Delete Script"));
    mBtnDelete->setMinimumSize(mBtnDelete->sizeHint() * 1.2);

    mBtnRename = new QPushButton(i18n("Rename..."), hb);
    mBtnRename->setToolTip(i18n("Rename Script"));
    hbHBoxLayout->addWidget(mBtnRename);

    mBtnDescription = new QPushButton(i18n("Edit description..."), hb);
    mBtnDescription->setToolTip(i18n("Edit Script Description"));
    hbHBoxLayout->addWidget(mBtnDescription);

    layout->addWidget(hb);
    setLayout(layout);

    connect(mBtnNew, &QPushButton::clicked, this, &SieveScriptListBox::slotNew);
    connect(mBtnDelete, &QPushButton::clicked, this, &SieveScriptListBox::slotDelete);
    connect(mBtnRename, &QPushButton::clicked, this, &SieveScriptListBox::slotRename);
    connect(mBtnDescription, &QPushButton::clicked, this, &SieveScriptListBox::slotEditDescription);

    connect(mBtnUp, &QPushButton::clicked, this, &SieveScriptListBox::slotUp);
    connect(mBtnDown, &QPushButton::clicked, this, &SieveScriptListBox::slotDown);
    connect(mBtnTop, &QPushButton::clicked, this, &SieveScriptListBox::slotTop);
    connect(mBtnBottom, &QPushButton::clicked, this, &SieveScriptListBox::slotBottom);

    connect(mSieveListScript, &QListWidget::itemSelectionChanged, this, &SieveScriptListBox::updateButtons);
    connect(mSieveListScript, &QListWidget::itemActivated, this, &SieveScriptListBox::slotItemActived);
    connect(mSieveListScript, &QListWidget::itemDoubleClicked, this, &SieveScriptListBox::slotEditDescription);
    updateButtons();
}

SieveScriptListBox::~SieveScriptListBox()
{
}

void SieveScriptListBox::slotItemActived(QListWidgetItem *item)
{
    if (item) {
        SieveScriptListItem *itemScript = static_cast<SieveScriptListItem *>(item);
        Q_EMIT activatePage(itemScript->scriptPage());
    }
}

void SieveScriptListBox::updateButtons()
{
    const int currentIndex = mSieveListScript->currentRow();
    const bool theFirst = (currentIndex == 0);
    const int numberOfElement(mSieveListScript->count());
    const bool theLast = (currentIndex >= numberOfElement - 1);

    const QList<QListWidgetItem *> lst = mSieveListScript->selectedItems();
    mBtnDelete->setEnabled(!lst.isEmpty());
    mBtnRename->setEnabled(lst.count() == 1);
    mBtnDescription->setEnabled(lst.count() == 1);
    mBtnBottom->setEnabled(!lst.isEmpty() && !theLast);
    mBtnTop->setEnabled(!lst.isEmpty() && !theFirst);
    mBtnDown->setEnabled(!lst.isEmpty() && !theLast);
    mBtnUp->setEnabled(!lst.isEmpty() && !theFirst);
}

SieveScriptPage *SieveScriptListBox::createNewScript(const QString &newName, const QString &description)
{
    SieveScriptListItem *item = new SieveScriptListItem(newName, mSieveListScript);
    item->setDescription(description);
    SieveScriptPage *page = new SieveScriptPage;
    item->setScriptPage(page);
    Q_EMIT addNewPage(page);
    Q_EMIT enableButtonOk(true);
    mSieveListScript->setCurrentItem(item);
    updateButtons();
    return page;
}

void SieveScriptListBox::slotNew()
{
    const QString newName = QInputDialog::getText(this, i18n("New Script"), i18n("Add new name:"));
    if (!newName.trimmed().isEmpty()) {
        createNewScript(newName);
        Q_EMIT valueChanged();
    }
}

void SieveScriptListBox::slotDelete()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        if (KMessageBox::warningYesNo(this, i18n("Do you want to delete \"%1\" script?", item->text()), i18n("Delete script")) == KMessageBox::Yes) {
            SieveScriptListItem *itemScript = static_cast<SieveScriptListItem *>(item);
            Q_EMIT removePage(itemScript->scriptPage());
            delete item;
            Q_EMIT enableButtonOk(mSieveListScript->count() > 0);
            updateButtons();
            Q_EMIT valueChanged();
        }
    }
}

void SieveScriptListBox::slotRename()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const QString newName = QInputDialog::getText(this, i18n("Rename"), i18n("Add new name:"), QLineEdit::Normal, item->text());
        if (!newName.trimmed().isEmpty()) {
            item->setText(newName);
            Q_EMIT valueChanged();
        }
    }
}

void SieveScriptListBox::slotEditDescription()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        SieveScriptListItem *sieveItem = static_cast<SieveScriptListItem *>(item);
        QPointer<SieveScriptDescriptionDialog> dlg = new SieveScriptDescriptionDialog(this);
        dlg->setDescription(sieveItem->description());
        if (dlg->exec()) {
            sieveItem->setDescription(dlg->description());
            Q_EMIT valueChanged();
        }
        delete dlg;
    }
}

void SieveScriptListBox::slotTop()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const int currentIndex = mSieveListScript->currentRow();
        if (currentIndex != 0) {
            item = mSieveListScript->takeItem(currentIndex);
            mSieveListScript->insertItem(0, item);
            mSieveListScript->setCurrentItem(item);
            Q_EMIT valueChanged();
        }
    }
}

void SieveScriptListBox::slotBottom()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const int currentIndex = mSieveListScript->currentRow();
        if (currentIndex != mSieveListScript->count() - 1) {
            item = mSieveListScript->takeItem(currentIndex);
            mSieveListScript->insertItem(mSieveListScript->count(), item);
            mSieveListScript->setCurrentItem(item);
            Q_EMIT valueChanged();
        }
    }
}

void SieveScriptListBox::slotDown()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const int currentIndex = mSieveListScript->currentRow();
        if (currentIndex < mSieveListScript->count() - 1) {
            item = mSieveListScript->takeItem(currentIndex);
            mSieveListScript->insertItem(currentIndex + 1, item);
            mSieveListScript->setCurrentItem(item);
            Q_EMIT valueChanged();
        }
    }
}

void SieveScriptListBox::slotUp()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const int currentIndex = mSieveListScript->currentRow();
        if (currentIndex >= 1) {
            item = mSieveListScript->takeItem(currentIndex);
            mSieveListScript->insertItem(currentIndex - 1, item);
            mSieveListScript->setCurrentItem(item);
            Q_EMIT valueChanged();
        }
    }
}

QString SieveScriptListBox::generatedScript(QString &requires) const
{
    QString resultScript;
    QStringList lstRequires;
    const int numberOfScripts(mSieveListScript->count());
    for (int i = 0; i < numberOfScripts; ++i) {
        SieveScriptListItem *item = static_cast<SieveScriptListItem *>(mSieveListScript->item(i));
        if (i != 0) {
            resultScript += QLatin1String("\n\n");
        }
        resultScript += QLatin1Char('#') + defaultScriptName() + item->text() + QLatin1Char('\n');
        resultScript += item->generatedScript(lstRequires);
    }

    QStringList endRequires;
    Q_FOREACH (const QString &r, lstRequires) {
        if (!endRequires.contains(r)) {
            endRequires.append(r);
            requires += QStringLiteral("require \"%1\";\n").arg(r);
        }
    }

    return resultScript;
}

void SieveScriptListBox::clear()
{
    mScriptNumber = 0;
    Q_EMIT enableButtonOk(false);
    //Clear tabpage
    mSieveListScript->clear();
    updateButtons();
}

void SieveScriptListBox::loadScript(const QDomDocument &doc, QString &error)
{
    clear();
    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    SieveScriptPage *currentPage = Q_NULLPTR;
    ParseSieveScriptTypeBlock typeBlock = TypeUnknown;
    loadBlock(n, currentPage, typeBlock, error);
}

void SieveScriptListBox::loadBlock(QDomNode &n, SieveScriptPage *currentPage, ParseSieveScriptTypeBlock typeBlock, QString &error)
{
    QString scriptName;
    QString comment;
    bool hasCreatedAIfBlock = false;
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            //qCDebug(LIBKSIEVE_LOG)<<" tagName "<<tagName;
            if (tagName == QLatin1String("control")) {
                //Create a new page when before it was "onlyactions"
                if (typeBlock == TypeBlockAction) {
                    currentPage = Q_NULLPTR;
                }
                if (e.hasAttribute(QStringLiteral("name"))) {
                    const QString controlType = e.attribute(QStringLiteral("name"));
                    //qCDebug(LIBKSIEVE_LOG)<<" controlType"<<controlType;
                    if (controlType == QLatin1String("if")) {
                        typeBlock = TypeBlockIf;
                        if (!currentPage || hasCreatedAIfBlock) {
                            currentPage = createNewScript(scriptName.isEmpty() ? createUniqName() : scriptName, comment);
                        }
                        hasCreatedAIfBlock = true;
                        comment.clear();
                        currentPage->blockIfWidget()->loadScript(e, false, error);
                    } else if (controlType == QLatin1String("elsif")) {
                        typeBlock = TypeBlockElsif;
                        if (!currentPage) {
                            qCDebug(LIBKSIEVE_LOG) << " script is not correct missing if block";
                            currentPage = createNewScript(scriptName.isEmpty() ? createUniqName() : scriptName, comment);
                        }
                        SieveScriptBlockWidget *blockWidget = currentPage->addScriptBlock(KSieveUi::SieveWidgetPageAbstract::BlockElsIf);
                        if (blockWidget) {
                            blockWidget->loadScript(e, false, error);
                        }
                    } else if (controlType == QLatin1String("else")) {
                        typeBlock = TypeBlockElse;
                        if (!currentPage) {
                            currentPage = createNewScript(scriptName.isEmpty() ? createUniqName() : scriptName, comment);
                            qCDebug(LIBKSIEVE_LOG) << " script is not correct missing if block";
                        }
                        SieveScriptBlockWidget *blockWidget = currentPage->addScriptBlock(KSieveUi::SieveWidgetPageAbstract::BlockElse);
                        if (blockWidget) {
                            blockWidget->loadScript(e, false, error);
                        }
                        //We are sure that we can't have another elsif
                        currentPage = Q_NULLPTR;
                    } else if (controlType == QLatin1String("foreverypart")) {
                        typeBlock = TypeBlockForeachBlock;
                        if (!currentPage) {
                            currentPage = createNewScript(scriptName.isEmpty() ? createUniqName() : scriptName, comment);
                            comment.clear();
                        }
                        if (currentPage->forEveryPartWidget()) {
                            currentPage->forEveryPartWidget()->loadScript(e, error);
                        } else {
                            error += i18n("forEveryPart is not supported by your server") + QLatin1Char('\n');
                        }
                        //TODO verify it.
                        QDomNode block = e.firstChildElement(QStringLiteral("block")).firstChild();
                        loadBlock(block, currentPage, typeBlock, error);
                    } else if (controlType == QLatin1String("require")) {
                        //Nothing. autogenerated
                    } else {
                        qCDebug(LIBKSIEVE_LOG) << " unknown controlType :" << controlType;
                    }
                }
            } else if (tagName == QLatin1String("comment")) {
                QString str(e.text());
                if (str.contains(defaultScriptName())) {
                    scriptName = str.remove(defaultScriptName());
                } else {
                    if (!comment.isEmpty()) {
                        comment += QLatin1Char('\n');
                    }
                    comment += e.text();
                }
            } else if (tagName == QLatin1String("action")) {
                if (e.hasAttribute(QStringLiteral("name"))) {
                    const QString actionName = e.attribute(QStringLiteral("name"));
                    if (actionName == QLatin1String("include")) {
                        if (!currentPage || (typeBlock == TypeBlockIf) || (typeBlock == TypeBlockElse) || (typeBlock == TypeBlockElsif)) {
                            currentPage = createNewScript(scriptName.isEmpty() ? createUniqName() : scriptName, comment);
                            comment.clear();
                        }
                        typeBlock = TypeBlockInclude;
                        if (currentPage->includeWidget()) {
                            currentPage->includeWidget()->loadScript(e, error);
                        } else {
                            qCDebug(LIBKSIEVE_LOG) << " include not supported";
                        }
                    } else if (actionName == QLatin1String("global")) {
                        if (!currentPage || (typeBlock == TypeBlockIf) || (typeBlock == TypeBlockElse) || (typeBlock == TypeBlockElsif)) {
                            currentPage = createNewScript(scriptName.isEmpty() ? createUniqName() : scriptName, comment);
                            comment.clear();
                        }
                        typeBlock = TypeBlockGlobal;
                        if (currentPage->globalVariableWidget()) {
                            currentPage->globalVariableWidget()->loadScript(e, error);
                        } else {
                            qCDebug(LIBKSIEVE_LOG) << " globalVariable not supported";
                        }
                    } else if (actionName == QLatin1String("set") && (typeBlock == TypeBlockGlobal)) {
                        if (currentPage->globalVariableWidget()) {
                            currentPage->globalVariableWidget()->loadSetVariable(e, error);
                        } else {
                            qCDebug(LIBKSIEVE_LOG) << " globalVariable not supported";
                        }
                    } else {
                        if (!currentPage || (typeBlock == TypeBlockIf) || (typeBlock == TypeBlockElse) || (typeBlock == TypeBlockElsif)) {
                            currentPage = createNewScript(scriptName.isEmpty() ? createUniqName() : scriptName, comment);
                        }
                        typeBlock = TypeBlockAction;
                        comment.clear();
                        currentPage->blockIfWidget()->loadScript(e, true, error);
                        //qCDebug(LIBKSIEVE_LOG)<<" unknown action name: "<<actionName;
                    }
                }
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else {
                qCDebug(LIBKSIEVE_LOG) << " unknown tagname" << tagName;
            }
        }
        n = n.nextSibling();
    }
}

QString SieveScriptListBox::createUniqName()
{
    const QString pattern = i18n("Script part %1", mScriptNumber);
    ++mScriptNumber;
    return pattern;
}

