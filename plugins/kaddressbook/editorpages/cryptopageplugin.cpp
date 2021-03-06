/*
    Copyright (c) 2004 Klar�vdalens Datakonsult AB
                  2009 Tobias Koenig <tokoe@kde.org>

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

#include "cryptopageplugin.h"

#include <qplugin.h>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include <kcontacts/addressee.h>
#include <kcombobox.h>
#include <qdebug.h>
#include <QHBoxLayout>
#include <kiconloader.h>
#include <KLocalizedString>

#include "gpgme++/data.h"
#include "gpgme++/key.h"

#include "Libkleo/KeyRequester"
#include "Libkleo/Enum"

using namespace Akonadi;

CryptoPagePlugin::CryptoPagePlugin()
    : mReadOnly(false)
{
    KIconLoader::global()->addAppDir(QStringLiteral("libkleopatra"));

    QGridLayout *topLayout = new QGridLayout(this);
    topLayout->setColumnStretch(1, 1);
    topLayout->setRowStretch(4, 1);

    QGroupBox *protGB = new QGroupBox(i18n("Allowed Protocols"), this);
    QLayout *protGBLayout = new QVBoxLayout;
    topLayout->addWidget(protGB, 0, 0, 1, 2);

    uint msgFormat = 1;
    for (uint i = 0; i < NumberOfProtocols; ++i) {
        Kleo::CryptoMessageFormat f = static_cast<Kleo::CryptoMessageFormat>(msgFormat);
        mProtocolCB[ i ] = new QCheckBox(Kleo::cryptoMessageFormatToLabel(f), protGB);
        protGBLayout->addWidget(mProtocolCB[i]);

        // Iterating over a bitfield means *2 every time
        msgFormat *= 2;
    }
    protGB->setLayout(protGBLayout);

    QLabel *l = new QLabel(i18n("Preferred OpenPGP encryption key:"), this);
    topLayout->addWidget(l, 1, 0);

    mPgpKey = new Kleo::EncryptionKeyRequester(true, Kleo::EncryptionKeyRequester::OpenPGP, this);
    topLayout->addWidget(mPgpKey, 1, 1);

    l = new QLabel(i18n("Preferred S/MIME encryption certificate:"), this);
    topLayout->addWidget(l, 2, 0);

    mSmimeCert = new Kleo::EncryptionKeyRequester(true, Kleo::EncryptionKeyRequester::SMIME, this);
    topLayout->addWidget(mSmimeCert, 2, 1);

    QGroupBox *box = new QGroupBox(i18n("Message Preference"), this);
    QLayout *boxLayout = new QVBoxLayout;
    topLayout->addWidget(box, 3, 0, 1, 2);

    // Send preferences/sign (see kleo/kleo/enum.h)
    QWidget *hbox = new QWidget(box);
    QHBoxLayout *hboxHBoxLayout = new QHBoxLayout(hbox);
    hboxHBoxLayout->setMargin(0);

    l = new QLabel(i18n("Sign:"), hbox);
    hboxHBoxLayout->addWidget(l);

    mSignPref = new KComboBox(hbox);
    hboxHBoxLayout->addWidget(mSignPref);
    l->setBuddy(mSignPref);
    mSignPref->setEditable(false);
    for (unsigned int i = Kleo::UnknownSigningPreference; i < Kleo::MaxSigningPreference; ++i)
        mSignPref->addItem(Kleo::signingPreferenceToLabel(
                               static_cast<Kleo::SigningPreference>(i)));
    boxLayout->addWidget(hbox);

    // Send preferences/encrypt (see kleo/kleo/enum.h)
    hbox = new QWidget(box);
    hboxHBoxLayout = new QHBoxLayout(hbox);
    hboxHBoxLayout->setMargin(0);

    l = new QLabel(i18n("Encrypt:"), hbox);
    hboxHBoxLayout->addWidget(l);

    mCryptPref = new KComboBox(hbox);
    hboxHBoxLayout->addWidget(mCryptPref);
    l->setBuddy(mCryptPref);
    mCryptPref->setEditable(false);
    for (unsigned int i = Kleo::UnknownPreference; i < Kleo::MaxEncryptionPreference; ++i)
        mCryptPref->addItem(Kleo::encryptionPreferenceToLabel(
                                static_cast<Kleo::EncryptionPreference>(i)));
    boxLayout->addWidget(hbox);
    box->setLayout(boxLayout);
}

CryptoPagePlugin::~CryptoPagePlugin()
{
}

QString CryptoPagePlugin::title() const
{
    return i18n("Crypto Settings");
}

void CryptoPagePlugin::loadContact(const KContacts::Addressee &contact)
{
    const QStringList protocolPrefs = contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOPROTOPREF")).split(QLatin1Char(','), QString::SkipEmptyParts);
    const uint cryptoFormats = Kleo::stringListToCryptoMessageFormats(protocolPrefs);

    uint msgFormat = 1;
    for (uint i = 0; i < NumberOfProtocols; ++i, msgFormat *= 2) {
        mProtocolCB[i]->setChecked(cryptoFormats & msgFormat);
    }

    mSignPref->setCurrentIndex(Kleo::stringToSigningPreference(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOSIGNPREF"))));
    mCryptPref->setCurrentIndex(Kleo::stringToEncryptionPreference(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOENCRYPTPREF"))));

    // We don't use the contents of addr->key(...) because we want just a ref.
    // to the key/cert. stored elsewhere.

    mPgpKey->setFingerprints(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("OPENPGPFP")).split(QLatin1Char(','), QString::SkipEmptyParts));
    mSmimeCert->setFingerprints(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("SMIMEFP")).split(QLatin1Char(','), QString::SkipEmptyParts));
}

void CryptoPagePlugin::storeContact(KContacts::Addressee &contact) const
{
    uint cryptoFormats = 0;
    uint msgFormat = 1;
    for (uint i = 0; i < NumberOfProtocols; ++i, msgFormat *= 2) {
        if (mProtocolCB[ i ]->isChecked()) {
            cryptoFormats |= msgFormat;
        }
    }

    const QStringList protocolPref = Kleo::cryptoMessageFormatsToStringList(cryptoFormats);
    if (!protocolPref.isEmpty()) {
        contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOPROTOPREF"), protocolPref.join(QStringLiteral(",")));
    } else {
        contact.removeCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOPROTOPREF"));
    }

    const Kleo::SigningPreference signPref = static_cast<Kleo::SigningPreference>(mSignPref->currentIndex());
    if (signPref != Kleo::UnknownSigningPreference) {
        contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOSIGNPREF"), QLatin1String(Kleo::signingPreferenceToString(signPref)));
    } else {
        contact.removeCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOSIGNPREF"));
    }

    const Kleo::EncryptionPreference encryptPref = static_cast<Kleo::EncryptionPreference>(mCryptPref->currentIndex());
    if (encryptPref != Kleo::UnknownPreference) {
        contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOENCRYPTPREF"), QLatin1String(Kleo::encryptionPreferenceToString(encryptPref)));
    } else {
        contact.removeCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOENCRYPTPREF"));
    }

    const QStringList pfp = mPgpKey->fingerprints();
    const QStringList sfp = mSmimeCert->fingerprints();

    if (!pfp.isEmpty()) {
        contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("OPENPGPFP"), pfp.join(QStringLiteral(",")));
    } else {
        contact.removeCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("OPENPGPFP"));
    }

    if (!sfp.isEmpty()) {
        contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("SMIMEFP"), sfp.join(QStringLiteral(",")));
    } else {
        contact.removeCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("SMIMEFP"));
    }
}

void CryptoPagePlugin::setReadOnly(bool readOnly)
{
    mReadOnly = readOnly;
    for (uint i = 0; i < NumberOfProtocols; ++i) {
        mProtocolCB[ i ]->setEnabled(!readOnly);
    }

    mSignPref->setEnabled(!readOnly);
    mCryptPref->setEnabled(!readOnly);
    mPgpKey->setEnabled(!readOnly);
    mSmimeCert->setEnabled(!readOnly);
}

