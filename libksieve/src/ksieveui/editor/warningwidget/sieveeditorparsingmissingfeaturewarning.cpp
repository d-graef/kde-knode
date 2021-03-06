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

#include "sieveeditorparsingmissingfeaturewarning.h"
#include "autocreatescripts/sievescriptparsingerrordialog.h"

#include <KLocalizedString>
#include <QAction>

#include <QPointer>

using namespace KSieveUi;

SieveEditorParsingMissingFeatureWarning::SieveEditorParsingMissingFeatureWarning(TextEditorType type, QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setCloseButtonVisible(false);
    setMessageType(Error);
    setText(i18n("Some errors were found during parsing. <a href=\"sieveerrordetails\">(Details...)</a>"));
    connect(this, &SieveEditorParsingMissingFeatureWarning::linkActivated, this, &SieveEditorParsingMissingFeatureWarning::slotShowDetails);

    switch (type) {
    case TextEditor: {
        QAction *action = new QAction(i18n("Switch in graphical mode"), this);
        connect(action, &QAction::triggered, this, &SieveEditorParsingMissingFeatureWarning::slotSwitchInGraphicalMode);
        addAction(action);

        action = new QAction(i18n("Keep in text mode"), this);
        connect(action, &QAction::triggered, this, &SieveEditorParsingMissingFeatureWarning::slotInActualMode);
        addAction(action);
        break;
    }
    case GraphicEditor: {
        QAction *action = new QAction(i18n("Switch in text mode"), this);
        connect(action, &QAction::triggered, this, &SieveEditorParsingMissingFeatureWarning::slotSwitchInTextMode);
        addAction(action);

        action = new QAction(i18n("Keep in Graphical mode"), this);
        connect(action, &QAction::triggered, this, &SieveEditorParsingMissingFeatureWarning::slotInActualMode);
        addAction(action);
        break;
    }
    }

    setWordWrap(true);
}

SieveEditorParsingMissingFeatureWarning::~SieveEditorParsingMissingFeatureWarning()
{
}

QString SieveEditorParsingMissingFeatureWarning::initialScript() const
{
    return mScript;
}

void SieveEditorParsingMissingFeatureWarning::slotShowDetails(const QString &content)
{
    if (content == QLatin1String("sieveerrordetails")) {
        QPointer<SieveScriptParsingErrorDialog> dlg = new SieveScriptParsingErrorDialog(this);
        dlg->setError(mScript, mErrors);
        dlg->exec();
        delete dlg;
    }
}

void SieveEditorParsingMissingFeatureWarning::setErrors(const QString &initialScript, const QString &errors)
{
    mErrors = errors;
    mScript = initialScript;
}

void SieveEditorParsingMissingFeatureWarning::slotSwitchInTextMode()
{
    Q_EMIT switchToTextMode();
    setVisible(false);
}

void SieveEditorParsingMissingFeatureWarning::slotSwitchInGraphicalMode()
{
    Q_EMIT switchToGraphicalMode();
    setVisible(false);
}

void SieveEditorParsingMissingFeatureWarning::slotInActualMode()
{
    setVisible(false);
}

