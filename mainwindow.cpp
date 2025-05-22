#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "util.h"
#include "editor.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    process (new QProcess(this)),
    fileModel(new QFileSystemModel(this))
{
    ui->setupUi(this);

    this->ui->actionSave->setEnabled(false);
    this->setCentralWidget(ui->stackedWidget);
    this->ui->stackedWidget->setCurrentWidget(this->ui->page);
    QDir::setCurrent(QDir::homePath());


    setUIChanges();
    createSearchAndReplaceWidgets();

    // TODO: find alternative
    // ui->plainTextEdit->installEventFilter(this);


    // searchReplaceWidget = new SearchAndReplace(openEditor->getPte());
    connectSignals();
}

MainWindow::~MainWindow()
{
    if (process->isOpen()) {
        process->write("exit\n");
        process->waitForFinished();
    }

    delete searchReplaceWidget;
    delete process;
    delete ui;
}


void MainWindow::openFileAction()
{
    // asks to save if they have any changes on current file they are working on before opening dialog
    if(openEditor != nullptr && openEditor->unsavedChanges()){
        QMessageBox::StandardButton saveFileQuestion = QMessageBox::question(this, tr("Save Changes?"),
                                                                             tr("Would you like To Save Changes Before Opening a New Folder?"),
                                                                             QMessageBox::Save | QMessageBox::Discard, QMessageBox::Save);

        if(saveFileQuestion == QMessageBox::Save){
            openEditor->saveFile();
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, ("Choose File To Open"));

    // setOption(QFileDialog.ReadOnly, true);
    openFile(fileName);
    getAllFilesInDirectory();
    adjustSearchLineEditPosition();
}



void MainWindow::updateStatusBarCursorPosition()
{

    qDebug()<< "Not Implemented";
    // auto cursor = this->ui->plainTextEdit->textCursor();

    // int col = cursor.columnNumber() + 1;
    // int line = cursor.blockNumber() + 1;
    // QString text = "LN: " + QString::number(line) + ", COL: " + QString::number(col);
    // statusBar()->showMessage(text);
    // lineAndColStatusLabel->setText(text);
}



void MainWindow::initTerminalBox()
{

    this->ui->terminalBox->clear(); // clears the text in case they are switching files
    // maybe remove, or leave to a setting if they want to

    process->start(util::getShellCommand());

    if (!process->waitForStarted()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to start the command process"));
        return;
    }
    process->setWorkingDirectory(QFileInfo(openEditor->fileName()).absolutePath());
    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::on_StdoutAvailable);
    connect(process, &QProcess::readyReadStandardError, this, &MainWindow::on_StderrAvailable); // Connect the error output signal

}


void MainWindow::on_StdoutAvailable()
{ // when it is ready to be read, it reads the output and prints it to the terminal box
    if(!process->isOpen()){
        return;
    }
    QByteArray terminalOutput = process->readAllStandardOutput();
    ui->terminalBox->appendPlainText(terminalOutput);
}

void MainWindow::on_StderrAvailable(){

    if(!process->isOpen()){
        return;
    }
    QByteArray terminalOutput = process->readAllStandardError();
    // outputs the error to the terminal in red
    ui->terminalBox->appendHtml("<span style = 'color: red;'>" + terminalOutput + "</span>");
}


void MainWindow::writeToTerminal()
{
    if (process->isOpen()){
        QByteArray inputByteArray(ui->inputTerminalCommand->text().toUtf8() + "\n") ;
        char *userText = inputByteArray.data();

        process->write(userText); // inputs the user command into the terminal
    }
    ui->inputTerminalCommand->clear(); // clears the input field for the user
}

void MainWindow::setUIChanges()
{
    this->ui->terminalDockWidget->hide();
    this->ui->fileTreeDockWidget->hide();

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea); // makes the file explorer, whether right or left fill the space instead of the terminal

    this->ui->openEditorsTabWidget->removeTab(0);
    this->ui->openEditorsTabWidget->removeTab(1); // for some reason cant remove them in the ui design, has to be in code

    // searchAndReplaceContainer->hide();

}

void MainWindow::runButton()
{
    if(process->isOpen()){
        showTerminal();
        QString runPythonCommand = QString("%1 -u \"%2\"").arg(util::getPythonRunCommand(), openEditor->fileName());
        QByteArray runFileCommand(runPythonCommand.toUtf8() + "\n") ;

        process->write(runFileCommand.data()); // inputs the user command into the terminal
        process->waitForBytesWritten();
    }
}

