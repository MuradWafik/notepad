#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QRegularExpression>
#include <QStringView>
#include <QColor>
#include <QPlainTextEdit>
struct match{
    qsizetype start, end; // choosing to do end for less syntax to see if something is contained within another, like a comment within a string to avoid (hopefully)
};


class SyntaxHighlighter
{
public:
    // startOffset = character index to start search from... no need to rerun the regex on start of file when they append at end
    static void searchTextForMatches(QStringView text, qsizetype startOffset = 0);

    static void highlightText(QStringView text, QPlainTextEdit* pte, qsizetype startOffset = 0);
public:
    // switch to file later on to allow it to change (themes...)
    inline static QColor commentColor{22, 120, 13};
    inline static QColor stringColor{120, 88, 13};
    inline static QColor keywordColor{28, 76, 189};
    inline static QColor classColor{186, 194, 31};
    inline static QColor functionColor{145, 20, 47};
private:

    static inline void populateMatchSet(QSet<match>& set, const QRegularExpressionMatchIterator& matchIterator){
        for(const auto& match : matchIterator){
            set.insert({match.capturedStart(), match.capturedEnd()});
        }
    }

    static void highlightType(const QSet<match>& toHighlight,const QColor& color, QPlainTextEdit* pte);



private:

    const inline static QRegularExpression stringRegex{R"("""(?:.|\n)*?"""|\'\'\'(?:.|\n)*?\'\'\'|\"(?:\\.|[^\"\\])*\"|\'(?:\\.|[^\\"\\])*\')"};
    inline static QSet<match> strings{};

    const inline static QRegularExpression commentRegex{R"(\#.*)"};
    inline static QSet<match> comments{};

    const inline static QRegularExpression keywordsRegex{R"(\b(def|class|if|else|elif|return|import|from|while|for|in|try|except|with|as|pass|yield|async|await|None|True|False)\b)"};
    inline static QSet<match> keywords{};

    const inline static QRegularExpression functionRegex{R"(\bdef\s+([a-zA-Z_][a-zA-Z0-9_]*)\b)"};
    inline static QSet<match> functions{};

    const inline static QRegularExpression classRegex{R"(\bclass\s+([a-zA-Z_][a-zA9_]*)\b)"};
    inline static QSet<match> classes{};

};

inline size_t qHash(const match& m, size_t seed = 0) { // seed is sometimes used for nested hashes? something for QT
    return size_t(m.start) ^ (size_t(m.end) << 1) ^ seed;
}

inline bool operator==(const match& lhs, const match& rhs) {
    return lhs.start == rhs.start && lhs.end == rhs.end;
}

#endif // SYNTAXHIGHLIGHTER_H
