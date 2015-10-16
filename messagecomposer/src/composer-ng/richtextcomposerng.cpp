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

#include "richtextcomposerng.h"
#include <kpimtextedit/richtextcomposercontroler.h>
#include <kpimtextedit/richtextcomposerimages.h>

#include <pimcommon/autocorrection.h>

#include <QTextBlock>

//#include <part/textpart.h>
#include "settings/messagecomposersettings.h"
#include <grantlee/markupdirector.h>
#include <grantlee/plaintextmarkupbuilder.h>

using namespace MessageComposer;

class MessageComposer::RichTextComposerNgPrivate
{
public:
    RichTextComposerNgPrivate(RichTextComposerNg *q)
        : autoCorrection(Q_NULLPTR),
          richtextComposer(q)
    {
    }

    void fixHtmlFontSize(QString &cleanHtml);
    QString toWrappedPlainText() const;
    QString toWrappedPlainText(QTextDocument *doc) const;
    QString toCleanPlainText(const QString &plainText = QString()) const;
    void fixupTextEditString(QString &text) const;
    QString toCleanHtml() const;

    PimCommon::AutoCorrection *autoCorrection;
    RichTextComposerNg *richtextComposer;
};


RichTextComposerNg::RichTextComposerNg(QWidget *parent)
    : KPIMTextEdit::RichTextComposer(parent),
      d(new MessageComposer::RichTextComposerNgPrivate(this))
{

}

RichTextComposerNg::~RichTextComposerNg()
{
    delete d;
}

PimCommon::AutoCorrection *RichTextComposerNg::autocorrection() const
{
    return d->autoCorrection;
}

void RichTextComposerNg::setAutocorrection(PimCommon::AutoCorrection *autocorrect)
{
    d->autoCorrection = autocorrect;
}

void RichTextComposerNg::setAutocorrectionLanguage(const QString &lang)
{
    if (d->autoCorrection) {
        d->autoCorrection->setLanguage(lang);
    }
}

static bool isSpecial(const QTextCharFormat &charFormat)
{
    return charFormat.isFrameFormat() || charFormat.isImageFormat() ||
           charFormat.isListFormat() || charFormat.isTableFormat() || charFormat.isTableCellFormat();
}

