#include "syntaxhighlighter.h"
#include <QTextBlock>
#include <QTextCursor>
SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) :
    QSyntaxHighlighter(parent)
{

    HighlightingRule rule;

    keywordFormat.setForeground(keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    rule.pattern = keywordsRegex;
    rule.format = keywordFormat;
    highlightingRules.append(rule);
    // highlightingRules.emplaceBack(keywordsRegex, keywordFormat);

    functionFormat.setForeground(functionColor);
    rule.pattern = functionRegex;
    rule.format = functionFormat;
    highlightingRules.append(rule);
    // highlightingRules.emplaceBack(functionRegex, functionFormat);


    classFormat.setForeground(classColor);
    rule.pattern = classRegex;
    rule.format = classFormat;
    highlightingRules.append(rule);
    // highlightingRules.emplaceBack(classRegex, classFormat);

    singleLineStringFormat.setForeground(stringColor);
    rule.format = singleLineStringFormat;
    rule.pattern = singleLineStringRegex;
    highlightingRules.append(rule);
    // highlightingRules.emplaceBack(singleLineStringRegex, singleLineStringFormat);

    commentFormat.setForeground(commentColor);
    rule.format = commentFormat;
    rule.pattern = commentRegex;
    highlightingRules.append(rule);
    // highlightingRules.emplace_back(commentRegex, commentFormat);
}


// void SyntaxHighlighter::extractStringsAndComments(const QString& line, int lineOffset)
// {
//     bool inString = false;
//     int commentStart = -1;

//     for (int i = 0; i < line.length(); ++i) {
//         QChar c = line[i];
//         if (c == '"') {
//             inString = !inString;
//         }
//         else if (c == '#' && !inString) {
//             commentStart = i;
//             break;
//         }
//     }

//     QTextCursor cursor{pte->document()};
//     QTextCharFormat format{};
//     // highlighting strings before the comment
//     format.setForeground(stringColor);
//     // inString = false;
//     int iterations = (commentStart == -1 ? line.length() : commentStart);
//     for (int i = 0; i < iterations; ++i) {
//         if (line[i] == '"') {
//             int start = i++;
//             while ((i < iterations) && (line[i] != '"')) {
//                 ++i;
//             }
//             if (i < iterations) ++i;

//             int globalStart = lineOffset + start;
//             int globalLength = i - start;

//             cursor.setPosition(globalStart);
//             cursor.setPosition(globalStart + globalLength, QTextCursor::KeepAnchor);
//             cursor.setCharFormat(format);
//         }
//     }

//     // highlight comment
//     if (commentStart != -1) {
//         format.setForeground(commentColor);
//         int globalStart = lineOffset + commentStart;
//         int globalLength = line.length() - commentStart;

//         cursor.setPosition(globalStart);
//         cursor.setPosition(globalStart + globalLength, QTextCursor::KeepAnchor);
//         cursor.setCharFormat(format);
//     }
// }

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
