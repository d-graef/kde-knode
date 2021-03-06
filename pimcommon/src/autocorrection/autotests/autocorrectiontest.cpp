/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "autocorrectiontest.h"
#include "../autocorrection.h"
#include "settings/pimcommonsettings.h"
#include <qtest.h>
#include "pimcommon_debug.h"
#include <QTextDocument>

AutoCorrectionTest::AutoCorrectionTest()
{
    mConfig = KSharedConfig::openConfig(QStringLiteral("autocorrectiontestrc"));
    PimCommon::PimCommonSettings::self()->setSharedConfig(mConfig);
    PimCommon::PimCommonSettings::self()->load();
}

AutoCorrectionTest::~AutoCorrectionTest()
{
}

void AutoCorrectionTest::shouldHaveDefaultValue()
{
    PimCommon::AutoCorrection autocorrection;
    QVERIFY(!autocorrection.isEnabledAutoCorrection());
    QVERIFY(!autocorrection.isUppercaseFirstCharOfSentence());
    QVERIFY(!autocorrection.isFixTwoUppercaseChars());
    QVERIFY(!autocorrection.isSingleSpaces());
    QVERIFY(!autocorrection.isAutoFractions());
    QVERIFY(!autocorrection.isCapitalizeWeekDays());
    QVERIFY(!autocorrection.isReplaceDoubleQuotes());
    QVERIFY(!autocorrection.isReplaceSingleQuotes());
    QVERIFY(!autocorrection.isAdvancedAutocorrect());
    QVERIFY(!autocorrection.isAutoFormatUrl());
    QVERIFY(!autocorrection.isAutoBoldUnderline());
    QVERIFY(!autocorrection.isSuperScript());
    QVERIFY(!autocorrection.isAddNonBreakingSpace());
}

void AutoCorrectionTest::shouldRestoreValue()
{
    PimCommon::AutoCorrection autocorrection;
    //TODO
}

void AutoCorrectionTest::shouldUpperCaseFirstCharOfSentence()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setUppercaseFirstCharOfSentence(true);

    //Uppercase here.
    QTextDocument doc;
    QString text = QStringLiteral("foo");
    doc.setPlainText(text);
    int position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QStringLiteral("Foo"));

    //IT's not first char -> not uppercase
    text = QStringLiteral(" foo");
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);

    //It's already uppercase
    text = QStringLiteral("Foo");
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);

    //Word is after a ". "
    text = QStringLiteral("Foo. foo");
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QStringLiteral("Foo. Foo"));
    QCOMPARE(position, text.length());

}

void AutoCorrectionTest::shouldFixTwoUpperCaseChars()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setFixTwoUppercaseChars(true);

    //Remove two uppercases
    QTextDocument doc;
    QString text = QStringLiteral("FOo");
    doc.setPlainText(text);
    int position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QStringLiteral("Foo"));

    //There is not two uppercase
    text = QStringLiteral("foo");
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);

    text = QStringLiteral("Foo");
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);

    //There is a uppercase word
    text = QStringLiteral("FOO");
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);

    //Exclude 2 upper letter
    text = QStringLiteral("ABc");
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QStringLiteral("Abc"));

    QSet<QString> exception;
    exception.insert(QStringLiteral("ABc"));
    autocorrection.setTwoUpperLetterExceptions(exception);
    text = QStringLiteral("ABc");
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);
}

void AutoCorrectionTest::shouldReplaceSingleQuote()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setReplaceSingleQuotes(true);
    PimCommon::AutoCorrection::TypographicQuotes simpleQuote;
    simpleQuote.begin = QLatin1Char('A');
    simpleQuote.end = QLatin1Char('B');

    autocorrection.setTypographicSingleQuotes(simpleQuote);

    QTextDocument doc;
    QString text = QStringLiteral("sss");
    doc.setPlainText(QStringLiteral("'") + text);
    int position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QString(simpleQuote.begin + text));

    text = QStringLiteral("sss");
    doc.setPlainText(text + QStringLiteral("'"));
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QString(text + simpleQuote.end));

    text = QStringLiteral("sss");
    doc.setPlainText(QStringLiteral("'") + text + QStringLiteral("'"));
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QString(simpleQuote.begin + text + simpleQuote.end));

}

void AutoCorrectionTest::shouldReplaceDoubleQuote()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setReplaceDoubleQuotes(true);
    PimCommon::AutoCorrection::TypographicQuotes doubleQuote;
    doubleQuote.begin = QLatin1Char('A');
    doubleQuote.end = QLatin1Char('B');

    autocorrection.setTypographicDoubleQuotes(doubleQuote);

    QTextDocument doc;
    QString text = QStringLiteral("sss");

    doc.setPlainText(QLatin1Char('"') + text);
    int position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QString(doubleQuote.begin + text));

    text = QStringLiteral("sss");
    doc.setPlainText(text + QStringLiteral("\""));
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QString(text + doubleQuote.end));

    text = QStringLiteral("sss");
    doc.setPlainText(QStringLiteral("\"") + text + QStringLiteral("\""));
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QString(doubleQuote.begin + text + doubleQuote.end));
}

void AutoCorrectionTest::shouldNotReplaceUppercaseLetter()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setFixTwoUppercaseChars(true);
    QSet<QString> exceptions;
    exceptions.insert(QStringLiteral("ABc"));
    autocorrection.setTwoUpperLetterExceptions(exceptions);

    QTextDocument doc;
    QString text = QStringLiteral("foo ABc");
    doc.setPlainText(text);
    int position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);
}

