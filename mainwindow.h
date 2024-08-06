#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile> // open files
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
#include "searchandreplaceobject.h"

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
    void on_StderrAvailable(); // errors from the terminal

    void on_inputTerminalCommand_returnPressed();

    void setUIChanges();

    void on_pushButton_clicked();



    void on_actionShow_Terminal_triggered();

    void on_actionHide_Terminal_triggered();

    void getAllFilesInDirectory();
    void getAllFilesInDirectory(QString directory);

    void on_fileListTree_doubleClicked(const QModelIndex &index);

    void openFile(const QString filePath);

    void updateTerminalAndOutput();
    void adjustSearchLineEditPosition();
    void resizeEvent(QResizeEvent*);


    void saveFile();

    void createSearchAndReplaceWidgets();



    void on_actionFind_Replace_triggered();


    void on_actionOpen_Folder_triggered();

    void on_actionUndo_triggered();
    void updateWindowTitle();

    QString getShellCommand();

    void on_actionShow_File_P_triggered();

    void on_actionClear_Terminal_triggered();

    void on_actionRedo_triggered();

    void on_actionSelect_All_triggered();

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

    bool textIsSameAfterSave;

    // QWidget* statusBarWidget;
    // QHBoxLayout* statusBarLayout;
    QLabel* lineAndColStatusLabel;
    QLabel* searchAndReplaceStatusLabel;
    searchAndReplaceObject* searchReplaceWidget;
    // searchAndReplaceObject obj;
};
#endif // MAINWINDOW_H
