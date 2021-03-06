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

#include "sieveconditionaddress.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/selectaddresspartcombobox.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "widgets/selectheadertypecombobox.h"
#include "editor/sieveeditorutil.h"

#include <QLineEdit>
#include <KLocalizedString>

#include <QHBoxLayout>
#include <QLabel>
#include "libksieve_debug.h"
#include <QDomNode>

using namespace KSieveUi;

SieveConditionAddress::SieveConditionAddress(QObject *parent)
    : SieveCondition(QStringLiteral("address"), i18n("Address"), parent)
{
}

SieveCondition *SieveConditionAddress::newAction()
{
    return new SieveConditionAddress;
}

QWidget *SieveConditionAddress::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectAddressPartComboBox *selectAddressPart = new SelectAddressPartComboBox;
    connect(selectAddressPart, &SelectAddressPartComboBox::valueChanged, this, &SieveConditionAddress::valueChanged);
    selectAddressPart->setObjectName(QStringLiteral("addresspartcombobox"));
    lay->addWidget(selectAddressPart);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    lay->addLayout(grid);
    SelectMatchTypeComboBox *selectMatchCombobox = new SelectMatchTypeComboBox;
    connect(selectMatchCombobox, &SelectMatchTypeComboBox::valueChanged, this, &SieveConditionAddress::valueChanged);
    selectMatchCombobox->setObjectName(QStringLiteral("matchtypecombobox"));
    grid->addWidget(selectMatchCombobox, 0, 0);

    SelectHeaderTypeComboBox *selectHeaderType = new SelectHeaderTypeComboBox;
    connect(selectHeaderType, &SelectHeaderTypeComboBox::valueChanged, this, &SieveConditionAddress::valueChanged);
    selectHeaderType->setObjectName(QStringLiteral("headertypecombobox"));
    grid->addWidget(selectHeaderType, 0, 1);

    QLabel *lab = new QLabel(i18n("address:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *edit = new QLineEdit;
    connect(edit, &QLineEdit::textChanged, this, &SieveConditionAddress::valueChanged);
    edit->setClearButtonEnabled(true);
    edit->setPlaceholderText(i18n("Use ; to separate emails"));
    grid->addWidget(edit, 1, 1);
    edit->setObjectName(QStringLiteral("editaddress"));
    return w;
}

QString SieveConditionAddress::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtypecombobox"));
    bool isNegative = false;
    const QString matchTypeStr = selectMatchCombobox->code(isNegative);

    const SelectAddressPartComboBox *selectAddressPart = w->findChild<SelectAddressPartComboBox *>(QStringLiteral("addresspartcombobox"));
    const QString selectAddressPartStr = selectAddressPart->code();

    const SelectHeaderTypeComboBox *selectHeaderType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headertypecombobox"));
    const QString selectHeaderTypeStr = selectHeaderType->code();

    const QLineEdit *edit = w->findChild<QLineEdit *>(QStringLiteral("editaddress"));
    const QString addressStr = AutoCreateScriptUtil::createAddressList(edit->text().trimmed(), false);
    return AutoCreateScriptUtil::negativeString(isNegative) + QStringLiteral("address %1 %2 %3 %4").arg(selectAddressPartStr).arg(matchTypeStr).arg(selectHeaderTypeStr).arg(addressStr);
}

QStringList SieveConditionAddress::needRequires(QWidget *w) const
{
    const SelectAddressPartComboBox *selectAddressPart = w->findChild<SelectAddressPartComboBox *>(QStringLiteral("addresspartcombobox"));
    return QStringList() << selectAddressPart->extraRequire();
}

QString SieveConditionAddress::help() const
{
    return i18n("The \"address\" test matches Internet addresses in structured headers that contain addresses.  It returns true if any header contains any key in the specified part of the address, as modified by the comparator and the match keyword.");
}

bool SieveConditionAddress::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition, QString &error)
{
    int index = 0;
    int indexStr = 0;
    QDomNode node = element.firstChild();
    QStringList lstTagValue;
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                lstTagValue << e.text();
                ++index;
            } else if (tagName == QLatin1String("str")) {
                if (indexStr == 0) {
                    SelectHeaderTypeComboBox *selectHeaderType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headertypecombobox"));
                    selectHeaderType->setCode(e.text());
                } else if (indexStr == 1) {
                    QLineEdit *edit = w->findChild<QLineEdit *>(QStringLiteral("editaddress"));
                    edit->setText(AutoCreateScriptUtil::quoteStr(e.text()));
                } else {
                    tooManyArgument(tagName, indexStr, 2, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveConditionAddress::setParamWidgetValue too many argument :" << index;
                }
                ++indexStr;
            } else if (tagName == QLatin1String("list")) {
                if (indexStr == 0) {
                    SelectHeaderTypeComboBox *selectHeaderType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headertypecombobox"));
                    selectHeaderType->setCode(AutoCreateScriptUtil::listValueToStr(e));
                } else if (indexStr == 1) {
                    QLineEdit *edit = w->findChild<QLineEdit *>(QStringLiteral("editaddress"));
                    edit->setText(AutoCreateScriptUtil::listValueToStr(e));
                } else {
                    tooManyArgument(tagName, indexStr, 2, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveConditionAddress::setParamWidgetValue too many argument :" << index;
                }
                ++indexStr;
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveConditionAddress::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    if (lstTagValue.count() == 1) {
        QString specificError;
        SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtypecombobox"));
        selectMatchCombobox->setCode(AutoCreateScriptUtil::tagValueWithCondition(lstTagValue.at(0), notCondition), name(), specificError);
        if (!specificError.isEmpty()) { //Test if default match type == is
            SelectAddressPartComboBox *selectAddressPart = w->findChild<SelectAddressPartComboBox *>(QStringLiteral("addresspartcombobox"));
            selectAddressPart->setCode(AutoCreateScriptUtil::tagValue(lstTagValue.at(0)), name(), error);
        }
    } else if (lstTagValue.count() == 2) {
        SelectAddressPartComboBox *selectAddressPart = w->findChild<SelectAddressPartComboBox *>(QStringLiteral("addresspartcombobox"));
        selectAddressPart->setCode(AutoCreateScriptUtil::tagValue(lstTagValue.at(0)), name(), error);
        SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtypecombobox"));
        selectMatchCombobox->setCode(AutoCreateScriptUtil::tagValueWithCondition(lstTagValue.at(1), notCondition), name(), error);
    } else if (lstTagValue.count() > 2) {
        tooManyArgument(QStringLiteral("tag"), lstTagValue.count(), 2, error);
        qCDebug(LIBKSIEVE_LOG) << "SieveConditionAddress::setParamWidgetValue too many argument :" << lstTagValue.count();
    }
    return true;
}

QUrl SieveConditionAddress::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

