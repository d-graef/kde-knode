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

#include "job/signjob.h"

#include "contentjobbase_p.h"
#include "Libkleo/CryptoBackendFactory"
#include "Libkleo/CryptoBackend"
#include "Libkleo/Enum"
#include "Libkleo/SignJob"
#include "utils/util.h"

#include "messagecomposer_debug.h"
#include <kmime/kmime_headers.h>
#include <kmime/kmime_message.h>
#include <kmime/kmime_content.h>

#include <gpgme++/global.h>
#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>
#include <sstream>

using namespace MessageComposer;

class MessageComposer::SignJobPrivate : public ContentJobBasePrivate
{
public:
    SignJobPrivate(SignJob *qq)
        : ContentJobBasePrivate(qq)
        , content(Q_NULLPTR)
    {
    }

    KMime::Content *content;
    std::vector<GpgME::Key> signers;
    Kleo::CryptoMessageFormat format;

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

    GpgME::SignatureMode signingMode(Kleo::CryptoMessageFormat f)
    {
        switch (f) {
        case Kleo::SMIMEOpaqueFormat:
            return GpgME::NormalSignatureMode;
        case Kleo::InlineOpenPGPFormat:
            return GpgME::Clearsigned;
        default:
        case Kleo::SMIMEFormat:
        case Kleo::OpenPGPMIMEFormat:
            return GpgME::Detached;
        }
    }

    Q_DECLARE_PUBLIC(SignJob)
};

SignJob::SignJob(QObject *parent)
    : ContentJobBase(*new SignJobPrivate(this), parent)
{
}

SignJob::~SignJob()
{
}

void SignJob::setContent(KMime::Content *content)
{
    Q_D(SignJob);

    d->content = content;
}

void SignJob::setCryptoMessageFormat(Kleo::CryptoMessageFormat format)
{
    Q_D(SignJob);

    // There *must* be a concrete format set at this point.
    Q_ASSERT(format == Kleo::OpenPGPMIMEFormat
             || format == Kleo::InlineOpenPGPFormat
             || format == Kleo::SMIMEFormat
             || format == Kleo::SMIMEOpaqueFormat);
    d->format = format;
}

void SignJob::setSigningKeys(std::vector<GpgME::Key> &signers)
{
    Q_D(SignJob);

    d->signers = signers;
}

KMime::Content *SignJob::origContent()
{
    Q_D(SignJob);

    return d->content;

}

void SignJob::process()
{
    Q_D(SignJob);
    Q_ASSERT(d->resultContent == 0);   // Not processed before.

    // if setContent hasn't been called, we assume that a subjob was added
    // and we want to use that
    if (!d->content) {
        Q_ASSERT(d->subjobContents.size() == 1);
        d->content = d->subjobContents.first();
    }

    //d->resultContent = new KMime::Content;

    const Kleo::CryptoBackend::Protocol *proto = Q_NULLPTR;
    if (d->format & Kleo::AnyOpenPGP) {
        proto = Kleo::CryptoBackendFactory::instance()->openpgp();
    } else if (d->format & Kleo::AnySMIME) {
        proto = Kleo::CryptoBackendFactory::instance()->smime();
    }

    Q_ASSERT(proto);

    qCDebug(MESSAGECOMPOSER_LOG) << "creating signJob from:" << proto->name() << proto->displayName();
    std::unique_ptr<Kleo::SignJob> job(proto->signJob(!d->binaryHint(d->format), d->format == Kleo::InlineOpenPGPFormat));
    // for now just do the main recipients
    QByteArray signature;

    d->content->assemble();

    // replace simple LFs by CRLFs for all MIME supporting CryptPlugs
    // according to RfC 2633, 3.1.1 Canonicalization
    QByteArray content;
    if (d->format & Kleo::InlineOpenPGPFormat) {
        content = d->content->body();
    } else if (!(d->format & Kleo::SMIMEOpaqueFormat)) {

        // replace "From " and "--" at the beginning of lines
        // with encoded versions according to RfC 3156, 3
        // Note: If any line begins with the string "From ", it is strongly
        //   suggested that either the Quoted-Printable or Base64 MIME encoding
        //   be applied.
        if (d->content->contentTransferEncoding()->encoding() == KMime::Headers::CEquPr ||
                d->content->contentTransferEncoding()->encoding() == KMime::Headers::CE7Bit) {
            QByteArray body = d->content->encodedBody();
            bool changed = false;
            QList<QByteArray> search;
            QList<QByteArray> replacements;

            search       << "From "
                         << "from "
                         << "-";
            replacements << "From=20"
                         << "from=20"
                         << "=2D";

            if (d->content->contentTransferEncoding()->encoding() == KMime::Headers::CE7Bit) {
                for (int i = 0; i < search.size(); ++i) {
                    const auto pos = body.indexOf(search[i]);
                    if (pos == 0 || (pos > 0 && body.at(pos - 1) == '\n')) {
                        changed = true;
                        break;
                    }
                }
                if (changed) {
                    d->content->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
                    d->content->assemble();
                    body = d->content->encodedBody();
                }
            }

            for (int i = 0; i < search.size(); ++i) {
                const auto pos = body.indexOf(search[i]);
                if (pos == 0 || (pos > 0 && body.at(pos - 1) == '\n')) {
                    changed = true;
                    body.replace(pos, search[i].size(), replacements[i]);
                }
            }

            if (changed) {
                qCDebug(MESSAGECOMPOSER_LOG) << "Content changed";
                d->content->setBody(body);
                d->content->contentTransferEncoding()->setDecoded(false);
            }
        }

        content = KMime::LFtoCRLF(d->content->encodedContent());
    } else {                    // SMimeOpaque doesn't need LFtoCRLF, else it gets munged
        content = d->content->encodedContent();
    }

    // FIXME: Make this async
    GpgME::SigningResult res = job->exec(d->signers,
                                         content,
                                         d->signingMode(d->format),
                                         signature);

    // exec'ed jobs don't delete themselves
    job->deleteLater();

    if (res.error().code()) {
        qCDebug(MESSAGECOMPOSER_LOG) << "signing failed:" << res.error().asString();
        //        job->showErrorDialog( globalPart()->parentWidgetForGui() );
        setError(res.error().code());
        setErrorText(QString::fromLocal8Bit(res.error().asString()));
    } else {
        QByteArray signatureHashAlgo =  res.createdSignature(0).hashAlgorithmAsString();
        d->resultContent = MessageComposer::Util::composeHeadersAndBody(d->content, signature, d->format, true, signatureHashAlgo);
    }

    emitResult();
}