void MainWindow::getAllFilesInDirectory()
{
    QDir directory = QFileInfo(openEditor->fileName()).dir();

    QStringList filePaths(Qt::CaseInsensitive);
    filePaths << "*.py" << "*.txt" << "*.md" << "*.csv";

    fileModel->setNameFilters(filePaths);
    fileModel->setNameFilterDisables(false); // makes the files that arent within filter hidden instead of shown as disabled

    this->ui->fileListTree->setModel(fileModel);
    this->ui->fileListTree->setContextMenuPolicy(Qt::CustomContextMenu); // allows the right click to show custom menu


    fileModel->setRootPath(directory.path());
    ui->fileListTree->setRootIndex(fileModel->index(directory.path()));
}

void MainWindow::getAllFilesInDirectory(QString &directory)
{

    QStringList filePaths(Qt::CaseInsensitive);
    filePaths << "*.py" << "*.txt" << "*.md" << "*.csv";

    fileModel->setNameFilters(filePaths);
    fileModel->setNameFilterDisables(false); // makes the files that arent within filter hidden instead of shown as disabled

    this->ui->fileListTree->setModel(fileModel);
    fileModel->setRootPath(directory);
    ui->fileListTree->setRootIndex(fileModel->index(directory));
}



void MainWindow::openFile(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)){
        QString errorMessage{"Can Not Open File " + file.errorString()};
        QMessageBox::warning(this, tr("Warning"), tr(errorMessage.toStdString().c_str()));
        return;
    }

    if(ui->stackedWidget->currentIndex() == 0){
        this->ui->stackedWidget->setCurrentIndex(1); // sets the page to the text editor page
    }

    this->ui->actionSave->setEnabled(true); // if they successfully opened a file, the save button can be used for it

    editor* nextPage = new editor(this->ui->openEditorsTabWidget);
    openEditor = nextPage;

    int newTab = this->ui->openEditorsTabWidget->addTab(nextPage, file.fileName());
    this->ui->tabWidget->setCurrentIndex(newTab);

    nextPage->openFile(file);

    file.close();

    this->ui->fileTreeDockWidget->showNormal();
    this->ui->terminalDockWidget->showNormal(); // shows both docks, file explorer, and output


    updateTerminalAndOutput();
    // updateWindowTitle();

}

void MainWindow::updateTerminalAndOutput()
{
    if(process->state() == QProcess::Running){
        process->setWorkingDirectory(QFileInfo(openEditor->fileName()).absolutePath());
    }
    // else if(process->state() == QProcess::Starting){
    // }
    else if(process->state() == QProcess::NotRunning){
        // if the process isnt running, initialize it
        initTerminalBox();
    }
}


void MainWindow::adjustSearchLineEditPosition()
{
    qDebug()<< "Not Implemented";
    const int margin = 10; // Margin from the top and right edges
    // QPoint topRight = openEditor->getPte()->rect().topRight();
    // searchReplaceWidget->move(topRight.x()- searchReplaceWidget->width() - margin, topRight.y() + margin);
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    adjustSearchLineEditPosition();
    QMainWindow::resizeEvent(event);
}


void MainWindow::createSearchAndReplaceWidgets()
{
    // THE BOTTOM STATUS BAR PART
    // QWidget* statusBarWidget = new QWidget;
    // QHBoxLayout* statusBarLayout = new QHBoxLayout;
    // statusBarWidget->setLayout(statusBarLayout);
    // lineAndColStatusLabel = new QLabel(this);
    // searchAndReplaceStatusLabel = new QLabel(this);

    // statusBarLayout->addWidget(lineAndColStatusLabel);
    // statusBarLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    // statusBarLayout->addWidget(searchAndReplaceStatusLabel);
    // ui->statusbar->addWidget(statusBarWidget, 1); // adds the widget and makes it stretch to fill entire status bar
}


