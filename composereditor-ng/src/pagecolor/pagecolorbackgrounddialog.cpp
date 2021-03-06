/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "pagecolorbackgrounddialog.h"
#include "pagecolorbackgroundwidget.h"
#include "extendattributes/extendattributesbutton.h"

#include <KLocalizedString>
#include <KSeparator>
#include <QUrl>

#include <QVBoxLayout>
#include <QWebElement>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

namespace ComposerEditorNG
{

class PageColorBackgroundDialogPrivate
{
public:
    PageColorBackgroundDialogPrivate(const QWebElement &element, PageColorBackgroundDialog *qq)
        : webElement(element),
          q(qq)
    {
        q->setWindowTitle(i18n("Page Color and Background"));
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
        QWidget *mainWidget = new QWidget(q);
        QVBoxLayout *mainLayout = new QVBoxLayout;
        q->setLayout(mainLayout);
        mainLayout->addWidget(mainWidget);
        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        q->connect(buttonBox, &QDialogButtonBox::accepted, q, &QDialog::accept);
        q->connect(buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);
        mainLayout->addWidget(buttonBox);

        QVBoxLayout *layout = new QVBoxLayout(mainWidget);
        pageColorWidget = new PageColorBackgroundWidget;
        layout->addWidget(pageColorWidget);

        ExtendAttributesButton *button = new ExtendAttributesButton(webElement, ExtendAttributesDialog::Body, q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        layout->addWidget(button);

        layout->addWidget(new KSeparator);

        q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotOkClicked()));
        q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotApplyClicked()));
        updateSettings();
    }

    void updateSettings();

    void applyChanges();

    void _k_slotOkClicked();
    void _k_slotApplyClicked();
    void _k_slotWebElementChanged();

    PageColorBackgroundWidget *pageColorWidget;
    QWebElement webElement;
    PageColorBackgroundDialog *q;
};

void PageColorBackgroundDialogPrivate::_k_slotWebElementChanged()
{
    updateSettings();
}

void PageColorBackgroundDialogPrivate::updateSettings()
{
    if (!webElement.isNull()) {
        if (webElement.hasAttribute(QStringLiteral("bgcolor")) ||
                webElement.hasAttribute(QStringLiteral("text")) ||
                webElement.hasAttribute(QStringLiteral("link")) ||
                webElement.hasAttribute(QStringLiteral("vlink")) ||
                webElement.hasAttribute(QStringLiteral("alink"))) {
            pageColorWidget->setUseDefaultColor(false);
            pageColorWidget->setPageBackgroundColor(QColor(webElement.attribute(QStringLiteral("bgcolor"))));
            pageColorWidget->setTextColor(QColor(webElement.attribute(QStringLiteral("text"))));
            pageColorWidget->setLinkColor(QColor(webElement.attribute(QStringLiteral("link"))));
            pageColorWidget->setActiveLinkColor(QColor(webElement.attribute(QStringLiteral("alink"))));
            pageColorWidget->setVisitedLinkColor(QColor(webElement.attribute(QStringLiteral("vlink"))));
        } else {
            pageColorWidget->setUseDefaultColor(true);
        }
        if (webElement.hasAttribute(QStringLiteral("background"))) {
            pageColorWidget->setBackgroundImageUrl(QUrl(webElement.attribute(QStringLiteral("background"))));
        }
    }
}

void PageColorBackgroundDialogPrivate::_k_slotOkClicked()
{
    applyChanges();
    q->accept();
}

void PageColorBackgroundDialogPrivate::_k_slotApplyClicked()
{
    applyChanges();
}

void PageColorBackgroundDialogPrivate::applyChanges()
{
    if (!webElement.isNull()) {
        if (pageColorWidget->useDefaultColor()) {
            webElement.removeAttribute(QStringLiteral("bgcolor"));
            webElement.removeAttribute(QStringLiteral("text"));
            webElement.removeAttribute(QStringLiteral("link"));
            webElement.removeAttribute(QStringLiteral("alink"));
            webElement.removeAttribute(QStringLiteral("vlink"));
        } else {
            QColor col = pageColorWidget->pageBackgroundColor();
            if (col.isValid()) {
                webElement.setAttribute(QStringLiteral("bgcolor"), col.name());
            }
            col = pageColorWidget->textColor();
            if (col.isValid()) {
                webElement.setAttribute(QStringLiteral("text"), col.name());
            }
            col = pageColorWidget->linkColor();
            if (col.isValid()) {
                webElement.setAttribute(QStringLiteral("link"), col.name());
            }
            col = pageColorWidget->activeLinkColor();
            if (col.isValid()) {
                webElement.setAttribute(QStringLiteral("alink"), col.name());
            }
            col = pageColorWidget->visitedLinkColor();
            if (col.isValid()) {
                webElement.setAttribute(QStringLiteral("vlink"), col.name());
            }
        }
        if (pageColorWidget->backgroundImageUrl().isEmpty()) {
            webElement.removeAttribute(QStringLiteral("background"));
        } else {
            webElement.setAttribute(QStringLiteral("background"), pageColorWidget->backgroundImageUrl().url());
        }
    }
}

PageColorBackgroundDialog::PageColorBackgroundDialog(const QWebElement &element, QWidget *parent)
    : QDialog(parent), d(new PageColorBackgroundDialogPrivate(element, this))
{
}

PageColorBackgroundDialog::~PageColorBackgroundDialog()
{
    delete d;
}

}

#include "moc_pagecolorbackgrounddialog.cpp"
