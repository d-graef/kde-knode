/*
    main.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004,2008 Klar�vdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "aboutdata.h"
#include "kleopatraapplication.h"
#include "mainwindow.h"

#include <kdelibs4configmigrator.h>

#include <commands/reloadkeyscommand.h>
#include <commands/selftestcommand.h>

#include <utils/gnupg-helper.h>
#include <utils/archivedefinition.h>

#ifdef HAVE_USABLE_ASSUAN
# include <uiserver/uiserver.h>
# include <uiserver/assuancommand.h>
# include <uiserver/echocommand.h>
# include <uiserver/decryptcommand.h>
# include <uiserver/verifycommand.h>
# include <uiserver/decryptverifyfilescommand.h>
# include <uiserver/decryptfilescommand.h>
# include <uiserver/verifyfilescommand.h>
# include <uiserver/prepencryptcommand.h>
# include <uiserver/prepsigncommand.h>
# include <uiserver/encryptcommand.h>
# include <uiserver/signcommand.h>
# include <uiserver/signencryptfilescommand.h>
# include <uiserver/selectcertificatecommand.h>
# include <uiserver/importfilescommand.h>
# include <uiserver/createchecksumscommand.h>
# include <uiserver/verifychecksumscommand.h>
#else
namespace Kleo
{
class UiServer;
}
#endif

#include <Libkleo/ChecksumDefinition>

#include "kleopatra_debug.h"
#include "kleopatra_options.h"

#include <KDBusService>

#include <KLocalizedString>
#include <kiconloader.h>
#include <QSplashScreen>
#include <kmessagebox.h>

#include <QTextDocument> // for Qt::escape
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <QEventLoop>
#include <QThreadPool>

#include <gpgme++/global.h>
#include <gpgme++/error.h>

#include <boost/shared_ptr.hpp>

#include <cassert>
#include <iostream>
#include <QCommandLineParser>

using namespace boost;

static const int SPLASHSCREEN_TIMEOUT = 5000; // 5s

namespace
{
template <typename T>
boost::shared_ptr<T> make_shared_ptr(T *t)
{
    return t ? boost::shared_ptr<T>(t) : boost::shared_ptr<T>();
}
}

static QPixmap UserIcon_nocached(const char *name)
{
    // KIconLoader insists on caching all pixmaps. Since the splash
    // screen is a particularly large 'icon' and used only once,
    // caching is unneccesary and just hurts startup performance.
    KIconLoader *const il = KIconLoader::global();
    assert(il);
    const QString iconPath = il->iconPath(QLatin1String(name), KIconLoader::User);
    return iconPath.isEmpty() ? il->unknown() : QPixmap(iconPath);
}

#ifndef QT_NO_SPLASHSCREEN
class SplashScreen : public QSplashScreen
{
    QBasicTimer m_timer;
public:
    SplashScreen()
        : QSplashScreen(UserIcon_nocached("kleopatra_splashscreen"), Qt::WindowStaysOnTopHint),
          m_timer()
    {
        m_timer.start(SPLASHSCREEN_TIMEOUT, this);
    }

protected:
    void timerEvent(QTimerEvent *ev) Q_DECL_OVERRIDE {
        if (ev->timerId() == m_timer.timerId())
        {
            m_timer.stop();
            hide();
        } else {
            QSplashScreen::timerEvent(ev);
        }
    }

};
#else
class SplashScreen {};
#endif // QT_NO_SPLASHSCREEN

static bool selfCheck(SplashScreen &splash)
{
#ifndef QT_NO_SPLASHSCREEN
    splash.showMessage(i18n("Performing Self-Check..."));
#endif
    Kleo::Commands::SelfTestCommand cmd(0);
    cmd.setAutoDelete(false);
    cmd.setAutomaticMode(true);
#ifndef QT_NO_SPLASHSCREEN
    cmd.setSplashScreen(&splash);
#endif
    QEventLoop loop;
    QObject::connect(&cmd, &Kleo::Commands::SelfTestCommand::finished, &loop, &QEventLoop::quit);
#ifndef QT_NO_SPLASHSCREEN
    QObject::connect(&cmd, SIGNAL(info(QString)), &splash, SLOT(showMessage(QString)));
#endif
    QTimer::singleShot(0, &cmd, &Kleo::Command::start);   // start() may Q_EMIT finished()...
    loop.exec();
    if (cmd.isCanceled()) {
#ifndef QT_NO_SPLASHSCREEN
        splash.showMessage(i18nc("did not pass", "Self-Check Failed"));
#endif
        return false;
    } else {
#ifndef QT_NO_SPLASHSCREEN
        splash.showMessage(i18n("Self-Check Passed"));
#endif
        return true;
    }
}

static void fillKeyCache(SplashScreen *splash, Kleo::UiServer *server)
{

    QEventLoop loop;
    Kleo::ReloadKeysCommand *cmd = new Kleo::ReloadKeysCommand(0);
    QObject::connect(cmd, &Kleo::Commands::SelfTestCommand::finished, &loop, &QEventLoop::quit);
#ifdef HAVE_USABLE_ASSUAN
    QObject::connect(cmd, SIGNAL(finished()), server, SLOT(enableCryptoCommands()));
#else
    Q_UNUSED(server);
#endif
#ifndef QT_NO_SPLASHSCREEN
    splash->showMessage(i18n("Loading certificate cache..."));
#else
    Q_UNUSED(splash);
#endif
    cmd->start();
    loop.exec();
#ifndef QT_NO_SPLASHSCREEN
    splash->showMessage(i18n("Certificate cache loaded."));
#endif
}

int main(int argc, char **argv)
{
    KleopatraApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QTime timer;
    timer.start();

    KLocalizedString::setApplicationDomain("kleopatra");

    KDBusService service(KDBusService::Unique);
    QObject::connect(&service, &KDBusService::activateRequested,
                     &app, &KleopatraApplication::slotActivateRequested);
    QObject::connect(&app, &KleopatraApplication::setExitValue,
    &service, [&service](int i) {
        service.setExitValue(i);
    });

    AboutData aboutData;

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    kleopatra_options(&parser);

    parser.process(QApplication::arguments());
    aboutData.processCommandLine(&parser);

    Kdelibs4ConfigMigrator migrate(QStringLiteral("kleopatra"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kleopatrarc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("kleopatra.rc"));
    migrate.migrate();

    qCDebug(KLEOPATRA_LOG) << "Startup timing:" << timer.elapsed() << "ms elapsed: Application created";

    // Initialize GpgME
    const GpgME::Error gpgmeInitError = GpgME::initializeLibrary(0);

    {
        const unsigned int threads = QThreadPool::globalInstance()->maxThreadCount();
        QThreadPool::globalInstance()->setMaxThreadCount(qMax(2U, threads));
    }
    if (gpgmeInitError) {
        KMessageBox::sorry(0, xi18nc("@info",
                                     "<para>The version of the <application>GpgME</application> library you are running against "
                                     "is older than the one that the <application>GpgME++</application> library was built against.</para>"
                                     "<para><application>Kleopatra</application> will not function in this setting.</para>"
                                     "<para>Please ask your administrator for help in resolving this issue.</para>"),
                           i18nc("@title", "GpgME Too Old"));
        return EXIT_FAILURE;
    }

    SplashScreen splash;

    const QString installPath = Kleo::gpg4winInstallPath();
    Kleo::ChecksumDefinition::setInstallPath(installPath);
    Kleo::ArchiveDefinition::setInstallPath(installPath);

    int rc;
#ifdef HAVE_USABLE_ASSUAN
    try {
        Kleo::UiServer server(parser.value(QStringLiteral("uiserver-socket")));

        qCDebug(KLEOPATRA_LOG) << "Startup timing:" << timer.elapsed() << "ms elapsed: UiServer created";

        QObject::connect(&server, &Kleo::UiServer::startKeyManagerRequested, &app, &KleopatraApplication::openOrRaiseMainWindow);

        QObject::connect(&server, &Kleo::UiServer::startConfigDialogRequested, &app, &KleopatraApplication::openOrRaiseConfigDialog);

#define REGISTER( Command ) server.registerCommandFactory( boost::shared_ptr<Kleo::AssuanCommandFactory>( new Kleo::GenericAssuanCommandFactory<Kleo::Command> ) )
        REGISTER(CreateChecksumsCommand);
        REGISTER(DecryptCommand);
        REGISTER(DecryptFilesCommand);
        REGISTER(DecryptVerifyFilesCommand);
        REGISTER(EchoCommand);
        REGISTER(EncryptCommand);
        REGISTER(EncryptFilesCommand);
        REGISTER(EncryptSignFilesCommand);
        REGISTER(ImportFilesCommand);
        REGISTER(PrepEncryptCommand);
        REGISTER(PrepSignCommand);
        REGISTER(SelectCertificateCommand);
        REGISTER(SignCommand);
        REGISTER(SignEncryptFilesCommand);
        REGISTER(SignFilesCommand);
#ifndef QT_NO_DIRMODEL
        REGISTER(VerifyChecksumsCommand);
#endif // QT_NO_DIRMODEL
        REGISTER(VerifyCommand);
        REGISTER(VerifyFilesCommand);
#undef REGISTER

        server.start();
        qCDebug(KLEOPATRA_LOG) << "Startup timing:" << timer.elapsed() << "ms elapsed: UiServer started";
#endif

        const bool daemon = parser.isSet(QStringLiteral("daemon"));
        if (!daemon && app.isSessionRestored()) {
            app.restoreMainWindow();
        }

#ifndef QT_NO_SPLASHSCREEN
        // Don't show splash screen if daemon or session restore
        if (!(daemon || app.isSessionRestored())) {
            splash.show();
        }
#endif
        if (!selfCheck(splash)) {
            return 1;
        }
        qCDebug(KLEOPATRA_LOG) << "Startup timing:" << timer.elapsed() << "ms elapsed: SelfCheck completed";

#ifdef HAVE_USABLE_ASSUAN
        fillKeyCache(&splash, &server);
#else
        fillKeyCache(&splash, 0);
#endif
        qCDebug(KLEOPATRA_LOG) << "Startup timing:" << timer.elapsed() << "ms elapsed: KeyCache loaded";

#ifndef QT_NO_SYSTEMTRAYICON
        app.startMonitoringSmartCard();
#endif

        app.setIgnoreNewInstance(false);

        if (!daemon) {
            const QString err = app.newInstance(parser);
            if (!err.isEmpty()) {
                std::cerr << i18n("Invalid arguments: %1", err).toLocal8Bit().constData() << "\n";
                exit(1);
            }
            qCDebug(KLEOPATRA_LOG) << "Startup timing:" << timer.elapsed() << "ms elapsed: new instance created";
#ifndef QT_NO_SPLASHSCREEN
            splash.finish(app.mainWindow());
#endif // QT_NO_SPLASHSCREEN
        }

        rc = app.exec();

#ifdef HAVE_USABLE_ASSUAN
        app.setIgnoreNewInstance(true);
        QObject::disconnect(&server, &Kleo::UiServer::startKeyManagerRequested, &app, &KleopatraApplication::openOrRaiseMainWindow);
        QObject::disconnect(&server, &Kleo::UiServer::startConfigDialogRequested, &app, &KleopatraApplication::openOrRaiseConfigDialog);

        server.stop();
        server.waitForStopped();
    } catch (const std::exception &e) {
        QMessageBox::information(0, i18n("GPG UI Server Error"),
                                 i18n("<qt>The Kleopatra GPG UI Server Module could not be initialized.<br/>"
                                      "The error given was: <b>%1</b><br/>"
                                      "You can use Kleopatra as a certificate manager, but cryptographic plugins that "
                                      "rely on a GPG UI Server being present might not work correctly, or at all.</qt>",
                                      QString::fromUtf8(e.what()).toHtmlEscaped()));
#ifndef QT_NO_SYSTEMTRAYICON
        app.startMonitoringSmartCard();
#endif
        app.setIgnoreNewInstance(false);
        rc = app.exec();
        app.setIgnoreNewInstance(true);
    }
#endif

    return rc;
}