void MainWindow::openFolderDialog()
{
    // if they try to open folder while working on something that is not saved, it asks to save beforehand
    /*if(!ui->editorWidget->getText().isEmpty() && this->ui->editorWidget->getPte()->document()->isModified()){
        QMessageBox::StandardButton saveFileQuestion = QMessageBox::question(this, tr("Save Changes?"), tr("Would you like To Save Changes Before Opening a New Folder?")
                                                                             , QMessageBox::Save | QMessageBox::Discard, QMessageBox::Save);

        // TODO: add save options, if either never save or always open file in same tab call save instead
        if(saveFileQuestion == QMessageBox::Save) saveFile();
    }
    */

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    tr("/home"),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        QMessageBox::warning(this, tr("Warning"), tr("Unable To Open Folder"));
        return;
    }

    this->ui->stackedWidget->setCurrentWidget(this->ui->page_2); // sets the page to the text editor page
    this->ui->fileTreeDockWidget->showNormal();
    updateTerminalAndOutput();

    getAllFilesInDirectory(dir);
    setWindowTitle(dir);
}

void MainWindow::openFileWhileEditing(const QString& path){
    // checks to see if there are changes with the file before opening
    /*
    if(this->ui->editorWidget->getPte()->document()->isModified()){
        QMessageBox::StandardButton saveQuestion = QMessageBox::question(this, tr("Save?"),
                                                                         tr("Would you like to save changes before switching"),
                                                                         QMessageBox::Save | QMessageBox::Discard, QMessageBox::Save);
        // TODO: See open folder dialog todo
        if(saveQuestion == QMessageBox::Save) saveFile();
    }
    */
    qDebug()<< "Not Implemented";
    openFile(path);
}



bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    qDebug()<< "Not Implemented";
    // if (obj == openEditor->getPte()) {
    //     if (event->type() == QEvent::KeyPress) {
    //         QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    //         if((keyEvent->modifiers() & Qt::ControlModifier) && (keyEvent->key() == Qt::Key_Slash)) {

    //             openEditor->commentLines();
    //             return true;
    //         }
    //     }
    // }
    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::showCustomContextMenu(const QPoint &pos){
    QModelIndex index = this->ui->fileListTree->indexAt(pos);
    QString fileToOpenPath = fileModel->filePath(index);

    QMenu contextMenu(tr("Context menu"), this);


    QAction newPythonAction(tr("New Python File"), this);
    connect(&newPythonAction, &QAction::triggered, this,[this, pos]{
        createPythonFile(pos);
    });
    contextMenu.addAction(&newPythonAction);

    QAction newTextFileAction(tr("New Text File"), this);
    connect(&newTextFileAction, &QAction::triggered, this,[this, pos]{
        createTextFile(pos);
    });
    contextMenu.addAction(&newTextFileAction);



    QAction deleteAction("Delete", this);
    connect(&deleteAction, &QAction::triggered, this, [this, &fileToOpenPath]() {
        qDebug()<< "Not Implemented";
        // if(fileToOpenPath == currentFile){
        //     QMessageBox::warning(this, tr("Error"), tr("Can not delete file that is currently open"));
        //     return;
        // }
        // QFile::remove(fileToOpenPath);
    });

    QAction openAction("Open", this);
    connect(&openAction, &QAction::triggered, this, [this, &fileToOpenPath]() { openFileWhileEditing(fileToOpenPath); });

    if (index.isValid()){ // if they clicked on a part that has a file
        // actions that need a file to be clicked on
        contextMenu.addAction(&deleteAction);

        contextMenu.addAction(&openAction);
    }


    contextMenu.exec(this->ui->fileListTree->viewport()->mapToGlobal(pos));

}


void MainWindow::createPythonFile(const QPoint &clickPoint) {

    QPoint globalPos = this->ui->fileListTree->viewport()->mapToGlobal(clickPoint);

    // finds the click point to place it at the same position of the click, while only being parented to the main window
    QPoint mainWindowPos = this->mapFromGlobal(globalPos);

    QString fileDefaultText = "file.py";

    QLineEdit* fileNameLine = new QLineEdit(this);
    fileNameLine->move(mainWindowPos);
    fileNameLine->setText(fileDefaultText);
    fileNameLine->setFixedWidth(150);
    fileNameLine->show();
    fileNameLine->setFocus();

    fileNameLine->setSelection(0, fileDefaultText.length()-3 ); // removes the .py suffix in the default selection so user can auto change file name


    // connects return pressed signal to create the file
    connect(fileNameLine, &QLineEdit::returnPressed, this, [this, fileNameLine]() {
        QString fileName = fileNameLine->text();
        if (!fileName.endsWith(".py")) {
            fileName += ".py";
        }

        QDir filePath = QFileInfo(openEditor->fileName()).dir();
        QFile file(filePath.absolutePath() + "/" + fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QString error{QString( "Unable to open file ") + file.errorString()};
            QMessageBox::warning(this, tr("Error"), error.toStdString().c_str());
            // delete lineedit and return if error opening file
            delete fileNameLine;
            return;
        }
        file.close();
        fileNameLine->deleteLater();
    });

    // deletes line edit if user clicks out of it, or if it loses focus in any way
    connect(fileNameLine, &QLineEdit::editingFinished, this, [fileNameLine]() {
        fileNameLine->deleteLater();
    });
}

