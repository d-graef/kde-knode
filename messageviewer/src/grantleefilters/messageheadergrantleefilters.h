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

#ifndef MESSAGEHEADERGRANTLEEFILTERS_H
#define MESSAGEHEADERGRANTLEEFILTERS_H

#include <QObject>
#include <grantlee/taglibraryinterface.h>

/**
 * The grantlee's plugin to define filters
 */

class MessageHeaderGrantleeFilters : public QObject, public Grantlee::TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(Grantlee::TagLibraryInterface)
    Q_PLUGIN_METADATA(IID "org.grantlee.TagLibraryInterface")
public:
    explicit MessageHeaderGrantleeFilters(QObject *parent = Q_NULLPTR);
    ~MessageHeaderGrantleeFilters();
    QHash<QString, Grantlee::Filter *> filters(const QString &name = QString()) Q_DECL_OVERRIDE;
};

#endif // MESSAGEHEADERGRANTLEEFILTERS_H
