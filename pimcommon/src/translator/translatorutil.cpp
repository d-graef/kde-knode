/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "translatorutil.h"
#include <KComboBox>
using namespace PimCommon;

TranslatorUtil::TranslatorUtil()
{

}

QPair<QString, QString> TranslatorUtil::pair(TranslatorUtil::languages lang)
{
    QPair<QString, QString> ret;
    switch (lang) {
    case automatic:
        ret = QPair<QString, QString>(i18n("Detect language"), QStringLiteral("auto"));
        break;
    case en:
        ret = QPair<QString, QString>(i18n("English"), QStringLiteral("en"));
        break;
    case zh:
        ret = QPair<QString, QString>(i18n("Chinese (Simplified)"), QStringLiteral("zh"));
        break;
    case zt:
        ret = QPair<QString, QString>(i18n("Chinese (Traditional)"), QStringLiteral("zt"));
        break;
    case nl:
        ret = QPair<QString, QString>(i18n("Dutch"), QStringLiteral("nl"));
        break;
    case fr:
        ret = QPair<QString, QString>(i18n("French"), QStringLiteral("fr"));
        break;
    case de:
        ret = QPair<QString, QString>(i18n("German"), QStringLiteral("de"));
        break;
    case el:
        ret = QPair<QString, QString>(i18n("Greek"), QStringLiteral("el"));
        break;
    case it:
        ret = QPair<QString, QString>(i18n("Italian"), QStringLiteral("it"));
        break;
    case ja:
        ret = QPair<QString, QString>(i18n("Japanese"), QStringLiteral("ja"));
        break;
    case ko:
        ret = QPair<QString, QString>(i18n("Korean"), QStringLiteral("ko"));
        break;
    case pt:
        ret = QPair<QString, QString>(i18n("Portuguese"), QStringLiteral("pt"));
        break;
    case ru:
        ret = QPair<QString, QString>(i18n("Russian"), QStringLiteral("ru"));
        break;
    case es:
        ret = QPair<QString, QString>(i18n("Spanish"), QStringLiteral("es"));
        break;
    case af:
        ret = QPair<QString, QString>(i18n("Afrikaans"), QStringLiteral("af"));
        break;
    case sq:
        ret = QPair<QString, QString>(i18n("Albanian"), QStringLiteral("sq"));
        break;
    case ar:
        ret = QPair<QString, QString>(i18n("Arabic"), QStringLiteral("ar"));
        break;
    case hy:
        ret = QPair<QString, QString>(i18n("Armenian"), QStringLiteral("hy"));
        break;
    case az:
        ret = QPair<QString, QString>(i18n("Azerbaijani"), QStringLiteral("az"));
        break;
    case eu:
        ret = QPair<QString, QString>(i18n("Basque"), QStringLiteral("eu"));
        break;
    case be:
        ret = QPair<QString, QString>(i18n("Belarusian"), QStringLiteral("be"));
        break;
    case bg:
        ret = QPair<QString, QString>(i18n("Bulgarian"), QStringLiteral("bg"));
        break;
    case ca:
        ret = QPair<QString, QString>(i18n("Catalan"), QStringLiteral("ca"));
        break;
    case zh_cn_google: // For google only
        ret = QPair<QString, QString>(i18n("Chinese (Simplified)"), QStringLiteral("zh-CN")); // For google only
        break;
    case zh_tw_google: // For google only
        ret = QPair<QString, QString>(i18n("Chinese (Traditional)"), QStringLiteral("zh-TW")); // For google only
        break;
    case hr:
        ret = QPair<QString, QString>(i18n("Croatian"), QStringLiteral("hr"));
        break;
    case cs:
        ret = QPair<QString, QString>(i18n("Czech"), QStringLiteral("cs"));
        break;
    case da:
        ret = QPair<QString, QString>(i18n("Danish"), QStringLiteral("da"));
        break;
    case et:
        ret = QPair<QString, QString>(i18n("Estonian"), QStringLiteral("et"));
        break;
    case tl:
        ret = QPair<QString, QString>(i18n("Filipino"), QStringLiteral("tl"));
        break;
    case fi:
        ret = QPair<QString, QString>(i18n("Finnish"), QStringLiteral("fi"));
        break;
    case gl:
        ret = QPair<QString, QString>(i18n("Galician"), QStringLiteral("gl"));
        break;
    case ka:
        ret = QPair<QString, QString>(i18n("Georgian"), QStringLiteral("ka"));
        break;
    case ht:
        ret = QPair<QString, QString>(i18n("Haitian Creole"), QStringLiteral("ht"));
        break;
    case iw:
        ret = QPair<QString, QString>(i18n("Hebrew"), QStringLiteral("iw"));
        break;
    case hi:
        ret = QPair<QString, QString>(i18n("Hindi"), QStringLiteral("hi"));
        break;
    case hu:
        ret = QPair<QString, QString>(i18n("Hungarian"), QStringLiteral("hu"));
        break;
    case is:
        ret = QPair<QString, QString>(i18n("Icelandic"), QStringLiteral("is"));
        break;
    case id:
        ret = QPair<QString, QString>(i18n("Indonesian"), QStringLiteral("id"));
        break;
    case ga:
        ret = QPair<QString, QString>(i18n("Irish"), QStringLiteral("ga"));
        break;
    case lv:
        ret = QPair<QString, QString>(i18n("Latvian"), QStringLiteral("lv"));
        break;
    case lt:
        ret = QPair<QString, QString>(i18n("Lithuanian"), QStringLiteral("lt"));
        break;
    case mk:
        ret = QPair<QString, QString>(i18n("Macedonian"), QStringLiteral("mk"));
        break;
    case ms:
        ret = QPair<QString, QString>(i18n("Malay"), QStringLiteral("ms"));
        break;
    case mt:
        ret = QPair<QString, QString>(i18n("Maltese"), QStringLiteral("mt"));
        break;
    case no:
        ret = QPair<QString, QString>(i18n("Norwegian"), QStringLiteral("no"));
        break;
    case fa:
        ret = QPair<QString, QString>(i18n("Persian"), QStringLiteral("fa"));
        break;
    case pl:
        ret = QPair<QString, QString>(i18n("Polish"), QStringLiteral("pl"));
        break;
    case ro:
        ret = QPair<QString, QString>(i18n("Romanian"), QStringLiteral("ro"));
        break;
    case sr:
        ret = QPair<QString, QString>(i18n("Serbian"), QStringLiteral("sr"));
        break;
    case sk:
        ret = QPair<QString, QString>(i18n("Slovak"), QStringLiteral("sk"));
        break;
    case sl:
        ret = QPair<QString, QString>(i18n("Slovenian"), QStringLiteral("sl"));
        break;
    case sw:
        ret = QPair<QString, QString>(i18n("Swahili"), QStringLiteral("sw"));
        break;
    case sv:
        ret = QPair<QString, QString>(i18n("Swedish"), QStringLiteral("sv"));
        break;
    case th:
        ret = QPair<QString, QString>(i18n("Thai"), QStringLiteral("th"));
        break;
    case tr:
        ret = QPair<QString, QString>(i18n("Turkish"), QStringLiteral("tr"));
        break;
    case uk:
        ret = QPair<QString, QString>(i18n("Ukrainian"), QStringLiteral("uk"));
        break;
    case ur:
        ret = QPair<QString, QString>(i18n("Urdu"), QStringLiteral("ur"));
        break;
    case vi:
        ret = QPair<QString, QString>(i18n("Vietnamese"), QStringLiteral("vi"));
        break;
    case cy:
        ret = QPair<QString, QString>(i18n("Welsh"), QStringLiteral("cy"));
        break;
    case yi:
        ret = QPair<QString, QString>(i18n("Yiddish"), QStringLiteral("yi"));
        break;
    }
    return ret;
}

void PimCommon::TranslatorUtil::addPairToMap(QMap<QString, QString> &map, const QPair<QString, QString> &pair)
{
    map.insert(pair.first, pair.second);
}

void PimCommon::TranslatorUtil::addItemToFromComboBox(KComboBox *combo, const QPair<QString, QString> &pair)
{
    combo->addItem(pair.first, pair.second);
}

