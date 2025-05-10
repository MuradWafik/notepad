#include "syntaxhighlighter.h"

void SyntaxHighlighter::searchTextForMatches(QStringView text, qsizetype startOffset){


    QRegularExpressionMatchIterator commentIterator = commentRegex.globalMatchView(text, startOffset);
    populateMatchSet(comments, commentIterator);

    QRegularExpressionMatchIterator keywordsIterator = keywordsRegex.globalMatchView(text, startOffset);
    populateMatchSet(keywords, keywordsIterator);

    QRegularExpressionMatchIterator stringIterator = stringRegex.globalMatchView(text, startOffset);
    populateMatchSet(strings, stringIterator);

    QRegularExpressionMatchIterator functionIterator = classRegex.globalMatchView(text, startOffset);
    populateMatchSet(functions, functionIterator);

    QRegularExpressionMatchIterator classIterator = classRegex.globalMatchView(text, startOffset);
    populateMatchSet(classes, classIterator);
}


void SyntaxHighlighter::highlightText(QStringView text, QPlainTextEdit* pte, qsizetype startOffset){
    searchTextForMatches(text, startOffset);

    highlightType(comments, commentColor, pte);
    highlightType(strings, stringColor, pte);
    highlightType(keywords, keywordColor, pte);
    highlightType(classes, classColor, pte);
    highlightType(functions, functionColor, pte);

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

    // QTextCharFormat plainFormat(highlightCursor.charFormat());

    // QTextCharFormat colorFormat = plainFormat;
    // colorFormat.setBackground(color);
    // for(const auto& match : toHighlight){



    //     highlightCursor = document->find()
    //     while (!highlightCursor.isNull() && !highlightCursor.atEnd()){
    //         // highlightCursor = document->find(text, highlightCursor, flag);
    //         // highlightCursor{}

    //         if (!highlightCursor.isNull()){
    //             // highlightCursor.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);
    //             highlightCursor.mergeCharFormat(colorFormat);

    //         }
    //     }
    //     cursor.endEditBlock();
    // }

}