bool RichTextComposerNg::processAutoCorrection(QKeyEvent *e)
{
    if (d->autoCorrection && d->autoCorrection->isEnabledAutoCorrection()) {
        if ((e->key() == Qt::Key_Space) || (e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) {
            if (!isLineQuoted(textCursor().block().text()) && !textCursor().hasSelection()) {
                const QTextCharFormat initialTextFormat = textCursor().charFormat();
                const bool richText = (textMode() == RichTextComposer::Rich);
                int position = textCursor().position();
                const bool addSpace = d->autoCorrection->autocorrect(richText, *document(), position);
                QTextCursor cur = textCursor();
                cur.setPosition(position);
                if (overwriteMode() && e->key() == Qt::Key_Space) {
                    if (addSpace) {
                        const QChar insertChar = QLatin1Char(' ');
                        if (!cur.atBlockEnd()) {
                            cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
                        }
                        if (richText && !isSpecial(initialTextFormat)) {
                            cur.insertText(insertChar, initialTextFormat);
                        } else {
                            cur.insertText(insertChar);
                        }
                        setTextCursor(cur);
                    }
                } else {
                    const bool spacePressed = (e->key() == Qt::Key_Space);
                    const QChar insertChar = spacePressed ? QLatin1Char(' ') : QLatin1Char('\n');
                    if (richText && !isSpecial(initialTextFormat)) {
                        if ((spacePressed && addSpace) || !spacePressed) {
                            cur.insertText(insertChar, initialTextFormat);
                        }
                    } else {
                        if ((spacePressed && addSpace) || !spacePressed) {
                            cur.insertText(insertChar);
                        }
                    }
                    setTextCursor(cur);
                }
                return true;
            }
        }
    }
    return false;
}

void RichTextComposerNgPrivate::fixHtmlFontSize(QString &cleanHtml)
{
    static const QString FONTSTYLEREGEX = QStringLiteral("<span style=\".*font-size:(.*)pt;.*</span>");
    QRegExp styleRegex(FONTSTYLEREGEX);
    styleRegex.setMinimal(true);

    int offset = styleRegex.indexIn(cleanHtml, 0);
    while (offset != -1) {
        // replace all the matching text with the new line text
        bool ok = false;
        const QString fontSizeStr = styleRegex.cap(1);
        const int ptValue = fontSizeStr.toInt(&ok);
        if (ok) {
            double emValue = (double)ptValue / 12;
            const QString emValueStr = QString::number(emValue, 'g', 2);
            cleanHtml.replace(styleRegex.pos(1), QString(fontSizeStr + QLatin1String("px")).length(), emValueStr + QLatin1String("em"));
        }
        // advance the search offset to just beyond the last replace
        offset += styleRegex.matchedLength();
        // find the next occurrence
        offset = styleRegex.indexIn(cleanHtml, offset);
    }
}

QString RichTextComposerNgPrivate::toWrappedPlainText() const
{
    QTextDocument *doc = richtextComposer->document();
    return toWrappedPlainText(doc);
}

QString RichTextComposerNgPrivate::toWrappedPlainText(QTextDocument *doc) const
{
    QString temp;
    QRegExp rx(QStringLiteral("(http|ftp|ldap)s?\\S+-$"));
    QTextBlock block = doc->begin();
    while (block.isValid()) {
        QTextLayout *layout = block.layout();
        const int numberOfLine(layout->lineCount());
        bool urlStart = false;
        for (int i = 0; i < numberOfLine; ++i) {
            QTextLine line = layout->lineAt(i);
            QString lineText = block.text().mid(line.textStart(), line.textLength());

            if (lineText.contains(rx) ||
                    (urlStart && !lineText.contains(QLatin1Char(' ')) &&
                     lineText.endsWith(QLatin1Char('-')))) {
                // don't insert line break in URL
                temp += lineText;
                urlStart = true;
            } else {
                temp += lineText + QLatin1Char('\n');
            }
        }
        block = block.next();
    }

    // Remove the last superfluous newline added above
    if (temp.endsWith(QLatin1Char('\n'))) {
        temp.chop(1);
    }

    fixupTextEditString(temp);
    return temp;
}


void RichTextComposerNgPrivate::fixupTextEditString(QString &text) const
{
    // Remove line separators. Normal \n chars are still there, so no linebreaks get lost here
    text.remove(QChar::LineSeparator);

    // Get rid of embedded images, see QTextImageFormat documentation:
    // "Inline images are represented by an object replacement character (0xFFFC in Unicode) "
    text.remove(0xFFFC);

    // In plaintext mode, each space is non-breaking.
    text.replace(QChar::Nbsp, QChar::fromLatin1(' '));
}

QString RichTextComposerNgPrivate::toCleanPlainText(const QString &plainText) const
{
    QString temp = plainText.isEmpty() ? richtextComposer->toPlainText() : plainText;
    fixupTextEditString(temp);
    return temp;
}

#if 0
void RichTextComposerNg::fillComposerTextPart(MessageComposer::TextPart *textPart)
{
    if (composerControler()->isFormattingUsed() && MessageComposer::MessageComposerSettings::self()->improvePlainTextOfHtmlMessage()) {
        Grantlee::PlainTextMarkupBuilder *pb = new Grantlee::PlainTextMarkupBuilder();

        Grantlee::MarkupDirector *pmd = new Grantlee::MarkupDirector(pb);
        pmd->processDocument(document());
        const QString plainText = pb->getResult();
        textPart->setCleanPlainText(d->toCleanPlainText(plainText));
        QTextDocument *doc = new QTextDocument(plainText);
        doc->adjustSize();

        textPart->setWrappedPlainText(d->toWrappedPlainText(doc));
        delete doc;
        delete pmd;
        delete pb;
    } else {
        textPart->setCleanPlainText(d->toCleanPlainText());
        textPart->setWrappedPlainText(d->toWrappedPlainText());
    }
    textPart->setWordWrappingEnabled(lineWrapMode() == QTextEdit::FixedColumnWidth);
    if (composerControler()->isFormattingUsed()) {
        QString cleanHtml = d->toCleanHtml();
        d->fixHtmlFontSize(cleanHtml);
        textPart->setCleanHtml(cleanHtml);
        textPart->setEmbeddedImages(composerControler()->composerImages()->embeddedImages());
    }

}
#endif
QString RichTextComposerNgPrivate::toCleanHtml() const
{
    QString result = richtextComposer->toHtml();

    static const QString EMPTYLINEHTML = QStringLiteral(
            "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; "
            "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; \">&nbsp;</p>");

    // Qt inserts various style properties based on the current mode of the editor (underline,
    // bold, etc), but only empty paragraphs *also* have qt-paragraph-type set to 'empty'.
    static const QString EMPTYLINEREGEX = QStringLiteral(
            "<p style=\"-qt-paragraph-type:empty;(.*)</p>");

    static const QString OLLISTPATTERNQT = QStringLiteral(
            "<ol style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px;");

    static const QString ULLISTPATTERNQT = QStringLiteral(
            "<ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px;");

    static const QString ORDEREDLISTHTML = QStringLiteral(
            "<ol style=\"margin-top: 0px; margin-bottom: 0px;");

    static const QString UNORDEREDLISTHTML = QStringLiteral(
                "<ul style=\"margin-top: 0px; margin-bottom: 0px;");

    // fix 1 - empty lines should show as empty lines - MS Outlook treats margin-top:0px; as
    // a non-existing line.
    // Although we can simply remove the margin-top style property, we still get unwanted results
    // if you have three or more empty lines. It's best to replace empty <p> elements with <p>&nbsp;</p>.

    QRegExp emptyLineFinder(EMPTYLINEREGEX);
    emptyLineFinder.setMinimal(true);

    // find the first occurrence
    int offset = emptyLineFinder.indexIn(result, 0);
    while (offset != -1) {
        // replace all the matching text with the new line text
        result.replace(offset, emptyLineFinder.matchedLength(), EMPTYLINEHTML);
        // advance the search offset to just beyond the last replace
        offset += EMPTYLINEHTML.length();
        // find the next occurrence
        offset = emptyLineFinder.indexIn(result, offset);
    }

    // fix 2a - ordered lists - MS Outlook treats margin-left:0px; as
    // a non-existing number; e.g: "1. First item" turns into "First Item"
    result.replace(OLLISTPATTERNQT, ORDEREDLISTHTML);

    // fix 2b - unordered lists - MS Outlook treats margin-left:0px; as
    // a non-existing bullet; e.g: "* First bullet" turns into "First Bullet"
    result.replace(ULLISTPATTERNQT, UNORDEREDLISTHTML);

    return result;
}
