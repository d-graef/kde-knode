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

#include "previewwidget.h"

using namespace GrantleeThemeEditor;

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent),
      mPrinting(false)
{
}

PreviewWidget::~PreviewWidget()
{

}

void PreviewWidget::updateViewer()
{
}

void PreviewWidget::setPrinting(bool printMode)
{
    if (mPrinting != printMode) {
        mPrinting = printMode;
        updateViewer();
    }
}

bool PreviewWidget::printing() const
{
    return mPrinting;
}

void PreviewWidget::loadConfig()
{

}

void PreviewWidget::createScreenShot(const QStringList &lstFileName)
{
}

void PreviewWidget::setThemePath(const QString &projectDirectory, const QString &mainPageFileName)
{
}

void PreviewWidget::slotMainFileNameChanged(const QString &)
{

}

void PreviewWidget::slotExtraHeaderDisplayChanged(const QStringList &headers)
{

}

