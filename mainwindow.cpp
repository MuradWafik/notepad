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

    // connects the scroll bars of the text box the user types in with the line number text
    connect(ui->plainTextEdit->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->lineNumPlainTextEdit->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->lineNumPlainTextEdit->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->plainTextEdit->verticalScrollBar(), SLOT(setValue(int)));

    setUIChanges();

    process = new QProcess(this);
    outputProcess = new QProcess(this);


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

void MainWindow::createLineNumbersOnFileOpen(const int lineNumbers){
    this->ui->stackedWidget->setCurrentWidget(this->ui->page_2); // sets the page to the text editor page

    this->ui->dockWidget_4->showNormal();
    this->ui->dockWidget_2->showNormal(); // shows both docks, file explorer, and output

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
        QMessageBox::warning(this, "Warning", "Can Not Open File: " + file.errorString());

        return;
    }


    openFile(fileName);
    getAllFilesInDirectory();


}


void MainWindow::on_actionSave_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save As");
    QFile file(fileName);

    if(!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(this, "Warning", "Can Not Save File: " + file.errorString());
        return;
    }

    currentFile = fileName;
    this->ui->actionSave->setEnabled(true); // just like open, if they saved as, they can just do quick save to save to that location
    setWindowTitle(fileName);
    QTextStream out(&file);
    QString currentText = ui->plainTextEdit->toPlainText();
    out<< currentText; // writes to the text file chosen what is in the text box
    file.close();

    fileContentAfterSave = currentText;

}


