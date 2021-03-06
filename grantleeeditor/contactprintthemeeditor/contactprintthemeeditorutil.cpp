/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "contactprintthemeeditorutil.h"

ContactPrintThemeEditorutil::ContactPrintThemeEditorutil()
{

}

QString ContactPrintThemeEditorutil::defaultContact() const
{
    const QString contact = QStringLiteral("BEGIN:VCARD\n"
                                           "ADR;TYPE=home:;;10 street Eiffel Tower\n;Paris;;75016;France\n"
                                           "EMAIL:test@kde.org\n"
                                           "FN:Test\n"
                                           "N:Test;;;;\n"
                                           "ORG:kde\n"
                                           "TEL;TYPE=HOME:+33 12345678\n"
                                           "UID:{851e0b81-8f95-40d2-a6a6-a9dbee2be12d}\n"
                                           "URL:www.kde.org\n"
                                           "VERSION:3.0\n"
                                           "END:VCARD");
    return contact;
}

QString ContactPrintThemeEditorutil::defaultTemplate() const
{
    const QString templateStr = QStringLiteral("<html>\n"
                                "{% if contacts %}\n"
                                "{% for contact in contacts %}\n"
                                "<h1>{{ contact.realName|safe }}</h1>\n"
                                "<p>{{ contact.emails|safe }}</p>\n"
                                "<p>{{ contact.note|safe }}</p>\n"
                                "<br>\n"
                                "<br>\n"
                                "{% endfor %}\n"
                                "{% endif %}\n"
                                "</html>");

    return templateStr;
}
