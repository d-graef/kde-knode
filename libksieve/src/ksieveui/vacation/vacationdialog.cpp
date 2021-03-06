/*
    vacationdialog.cpp

    Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "vacationdialog.h"
#include "vacationeditwidget.h"

#include "libksieve_debug.h"
#include <kiconloader.h>
#include <kmime/kmime_header_parsing.h>
#include <kwindowsystem.h>
#include <KSeparator>

#include <QApplication>
#include <QVBoxLayout>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
using KMime::Types::AddrSpecList;

using namespace KSieveUi;

VacationDialog::VacationDialog(const QString &caption, QWidget *parent,
                               bool modal)
    : QDialog(parent)
{
    setWindowTitle(caption);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &VacationDialog::slotAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &VacationDialog::slotRejected);
    okButton->setDefault(true);
    setModal(modal);
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(0);
    w->setLayout(vbox);
    vbox->addWidget(mVacationEditWidget);

    mVacationEditWidget = new VacationEditWidget;

    KSeparator *separator = new KSeparator;
    vbox->addWidget(separator);

    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);

    KWindowSystem::setIcons(winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small), IconSize(KIconLoader::Small)));
    readConfig();
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &VacationDialog::slotDialogDefaults);
}

VacationDialog::~VacationDialog()
{
    qCDebug(LIBKSIEVE_LOG) << "~VacationDialog()";
    writeConfig();
}

void VacationDialog::slotAccepted()
{
    Q_EMIT okClicked();
}

void VacationDialog::slotRejected()
{
    Q_EMIT cancelClicked();
}

void VacationDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "VacationDialog");
    group.writeEntry("Size", size());
}

void VacationDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "VacationDialog");
    const QSize size = group.readEntry("Size", QSize());
    if (size.isValid()) {
        resize(size);
    } else {
        resize(sizeHint().width(), sizeHint().height());
    }
}

bool VacationDialog::activateVacation() const
{
    return mVacationEditWidget->activateVacation();
}

void VacationDialog::setActivateVacation(bool activate)
{
    mVacationEditWidget->setActivateVacation(activate);
}

QString VacationDialog::messageText() const
{
    return mVacationEditWidget->messageText();
}

void VacationDialog::setMessageText(const QString &text)
{
    mVacationEditWidget->setMessageText(text);
}

QString VacationDialog::subject() const
{
    return mVacationEditWidget->subject();
}

void VacationDialog::setSubject(const QString &subject)
{
    return mVacationEditWidget->setSubject(subject);
}

VacationUtils::MailAction VacationDialog::mailAction() const
{
    return mVacationEditWidget->mailAction();
}

QString VacationDialog::mailActionRecipient() const
{
    return mVacationEditWidget->mailActionRecipient();
}

void VacationDialog::setMailAction(VacationUtils::MailAction action, const QString &recipient)
{
    mVacationEditWidget->setMailAction(action, recipient);
}

int VacationDialog::notificationInterval() const
{
    return mVacationEditWidget->notificationInterval();
}

void VacationDialog::setNotificationInterval(int days)
{
    mVacationEditWidget->setNotificationInterval(days);
}

AddrSpecList VacationDialog::mailAliases() const
{
    return mVacationEditWidget->mailAliases();
}

void VacationDialog::setMailAliases(const AddrSpecList &aliases)
{
    mVacationEditWidget->setMailAliases(aliases);
}

void VacationDialog::setMailAliases(const QString &aliases)
{
    mVacationEditWidget->setMailAliases(aliases);
}

QString VacationDialog::domainName() const
{
    return mVacationEditWidget->domainName();
}

void VacationDialog::setDomainName(const QString &domain)
{
    mVacationEditWidget->setDomainName(domain);
}

bool VacationDialog::domainCheck() const
{
    return mVacationEditWidget->domainCheck();
}

void VacationDialog::setDomainCheck(bool check)
{
    mVacationEditWidget->setDomainCheck(check);
}

bool VacationDialog::sendForSpam() const
{
    return mVacationEditWidget->sendForSpam();
}

void VacationDialog::setSendForSpam(bool enable)
{
    mVacationEditWidget->setSendForSpam(enable);
}

void VacationDialog::enableDomainAndSendForSpam(bool enable)
{
    mVacationEditWidget->enableDomainAndSendForSpam(enable);
}

void VacationDialog::slotDialogDefaults()
{
    mVacationEditWidget->setDefault();
}

void VacationDialog::enableDates(bool enable)
{
    mVacationEditWidget->enableDates(enable);
}

QDate VacationDialog::endDate() const
{
    return mVacationEditWidget->endDate();
}

void VacationDialog::setEndDate(const QDate &endDate)
{
    mVacationEditWidget->setEndDate(endDate);
}

QTime VacationDialog::endTime() const
{
    return mVacationEditWidget->endTime();
}

void VacationDialog::setEndTime(const QTime &endTime)
{
    mVacationEditWidget->setEndTime(endTime);
}

QDate VacationDialog::startDate() const
{
    return mVacationEditWidget->startDate();
}

void VacationDialog::setStartDate(const QDate &startDate)
{
    mVacationEditWidget->setStartDate(startDate);
}

QTime VacationDialog::startTime() const
{
    return mVacationEditWidget->startTime();
}

void VacationDialog::setStartTime(const QTime &startTime)
{
    mVacationEditWidget->setStartTime(startTime);
}
