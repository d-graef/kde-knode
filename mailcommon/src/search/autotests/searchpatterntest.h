/*
  Copyright (c) 2015 Tomas Trnka <tomastrnka@gmx.com>

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

#ifndef SEARCHPATTERNTEST_H
#define SEARCHPATTERNTEST_H

#include <QObject>

class SearchPatternTest : public QObject
{
    Q_OBJECT
public:
    explicit SearchPatternTest(QObject *parent = Q_NULLPTR);
    ~SearchPatternTest();
private Q_SLOTS:
    void shouldRuleRequirePart_data();
    void shouldRuleRequirePart();
};

#endif // SEARCHPATTERNTEST_H
