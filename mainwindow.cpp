#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , process (new QProcess(this))
{
    ui->setupUi(this);

    this->ui->actionSave->setEnabled(false);
    this->setCentralWidget(ui->stackedWidget);
    this->ui->stackedWidget->setCurrentWidget(this->ui->page);

    createSearchAndReplaceWidgets();

    QScrollBar* plainTextScrollBar = ui->plainTextEdit->verticalScrollBar();
    QScrollBar* lineNumberScrollBar = ui->lineNumPlainTextEdit->verticalScrollBar();

    connect(plainTextScrollBar, &QScrollBar::valueChanged, this, &MainWindow::synchronizeScrollbars);
    connect(lineNumberScrollBar, &QScrollBar::valueChanged, this, &MainWindow::synchronizeScrollbars);
    // connects the scroll bars of the text box the user types in with the line number text

    setUIChanges();

    connect(searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::searchForText);
}

MainWindow::~MainWindow()
{
    if (process->isOpen()) {
        process->write("exit\n");
        process->waitForFinished();
    }

    delete process;
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
        lineNumLabel->appendPlainText(numberToAdd);
    }
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, ("Choose File To Open"));

    openFile(fileName);
    getAllFilesInDirectory();
    adjustSearchLineEditPosition();
}


void MainWindow::on_actionSave_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save As");
    if (fileName.isEmpty()) {
        return;  // If the user cancels the save dialog, do nothing.
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Warning", "Can Not Save File: " + file.errorString());
        return;
    }

    currentFile = fileName;
    this->ui->actionSave->setEnabled(true); // can save now since a file is selected
    setWindowTitle(fileName);

    QTextStream out(&file);
    QString currentText = ui->plainTextEdit->toPlainText();
    out << currentText;  // sets the text content of that file equal to the current text
    file.close();

    fileContentAfterSave = currentText;  // updates saved content tracker.

}



void MainWindow::on_actionSave_triggered()
{
    saveFile();
}



void MainWindow::on_plainTextEdit_cursorPositionChanged()
{ // possibly later a tracker for current line num and column
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

    process->start("cmd.exe");
    // process = new QProcess(this);

    if (!process->waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start the command process.");
        return;
    }
    process->setWorkingDirectory(QFileInfo(currentFile).absolutePath());
    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::on_StdoutAvailable);
}


void MainWindow::on_StdoutAvailable()
{ // when it is ready to be read, it reads the output and prints it to the terminal box
    if(!process->isOpen()){
        qDebug() << "not open";
        return;
    }
    QByteArray terminalOutput = process->readAllStandardOutput();
    ui->terminalBox->appendPlainText(terminalOutput);
}


void MainWindow::on_inputTerminalCommand_returnPressed()
{
    if (process->isOpen()){
        QByteArray inputByteArray(ui->inputTerminalCommand->text().toUtf8() + "\n") ;
        char *userText = inputByteArray.data();

        process->write(userText); // inputs the user command into the terminal
    }
    ui->inputTerminalCommand->clear(); // clears the input field for the user
}

void MainWindow::setUIChanges(){
    ui->lineNumPlainTextEdit->viewport()->setCursor(Qt::ArrowCursor); // stops cursor from changing when hovering over line number
    ui->lineNumPlainTextEdit->setStyleSheet("QPlainTextEdit {background-color: black;}");
    ui->lineNumPlainTextEdit->verticalScrollBar()->hide();

    this->ui->dockWidget_2->hide();
    this->ui->dockWidget_4->hide();

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea); // makes the file explorer, whether right or left fill the space instead of the terminal

    searchAndReplaceContainer->hide();
}

void MainWindow::on_pushButton_clicked()
{
    if(process->isOpen()){
        runPythonCommand = QString("python -u \"%1\"").arg(currentFile);
        QByteArray runFileCommand(runPythonCommand.toUtf8() + "\n") ;
        char *userText = runFileCommand.data();

        process->write(userText); // inputs the user command into the terminal
        process->waitForBytesWritten();
    }
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

    ui->fileListTree->setRootIndex(fileModel->index(directory.path()));
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
            saveFile();
        }
        else{
            // does nothing for now ,, just moves on and opens the other file
        }
    }

    openFile(fileToOpenPath);
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

    updateTerminalAndOutput();
    // getAllFilesInDirectory();

}

