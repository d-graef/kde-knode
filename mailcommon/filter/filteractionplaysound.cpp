/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionplaysound.h"

#include "dialog/filteractionmissingargumentdialog.h"

//QT5 #include <phonon/mediaobject.h>

#include <KLocale>

#include <QtCore/QFile>
#include <QPointer>

using namespace MailCommon;

FilterActionPlaySound::FilterActionPlaySound( )
    : FilterActionWithTest( QLatin1String("play sound"), i18n( "Play Sound" ) )
    , mPlayer( 0 )
{
}

FilterActionPlaySound::~FilterActionPlaySound()
{
    delete mPlayer;
}

FilterAction* FilterActionPlaySound::newAction()
{
    return new FilterActionPlaySound();
}

FilterAction::ReturnCode FilterActionPlaySound::process(ItemContext& , bool) const
{
    if ( mParameter.isEmpty() )
        return ErrorButGoOn;
#if 0 //QT5
    if ( !mPlayer )
        mPlayer = Phonon::createPlayer( Phonon::NotificationCategory );

    mPlayer->setCurrentSource( mParameter );
    mPlayer->play();
#endif
    return GoOn;
}

SearchRule::RequiredPart FilterActionPlaySound::requiredPart() const
{
    return SearchRule::Envelope;
}


bool FilterActionPlaySound::argsFromStringInteractive( const QString &argsStr, const QString &filterName )
{
    bool needUpdate = false;
    argsFromString( argsStr );
    if (!QFile(mParameter).exists()){
        QPointer<FilterActionMissingSoundUrlDialog> dlg = new FilterActionMissingSoundUrlDialog( filterName, argsStr );
        if ( dlg->exec() ) {
            mParameter = dlg->soundUrl();
            needUpdate = true;
        }
        delete dlg;
    }
    return needUpdate;
}

