#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QTabWidget>
#include "searchandreplace.h"
#include "syntaxhighlighter.h"

class editor : public QWidget
{
    Q_OBJECT
public:
    explicit editor(QTabWidget *parent);
    ~editor();

    // multiple methods that just call on the same for the main plaintTextEdit
    inline QString getText() const{
        return textEdit->toPlainText();
    }

    inline void setText(const QString& text){
        textEdit->setPlainText(text);
    }
    // i guess compiler can access private members on inlined functions
    inline int blockCount() const{
        return textEdit->blockCount();
    }
    inline QPlainTextEdit* getPte() const{
        return textEdit;
    }

    void commentLines(); // base functionality after clicking Ctrl + /, checks what actions should be done
    void addComments(); // adds comments to blocks of code
    void removeComments(); // uncomments a block of comments

    void saveFile();
    void saveAs();

    // true means there are changes not saved in the file (for actions like opening another)
    inline bool unsavedChanges(){
        return (!textEdit->toPlainText().isEmpty() && textEdit->document()->isModified());
    }

    inline QString fileName(){
        return currentFile;
    }

    void openFile(QFile& file);
    void showSearchAndReplace();

private:
    QPlainTextEdit *textEdit, *lineNumberTextEdit;
    QHBoxLayout *layout;
    const inline static QFont font{"Courier"};
    int previousNumberOfLines = 0;
    QString currentFile;
    QTabWidget* parent;
    SearchAndReplace* searchAndReplace;
    SyntaxHighlighter syntaxHighlighter;

private:
    void createLineNumbersOnFileOpen(int lineNumbers);
private slots:
    void synchronizeScrollBars(); // matches the scroll value for the text and the line numbers
    void calculateNumberOfLines(int newBlockCount);
    void updateTabTitle(); // add the * to the tab title if it has unsaved changes

    void resizeEvent(QResizeEvent*) override;
};


#endif // EDITOR_H
