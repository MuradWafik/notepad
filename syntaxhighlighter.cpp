#include "syntaxhighlighter.h"
#include <QTextBlock>

SyntaxHighlighter::SyntaxHighlighter(QPlainTextEdit* pte) : pte(pte)
{}


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
    searchTextForMatches(startOffset);

    highlightType(keywords, keywordColor);
    highlightType(classes, classColor);
    highlightType(functions, functionColor);


    QTextDocument *document = pte->document();

    for (QTextBlock block = document->begin(); block != document->end(); block = block.next()) {
        QString lineText = block.text();
        int lineStartPos = block.position(); // character offset in the full document
        // Use lineText and lineStartPos as needed
        extractStringsAndComments(lineText, lineStartPos);

    }
}

void SyntaxHighlighter::highlightType(const QSet<match>& toHighlight, const QColor& color)
{

    QTextDocument *document = pte->document();

    QTextCursor cursor{document};
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

    QTextCursor cursor{pte->document()};
    QTextCharFormat format{};

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

    // highlighting strings before the comment
    format.setForeground(stringColor);
    // inString = false;
    int iterations = (commentStart == -1 ? line.length() : commentStart);
    for (int i = 0; i < iterations; ++i) {
        if (line[i] == '"') {
            int start = i++;
            while (i < iterations && line[i] != '"') { ++i; }
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

