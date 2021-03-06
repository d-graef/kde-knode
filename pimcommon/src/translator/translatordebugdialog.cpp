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

#include "translatordebugdialog.h"
#include "util/pimutil.h"
#include "kpimtextedit/plaintexteditorwidget.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

TranslatorDebugDialog::TranslatorDebugDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Translator Debug"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mUser1Button = new QPushButton;
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TranslatorDebugDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TranslatorDebugDialog::reject);
    mUser1Button->setText(i18n("Save As..."));
    connect(mUser1Button, &QPushButton::clicked, this, &TranslatorDebugDialog::slotSaveAs);

    mEdit = new KPIMTextEdit::PlainTextEditorWidget;
    mEdit->setReadOnly(true);
    mainLayout->addWidget(mEdit);
    mainLayout->addWidget(buttonBox);

    readConfig();
    mUser1Button->setEnabled(!mEdit->toPlainText().isEmpty());
}

TranslatorDebugDialog::~TranslatorDebugDialog()
{
    writeConfig();
}

void TranslatorDebugDialog::setDebug(const QString &debugStr)
{
    mEdit->setPlainText(debugStr);
    mUser1Button->setEnabled(!debugStr.isEmpty());
}

void TranslatorDebugDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "TranslatorDebugDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void TranslatorDebugDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "TranslatorDebugDialog");
    group.writeEntry("Size", size());
}

void TranslatorDebugDialog::slotSaveAs()
{
    const QString filter = i18n("All Files (*)");
    PimCommon::Util::saveTextAs(mEdit->toPlainText(), filter, this);
}

