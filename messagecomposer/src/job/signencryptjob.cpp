/*
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "job/signencryptjob.h"

#include "contentjobbase_p.h"
#include "Libkleo/CryptoBackendFactory"
#include "Libkleo/CryptoBackend"
#include "Libkleo/Enum"
#include "Libkleo/SignEncryptJob"
#include "utils/util.h"

#include "messagecomposer_debug.h"
#include <kmime/kmime_message.h>
#include <kmime/kmime_content.h>
#include <kmime/kmime_headers.h>

#include <gpgme++/global.h>
#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>
#include <sstream>

using namespace MessageComposer;

class MessageComposer::SignEncryptJobPrivate : public ContentJobBasePrivate
{
public:
    SignEncryptJobPrivate(SignEncryptJob *qq)
        : ContentJobBasePrivate(qq)
        , content(Q_NULLPTR)
    {
    }

    KMime::Content *content;
    std::vector<GpgME::Key> signers;
    Kleo::CryptoMessageFormat format;

    std::vector<GpgME::Key> encKeys;
    QStringList recipients;

    // copied from messagecomposer.cpp
    bool binaryHint(Kleo::CryptoMessageFormat f)
    {
        switch (f) {
        case Kleo::SMIMEFormat:
        case Kleo::SMIMEOpaqueFormat:
            return true;
        default:
        case Kleo::OpenPGPMIMEFormat:
        case Kleo::InlineOpenPGPFormat:
            return false;
        }
    }

    Q_DECLARE_PUBLIC(SignEncryptJob)
};

SignEncryptJob::SignEncryptJob(QObject *parent)
    : ContentJobBase(*new SignEncryptJobPrivate(this), parent)
{
}

SignEncryptJob::~SignEncryptJob()
{
}

void SignEncryptJob::setContent(KMime::Content *content)
{
    Q_D(SignEncryptJob);

    Q_ASSERT(content);

    d->content = content;
}

void SignEncryptJob::setCryptoMessageFormat(Kleo::CryptoMessageFormat format)
{
    Q_D(SignEncryptJob);

    // There *must* be a concrete format set at this point.
    Q_ASSERT(format == Kleo::OpenPGPMIMEFormat
             || format == Kleo::InlineOpenPGPFormat
             || format == Kleo::SMIMEFormat
             || format == Kleo::SMIMEOpaqueFormat);
    d->format = format;
}

void SignEncryptJob::setSigningKeys(std::vector<GpgME::Key> &signers)
{
    Q_D(SignEncryptJob);

    d->signers = signers;
}

KMime::Content *SignEncryptJob::origContent()
{
    Q_D(SignEncryptJob);

    return d->content;

}

void SignEncryptJob::setEncryptionKeys(const std::vector<GpgME::Key> &keys)
{
    Q_D(SignEncryptJob);

    d->encKeys = keys;
}

void SignEncryptJob::setRecipients(const QStringList &recipients)
{
    Q_D(SignEncryptJob);

    d->recipients = recipients;
}

QStringList SignEncryptJob::recipients() const
{
    Q_D(const SignEncryptJob);

    return d->recipients;
}

std::vector<GpgME::Key> SignEncryptJob::encryptionKeys() const
{
    Q_D(const SignEncryptJob);

    return d->encKeys;
}

void SignEncryptJob::process()
{
    Q_D(SignEncryptJob);
    Q_ASSERT(d->resultContent == 0);   // Not processed before.

    // if setContent hasn't been called, we assume that a subjob was added
    // and we want to use that
    if (!d->content || !d->content->hasContent()) {
        Q_ASSERT(d->subjobContents.size() == 1);
        d->content = d->subjobContents.first();
    }

    const Kleo::CryptoBackend::Protocol *proto = Q_NULLPTR;
    if (d->format & Kleo::AnyOpenPGP) {
        proto = Kleo::CryptoBackendFactory::instance()->openpgp();
    } else if (d->format & Kleo::AnySMIME) {
        proto = Kleo::CryptoBackendFactory::instance()->smime();
    } else {
        return;
    }

    //d->resultContent = new KMime::Content;

    qCDebug(MESSAGECOMPOSER_LOG) << "creating signencrypt from:" << proto->name() << proto->displayName();
    std::unique_ptr<Kleo::SignEncryptJob> job(proto->signEncryptJob(!d->binaryHint(d->format), d->format == Kleo::InlineOpenPGPFormat));
    QByteArray encBody;
    d->content->assemble();

    // replace simple LFs by CRLFs for all MIME supporting CryptPlugs
    // according to RfC 2633, 3.1.1 Canonicalization
    QByteArray content;
    if (d->format & Kleo::InlineOpenPGPFormat) {
        content = d->content->body();
    } else if (!(d->format & Kleo::SMIMEOpaqueFormat))  {
        content = KMime::LFtoCRLF(d->content->encodedContent());
    } else {                    // SMimeOpaque doesn't need LFtoCRLF, else it gets munged
        content = d->content->encodedContent();
    }

    // FIXME: Make this async
    const std::pair<GpgME::SigningResult, GpgME::EncryptionResult> res = job->exec(d->signers, d->encKeys,
            content,
            false,
            encBody);

    // exec'ed jobs don't delete themselves
    job->deleteLater();

    if (res.first.error()) {
        qCDebug(MESSAGECOMPOSER_LOG) << "signing failed:" << res.first.error().asString();
        setError(res.first.error().code());
        setErrorText(QString::fromLocal8Bit(res.first.error().asString()));
        emitResult();
        return;
    }
    if (res.second.error()) {
        qCDebug(MESSAGECOMPOSER_LOG) << "encrypyting failed:" << res.second.error().asString();
        setError(res.second.error().code());
        setErrorText(QString::fromLocal8Bit(res.second.error().asString()));
        emitResult();
        return;
    }

    QByteArray signatureHashAlgo =  res.first.createdSignature(0).hashAlgorithmAsString();

    d->resultContent = MessageComposer::Util::composeHeadersAndBody(d->content, encBody, d->format, true, signatureHashAlgo);

    emitResult();
}

