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

#include "sievescriptdebuggerfontendwidget.h"

#include <QVBoxLayout>
#include <QSplitter>
#include "editor/sievetexteditwidget.h"
#include "editor/sievetextedit.h"

using namespace KSieveUi;

SieveScriptDebuggerFontEndWidget::SieveScriptDebuggerFontEndWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QSplitter *splitter = new QSplitter;
    splitter->setObjectName(QStringLiteral("splitter"));
    mainLayout->addWidget(splitter);

    mSieveTextEditWidget = new KSieveUi::SieveTextEditWidget(this);
    mSieveTextEditWidget->setObjectName(QStringLiteral("sievetexteditwidget"));
    splitter->addWidget(mSieveTextEditWidget);

}

SieveScriptDebuggerFontEndWidget::~SieveScriptDebuggerFontEndWidget()
{

}

QString SieveScriptDebuggerFontEndWidget::script() const
{
    return mSieveTextEditWidget->textEdit()->toPlainText();
}

void SieveScriptDebuggerFontEndWidget::setScript(const QString &script)
{
    mSieveTextEditWidget->textEdit()->setPlainText(script);
}

