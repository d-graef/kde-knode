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

#include "sieveconditionconvert.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/commonwidgets/selectconvertparameterwidget.h"
#include "autocreatescripts/commonwidgets/selectmimetypecombobox.h"

#include <KLocalizedString>

#include <QLabel>
#include <QDomNode>
#include "libksieve_debug.h"
#include <QGridLayout>

using namespace KSieveUi;
SieveConditionConvert::SieveConditionConvert(QObject *parent)
    : SieveCondition(QStringLiteral("convert"), i18n("Convert"), parent)
{
}

SieveCondition *SieveConditionConvert::newAction()
{
    return new SieveConditionConvert;
}

QWidget *SieveConditionConvert::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QGridLayout *lay = new QGridLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("From:"));
    lay->addWidget(lab, 0, 0);

    SelectMimeTypeComboBox *fromMimeType = new SelectMimeTypeComboBox;
    fromMimeType->setObjectName(QStringLiteral("from"));
    connect(fromMimeType, &SelectMimeTypeComboBox::valueChanged, this, &SieveConditionConvert::valueChanged);
    lay->addWidget(fromMimeType, 0, 1);

    lab = new QLabel(i18n("To:"));
    lay->addWidget(lab, 0, 2);

    SelectMimeTypeComboBox *toMimeType = new SelectMimeTypeComboBox;
    connect(toMimeType, &SelectMimeTypeComboBox::valueChanged, this, &SieveConditionConvert::valueChanged);
    toMimeType->setObjectName(QStringLiteral("to"));
    lay->addWidget(toMimeType, 0, 3);

    lab = new QLabel(i18n("Parameters:"));
    lay->addWidget(lab, 1, 0);

    SelectConvertParameterWidget *params = new SelectConvertParameterWidget;
    connect(params, &SelectConvertParameterWidget::valueChanged, this, &SieveConditionConvert::valueChanged);
    params->setObjectName(QStringLiteral("params"));
    lay->addWidget(params, 1, 1, 2, 3);
    return w;
}

QString SieveConditionConvert::code(QWidget *w) const
{
    QString result = QStringLiteral("convert ");
    const SelectMimeTypeComboBox *fromMimeType = w->findChild<SelectMimeTypeComboBox *>(QStringLiteral("from"));
    const QString fromMimeTypeStr = fromMimeType->code();
    result += QStringLiteral("%1 ").arg(fromMimeTypeStr);

    const SelectMimeTypeComboBox *toMimeType = w->findChild<SelectMimeTypeComboBox *>(QStringLiteral("to"));
    const QString toMimeTypeStr = toMimeType->code();
    result += QStringLiteral("%1 ").arg(toMimeTypeStr);

    const SelectConvertParameterWidget *params = w->findChild<SelectConvertParameterWidget *>(QStringLiteral("params"));
    const QString paramsStr = params->code();
    if (!paramsStr.isEmpty()) {
        result += QStringLiteral("%1").arg(paramsStr);
    }
    return result;
}

QStringList SieveConditionConvert::needRequires(QWidget *) const
{
    return QStringList() << QStringLiteral("convert");
}

bool SieveConditionConvert::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionConvert::serverNeedsCapability() const
{
    return QStringLiteral("convert");
}

QString SieveConditionConvert::help() const
{
    return i18n("The \"convert\" action specifies that all body parts with a media type equal to \"media-type\" be converted to the media type in \"media-type\" using conversion parameters.");
}

bool SieveConditionConvert::setParamWidgetValue(const QDomElement &element, QWidget *w, bool /*notCondition*/, QString &error)
{
    int index = 0;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                if (index == 0) {
                    SelectMimeTypeComboBox *fromMimeType = w->findChild<SelectMimeTypeComboBox *>(QStringLiteral("from"));
                    fromMimeType->setCode(e.text(), name(), error);
                } else if (index == 1) {
                    SelectMimeTypeComboBox *toMimeType = w->findChild<SelectMimeTypeComboBox *>(QStringLiteral("to"));
                    toMimeType->setCode(e.text(), name(), error);
                } else {
                    tooManyArgument(tagName, index, 2, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveActionConvert::setParamWidgetValue too many argument :" << index;
                }
                ++index;
            } else if (tagName == QLatin1String("list")) {
                SelectConvertParameterWidget *params = w->findChild<SelectConvertParameterWidget *>(QStringLiteral("params"));
                params->setCode(AutoCreateScriptUtil::listValue(e), error);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << "SieveActionConvert::setParamWidgetValue unknown tag " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

