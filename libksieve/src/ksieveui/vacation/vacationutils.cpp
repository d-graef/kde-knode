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

#include "vacationutils.h"
#include "vacationscriptextractor.h"
#include "sieve-vacation.h"
#include <KIdentityManagement/IdentityManager>
#include <KIdentityManagement/Identity>

#include <KLocalizedString>
#include <QDate>
#include <QLocale>

using KMime::Types::AddrSpecList;
using namespace KSieveUi;

static inline QString dotstuff(QString s)     // krazy:exclude=passbyvalue
{
    if (s.startsWith(QLatin1Char('.'))) {
        return QLatin1Char('.') + s.replace(QLatin1String("\n."), QStringLiteral("\n.."));
    } else {
        return s.replace(QLatin1String("\n."), QStringLiteral("\n.."));
    }
}

static inline QString stringReplace(QString s)
{
    s = s.replace(QRegExp(QStringLiteral("[\n\t]+")), QStringLiteral(" "));
    return s.replace(QLatin1Char('\"'), QStringLiteral("\\\""));
}

QString VacationUtils::defaultSubject()
{
    return i18n("Out of office till %1", QLocale().toString(QDate::currentDate().addDays(1)));
}

QString VacationUtils::defaultMessageText()
{
    return i18n("I am out of office till %1.\n"
                "\n"
                "In urgent cases, please contact Mrs. \"vacation replacement\"\n"
                "\n"
                "email: \"email address of vacation replacement\"\n"
                "phone: +49 711 1111 11\n"
                "fax.:  +49 711 1111 12\n"
                "\n"
                "Yours sincerely,\n"
                "-- \"enter your name and email address here\"\n",
                QLocale().toString(QDate::currentDate().addDays(1)));
}

int VacationUtils::defaultNotificationInterval()
{
    return 7; // days
}

QStringList VacationUtils::defaultMailAliases()
{
    QStringList sl;
    KIdentityManagement::IdentityManager manager(true);
    KIdentityManagement::IdentityManager::ConstIterator end(manager.end());
    for (KIdentityManagement::IdentityManager::ConstIterator it = manager.begin(); it != end ; ++it) {
        if (!(*it).primaryEmailAddress().isEmpty()) {
            sl.push_back((*it).primaryEmailAddress());
        }
        sl += (*it).emailAliases();
    }
    return sl;
}

bool VacationUtils::defaultSendForSpam()
{
    return VacationSettings::outOfOfficeReactToSpam();
}

QString VacationUtils::defaultDomainName()
{
    return VacationSettings::outOfOfficeDomain();
}

QDate VacationUtils::defaultStartDate()
{
    return QDate::currentDate();
}

QDate VacationUtils::defaultEndDate()
{
    return defaultStartDate().addDays(7);
}

bool VacationUtils::parseScript(const QString &script, QString &messageText,
        QString &subject,
        int &notificationInterval, QStringList &aliases,
        bool &sendForSpam, QString &domainName,
        QDate &startDate, QDate &endDate)
{
    if (script.trimmed().isEmpty()) {
        messageText = VacationUtils::defaultMessageText();
        subject = VacationUtils::defaultSubject();
        notificationInterval = VacationUtils::defaultNotificationInterval();
        aliases = VacationUtils::defaultMailAliases();
        sendForSpam = VacationUtils::defaultSendForSpam();
        domainName = VacationUtils::defaultDomainName();
        return true;
    }

    // The trimmed() call below prevents parsing errors. The
    // slave somehow omits the last \n, which results in a lone \r at
    // the end, leading to a parse error.
    const QByteArray scriptUTF8 = script.trimmed().toUtf8();
    qCDebug(LIBKSIEVE_LOG) << "scriptUtf8 = \"" + scriptUTF8 + "\"";
    KSieve::Parser parser(scriptUTF8.begin(),
                          scriptUTF8.begin() + scriptUTF8.length());
    VacationDataExtractor vdx;
    SpamDataExtractor sdx;
    DomainRestrictionDataExtractor drdx;
    DateExtractor dtx;
    KSieveExt::MultiScriptBuilder tsb(&vdx, &sdx, &drdx, &dtx);
    parser.setScriptBuilder(&tsb);
    if (!parser.parse()) {
        return false;
    }
    messageText = vdx.messageText().trimmed();
    if (!vdx.subject().isEmpty()) {
        subject = vdx.subject().trimmed();
    }
    notificationInterval = vdx.notificationInterval();
    aliases = vdx.aliases();
    if (!VacationSettings::allowOutOfOfficeUploadButNoSettings()) {
        sendForSpam = !sdx.found();
        domainName = drdx.domainName();
    }
    startDate = dtx.startDate();
    endDate = dtx.endDate();
    return true;
}

QString VacationUtils::composeScript(const QString &messageText,
        const QString &subject,
        int notificationInterval,
        const AddrSpecList &addrSpecs,
        bool sendForSpam, const QString &domain,
        const QDate &startDate, const QDate &endDate)
{
    QString addressesArgument;
    QStringList aliases;
    if (!addrSpecs.empty()) {
        addressesArgument += QLatin1String(":addresses [ ");
        QStringList sl;
        AddrSpecList::const_iterator end = addrSpecs.constEnd();
        for (AddrSpecList::const_iterator it = addrSpecs.begin() ; it != end; ++it) {
            sl.push_back(QLatin1Char('"') + (*it).asString().replace(QLatin1Char('\\'), QStringLiteral("\\\\")).replace(QLatin1Char('"'), QStringLiteral("\\\"")) + QLatin1Char('"'));
            aliases.push_back((*it).asString());
        }
        addressesArgument += sl.join(QStringLiteral(", ")) + QLatin1String(" ] ");
    }

    QString script = QStringLiteral("require \"vacation\";\n");
    if (startDate.isValid() && endDate.isValid()) {
        script += QStringLiteral("require \"relational\";\n"
                                 "require \"date\";\n\n");
    } else {
        script += QStringLiteral("\n");
    }

    if (!sendForSpam)
        script += QStringLiteral("if header :contains \"X-Spam-Flag\" \"YES\""
                                 " { keep; stop; }\n");  // FIXME?

    if (!domain.isEmpty()) { // FIXME
        script += QStringLiteral("if not address :domain :contains \"from\" \"%1\" { keep; stop; }\n").arg(domain);
    }

    if (startDate.isValid() && endDate.isValid()) {
        script += QStringLiteral("if not allof(currentdate :value \"ge\" \"date\" \"%1\","
                                 " currentdate :value \"le\" \"date\" \"%2\")"
                                 " { keep; stop; }\n").arg(startDate.toString(Qt::ISODate),
                                         endDate.toString(Qt::ISODate));
    }

    script += QLatin1String("vacation ");
    script += addressesArgument;
    if (notificationInterval > 0) {
        script += QStringLiteral(":days %1 ").arg(notificationInterval);
    }

    if (!subject.trimmed().isEmpty()) {
        script += QStringLiteral(":subject \"%1\" ").arg(stringReplace(subject).trimmed());
    }

    script += QStringLiteral("text:\n");
    script += dotstuff(messageText.isEmpty() ? VacationUtils::defaultMessageText() : messageText);
    script += QStringLiteral("\n.\n;\n");
    return script;
}
