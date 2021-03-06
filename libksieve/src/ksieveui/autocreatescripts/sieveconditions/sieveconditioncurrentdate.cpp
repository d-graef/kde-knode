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

#include "sieveconditioncurrentdate.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/selectdatewidget.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QDomNode>
#include "libksieve_debug.h"

using namespace KSieveUi;

SieveConditionCurrentDate::SieveConditionCurrentDate(QObject *parent)
    : SieveCondition(QStringLiteral("currentdate"), i18n("Currentdate"), parent)
{
}

SieveCondition *SieveConditionCurrentDate::newAction()
{
    return new SieveConditionCurrentDate;
}

QWidget *SieveConditionCurrentDate::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectMatchTypeComboBox *matchTypeCombo = new SelectMatchTypeComboBox;
    matchTypeCombo->setObjectName(QStringLiteral("matchtype"));
    lay->addWidget(matchTypeCombo);
    connect(matchTypeCombo, &SelectMatchTypeComboBox::valueChanged, this, &SieveConditionCurrentDate::valueChanged);

    SelectDateWidget *dateWidget = new SelectDateWidget;
    connect(dateWidget, &SelectDateWidget::valueChanged, this, &SieveConditionCurrentDate::valueChanged);
    dateWidget->setObjectName(QStringLiteral("datewidget"));
    lay->addWidget(dateWidget);

    return w;
}

QString SieveConditionCurrentDate::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtype"));
    bool isNegative = false;
    const QString matchTypeStr = selectMatchCombobox->code(isNegative);

    const SelectDateWidget *dateWidget = w->findChild<SelectDateWidget *>(QStringLiteral("datewidget"));
    const QString dateWidgetStr = dateWidget->code();

    return AutoCreateScriptUtil::negativeString(isNegative) + QStringLiteral("currentdate %1 %2").arg(matchTypeStr).arg(dateWidgetStr);
}

bool SieveConditionCurrentDate::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionCurrentDate::serverNeedsCapability() const
{
    return QStringLiteral("date");
}

QStringList SieveConditionCurrentDate::needRequires(QWidget *) const
{
    return QStringList() << QStringLiteral("date");
}

QString SieveConditionCurrentDate::help() const
{
    return i18n("The currentdate test is similar to the date test, except that it operates on the current date/time rather than a value extracted from the message header.");
}

bool SieveConditionCurrentDate::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition, QString &error)
{
    int index = 0;
    QString type;
    QString value;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                if (index == 0) {
                    type = e.text();
                } else if (index == 1) {
                    value = e.text();
                } else {
                    tooManyArgument(tagName, index, 2, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveConditionCurrentDate::setParamWidgetValue too many argument :" << index;
                }
                ++index;
            } else if (tagName == QLatin1String("tag")) {
                SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtype"));
                selectMatchCombobox->setCode(AutoCreateScriptUtil::tagValueWithCondition(e.text(), notCondition), name(), error);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << "SieveConditionCurrentDate::setParamWidgetValue unknown tag " << tagName;
            }
        }
        node = node.nextSibling();
    }
    SelectDateWidget *dateWidget = w->findChild<SelectDateWidget *>(QStringLiteral("datewidget"));
    dateWidget->setCode(type, value);
    return true;
}

QUrl SieveConditionCurrentDate::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