void MainWindow::updateTerminalAndOutput(){
    if(process->state() == QProcess::Running){
        process->setWorkingDirectory(QFileInfo(currentFile).absolutePath());
    }
    else if(process->state() == QProcess::Starting){
        process->start("cmd.exe");
    }
    else if(process->state() == QProcess::NotRunning){
        // if the process isnt running, initialize it
        initTerminalBox();
    }
}


void MainWindow::adjustSearchLineEditPosition()
{
    // searchAndReplaceParent->setGeometry()
    // searchAndReplaceParent->addWidget(searchLineEdit);
    // creates the search bar to search the text



    // searchLineEdit->hide(); commented out for testing for now
    const int margin = 10; // Margin from the top and right edges
    QPoint topRight = ui->plainTextEdit->rect().topRight();

    searchAndReplaceContainer->move(topRight.x() - searchAndReplaceContainer->width() - margin, topRight.y() + margin);
    // searchAndReplaceContainer->move(topRight.x() - searchAndReplaceContainer->width() - margin, topRight.y() + margin);
    searchAndReplaceParent->setSpacing(2);
    checkBoxesParent->setSpacing(2);

}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    adjustSearchLineEditPosition();
}


void MainWindow::searchForText(const QString &text){

    removeHighlights(); // removes any text that was previously highlighted
    foundTextMatchCount = 0;  // resets it
    foundOccurrences.clear(); // clears the vector storing all instances

    if (text.isEmpty()) {

        return; // returns if empty string
    }

    QTextDocument *document = ui->plainTextEdit->document();
    QTextCursor highlightCursor(document);
    QTextCursor cursor(document);

    cursor.beginEditBlock();

    QTextCharFormat plainFormat(highlightCursor.charFormat());
    QTextCharFormat colorFormat = plainFormat;
    colorFormat.setBackground(Qt::blue);

    bool caseSensitive = isCaseSensitive->isChecked();
    bool matchWhole = isMatchWholeWord->isChecked();
    // QTextDocument* doc = this->ui->plainTextEdit->document();
    QTextDocument::FindFlags flag;


    if (caseSensitive) { // or equals operator, if true it is added
        flag |= QTextDocument::FindCaseSensitively;
    }
    if (matchWhole) {
        flag |= QTextDocument::FindWholeWords;
    }

    while (!highlightCursor.isNull() && !highlightCursor.atEnd()){
        highlightCursor = document->find(text, highlightCursor, flag);

        if (!highlightCursor.isNull()){
            // highlightCursor.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);
            highlightCursor.mergeCharFormat(colorFormat);

            foundOccurrences.append(highlightCursor);
            foundTextMatchCount++;
        }
    }

    cursor.endEditBlock();
    statusBar()->showMessage(QString("Occurrences: %1").arg(foundTextMatchCount));
}

void MainWindow::removeHighlights(){
    QTextDocument *document = ui->plainTextEdit->document();
    QTextCursor cursor(document);
    cursor.beginEditBlock();

    QTextCharFormat plainFormat;
    plainFormat.setBackground(Qt::transparent); // Set background to transparent to remove highlight
    cursor.select(QTextCursor::Document);
    cursor.mergeCharFormat(plainFormat);

    cursor.endEditBlock();
    statusBar()->clearMessage();
}

void MainWindow::saveFile(){
    if (currentFile.isEmpty()) {

        on_actionSave_As_triggered();  // no file is open, instead calls save as
        return;
    }

    QFile file(currentFile);

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Warning", "Unable to save file: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    QString currentText = ui->plainTextEdit->toPlainText();
    out << currentText;  // overwrites file content with the updated text.

    file.close();

    fileContentAfterSave = currentText;
}

