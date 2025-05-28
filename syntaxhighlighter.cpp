#include "syntaxhighlighter.h"
#include <QTextBlock>
#include <QTextCursor>
SyntaxHighlighter::SyntaxHighlighter(QPlainTextEdit* pte) : pte(pte)
{
    // cursor = pte->textCursor();
    // keeping an instance variable completly breaks the app, typing makes text invisible being adding to the end,
    // highlighting starts being inconsistent and some characters are only partially highlighted somehow
    // cursor{pte->document()};
}


void SyntaxHighlighter::searchTextForMatches(qsizetype startOffset)
{
    const auto text = pte->toPlainText();
    QRegularExpressionMatchIterator keywordsIterator = keywordsRegex.globalMatchView(text, startOffset);
    populateMatchSet(keywords, keywordsIterator);

    QRegularExpressionMatchIterator functionIterator = classRegex.globalMatchView(text, startOffset);
    populateMatchSet(functions, functionIterator);

    QRegularExpressionMatchIterator classIterator = classRegex.globalMatchView(text, startOffset);
    populateMatchSet(classes, classIterator);

}


void SyntaxHighlighter::highlightText(qsizetype startOffset)
{
    // doesnt seem to work on its own, so now i re-set the flag (and block signals so tab title doesnt freak out)

    QTextDocument* doc = pte->document();
    doc->blockSignals(true); // stops the highlighting from emiting a textchange signal

    // cursor.beginEditBlock();

    bool docModifiedBeforeHand = pte->document()->isModified();

    QSet<match> emptySet;
    // old data is useless if you are editing at an earlier point
    // if(oldOffset <= startOffset || startOffset == 0){

        functions.clear();
        comments.clear();
        strings.clear();
        classes.clear();
        keywords.clear();
    // }

    resetFormating(startOffset);
    oldOffset = startOffset;
    searchTextForMatches(startOffset);

    highlightType(keywords, keywordColor);
    highlightType(classes, classColor);
    highlightType(functions, functionColor);


    QTextDocument *document = pte->document();

    for (QTextBlock block = document->begin(); block != document->end(); block = block.next()) {
        QString lineText = block.text();
        int lineStartPos = block.position(); // character offset in the full document

        extractStringsAndComments(lineText, lineStartPos);
    }


    // cursor.endEditBlock();

    doc->blockSignals(false); // reenables signals
    doc->setModified(docModifiedBeforeHand);

}

void SyntaxHighlighter::highlightType(const QSet<match>& toHighlight, const QColor& color)
{
    QTextCursor cursor{pte->document()};
    QTextCharFormat format{};
    format.setForeground(color);

    for(const auto& match : toHighlight){
        cursor.setPosition(match.start);
        cursor.setPosition(match.end, QTextCursor::KeepAnchor);
        // move from the start to the end of the block while anchored, thus selecting the text

        cursor.setCharFormat(format);

        cursor.setPosition(QTextCursor::Start);
    }

}

void SyntaxHighlighter::extractStringsAndComments(const QString& line, int lineOffset)
{
    bool inString = false;
    int commentStart = -1;

    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];
        if (c == '"') {
            inString = !inString;
        }
        else if (c == '#' && !inString) {
            commentStart = i;
            break;
        }
    }

    QTextCursor cursor{pte->document()};
    QTextCharFormat format{};
    // highlighting strings before the comment
    format.setForeground(stringColor);
    // inString = false;
    int iterations = (commentStart == -1 ? line.length() : commentStart);
    for (int i = 0; i < iterations; ++i) {
        if (line[i] == '"') {
            int start = i++;
            while ((i < iterations) && (line[i] != '"')) {
                ++i;
            }
            if (i < iterations) ++i;

            int globalStart = lineOffset + start;
            int globalLength = i - start;

            cursor.setPosition(globalStart);
            cursor.setPosition(globalStart + globalLength, QTextCursor::KeepAnchor);
            cursor.setCharFormat(format);
        }
    }

    // highlight comment
    if (commentStart != -1) {
        format.setForeground(commentColor);
        int globalStart = lineOffset + commentStart;
        int globalLength = line.length() - commentStart;

        cursor.setPosition(globalStart);
        cursor.setPosition(globalStart + globalLength, QTextCursor::KeepAnchor);
        cursor.setCharFormat(format);
    }
}


void SyntaxHighlighter::resetFormating(qsizetype start){
    if(pte == nullptr) return;

    QTextCursor cursor{pte->document()};
    QTextCharFormat format{};
    format.setForeground(Qt::white);

    // moves cursor from the start index, to the end of the document, clearing all formats
    cursor.setPosition(start);
    // cursor.setPosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.setPosition(pte->document()->characterCount()-1, QTextCursor::KeepAnchor);
    cursor.setCharFormat(format);
}
