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
#include "editor.h"

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
    void getAllFilesInDirectory();
    void getAllFilesInDirectory(QString &directory);
    void setUIChanges();

    void initTerminalBox();
    void connectSignals();


private slots:
    void openFileAction();

    void updateStatusBarCursorPosition(); // update the text of Line Number and Coloumn number on the bottom status bar

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

    void createSearchAndReplaceWidgets();

    void openFolderDialog();

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
    QProcess *process;

    QFileSystemModel *fileModel; // the file explorer  on the left for treeview

    editor* openEditor = nullptr;
    QVector<QTextCursor> foundOccurrences;

    QLabel* lineAndColStatusLabel;
    QLabel* searchAndReplaceStatusLabel;
    SearchAndReplace* searchReplaceWidget;
    // searchAndReplaceObject obj;
};
#endif // MAINWINDOW_H
