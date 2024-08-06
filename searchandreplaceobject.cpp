#include "searchandreplaceobject.h"

searchAndReplaceObject::searchAndReplaceObject(QPlainTextEdit* editor, QWidget* parent)
    : QDockWidget(parent)
    , editor(editor)
{
    setupUI(); // makes the ui items and signal connections in constructor
    connectSignalsAndSlots();
    this->hide();

}

void searchAndReplaceObject::setupUI(){
    // FULL NEW STRUCTURE
//  Blank EMPTY widget -- search and replace container
    // QVbox  -- full layout parent
    //     QHbox	-- line edits and iterate layout
    //         QVBOX -- search and replace parent
    //             Search -- searchtextlineedit
    //             Replace -- replacetextlineedit

    //         Empty Widget -- iterator blank widget
    //             QVBOX --iterate words layout
    //                 Iterator label box
    //                 QHbox -- prevandnextbuttonslayout
    //                      Prev button
    //                      Next button

    //     Empty Widget -- bottomempty widget
    //         QHbox -- bottom layout
    //             Replace Button
    //             Another Qhbox aligned to right --checkboxesparent
    //                 Match whole word button
    //                 Case sensitive button


// Ceates the ui objects in the structure above
    QFrame* searchAndReplaceContainer = new QFrame(this);
    searchAndReplaceContainer->setFrameShape(QFrame::Box);
    searchAndReplaceContainer->setAutoFillBackground(true);
    // searchAndReplaceContainer.bac
    this->setWidget(searchAndReplaceContainer);


    QVBoxLayout* fullLayoutParent = new QVBoxLayout(searchAndReplaceContainer);
    // fullLayoutParent->setAlignment(Qt::AlignHCenter);

    QHBoxLayout* lineEditsAndIterateLayout = new QHBoxLayout;
    QVBoxLayout* searchAndReplaceParent = new QVBoxLayout;

    searchTextLineEdit = new QLineEdit;
    searchAndReplaceParent->addWidget(searchTextLineEdit);
    replaceTextLineEdit = new QLineEdit;
    searchAndReplaceParent->addWidget(replaceTextLineEdit);


    QFrame* iteratorBlankWidget = new QFrame;
    iteratorBlankWidget->setFrameShape(QFrame::Box);

    QVBoxLayout* iterateWordsLayout = new QVBoxLayout(iteratorBlankWidget);

    occurenceIteratorLabel = new QLabel("0 / 0");
    occurenceIteratorLabel->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    occurenceIteratorLabel->setStyleSheet("QLabel{font-size: 20px;}");
    iterateWordsLayout->addWidget(occurenceIteratorLabel);


    QHBoxLayout* prevAndNextButtonsLayout = new QHBoxLayout;
    prevMatchButton = new QPushButton;
    connect(prevMatchButton, &QPushButton::clicked, this, &searchAndReplaceObject::goToPreviousSelection);
    nextMatchButton = new QPushButton;
    connect(nextMatchButton, &QPushButton::clicked, this, &searchAndReplaceObject::goToNextSelection);


    nextMatchButton->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowForward));
    // nextMatchButton.image
    prevMatchButton->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowBack)); // means left unless the layout of language is right to left
    // nextMatchButton->setStyleSheet("QPushButton:hover{ background-color: lightgray;}");
    nextMatchButton->setStyleSheet(""); // This resets to default

    prevMatchButton->setStyleSheet("QPushButton:hover{ background-color: lightgray;}");
    prevAndNextButtonsLayout->addWidget(prevMatchButton);
    prevAndNextButtonsLayout->addWidget(nextMatchButton);


    QFrame* bottomEmptyWidget = new QFrame;
    bottomEmptyWidget->setAutoFillBackground(true);
    bottomEmptyWidget->setFrameShape(QFrame::Box);
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomEmptyWidget);

    replaceTextButton = new QPushButton("Replace All");


    QHBoxLayout* checkBoxesParent = new QHBoxLayout;

    isCaseSensitive = new QCheckBox;
    checkBoxesParent->addWidget(isCaseSensitive);


    // checkBoxesParent->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    isMatchWholeWord = new QCheckBox;
    checkBoxesParent->addWidget(isMatchWholeWord);

    iterateWordsLayout->addLayout(prevAndNextButtonsLayout);

    bottomLayout->addWidget(replaceTextButton);
    bottomLayout->addLayout(checkBoxesParent);


    lineEditsAndIterateLayout->addLayout(searchAndReplaceParent);
    lineEditsAndIterateLayout->addWidget(iteratorBlankWidget);

    fullLayoutParent->addLayout(lineEditsAndIterateLayout);
    fullLayoutParent->addWidget(bottomEmptyWidget);


    isCaseSensitive->setToolTip("Case Sensitive");
    QIcon caseSensitveIcon = QIcon(":/imgs/gui-case-sensitive.svg");
    isCaseSensitive->setIcon(caseSensitveIcon);


    isMatchWholeWord->setToolTip("Match Whole Word");
    QIcon matchWholeIcon = QIcon(":/imgs/whole-word.svg");
    isMatchWholeWord->setIcon(matchWholeIcon);

    qApp->setStyleSheet(" QCheckBox:hover{background-color: light-gray;}");

    searchTextLineEdit->setPlaceholderText("Search Text");
    searchTextLineEdit->setFixedSize(160, 20);
    replaceTextLineEdit->setPlaceholderText("Replace With");
    replaceTextLineEdit->setFixedSize(160, 20);
    replaceTextButton->setFixedSize(120, 20);


    searchAndReplaceParent->setSpacing(2);
    checkBoxesParent->setSpacing(2);

}

