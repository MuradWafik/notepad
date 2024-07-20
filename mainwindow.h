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

    void createLineNumbersOnFileOpen(int lineNumbers);

    void synchronizeScrollbars();

    void initTerminalBox();
    void on_StdoutAvailable();

    void on_inputTerminalCommand_returnPressed();

    void setUIChanges();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QString currentFile = "";
    int previousNumberOfLines = 0;  // to see if line numbers (block count) are increased or decreased on change
    QProcess *process;
    QString lastCommand;
    bool ignoreNextOutput;
};
#endif // MAINWINDOW_H
