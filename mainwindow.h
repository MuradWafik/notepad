#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QDebug>
#include <QTextBlock>
#include <QProcess>
#include <QTreeView>
#include <QFileSystemModel>
#include <QCheckBox>
#include <QVector>
#include <QTextDocumentFragment>
#include "searchandreplace.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    inline static QString getShellCommand(){ // differentiates the terminial start based on the operating system
#ifdef _WIN32 // maybe change to user choice, maybe could use powershell
        return "cmd.exe";
#else
        return "/bin/sh";
#endif
    }

    void getAllFilesInDirectory();
    void getAllFilesInDirectory(QString &directory);
    void createLineNumbersOnFileOpen(int lineNumbers);
    void setUIChanges();

    void commentLines();
    void addComments();
    void removeComments();

    void initTerminalBox();
    void connectSignals();


private slots:
    void openFileAction();
    void saveAs();

    void updateStatusBarCursorPosition(); // update the text of Line Number and Coloumn number on the bottom status bar

    void calculateNumberOfLines(int newBlockCount); // technically name isnt the best, it also changes the numberings on the line number tab


    void synchronizeScrollbars(); // match the scroll bar value between the main text aream and the line numbers

    void on_StdoutAvailable(); // terminal output
    void on_StderrAvailable(); // errors from the terminal

    void writeToTerminal(); // pressing return in the line edit..

    void runButton();
    void showTerminal();

    void openFileWhileEditing(const QString& filePath);

    void openFile(const QString &filePath);

    void updateTerminalAndOutput();
    void adjustSearchLineEditPosition();
    void resizeEvent(QResizeEvent*) override;

    void saveFile();
    void createSearchAndReplaceWidgets();

    void openFolderDialog();
    void updateWindowTitle();

    void showCustomContextMenu(const QPoint &pos);

    void createPythonFile(const QPoint &clickPoint);
    void createTextFile(const QPoint &clickPoint);

    void newPythonFile();

    void newTextFile();

protected:
    // void keyPressEvent(QKeyEvent *event) override;  // Declaration of keyPressEvent
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::MainWindow *ui;
    QString currentFile = "";
    int previousNumberOfLines = 0;  // to see if line numbers (block count) are increased or decreased on change
    QProcess *process;

    QString runPythonCommand;
    QString fileContentAfterSave;

    QFileSystemModel *fileModel; // the file explorer  on the left for treeview


    QVector<QTextCursor> foundOccurrences;
    QString startTerminalCommand;

    QLabel* lineAndColStatusLabel;
    QLabel* searchAndReplaceStatusLabel;
    SearchAndReplace* searchReplaceWidget;
    // searchAndReplaceObject obj;
};
#endif // MAINWINDOW_H