void searchAndReplaceObject::connectSignalsAndSlots() {

    connect(isMatchWholeWord, &QCheckBox::clicked, this, [this](){
        searchForText(searchTextLineEdit->text());
    });

    connect(isCaseSensitive, &QCheckBox::clicked, this, [this](){
        searchForText(searchTextLineEdit->text());
    }); // re does the search if either button is clicked

    connect(replaceTextButton, &QPushButton::clicked, this, &searchAndReplaceObject::onReplaceClicked);

    connect(searchTextLineEdit, &QLineEdit::textEdited, this, [this]{
        searchForText(searchTextLineEdit->text());
    });
}

void searchAndReplaceObject::onReplaceClicked() {
    QString replaceText = replaceTextLineEdit->text();
    if (replaceText.isEmpty() || foundOccurrences.isEmpty()) {
        return;  // If the replacement text is empty or no occurrences found, do nothing.
    }

    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();  // Start a single undo block for the whole replacement operation.

    for (QTextCursor &occurrence : foundOccurrences) { // loops through the vector storing all instances of the items to removed
        occurrence.beginEditBlock();
        occurrence.insertText(replaceText);
        occurrence.endEditBlock();
    }

    cursor.endEditBlock();  // End the undo block.
    foundOccurrences.clear();  // Clear occurrences after replacement
}


void searchAndReplaceObject::searchForText(const QString& text){

    removeHighlights(); // removes any text that was previously highlighted
    foundOccurrences.clear(); // clears the vector storing all instances

    if (text.isEmpty()) {
        return; // returns if empty string
    }

    QTextDocument *document = editor->document();
    QTextCursor highlightCursor(document);
    QTextCursor cursor(document);

    cursor.beginEditBlock();

    QTextCharFormat plainFormat(highlightCursor.charFormat());
    QTextCharFormat colorFormat = plainFormat;
    colorFormat.setBackground(Qt::blue);

    QTextDocument::FindFlags flag;

    // or equals operator, if true it is added
    if (isCaseSensitive->isChecked()) flag |= QTextDocument::FindCaseSensitively;

    if (isMatchWholeWord->isChecked()) flag |= QTextDocument::FindWholeWords;

    while (!highlightCursor.isNull() && !highlightCursor.atEnd()){
        highlightCursor = document->find(text, highlightCursor, flag);

        if (!highlightCursor.isNull()){
                // highlightCursor.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);
            highlightCursor.mergeCharFormat(colorFormat);

            foundOccurrences.append(highlightCursor);
        }
    }
    cursor.endEditBlock();

    if(foundOccurrences.size() == 0) return; // if an item is found moves the cursor to the last item

    selectedOccurenceIndex = foundOccurrences.size();
    QString labelText = QString::number(selectedOccurenceIndex) + " / " + QString::number(foundOccurrences.size());
    occurenceIteratorLabel->setText(labelText);

    auto textcursor = editor->textCursor();
    textcursor.setPosition(foundOccurrences.at(selectedOccurenceIndex-1).position());
    editor->setTextCursor(textcursor);
};

void searchAndReplaceObject::removeHighlights(){
    QTextDocument *document = editor->document();
    QTextCursor cursor(document);
    cursor.beginEditBlock();

    QTextCharFormat plainFormat;
    plainFormat.setBackground(Qt::transparent); // Set background to transparent to remove highlight
    cursor.select(QTextCursor::Document);
    cursor.mergeCharFormat(plainFormat);

    cursor.endEditBlock();
}

void searchAndReplaceObject::showWidget(){
    this->showNormal();
    searchTextLineEdit->setFocus();
}

void searchAndReplaceObject::goToPreviousSelection(){

    if(selectedOccurenceIndex == 1) selectedOccurenceIndex = foundOccurrences.size(); // loops it around to restart at the top
    else selectedOccurenceIndex--;
    QString text = QString::number(selectedOccurenceIndex) + " / " + QString::number(foundOccurrences.size());
    occurenceIteratorLabel->setText(text);


    auto textcursor = editor->textCursor();
    textcursor.setPosition(foundOccurrences.at(selectedOccurenceIndex-1).position());
    editor->setTextCursor(textcursor);


}
void searchAndReplaceObject::goToNextSelection(){

    if(selectedOccurenceIndex == foundOccurrences.size()) selectedOccurenceIndex = 1;
    else{
        selectedOccurenceIndex++;
    }
    QString text = QString::number(selectedOccurenceIndex) + " / " + QString::number(foundOccurrences.size());
    occurenceIteratorLabel->setText(text);

    // if they press next and its at the maximum it loops back to 1
    auto textcursor = editor->textCursor();
    textcursor.setPosition(foundOccurrences.at(selectedOccurenceIndex-1).position());
    editor->setTextCursor(textcursor);
    // occurenceIteratorLabel->setText()

}


