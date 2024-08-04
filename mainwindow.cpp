#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , process (new QProcess(this))
    , fileModel(new QFileSystemModel(this))
{
    ui->setupUi(this);

    this->ui->actionSave->setEnabled(false);
    this->setCentralWidget(ui->stackedWidget);
    this->ui->stackedWidget->setCurrentWidget(this->ui->page);

    createSearchAndReplaceWidgets();

    // QScrollBar* plainTextScrollBar =
    // QScrollBar* lineNumberScrollBar =

    connect(ui->lineNumPlainTextEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::synchronizeScrollbars);
    connect(ui->plainTextEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::synchronizeScrollbars);
    // connects the scroll bars of the text box the user types in with the line number text


    setUIChanges();


    connect(searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::searchForText);


    // ignoreTextChanged = false; //flag to stop the text edited within, to activate the signal and get stuck in a recursive loop

    connect(this->ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::updateWindowTitle);

    ui->plainTextEdit->installEventFilter(this);
    startTerminalCommand = getShellCommand(); // differentiates the terminial start based on the operating system


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
    // asks to save if they have any changes on current file they are working on before opening dialog
    if(!ui->plainTextEdit->toPlainText().isEmpty() && !textIsSameAfterSave && !currentFile.isEmpty()){
        QMessageBox::StandardButton saveFileQuestion = QMessageBox::question(this, "Save Changes?", "Would you like To Save Changes Before Opening a New Folder?"
                                                                             , QMessageBox::Save | QMessageBox::Discard, QMessageBox::Save);

        if(saveFileQuestion == QMessageBox::Save){
            saveFile();
        }
    }

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
    // setWindowTitle(fileName);

    QTextStream out(&file);
    QString currentText = ui->plainTextEdit->toPlainText();
    out << currentText;  // sets the text content of that file equal to the current text
    file.close();

    fileContentAfterSave = currentText;  // updates saved content tracker.

    updateWindowTitle();
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

    process->start(startTerminalCommand);

    if (!process->waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start the command process.");
        return;
    }
    process->setWorkingDirectory(QFileInfo(currentFile).absolutePath());
    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::on_StdoutAvailable);
    connect(process, &QProcess::readyReadStandardError, this, &MainWindow::on_StderrAvailable); // Connect the error output signal

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

void MainWindow::on_StderrAvailable(){

    if(!process->isOpen()){
        qDebug() << "not open";
        return;
    }
    QByteArray terminalOutput = process->readAllStandardError();
    // outputs the error to the terminal in red
    ui->terminalBox->appendHtml("<span style = 'color: red;'>" + terminalOutput + "</span>");
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

    QFont font = ui->plainTextEdit->font();

    QFontMetrics metrics(font);
    int spaceWidth = metrics.horizontalAdvance(' ');

    // Set the tab stop distance to 4 spaces
    ui->plainTextEdit->setTabStopDistance(4 * spaceWidth);
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

    QStringList filePaths(Qt::CaseInsensitive);
    filePaths << "*.py" << "*.txt" << "*.md" << "*.csv";

    fileModel->setNameFilters(filePaths);
    fileModel->setNameFilterDisables(false); // makes the files that arent within filter hidden instead of shown as disabled

    this->ui->fileListTree->setModel(fileModel);
    fileModel->setRootPath(directory.path());
    ui->fileListTree->setRootIndex(fileModel->index(directory.path()));
}

void MainWindow::getAllFilesInDirectory(QString directory){

    QStringList filePaths(Qt::CaseInsensitive);
    filePaths << "*.py" << "*.txt" << "*.md" << "*.csv";

    fileModel->setNameFilters(filePaths);
    fileModel->setNameFilterDisables(false); // makes the files that arent within filter hidden instead of shown as disabled

    this->ui->fileListTree->setModel(fileModel);
    fileModel->setRootPath(directory);
    ui->fileListTree->setRootIndex(fileModel->index(directory));
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
            // saving file crashes for some reason unless it also opens it again afterwards
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

    QTextStream in(&file);

    QString text = in.readAll();
    ui->plainTextEdit->setPlainText(text);

    fileContentAfterSave = text;

    previousNumberOfLines = this->ui->plainTextEdit->blockCount();
    // the number of lines for the line counter, also stores the variable to see if the change was line added or removed
    createLineNumbersOnFileOpen(previousNumberOfLines);
    file.close();

    updateTerminalAndOutput();
    updateWindowTitle();
}

void MainWindow::updateTerminalAndOutput(){
    if(process->state() == QProcess::Running){
        process->setWorkingDirectory(QFileInfo(currentFile).absolutePath());
    }
    else if(process->state() == QProcess::Starting){
        qDebug() << "st starting";
    }
    else if(process->state() == QProcess::NotRunning){
        // if the process isnt running, initialize it
        initTerminalBox();
    }
}