void AutoCorrectionTest::shouldReplaceToTextFormat()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setAutoBoldUnderline(true);

    QTextDocument doc;
    //We don't use html => don't change it.
    QString text = QStringLiteral("*foo*");
    doc.setHtml(text);
    int position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);

    QString result = QStringLiteral("foo");
    doc.setHtml(text);
    position = text.length();
    autocorrection.autocorrect(true, doc, position);
    QCOMPARE(doc.toPlainText(), result);

    text = QStringLiteral("*foo*");
    doc.setHtml(text);
    position = text.length();
    autocorrection.autocorrect(true, doc, position);
    QCOMPARE(doc.toPlainText(), result);
    QTextCursor cursor(&doc);
    cursor.setPosition(2);
    QTextCharFormat charFormat = cursor.charFormat();
    QCOMPARE(charFormat.font().underline(), false);
    QCOMPARE(charFormat.font().bold(), true);
    QCOMPARE(charFormat.font().strikeOut(), false);

    text = QStringLiteral("_foo_");
    doc.setHtml(text);
    position = text.length();
    autocorrection.autocorrect(true, doc, position);
    QCOMPARE(doc.toPlainText(), result);
    cursor = QTextCursor(&doc);
    cursor.setPosition(2);
    charFormat = cursor.charFormat();
    QCOMPARE(charFormat.font().underline(), true);
    QCOMPARE(charFormat.font().bold(), false);
    QCOMPARE(charFormat.font().strikeOut(), false);

    text = QStringLiteral("-foo-");
    doc.setHtml(text);
    position = text.length();
    autocorrection.autocorrect(true, doc, position);
    QCOMPARE(doc.toPlainText(), result);
    cursor = QTextCursor(&doc);
    cursor.setPosition(2);
    charFormat = cursor.charFormat();
    QCOMPARE(charFormat.font().underline(), false);
    QCOMPARE(charFormat.font().bold(), false);
    QCOMPARE(charFormat.font().strikeOut(), true);

    //Don't convert it.
    text = QStringLiteral("-foo1");
    doc.setHtml(text);
    position = text.length();
    autocorrection.autocorrect(true, doc, position);
    QCOMPARE(doc.toPlainText(), text);

    text = QStringLiteral("*foo* blabla");
    position = 5;
    doc.setHtml(text);
    autocorrection.autocorrect(true, doc, position);
    result = QStringLiteral("foo blabla");
    QCOMPARE(doc.toPlainText(), result);
    QCOMPARE(position, 3);

    //TODO QCOMPARE(doc.toHtml(), text);

}

void AutoCorrectionTest::shouldReplaceAutoFraction()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setAutoFractions(true);

    QTextDocument doc;
    QString text = QStringLiteral("1/2");
    doc.setPlainText(text);
    int position = text.length();
    autocorrection.autocorrect(false, doc, position);

    QCOMPARE(doc.toPlainText(), QStringLiteral("½"));

    QString suffix = QStringLiteral(" after");
    text = QStringLiteral("1/2");
    position = 3;
    text += suffix;
    doc.setPlainText(text);
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QString(QStringLiteral("½") + suffix));
    QCOMPARE(position, 1);

}

void AutoCorrectionTest::shouldNotAddSpaceWhenWeAlreadyHaveASpace()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setSingleSpaces(true);
    QTextDocument doc;
    //We already a space => don't allow to add more
    QString text = QStringLiteral("FOO ");
    doc.setPlainText(text);
    int position = text.length();
    bool result = autocorrection.autocorrect(false, doc, position);
    QCOMPARE(result, false);

    //We can add a space
    text = QStringLiteral("FOO");
    doc.setPlainText(text);
    position = text.length();
    result = autocorrection.autocorrect(false, doc, position);
    QCOMPARE(result, true);

    //We have a space => don't add it.
    text = QStringLiteral("FOO ");
    position = text.length();
    QString fullText = text + QStringLiteral("FOO");
    doc.setPlainText(fullText);
    result = autocorrection.autocorrect(false, doc, position);
    QCOMPARE(result, false);
}

void AutoCorrectionTest::shouldAutocorrectWord()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setAdvancedAutocorrect(true);

    QTextDocument doc;
    //No changes
    QString text = QStringLiteral("FOO");
    doc.setPlainText(text);
    int position = text.length();
    int oldPosition = position;
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);
    QCOMPARE(position, oldPosition);

    //Convert word
    QHash<QString, QString> entries;
    const QString convertWord = QStringLiteral("BLABLA");
    entries.insert(text, convertWord);
    autocorrection.setAutocorrectEntries(entries);
    text = QStringLiteral("FOO");
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), convertWord);
    QCOMPARE(position, convertWord.length());

    QString suffix = QStringLiteral(" TOTO");
    text = QStringLiteral("FOO");
    position = text.length();
    text += suffix;
    doc.setPlainText(text);
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), QString(convertWord + suffix));
    //FIXME ? QCOMPARE(position, convertWord.length());

}

void AutoCorrectionTest::shouldNotUpperCaseFirstCharOfSentence()
{
    PimCommon::AutoCorrection autocorrection;
    autocorrection.setEnabledAutoCorrection(true);
    autocorrection.setUppercaseFirstCharOfSentence(true);
    QSet<QString> lst;
    lst.insert(QStringLiteral("Foo."));
    autocorrection.setUpperCaseExceptions(lst);

    //Uppercase here.
    QTextDocument doc;
    QString text = QStringLiteral("foo. blabla Foo. tt");
    doc.setPlainText(text);
    int position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QCOMPARE(doc.toPlainText(), text);

    autocorrection.setUpperCaseExceptions(QSet<QString>());
    doc.setPlainText(text);
    position = text.length();
    autocorrection.autocorrect(false, doc, position);
    QString result = QStringLiteral("foo. blabla Foo. Tt");
    QCOMPARE(doc.toPlainText(), result);

}

QTEST_MAIN(AutoCorrectionTest)
