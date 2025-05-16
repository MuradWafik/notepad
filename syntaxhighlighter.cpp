#include "syntaxhighlighter.h"
#include <QTextBlock>

void SyntaxHighlighter::searchTextForMatches(QStringView text, qsizetype startOffset){


    // QRegularExpressionMatchIterator commentIterator = commentRegex.globalMatchView(text, startOffset);
    // populateMatchSet(comments, commentIterator);

    QRegularExpressionMatchIterator keywordsIterator = keywordsRegex.globalMatchView(text, startOffset);
    populateMatchSet(keywords, keywordsIterator);

    // QRegularExpressionMatchIterator stringIterator = stringRegex.globalMatchView(text, startOffset);
    // populateMatchSet(strings, stringIterator);

    QRegularExpressionMatchIterator functionIterator = classRegex.globalMatchView(text, startOffset);
    populateMatchSet(functions, functionIterator);

    QRegularExpressionMatchIterator classIterator = classRegex.globalMatchView(text, startOffset);
    populateMatchSet(classes, classIterator);

}


void SyntaxHighlighter::highlightText(QStringView text, QPlainTextEdit* pte, qsizetype startOffset){
    searchTextForMatches(text, startOffset);

    // highlightType(comments, commentColor, pte);
    // highlightType(strings, stringColor, pte);
    highlightType(keywords, keywordColor, pte);
    highlightType(classes, classColor, pte);
    highlightType(functions, functionColor, pte);


    QTextDocument *document = pte->document();

    for (QTextBlock block = document->begin(); block != document->end(); block = block.next()) {
        QString lineText = block.text();
        int lineStartPos = block.position(); // character offset in the full document
        // Use lineText and lineStartPos as needed
        extractStringsAndComments(document, lineText, lineStartPos);
    }




}

void SyntaxHighlighter::highlightType(const QSet<match>& toHighlight, const QColor& color, QPlainTextEdit* pte){

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

void SyntaxHighlighter::extractStringsAndComments(QTextDocument* doc, const QString& line, int lineOffset) {
    bool inString = false;
    int commentStart = -1;

    QTextCursor cursor{doc};
    QTextCharFormat format{};

    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];
        if (c == '"') {
            inString = !inString;
        } else if (c == '#' && !inString) {
            commentStart = i;
            break;
        }
    }

    // Highlight strings before the comment
    format.setForeground(stringColor);
    // inString = false;
    int iterations = (commentStart == -1 ? line.length() : commentStart);
    for (int i = 0; i < iterations; ++i) {
        if (line[i] == '"') {
            int start = i++;
            while (i < iterations && line[i] != '"') ++i;
            if (i < iterations) ++i;

            int globalStart = lineOffset + start;
            int globalLength = i - start;

            cursor.setPosition(globalStart);
            cursor.setPosition(globalStart + globalLength, QTextCursor::KeepAnchor);
            cursor.setCharFormat(format);
        }
    }

    // Highlight comment
    if (commentStart != -1) {
        format.setForeground(commentColor);
        int globalStart = lineOffset + commentStart;
        int globalLength = line.length() - commentStart;

        cursor.setPosition(globalStart);
        cursor.setPosition(globalStart + globalLength, QTextCursor::KeepAnchor);
        cursor.setCharFormat(format);
    }
}

