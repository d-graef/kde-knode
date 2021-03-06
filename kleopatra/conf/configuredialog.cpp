/*
    configuredialog.cpp

    This file is part of kleopatra
    Copyright (C) 2000 Espen Sand, espen@kde.org
    Copyright (C) 2001-2002 Marc Mutz <mutz@kde.org>
    Copyright (c) 2004,2008 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
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

#include "configuredialog.h"
#include <kwindowsystem.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kcmultidialog.h>
#include <KLocalizedString>
#include <kconfiggroup.h>
#include <QApplication>
#include <KSharedConfig>
#include <QIcon>

#ifdef KLEO_STATIC_KCMODULES
# include <KDesktopFile>
# define KCM_IMPORT_PLUGIN( x ) extern "C" KCModule * create_##x( QWidget * parent = Q_NULLPTR, const QVariantList & args=QVariantList() );
# define addMyModule( x ) addModule( KCModuleInfo( KDesktopFile( "services", QLatin1String(#x) + QLatin1String(".desktop") ) ), create_##x() )
#else // KLEO_STATIC_KCMODULES
# define KCM_IMPORT_PLUGIN( x )
# define addMyModule( x ) addModule( QLatin1String(#x) )
#endif // KLEO_STATIC_KCMODULES

KCM_IMPORT_PLUGIN(kleopatra_config_dirserv)
KCM_IMPORT_PLUGIN(kleopatra_config_appear)
#ifdef HAVE_KLEOPATRACLIENT_LIBRARY
KCM_IMPORT_PLUGIN(kleopatra_config_cryptooperations)
KCM_IMPORT_PLUGIN(kleopatra_config_smimevalidation)
#endif
KCM_IMPORT_PLUGIN(kleopatra_config_gnupgsystem)

ConfigureDialog::ConfigureDialog(QWidget *parent)
    : KCMultiDialog(parent)
{
    setFaceType(KPageDialog::List);
    setWindowTitle(i18n("Configure"));
    KWindowSystem::setIcons(winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop)),
                            qApp->windowIcon().pixmap(IconSize(KIconLoader::Small), IconSize(KIconLoader::Small)));
    //QT5 showButton( User1, true );

    addMyModule(kleopatra_config_dirserv);
    addMyModule(kleopatra_config_appear);
#ifdef HAVE_KLEOPATRACLIENT_LIBRARY
    addMyModule(kleopatra_config_cryptooperations);
    addMyModule(kleopatra_config_smimevalidation);
#endif
    addMyModule(kleopatra_config_gnupgsystem);

    // We store the minimum size of the dialog on hide, because otherwise
    // the KCMultiDialog starts with the size of the first kcm, not
    // the largest one. This way at least after the first showing of
    // the largest kcm the size is kept.
    const KConfigGroup geometry(KSharedConfig::openConfig(), "Geometry");
    const int width = geometry.readEntry("ConfigureDialogWidth", 0);
    const int height = geometry.readEntry("ConfigureDialogHeight", 0);
    if (width != 0 && height != 0) {
        setMinimumSize(width, height);
    }
}

void ConfigureDialog::hideEvent(QHideEvent *e)
{
    const QSize minSize = minimumSizeHint();
    KConfigGroup geometry(KSharedConfig::openConfig(), "Geometry");
    geometry.writeEntry("ConfigureDialogWidth", minSize.width());
    geometry.writeEntry("ConfigureDialogHeight", minSize.height());
    KCMultiDialog::hideEvent(e);
}

ConfigureDialog::~ConfigureDialog()
{
}

#undef addMyModule
#undef KCM_IMPORT_PLUGIN

