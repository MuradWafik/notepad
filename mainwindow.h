#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile> // open files
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QObject>
#include <QScrollBar>
// #include <QPrinter>
// #include <QPrintDialog>
#include <QDebug>
#include <QTextBlock>
#include <QDebug>
#include <QProcess>
#include <QTreeView>
#include <QFileSystemModel>

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

private slots:
    void on_actionOpen_File_triggered();

    void on_actionSave_As_triggered();

    void on_actionSave_triggered();

    void on_plainTextEdit_cursorPositionChanged();

    void on_plainTextEdit_blockCountChanged(int newBlockCount);

    void createLineNumbersOnFileOpen(const int lineNumbers);

    void synchronizeScrollbars();

    void initTerminalBox();
    void on_StdoutAvailable();

    void on_inputTerminalCommand_returnPressed();

    void setUIChanges();

    void on_pushButton_clicked();



    void on_actionShow_Terminal_triggered();

    void on_actionHide_Terminal_triggered();

    void getAllFilesInDirectory();

    void on_fileListTree_doubleClicked(const QModelIndex &index);

    void openFile(const QString filePath);

    void updateTerminalAndOutput();
    void adjustSearchLineEditPosition();
    void resizeEvent(QResizeEvent*);

    void searchForText(const QString &text);
    void removeHighlights();



private:
    Ui::MainWindow *ui;
    QString currentFile = "";
    int previousNumberOfLines = 0;  // to see if line numbers (block count) are increased or decreased on change
    QProcess *process;
    // QProcess *outputProcess;
    QString runPythonCommand;
    QString fileContentAfterSave;


    QFileSystemModel *fileModel; // the file explorer  on the left for treeview

    QString outputContents; // for the output tab, it compares the diffrence for what the user input is

    QLineEdit* searchLineEdit;

    int foundTextMatchCount;

};
#endif // MAINWINDOW_H