void MainWindow::createTextFile(const QPoint &clickPoint) {
    QPoint globalPos = this->ui->fileListTree->viewport()->mapToGlobal(clickPoint);

    // finds the click point to place it at the same position of the click, while only being parented to the main window
    QPoint mainWindowPos = this->mapFromGlobal(globalPos);
    QString fileDefaultText = "file.txt";

    QLineEdit* fileNameLine = new QLineEdit(this);
    fileNameLine->move(mainWindowPos);
    fileNameLine->setText(fileDefaultText);
    fileNameLine->setFixedWidth(150);
    fileNameLine->show();
    fileNameLine->setFocus();

    fileNameLine->setSelection(0, fileDefaultText.length()-4 ); // removes the .txt suffix in the default selection so user can auto change file name


    // connects return pressed signal to create the file
    connect(fileNameLine, &QLineEdit::returnPressed, this, [this, fileNameLine]() {
        QString fileName = fileNameLine->text();
        if (!fileName.endsWith(".txt")) {
            fileName += ".txt";
        }

        QDir filePath = QFileInfo(openEditor->fileName()).dir();
        QFile file(filePath.absolutePath() + "/" + fileName);
        if (!file.open(QIODevice::WriteOnly)) {

            QString error{QString("Unable to open file ") + file.errorString()};
            QMessageBox::warning(this, tr("Error"), error.toStdString().c_str());
            // delete lineedit and return if error opening file
            delete fileNameLine;
            return;
        }
        file.close();
        fileNameLine->deleteLater();
    });

    // deletes line edit if user clicks out of it, or if it loses focus in any way
    connect(fileNameLine, &QLineEdit::editingFinished, this, [fileNameLine]() {
        fileNameLine->deleteLater();
    });
}


void MainWindow::newPythonFile()
{
    // QString fileName = QFileDialog::
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Create New Python File"),
                                                    QDir::homePath(),  // Default directory
                                                    tr("Python Files (*.py);;All Files (*)"));

    // do nothing if they cancel, maybe show message warning later on
    if (fileName.isEmpty()) {
        return;
    }

    // makes sure file has python extention
    if (!fileName.endsWith(".py")) {
        fileName += ".py";
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QString error{"Failed to create the file: " + file.errorString()};
        QMessageBox::warning(this, tr("Error"), tr(error.toStdString().c_str()));
        return;
    }

    file.close();

    openFile(fileName);
    getAllFilesInDirectory();
    adjustSearchLineEditPosition();
}

