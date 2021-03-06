/*
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "freebusyurldialog.h"

#include <KLineEdit>
#include <KLocalizedString>

#include <QBoxLayout>
#include "incidenceeditor_debug.h"
#include <QFrame>
#include <QLabel>
#include <KConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStandardPaths>

using namespace IncidenceEditorNG;

FreeBusyUrlDialog::FreeBusyUrlDialog(const AttendeeData::Ptr &attendee, QWidget *parent)
    : QDialog(parent)
{
    QFrame *topFrame = new QFrame(this);
    setModal(true);
    setWindowTitle(i18n("Edit Free/Busy Location"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(topFrame);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FreeBusyUrlDialog::reject);
    mainLayout->addWidget(buttonBox);
    okButton->setDefault(true);

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setMargin(0);

    mWidget = new FreeBusyUrlWidget(attendee, topFrame);
    topLayout->addWidget(mWidget);

    mWidget->loadConfig();
    connect(okButton, &QPushButton::clicked, this, &FreeBusyUrlDialog::slotOk);
}

void FreeBusyUrlDialog::slotOk()
{
    mWidget->saveConfig();
    accept();
}

FreeBusyUrlWidget::FreeBusyUrlWidget(const AttendeeData::Ptr &attendee, QWidget *parent)
    : QWidget(parent), mAttendee(attendee)
{
    QBoxLayout *topLayout = new QVBoxLayout(this);

    QLabel *label =
        new QLabel(xi18n("Location of Free/Busy information for %1 <placeholder>%2</placeholder>:",
                         mAttendee->name(), mAttendee->email()), this);
    topLayout->addWidget(label);

    mUrlEdit = new KLineEdit(this);
    mUrlEdit->setFocus();
    topLayout->addWidget(mUrlEdit);
}

FreeBusyUrlWidget::~FreeBusyUrlWidget()
{
}

static QString freeBusyUrlStore()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QStringLiteral("korganizer/freebusyurls");
}

void FreeBusyUrlWidget::loadConfig()
{
    qCDebug(INCIDENCEEDITOR_LOG);

    KConfig config(freeBusyUrlStore());
    mUrlEdit->setText(config.group(mAttendee->email()).readEntry("url"));
}

void FreeBusyUrlWidget::saveConfig()
{
    qCDebug(INCIDENCEEDITOR_LOG);

    const QString url = mUrlEdit->text();
    KConfig config(freeBusyUrlStore());
    config.group(mAttendee->email()).writeEntry("url", url);
}

