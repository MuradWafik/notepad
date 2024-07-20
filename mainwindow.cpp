#include "mainwindow.h"
#include "ui_mainwindow.h"

// using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->ui->actionSave->setEnabled(false);


    this->setCentralWidget(ui->stackedWidget);
    this->ui->stackedWidget->setCurrentWidget(this->ui->page);


    QScrollBar* plainTextScrollBar = ui->plainTextEdit->verticalScrollBar();
    QScrollBar* lineNumberScrollBar = ui->lineNumPlainTextEdit->verticalScrollBar();

    connect(plainTextScrollBar, &QScrollBar::valueChanged, this, &MainWindow::synchronizeScrollbars);
    connect(lineNumberScrollBar, &QScrollBar::valueChanged, this, &MainWindow::synchronizeScrollbars);

    setUIChanges();
}

MainWindow::~MainWindow()
{
    if (process->isOpen()) {
        process->write("exit\n");
        process->waitForFinished();
    }

    if(outputProcess->isOpen()){
        outputProcess->write("exit\n");
        outputProcess->waitForFinished();
    }




    delete ui;
}

void MainWindow::createLineNumbersOnFileOpen(int lineNumbers){
    this->ui->stackedWidget->setCurrentWidget(this->ui->page_2);
    this->ui->dockWidget_2->showNormal();
    this->ui->lineNumPlainTextEdit->clear(); // clears the text in case user opens another file

    QPlainTextEdit* lineNumLabel = this->ui->lineNumPlainTextEdit;
    for(int thisLineNumber = 1; thisLineNumber <= lineNumbers; thisLineNumber++){


        QString numberToAdd = QString::number(thisLineNumber);
        // lineNumLabel->appendPlainText(numberToAdd);
        lineNumLabel->appendHtml("<span style = 'text-align: right; align: right;'>" +numberToAdd + "</span>");
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
    connect(ui->plainTextEdit->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->lineNumPlainTextEdit->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->lineNumPlainTextEdit->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->plainTextEdit->verticalScrollBar(), SLOT(setValue(int)));

    initTerminalBox();
    initOutputBox(); // the 2 Qprocesses, one for regular commands, one for output from program
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

    QPlainTextEdit* lineNumberText = this->ui->lineNumPlainTextEdit; // reference to the text edit where the line numbers go
    if(previousNumberOfLines > newBlockCount) {
        // they deleted a line, also works for multiple lines since it does a loop
        int linesToRemove = previousNumberOfLines - newBlockCount;
        QTextCursor cursor(lineNumberText->textCursor());
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);

        for (int i = 0; i < linesToRemove; i++){
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
    int textEditScrollValue = ui->plainTextEdit->verticalScrollBar()->value();
    int lineNumScrollValue = ui->lineNumPlainTextEdit->verticalScrollBar()->value();

    if (textEditScrollValue != lineNumScrollValue) {
        ui->lineNumPlainTextEdit->verticalScrollBar()->setValue(textEditScrollValue);
        ui->plainTextEdit->verticalScrollBar()->setValue(textEditScrollValue);
    }

}


void MainWindow::initTerminalBox(){
    process = new QProcess(this);



    process->start("cmd.exe");
    if (!process->waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start the command process.");
        return;
    }
    process->setWorkingDirectory(QFileInfo(currentFile).absolutePath());

    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::on_StdoutAvailable);

    // connect(ui->inputTerminalCommand, &QLineEdit::returnPressed, this, &MainWindow::on_inputTerminalCommand_returnPressed);
}


void MainWindow::initOutputBox(){
    outputProcess = new QProcess(this);

    outputProcess->start("cmd.exe");

    if (!outputProcess->waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start the output command process.");
        return;
    }
    outputProcess->setWorkingDirectory(QFileInfo(currentFile).absolutePath());

    connect(outputProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::outputProgramContents);

}


void MainWindow::on_StdoutAvailable()
{ // when it is ready to be read, it reads the output and prints it to the terminal box
    QByteArray terminalOutput = process->readAllStandardOutput();
    // process->closeReadChannel(process->StandardOutput);
    // qDebug() << terminalOutput;
    ui->terminalBox->appendPlainText(terminalOutput);
}




void MainWindow::on_inputTerminalCommand_returnPressed()
{
    if (process->isOpen()){

        QByteArray inputByteArray(ui->inputTerminalCommand->text().toUtf8() + "\n") ;
        char *userText = inputByteArray.data();
        // int numBytes = process->write(ptr);
        qDebug() << inputByteArray;
        process->write(userText); // inputs the user command into the terminal
        // process->waitForBytesWritten();
        // process->closeWriteChannel();

    }

    ui->inputTerminalCommand->clear(); // clears the input field for the user

}

void MainWindow::setUIChanges(){
    ui->lineNumPlainTextEdit->viewport()->setCursor(Qt::ArrowCursor); // stops cursor from changing when hovering over line number
    ui->lineNumPlainTextEdit->setStyleSheet("QPlainTextEdit {background-color: black; text-align: right;}");
    ui->lineNumPlainTextEdit->verticalScrollBar()->hide();

    this->ui->dockWidget_2->hide();
    ui->outputText->setStyleSheet("QPlainTextEdit{background-color: black;}");
}

void MainWindow::on_pushButton_clicked()
{
    if(outputProcess->isOpen()){
        runPythonCommand = QString("python -u \"%1\"").arg(currentFile);
        QByteArray runFileCommand(runPythonCommand.toUtf8() + "\n") ;
        char *userText = runFileCommand.data();
        qDebug() << runFileCommand;
        outputProcess->write(userText); // inputs the user command into the terminal
        outputProcess->waitForBytesWritten();
        // process->closeWriteChannel();

    }
}


void MainWindow::on_actionNew_Terminal_triggered()
{
    this->ui->dockWidget_2->showNormal(); // if they press new terminal, it shows the widget for them both
}


void MainWindow::outputProgramContents(){
    this->ui->tabWidget->setCurrentWidget(this->ui->outputTab);
    QByteArray programOutput = outputProcess->readAllStandardOutput();


    QString stripped = QString::fromUtf8(programOutput);
    int indexToRemove = stripped.indexOf(currentFile, 0, Qt::CaseInsensitive); // finds where the file path is in the text, and removes everything before
    stripped = stripped.remove(0, indexToRemove);
    // QString stripped = QString::fromUtf8(programOutput).remove(runPythonCommand);
    // stripped = stripped.remove(currentFile); // "removes filler text from the output box like the filepath and the "python -u PATH" messages
    // ui->outputText->appendPlainText(stripped); // changes the tab from terminal to output, and adds the command prompt text to the textbox
    ui->outputText->appendHtml("<span style = 'color: #7FFF00;'>" + stripped + "</span");

}