void MainWindow::connectSignals(){ // relying on the connection of slots that the qt generated on_foo_bar as clangd would say

    // TODO: possibly disconnet and reconnect if they open new file with (new editor object?)
    // connect(openEditor->getPte(), &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBarCursorPosition);


    connect(this->ui->fileListTree, &QWidget::customContextMenuRequested, this, &MainWindow::showCustomContextMenu);
    connect(this->ui->fileListTree, &QTreeView::doubleClicked, this, [this](QModelIndex index){ // pass the same argument as the doubleClick slot has
        QString fileToOpenPath = fileModel->filePath(index);
        openFileWhileEditing(fileToOpenPath);
    });


    // MENU ACTION BAR BUTTONS
    connect(this->ui->actionShow_Terminal, &QAction::triggered, this, &MainWindow::showTerminal);
    connect(this->ui->actionNew_Text_File, &QAction::triggered, this, &MainWindow::newTextFile);
    connect(this->ui->actionNew, &QAction::triggered, this, &MainWindow::newPythonFile);

    connect(this->ui->actionOpen_File, &QAction::triggered, this, &MainWindow::openFileAction);
    connect(this->ui->actionOpen_Folder, &QAction::triggered, this, &MainWindow::openFolderDialog);

    connect(this->ui->actionSelect_All, &QAction::triggered, this, []{
        // While the check of current file might be useful like in undo, AFAIK, no harm in allowing this (in any case where text may exist out of file)
        // because of this doesnt need "this" in the capture
        QWidget* focusedWidget = QApplication::focusWidget();
        if (auto plainTextEdit = qobject_cast<QPlainTextEdit*>(focusedWidget)) {
            plainTextEdit->selectAll();
        }
        else if (auto lineEdit = qobject_cast<QLineEdit*>(focusedWidget)) {
            lineEdit->selectAll();
        } // tries to cast the focused widget into one of these, if possible calls on the built in select all function

    });
    connect(this->ui->actionUndo, &QAction::triggered, this, [this]{
        throw std::runtime_error("Not Implemented");
        // if(currentFile.isEmpty()) return;
        // QWidget* focusedWidget = QApplication::focusWidget();
        // if (auto plainTextEdit = qobject_cast<QPlainTextEdit*>(focusedWidget)) {
        //     plainTextEdit->undo();
        // }
        // else if (auto lineEdit = qobject_cast<QLineEdit*>(focusedWidget)) {
        //     lineEdit->undo();
        // } // tries to cast the focused widget into one of these, if possible calls on the built in undo function
    });
    connect(this->ui->actionRedo, &QAction::triggered, this, [this]{
        throw std::runtime_error("Not Implemented");
        // if(currentFile.isEmpty()) return;
        // QWidget* focusedWidget = QApplication::focusWidget();

        // if (auto plainTextEdit = qobject_cast<QPlainTextEdit*>(focusedWidget)) {
        //     plainTextEdit->redo();
        // }
        // else if (auto lineEdit = qobject_cast<QLineEdit*>(focusedWidget)) {
        //     lineEdit->redo();

        // } //  just like undo
        // //tries to cast the focused widget into one of these, if possible calls on the built in redo function
    });


    connect(this->ui->actionSave, &QAction::triggered, this, [this]{
        this->openEditor->saveFile();
    });


    connect(this->ui->actionHide_Terminal, &QAction::triggered, this, [this]{
        this->ui->terminalDockWidget->hide();
    });
    connect(this->ui->actionClear_Terminal, &QAction::triggered, this, [this]{
        this->ui->terminalBox->clear();
    });

    connect(this->ui->actionShow_File_Tree, &QAction::triggered, this, [this]{
        this->ui->fileTreeDockWidget->showNormal();
    });
    connect(this->ui->actionFind_Replace, &QAction::triggered, this, [this]{
        qDebug()<< "Not Implemented";
        // if(currentFile.isEmpty()) return;
        // searchReplaceWidget->showWidget();
        // searchReplaceWidget.showWidget();

        // adjustSearchLineEditPosition(); // so it is in the correct position based on the window size
    });

    // END OF MENU BAR ACTIONS


    connect(this->ui->runFileButton, &QPushButton::pressed, this, &MainWindow::runButton);
    connect(this->ui->inputTerminalCommand, &QLineEdit::returnPressed, this, &MainWindow::writeToTerminal);

    connect(this->ui->openEditorsTabWidget, &QTabWidget::tabBarClicked, this, [this]{
        openEditor = qobject_cast<editor*>(ui->openEditorsTabWidget->currentWidget());
    });
}


void MainWindow::newTextFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Create New Text File File"),
                                                    QDir::homePath(),  // Default directory
                                                    tr("Text Files (*.txt);;All Files (*)"));

    // do nothing if they cancel, maybe show message warning later on
    if (fileName.isEmpty()) {
        return;
    }

    // makes sure file has txt extention
    if (!fileName.endsWith(".txt")) {
        fileName += ".txt";
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {

        QString error{"Failed to create the file: "+ file.errorString()};
        QMessageBox::warning(this, tr("Error"), tr(error.toStdString().c_str()));
        return;
    }

    file.close();

    openFile(fileName);
    getAllFilesInDirectory();
    adjustSearchLineEditPosition();
}



void MainWindow::showTerminal(){
    this->ui->terminalDockWidget->showNormal(); // if they press new terminal, it shows the widget
}
