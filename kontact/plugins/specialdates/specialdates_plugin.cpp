/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2004-2005,2009 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "specialdates_plugin.h"
#include "sdsummarywidget.h"

#include <KontactInterface/Core>

#include <KAboutData>
#include <KLocalizedString>
#include <KIconLoader>

EXPORT_KONTACT_PLUGIN( SpecialdatesPlugin, specialdates )

SpecialdatesPlugin::SpecialdatesPlugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, 0 ), mAboutData( 0 )
{
  //QT5 setComponentData( KontactPluginFactory::componentData() );
  KIconLoader::global()->addAppDir( QLatin1String("kdepim") );
}

SpecialdatesPlugin::~SpecialdatesPlugin()
{
}

KontactInterface::Summary *SpecialdatesPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new SDSummaryWidget( this, parentWidget );
}

const KAboutData *SpecialdatesPlugin::aboutData() const
{
  if ( !mAboutData ) {
    mAboutData = new KAboutData( QLatin1String("specialdates"),
                                 i18n( "Special Dates Summary" ),
                                 QLatin1String("1.0"),
                                 i18n( "Kontact Special Dates Summary" ),
                                 KAboutLicense::LGPL,
                                 i18n( "Copyright © 2003 Tobias Koenig\n"
                                        "Copyright © 2004–2010 Allen Winter" ) );
    mAboutData->addAuthor( i18n( "Allen Winter" ),
                           i18n( "Current Maintainer" ), QLatin1String("winter@kde.org") );
    mAboutData->addAuthor( i18n( "Tobias Koenig" ),
                           QString(), QLatin1String("tokoe@kde.org") );
    mAboutData->setProductName( "kontact/specialdates");
  }
  return mAboutData;
}
#include "specialdates_plugin.moc"
