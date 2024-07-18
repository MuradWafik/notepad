#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTextBlock>
// #include <iostream>
// #include <string>
// using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->horizontalLayoutWidget);
    this->ui->actionSave->setEnabled(false);

    this->ui->lineNumTextEdit->setAlignment(Qt::AlignRight);
    // this->ui->lineNumTextEdit->setAlignment(Qt::AlignTop);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::createLineNumbersOnFileOpen(int lineNumbers){

    // QString text = this->ui->lineCountLabel->text();
    // QString text = this->ui->lineNumberTextEdit->toPlainText();
    QString text = this->ui->lineNumTextEdit->toPlainText();

    for(int thisLineNumber = 1; thisLineNumber <= lineNumbers; thisLineNumber++){
        // QLabel* lineLabel = this->ui->lineCountLabel;
        // QPlainTextEdit* lineLabel = this->ui->lineNumberTextEdit;
        QTextEdit* lineNumLabel = this->ui->lineNumTextEdit;

        QString numberToAdd = QString::number(thisLineNumber);
        // std::cout<< "line numbers " + numberToAdd.toStdString() + " \n";
        // lineLabel->setPlainText(lineLabel->toPlainText() + numberToAdd);
        lineNumLabel->append(numberToAdd);
    }
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, ("Choose File To Open"));
    QFile file(fileName);


    if(!file.open(QIODevice::ReadOnly | QFile::Text)){
        // QMessageBox::warning(this, "Warning", "Can Not Open File: ", file.errorString());
        QMessageBox::warning(this, "Warning", "Can Not Open File", file.errorString());
        return;
    }

    this->ui->plainTextEdit->setEnabled(true); // enables the text box so user can type in it after opening file


    currentFile = fileName;
    this->ui->actionSave->setEnabled(true); // if they successfully opened a file, the save button can be used for it


    setWindowTitle(fileName);
    QTextStream in(&file);
    QString text = in.readAll();
    ui->plainTextEdit->setPlainText(text);


    previousNumberOfLines = this->ui->plainTextEdit->blockCount();
    // the number of lines for the line counter, also stores the variable to see if the change was line added or removed
    createLineNumbersOnFileOpen(previousNumberOfLines);


    file.close();

}


void MainWindow::on_actionSave_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save As");
    QFile file(fileName);

    if(!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(this, "Warning", "Can Not Save File: ", file.errorString());
        return;
    }

    currentFile = fileName;
    this->ui->actionSave->setEnabled(true); // just like open, if they saved as, they can just do quick save to save to that location
    setWindowTitle(fileName);
    QTextStream out(&file);
    QString currentText = ui->plainTextEdit->toPlainText();
    out<< currentText; // writes to the text file chosen what is in the text box
    file.close();

}


void MainWindow::on_actionSave_triggered()
{
    // qDebug(currentFile.toStdString());
    // bool hasFileToSaveTo = !currentFile.isEmpty();
    // QString text;
    // if(!hasFileToSaveTo){
    //     // text = currentFile;
    // }
    // else{

    //     text = "no file";
    // }
    // QTextStream(stdout) << "current file: " + text;

    QFile file(currentFile);

    if(!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(this, "Warning", "Unable To Save File: ", file.errorString());
        return;
    }

    QTextStream out(&file);
    QString currentText = ui->plainTextEdit->toPlainText();
    out<< currentText; // writes to the text file chosen what is in the text box
    file.close();

}

void MainWindow::on_plainTextEdit_cursorPositionChanged()
{

    // QTextStream(stdout) << "number of lines " + numberOfLines;
    return;
//     QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(sender());
//     Q_ASSERT(edit);
//     QTextCursor cursor = edit->textCursor();
//     cursor.movePosition(QTextCursor::StartOfLine);

//     int lines = 1;
//     while(cursor.positionInBlock()>0) {
//         cursor.movePosition(QTextCursor::Up);
//         lines++;
//     }
//     QTextBlock block = cursor.block().previous();

//     while(block.isValid()) {
//         lines += block.lineCount();
//         block = block.previous();
//     }
//     ui->plainTextEdit->setPlainText(QString::number(lines));
}


// void MainWindow::on_plainTextEdit_blockCountChanged(int newBlockCount)
// {

//     if(previousNumberOfLines == 0){
//         return; // so it doesnt append on startup just a singular line with the full ammount
//     }
//     // int numberOfLines = this->ui->plainTextEdit->blockCount();
//     QTextStream(stdout) << "number of lines " + QString::number(previousNumberOfLines) + " vs new ammount " + QString::number(newBlockCount) << Qt::endl;


//     // QPlainTextEdit* lineNumberText = this->ui->lineNumberTextEdit;
//     QTextEdit* lineNumberText = this->ui->lineNumTextEdit;
//     if(previousNumberOfLines > newBlockCount){
//         // they deleted a line



//     }
//     else{
//     // this->ui->lineNumberTextEdit->setPlainText()
//     // lineNumberText->setPlainText(lineNumberText->toPlainText() + QString::number(newBlockCount) + "\n"); // appends the new line number to the end
//     lineNumberText->append(QString::number(newBlockCount));
//     }

//     previousNumberOfLines = newBlockCount; // updates the previous number of line variables so it can detect later changes
// }

void MainWindow::on_plainTextEdit_blockCountChanged(int newBlockCount)
{
    if(previousNumberOfLines == 0) {
        return; // so it doesn't append on startup just a singular line with the full amount
    }

    QTextStream(stdout) << "number of lines " + QString::number(previousNumberOfLines) + " vs new amount " + QString::number(newBlockCount) << Qt::endl;

    QTextEdit* lineNumberText = this->ui->lineNumTextEdit;
    if(previousNumberOfLines > newBlockCount) {
        // They deleted one or more lines, remove the corresponding line numbers
        int linesToRemove = previousNumberOfLines - newBlockCount;
        QTextCursor cursor(lineNumberText->textCursor());
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);

        for (int i = 0; i < linesToRemove; i++) {
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.deletePreviousChar();
        }
    } else {
        // they added one or more lines, append the new line numbers
        int linesToAdd = newBlockCount - previousNumberOfLines;
        for (int i = 1; i <= linesToAdd; i++) {
            lineNumberText->append(QString::number(previousNumberOfLines + i));
        }
    }

    previousNumberOfLines = newBlockCount; // Updates the previous number of lines variable so it can detect later changes
}


// void print(QString Text){
//     QTextStream(stdout) << "number of l"
// }



