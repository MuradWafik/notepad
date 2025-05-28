#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QRegularExpression>
#include <QStringView>
#include <QColor>
#include <QPlainTextEdit>

struct match{
    qsizetype start, end; // choosing to do end for less syntax to see if something is contained within another, like a comment within a string to avoid (hopefully)
};

// hashing functions to be placed in a set
inline size_t qHash(const match& m, size_t seed = 0) { // seed is sometimes used for nested hashes? something for QT
    return size_t(m.start) ^ (size_t(m.end) << 1) ^ seed;
}

inline bool operator==(const match& lhs, const match& rhs) {
    return lhs.start == rhs.start && lhs.end == rhs.end;
}


class SyntaxHighlighter
{
public:
    SyntaxHighlighter(QPlainTextEdit* pte);
    // methods are now instanced, where each editor has its own syntaxhighlighter

    // startOffset = character index to start search from... no need to rerun the regex on start of file when they append at end
    void highlightText(qsizetype startOffset = 0);
public:
    // switch to file later on to allow it to change (themes...)
    inline static QColor commentColor{22, 120, 13};
    inline static QColor stringColor{120, 88, 13};
    inline static QColor keywordColor{28, 76, 189};
    inline static QColor classColor{186, 194, 31};
    inline static QColor functionColor{145, 20, 47};
private:

    inline void populateMatchSet(QSet<match>& set, const QRegularExpressionMatchIterator& matchIterator){
        for(const auto& match : matchIterator){
            set.insert({match.capturedStart(), match.capturedEnd()});
        }
    }

    void searchTextForMatches(qsizetype startOffset);

    void highlightType(const QSet<match>& toHighlight,const QColor& color);

    /*
     * Since Regex system does not support variable length lookbehind, can not do checks for whether a '#' is actually within a string
     * or a '"' is within a comment, to solve this, a function is used to extract
     */
    void extractStringsAndComments(const QString& line, int lineOffset);

    // do reapply the highlights whilst editting, it must be stripped of all highlights from that point
    void resetFormating(qsizetype start);


private:
    QPlainTextEdit* pte;
    QSet<match> strings{};

    QSet<match> comments{};

    const inline static QRegularExpression keywordsRegex{R"(\b(def|class|if|else|elif|return|import|from|while|for|in|try|except|with|as|pass|yield|async|await|None|True|False)\b)"};
    QSet<match> keywords{};

    const inline static QRegularExpression functionRegex{R"(\bdef\s+([a-zA-Z_][a-zA-Z0-9_]*)\b)"};
    QSet<match> functions{};

    const inline static QRegularExpression classRegex{R"(\bclass\s+([a-zA-Z_][a-zA9_]*)\b)"};
    QSet<match> classes{};

    int oldOffset = 0;
    // tracker of where the last offset was of highlighting,
    // since if you adding to the end, no need to redo all highlights
    // but if its from an earlier line, probably better to do so and clear the set of past matches

};

#endif // SYNTAXHIGHLIGHTER_H