void MainWindow::replaceText(){
    QString replaceText = replaceLineEdit->text();
    if (replaceText.isEmpty() || foundOccurrences.isEmpty()) {
        return;  // If the replacement text is empty or no occurrences found, do nothing.
    }

    QTextCursor cursor = ui->plainTextEdit->textCursor();
    cursor.beginEditBlock();  // Start a single undo block for the whole replacement operation.

    for (QTextCursor &occurrence : foundOccurrences) { // loops through the vector storing all instances of the items to removed
        occurrence.beginEditBlock();
        occurrence.insertText(replaceText);
        occurrence.endEditBlock();
    }

    cursor.endEditBlock();  // End the undo block.

    statusBar()->showMessage(QString("Replaced %1 occurrences").arg(foundOccurrences.size()));
    foundOccurrences.clear();  // Clear occurrences after replacement
}

void MainWindow::createSearchAndReplaceWidgets(){
    // Structure
    // BLANK PARENT WIDGET
    //  - Horizontal Layout
    //      - Vertical Layout
    //          - Seacrch text line edit
    //          - Replace text line edit
    //          - Replace Text Button
    //      - Vertical Layout
    //          - Close menu button
    //          - Is casesensitive checkmark
    //          - Is match whole word checkmark
    searchAndReplaceContainer = new QWidget(ui->plainTextEdit);

    fullLayoutParent = new QHBoxLayout(searchAndReplaceContainer);
    fullLayoutParent->setAlignment(Qt::AlignHCenter);


    searchAndReplaceParent = new QVBoxLayout();
    checkBoxesParent = new QVBoxLayout();
    checkBoxesParent->setAlignment(Qt::AlignTop | Qt::AlignHCenter);


    searchLineEdit = new QLineEdit();
    replaceLineEdit = new QLineEdit();
    replaceTextButton = new QPushButton("Replace Text");
    connect(replaceTextButton, &QPushButton::clicked, this, &MainWindow::replaceText);

    isCaseSensitive = new QCheckBox();
    isCaseSensitive->setToolTip("Case Sensitive");
    connect(isCaseSensitive, &QCheckBox::clicked, this, [this](){
        searchForText(searchLineEdit->text());
    });

    isMatchWholeWord = new QCheckBox();
    isMatchWholeWord->setToolTip("Match Whole Word");
    connect(isMatchWholeWord, &QCheckBox::clicked, this, [this](){
        searchForText(searchLineEdit->text());
    });

    qApp->setStyleSheet(" QCheckBox:hover{background-color: light-grey;}");

    QIcon closeIcon = qApp->style()->standardIcon(QStyle::SP_TitleBarCloseButton);
    hidePopUpButton = new QPushButton();
    hidePopUpButton->setIcon(closeIcon);

    // hidePopUpButton.hove
    hidePopUpButton->setStyleSheet(" QPushButton:hover:!pressed{background-color: red;}");
    connect(hidePopUpButton, &QPushButton::pressed, this, [this](){
        searchAndReplaceContainer->hide();   // lambda to hide the search and replace item
        this->ui->plainTextEdit->setFocus(); // changes the focused widget to the text box
    });


    searchAndReplaceParent->addWidget(searchLineEdit);
    searchAndReplaceParent->addWidget(replaceLineEdit);
    searchAndReplaceParent->addWidget(replaceTextButton);

    checkBoxesParent->addWidget(hidePopUpButton);
    checkBoxesParent->addWidget(isCaseSensitive);
    checkBoxesParent->addWidget(isMatchWholeWord);


    fullLayoutParent->addLayout(searchAndReplaceParent);
    fullLayoutParent->addLayout(checkBoxesParent);

    searchAndReplaceContainer->setLayout(fullLayoutParent);


    searchLineEdit->setPlaceholderText("Search Text");
    searchLineEdit->setFixedSize(120, 30);
    replaceLineEdit->setPlaceholderText("Replace With");
    replaceLineEdit->setFixedSize(120, 30);
    replaceTextButton->setFixedSize(90, 30);


}


void MainWindow::on_actionFind_Replace_triggered()
{
    if(this->ui->plainTextEdit->hasFocus()){
        searchAndReplaceContainer->show();
        adjustSearchLineEditPosition(); // places it at correct location
        searchLineEdit->setFocus(); // makes the line edit for the text to search be the focus
    }

}