void MainWindow::on_actionSave_triggered()
{
    QFile file(currentFile);

    if(!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(this, "Warning", "Unable To Save File: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    QString currentText = ui->plainTextEdit->toPlainText();
    out<< currentText; // writes to the text file chosen what is in the text box
    file.close();

    fileContentAfterSave = currentText; // stores it to compare if the user has modified the text for switching documents
    // ie if they try to open something else, will check first if saved text before opening
    // if ineffieicnet could just use a bool that is set to true here and save as, then swapped to false if any text is typed in the main text box

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

    this->ui->terminalBox->clear(); // clears the text in case they are switching files
    // maybe remove, or leave to a setting if they want to



    if(process->isOpen()){
        qDebug() << "open";
    }

    process = new QProcess(this);
    process->start("cmd.exe");
    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::on_StdoutAvailable);







    if (!process->waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start the command process.");
        return;
    }
    process->setWorkingDirectory(QFileInfo(currentFile).absolutePath());



    // connect(ui->inputTerminalCommand, &QLineEdit::returnPressed, this, &MainWindow::on_inputTerminalCommand_returnPressed);
}


void MainWindow::initOutputBox(){

    this->ui->outputText->clear();

    // qDebug() << " open ";
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
    this->ui->dockWidget_4->hide();

    ui->outputText->setStyleSheet("QPlainTextEdit{background-color: black;}");

    // splitDockWidget(ui->dockWidget_3, ui->dockWidget_2, Qt::Orientation::Horizontal);
    // tabifyDockWidget()
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




void MainWindow::outputProgramContents(){
    this->ui->tabWidget->setCurrentWidget(this->ui->outputTab);
    QByteArray programOutput = outputProcess->readAllStandardOutput();

    QString stripped = QString::fromUtf8(programOutput);
    int indexToRemove = stripped.indexOf(currentFile, 0, Qt::CaseInsensitive); // finds where the file path is in the text, and removes everything before
    // stripped = stripped.remove(0, indexToRemove);
    if (indexToRemove != -1) {
        stripped = stripped.remove(0, indexToRemove);
    }


    stripped = QString::fromUtf8(programOutput).remove(runPythonCommand);
    stripped = stripped.remove(currentFile); // "removes filler text from the output box like the filepath and the "python -u PATH" messages
    ui->outputText->appendHtml("<span style = 'color: #7FFF00;'>" + stripped + "</span");
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea); // makes the file explorer, whether right or left fill the space instead of the terminal

}

void MainWindow::on_actionShow_Terminal_triggered()
{

    this->ui->dockWidget_2->showNormal(); // if they press new terminal, it shows the widget for them both
}


void MainWindow::on_actionHide_Terminal_triggered()
{
    this->ui->dockWidget_2->hide();
}

void MainWindow::getAllFilesInDirectory(){
    // QDir directory(currentFile);
    QDir directory = QFileInfo(currentFile).dir();

    // QStringList files = directory.entryList(QStringList() << "*.py" << "*.txt",QDir::Files);


    fileModel = new QFileSystemModel; // the file explorer  on the left for treeview
    // fileModel->setRootPath(directory.path());
    fileModel->setRootPath(currentFile);

    this->ui->fileListTree->setModel(fileModel);

    // ui->fileListTree->setRootIndex(model->index(QDir::currentPath()));
    ui->fileListTree->setRootIndex(fileModel->index(directory.path()));

    // qDebug() << directory.absolutePath();

}


void MainWindow::on_fileListTree_doubleClicked(const QModelIndex &index)
{

    QString fileToOpenPath = fileModel->filePath(index);

    // the current text on the document was altered and not saved
    if(ui->plainTextEdit->toPlainText() != fileContentAfterSave){
        QMessageBox::StandardButton saveFileQuestion = QMessageBox::question(this, "Save Before Swap",
                                                                             "Would You Like To Save the Previous File Before Switching?",
                                                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        // the default answer is yes, so if they close or somehow dont answer it is a yes

        if(saveFileQuestion == QMessageBox::Yes){
            qDebug()<< "its a yes";
        }
        else{
            qDebug()<< "its a no";
        }


    }

    openFile(fileToOpenPath);

    // //get full path
    // QString path = fileModel->filePath(index);
    // qDebug() << "path " + path;

}


void MainWindow::openFile(const QString filePath){


    QFile file(filePath);


    if(!file.open(QIODevice::ReadOnly | QFile::Text)){
        QMessageBox::warning(this, "Warning", "Can Not Open File " + file.errorString());

        return;
    }


    this->ui->actionSave->setEnabled(true); // if they successfully opened a file, the save button can be used for it
    currentFile = filePath;
    setWindowTitle(filePath);
    QTextStream in(&file);

    QString text = in.readAll();
    ui->plainTextEdit->setPlainText(text);

    fileContentAfterSave = text;


    previousNumberOfLines = this->ui->plainTextEdit->blockCount();
    // the number of lines for the line counter, also stores the variable to see if the change was line added or removed
    createLineNumbersOnFileOpen(previousNumberOfLines);
    file.close();



    /* initTerminalBox();
    initOutputBox();*/ // the 2 Qprocesses, one for regular commands, one for output from program

    updateTerminalAndOutput();
    // getAllFilesInDirectory();

}


void MainWindow::updateTerminalAndOutput(){


        qDebug() << "Update Terminal and Output";

        // qDebug() << "Process state: " << process->state();
        // qDebug() << "Output process state: " << outputProcess->state();

        // if (process->state() == QProcess::Running) {
        //     process->setWorkingDirectory(QFileInfo(currentFile).absolutePath());
        //     outputProcess->setWorkingDirectory(QFileInfo(currentFile).absolutePath());
        // } else if (process->state() == QProcess::NotRunning) {
        //     qDebug() << "Processes not running. Initializing...";
        //     initTerminalBox();
        //     initOutputBox();
        // }

        if(process->state() == QProcess::Running){
            process->setWorkingDirectory(QFileInfo(currentFile).absolutePath());
            outputProcess->setWorkingDirectory(QFileInfo(currentFile).absolutePath());
        }
        else if(process->state() == QProcess::Starting){
            // process->start("cmd.exe");
            // outputProcess->start("cmd.exe");
        }
        else if(process->state() == QProcess::NotRunning){
            qDebug()<< "here";
            // if the processes arent running, initialize them
            initTerminalBox();
            initOutputBox();
        }



}


void MainWindow::on_userInputRunningProgram_returnPressed()
{


    if (!outputProcess->isOpen()){
        return;
    }

    ui->userInputRunningProgram->clear(); // clears the input field for the user

    QByteArray programOutput = outputProcess->readAllStandardOutput();
    QByteArray errorOutput = outputProcess->readAllStandardError();
    QString outputString = QString::fromUtf8(programOutput);
    QString errorString = QString::fromUtf8(errorOutput);


    QByteArray inputByteArray(ui->userInputRunningProgram->text().toUtf8() + "\n") ;
    char *userText = inputByteArray.data();


    outputProcess->write(userText); // inputs the user command into the terminal




    if (programOutput.isEmpty() && errorOutput.isEmpty() && outputProcess->state() == QProcess::Running) {
        ui->userInputRunningProgram->setEnabled(true);
        qDebug() << "can input item";
    } else {
        ui->userInputRunningProgram->setEnabled(false);
    }

}