void MainWindow::adjustSearchLineEditPosition()
{
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
    adjustSearchLineEditPosition();
    QMainWindow::resizeEvent(event);   
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

     // or equals operator, if true it is added
    if (caseSensitive) flag |= QTextDocument::FindCaseSensitively;

    if (matchWhole) flag |= QTextDocument::FindWholeWords;

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
    if(currentFile.isEmpty()) {
        qDebug() << "Current file is empty. Triggering Save As dialog.";
        on_actionSave_As_triggered();  // on cases where option is available with no file, calls to save as
        return;
    }

    try {
        QFile file(currentFile);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Warning", "Unable to Save File " + file.errorString());
            throw std::runtime_error("Cannot open file for writing: " + file.errorString().toStdString());
            return;
        }
        QString text = this->ui->plainTextEdit->toPlainText();
        QTextStream out(&file);
        out << text;

        file.close();
        fileContentAfterSave = text;  // Update the saved content tracker.

        updateWindowTitle();
        // getAllFilesInDirectory();

    } catch (const std::exception &e) {
        QMessageBox::warning(this, "Warning", e.what());
    }
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

    statusBar()->showMessage(QString("Replaced %1 occurrences").arg(foundOccurrences.size()), 5); // clears message on statusbar after 5 seconds
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
    QIcon caseSensitveIcon = QIcon(":/imgs/gui-case-sensitive.svg");
    isCaseSensitive->setIcon(caseSensitveIcon);


    isMatchWholeWord = new QCheckBox();
    isMatchWholeWord->setToolTip("Match Whole Word");
    QIcon matchWholeIcon = QIcon(":/imgs/whole-word.svg");
    isMatchWholeWord->setIcon(matchWholeIcon);


    connect(isMatchWholeWord, &QCheckBox::clicked, this, [this](){
        searchForText(searchLineEdit->text());
        qDebug() << "called on match";
    });
    connect(isCaseSensitive, &QCheckBox::clicked, this, [this](){
        qDebug() << "called on case ";
        searchForText(searchLineEdit->text());
    }); // re does the search if either button is clicked


    qApp->setStyleSheet(" QCheckBox:hover{background-color: light-grey;}");

    QIcon closeIcon = qApp->style()->standardIcon(QStyle::SP_TitleBarCloseButton);
    hidePopUpButton = new QPushButton();
    hidePopUpButton->setIcon(closeIcon);
    hidePopUpButton->setStyleSheet(" QPushButton:hover:!pressed{background-color: red;}");
    connect(hidePopUpButton, &QPushButton::pressed, this, [this](){
        removeHighlights(); // if any text was selected before hand, the highlight gets removed
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

void MainWindow::on_actionOpen_Folder_triggered()
{
    // if they try to open folder while working on something that is not saved, it asks to save beforehand
    if(!ui->plainTextEdit->toPlainText().isEmpty() && !textIsSameAfterSave){
        QMessageBox::StandardButton saveFileQuestion = QMessageBox::question(this, "Save Changes?", "Would you like To Save Changes Before Opening a New Folder?"
                                 , QMessageBox::Save | QMessageBox::Discard, QMessageBox::Save);

        if(saveFileQuestion == QMessageBox::Save){
            saveFile();
        }
    }

    QString dir = QFileDialog::getExistingDirectory(this, "Open Directory",
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        QMessageBox::warning(this, "Warning", tr("Unable To Open Folder"));
        return;
    }

    this->ui->stackedWidget->setCurrentWidget(this->ui->page_2); // sets the page to the text editor page
    this->ui->dockWidget_4->showNormal();
    updateTerminalAndOutput();

    getAllFilesInDirectory(dir);
    setWindowTitle(dir);
}


void MainWindow::on_actionUndo_triggered()
{
    // undo
}

void MainWindow::updateWindowTitle(){
    // qDebug() << "called on update title";
    textIsSameAfterSave = fileContentAfterSave == this->ui->plainTextEdit->toPlainText();
    if(!textIsSameAfterSave){
        setWindowTitle(currentFile + " (Changes Not Saved)");
    }
    else if(textIsSameAfterSave && windowTitle() != currentFile){
        setWindowTitle(currentFile);
    }
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->plainTextEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Tab) {
                qDebug()<<"tab";
                QTextCursor cursor = ui->plainTextEdit->textCursor();
                cursor.insertText("   "); // Insert four spaces
                return true; // Event handled, don't pass it to the base class
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


QString MainWindow::getShellCommand() {
#ifdef _WIN32
    return "cmd.exe";
#else
    return "/bin/sh";
#endif
}
