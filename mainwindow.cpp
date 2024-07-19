#include "mainwindow.h"
#include "ui_mainwindow.h"

// using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->ui->actionSave->setEnabled(false);


    // this->ui->lineNumPlainTextEdit->setAlignment(Qt::AlignTop);

    // this
    // this->setCentralWidget(ui->horizontalLayoutWidget);
    this->setCentralWidget(ui->stackedWidget);
    this->ui->stackedWidget->setCurrentWidget(this->ui->page);


    QScrollBar* plainTextScrollBar = ui->plainTextEdit->verticalScrollBar();
    // plainTextScrollBar.set
    QScrollBar* lineNumberScrollBar = ui->lineNumPlainTextEdit->verticalScrollBar();
    connect(plainTextScrollBar, &QScrollBar::valueChanged, this, &MainWindow::synchronizeScrollbars);
    connect(lineNumberScrollBar, &QScrollBar::valueChanged, this, &MainWindow::synchronizeScrollbars);

    // connect(plainTextScrollBar, SIGNAL(sliderMoved(int)), lineNumberScrollBar, SLOT(setMaximum(int)));
    // maybe needed ********

    lineNumberScrollBar->hide();

    ui->lineNumPlainTextEdit->viewport()->setCursor(Qt::ArrowCursor); // stops cursor from changing when hovering over line number
    ui->lineNumPlainTextEdit->setStyleSheet("QPlainTextEdit {background-color: black; text-align: center;}");


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createLineNumbersOnFileOpen(int lineNumbers){
    // QString text = this->ui->lineCountLabel->text();
    // QString text = this->ui->lineNumberTextEdit->toPlainText();

    this->ui->stackedWidget->setCurrentWidget(this->ui->page_2);

    this->ui->lineNumPlainTextEdit->clear(); // clears the text in case user opens another file

    for(int thisLineNumber = 1; thisLineNumber <= lineNumbers; thisLineNumber++){
        // QLabel* lineLabel = this->ui->lineCountLabel;
        // QPlainTextEdit* lineLabel = this->ui->lineNumberTextEdit;
        QPlainTextEdit* lineNumLabel = this->ui->lineNumPlainTextEdit;

        QString numberToAdd = QString::number(thisLineNumber);
        // std::cout<< "line numbers " + numberToAdd.toStdString() + " \n";
        // lineLabel->setPlainText(lineLabel->toPlainText() + numberToAdd);
        // lineNumLabel->append(numberToAdd);
        lineNumLabel->appendPlainText(numberToAdd);
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

    // connects the scroll bars of the text box the user types in with the line number text
    // connect(ui->plainTextEdit->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->lineNumPlainTextEdit->verticalScrollBar(), SLOT(setValue(int)));
    // connect(ui->lineNumPlainTextEdit->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->plainTextEdit->verticalScrollBar(), SLOT(setValue(int)));
    // connect(ui->plainTextEdit->verticalScrollBar(), &QScrollBar::valueChanged, ui->lineNumPlainTextEdit->verticalScrollBar(), &QScrollBar::setValue);
    // connect(ui->lineNumPlainTextEdit->verticalScrollBar(), &QScrollBar::valueChanged, ui->plainTextEdit->verticalScrollBar(), &QScrollBar::setValue);

    // this->ui->lineNumPlainTextEdit->verticalScrollBar()->setValue(this->ui->lineNumPlainTextEdit->verticalScrollBar()->maximum());
    // this->ui->plainTextEdit->verticalScrollBar()->setValue(this->ui->plainTextEdit->verticalScrollBar()->maximum());

    synchronizeScrollbars();
    initTerminalBox();
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
    // std::cout<< this->ui->plainTextEdit->verticalScrollBar().valueChanged;
    // QTextStream(stdout) << "number of lines " + numberOfLines;
    return;

}

void MainWindow::on_plainTextEdit_blockCountChanged(int newBlockCount)
{
    if(previousNumberOfLines == 0) {
        return; // so it doesn't append on startup just a singular line with the full amount
    }

    // QTextStream(stdout) << "number of lines " + QString::number(previousNumberOfLines) + " vs new amount " + QString::number(newBlockCount) << Qt::endl;

    QTextStream(stdout) << this->ui->plainTextEdit->document()->documentLayout()->documentSize().height();
    QPlainTextEdit* lineNumberText = this->ui->lineNumPlainTextEdit; // reference to the text edit where the line numbers go
    if(previousNumberOfLines > newBlockCount) {
        // they deleted a line, also works for multiple lines since it does a loop
        int linesToRemove = previousNumberOfLines - newBlockCount;
        QTextCursor cursor(lineNumberText->textCursor());
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);

        for (int i = 0; i < linesToRemove; i++) {
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.deletePreviousChar();
        }

    }
    else{
        // they added one or more lines, append the new line numbers
        int linesToAdd = newBlockCount - previousNumberOfLines;
        for (int i = 1; i <= linesToAdd; i++) {
            lineNumberText->appendPlainText(QString::number(previousNumberOfLines + i));
        }
    }

    previousNumberOfLines = newBlockCount;
    // updates the previous line numbers variable so that it can reflect later on what type of line change was done
}


void MainWindow::synchronizeScrollbars()
{
    // int plaintTextSize = ui->plainTextEdit->verticalScrollBar()->maximum();
    // int lineNumSize = ui->lineNumPlainTextEdit->verticalScrollBar()->maximum();
    // // qDebug()<< "plain text size " + QString::number(plaintTextSize);

    // // qDebug()<< "line num size " + QString::number(lineNumSize);

    int textEditScrollValue = ui->plainTextEdit->verticalScrollBar()->value();
    int lineNumScrollValue = ui->lineNumPlainTextEdit->verticalScrollBar()->value();

    if (textEditScrollValue != lineNumScrollValue) {
        ui->lineNumPlainTextEdit->verticalScrollBar()->setValue(textEditScrollValue);
        ui->plainTextEdit->verticalScrollBar()->setValue(textEditScrollValue);
    }
    // if(plaintTextSize!= lineNumSize){
    //     ui->plainTextEdit->verticalScrollBar()->setMaximum(plaintTextSize);
    //     ui->plainTextEdit->verticalScrollBar()->setMaximum(plaintTextSize);
    // }

    // QCoreApplication::processEvents();
}


void MainWindow::initTerminalBox(){

    process = new QProcess(this);
    connect( process, SIGNAL(readyReadStandardOutput()), this, SLOT(on_StdoutAvailable()) );
    connect(ui->inputTerminalCommand, SIGNAL(returnPressed()), this, SLOT(on_inputTerminalCommand_returnPressed()));

    process->start("cmd.exe");
    process->setWorkingDirectory(currentFile);

}

void MainWindow::on_StdoutAvailable()
{ // when it is ready to be read, it reads the output and prints it to the terminal box
    QByteArray terminalOutput = process->readAllStandardOutput();
    ui->terminalBox->appendPlainText(terminalOutput);
}




void MainWindow::on_inputTerminalCommand_returnPressed()
{
    if (process->isOpen())
    {
        QByteArray ba(ui->inputTerminalCommand->text().toUtf8()  + '\n') ;
        char *ptr = ba.data();
        int numBytes = process->write(ptr);
    }
    // ui->terminalBox->clear();

}

