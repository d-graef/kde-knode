/*
    qgpgmeverifydetachedjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2007,2008 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

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

#ifndef __KLEO_QGPGMEVERIFYDETACHEDJOB_H__
#define __KLEO_QGPGMEVERIFYDETACHEDJOB_H__

#include "libkleo/verifydetachedjob.h"

#include "threadedjobmixin.h"

#include <gpgme++/verificationresult.h>

namespace Kleo
{

class QGpgMEVerifyDetachedJob
#ifdef Q_MOC_RUN
    : public VerifyDetachedJob
#else
    : public _detail::ThreadedJobMixin<VerifyDetachedJob, boost::tuple<GpgME::VerificationResult, QString, GpgME::Error> >
#endif
{
    Q_OBJECT
#ifdef Q_MOC_RUN
public Q_SLOTS:
    void slotFinished();
#endif
public:
    explicit QGpgMEVerifyDetachedJob(GpgME::Context *context);
    ~QGpgMEVerifyDetachedJob();

    /*! \reimp from VerifyDetachedJob */
    GpgME::Error start(const QByteArray &signature, const QByteArray &signedData) Q_DECL_OVERRIDE;

    /*! \reimp from VerifyDetachedJob */
    void start(const boost::shared_ptr<QIODevice> &signature, const boost::shared_ptr<QIODevice> &signedData) Q_DECL_OVERRIDE;

    /*! \reimp from VerifyDetachedJob */
    GpgME::VerificationResult exec(const QByteArray &signature,
                                   const QByteArray &signedData) Q_DECL_OVERRIDE;

    /*! \reimp from ThreadedJobMixin */
    void resultHook(const result_type &r) Q_DECL_OVERRIDE;

private:
    GpgME::VerificationResult mResult;
};

}

#endif // __KLEO_QGPGMEVERIFYDETACHEDJOB_H__
