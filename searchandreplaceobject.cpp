#include "searchandreplaceobject.h"

searchAndReplaceObject::searchAndReplaceObject(QPlainTextEdit* editor, QWidget* parent)
    : QWidget(parent)
    , editor(editor)
{
    setupUI(); // makes the ui items and signal connections in constructor
    connectSignalsAndSlots();
    this->hide();
}

void searchAndReplaceObject::setupUI(){
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


    QHBoxLayout* fullLayoutParent = new QHBoxLayout(this);
    fullLayoutParent->setAlignment(Qt::AlignHCenter);


    QVBoxLayout* searchAndReplaceParent = new QVBoxLayout();

    QVBoxLayout* checkBoxesParent = new QVBoxLayout();
    checkBoxesParent->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    searchTextLineEdit = new QLineEdit();
    replaceTextLineEdit = new QLineEdit();
    replaceTextButton = new QPushButton("Replace Text");


    isCaseSensitive = new QCheckBox();
    isCaseSensitive->setToolTip("Case Sensitive");
    QIcon caseSensitveIcon = QIcon(":/imgs/gui-case-sensitive.svg");
    isCaseSensitive->setIcon(caseSensitveIcon);


    isMatchWholeWord = new QCheckBox();
    isMatchWholeWord->setToolTip("Match Whole Word");
    QIcon matchWholeIcon = QIcon(":/imgs/whole-word.svg");
    isMatchWholeWord->setIcon(matchWholeIcon);

    qApp->setStyleSheet(" QCheckBox:hover{background-color: light-grey;}");

    QIcon closeIcon = qApp->style()->standardIcon(QStyle::SP_TitleBarCloseButton);
    QPushButton* hidePopUpButton = new QPushButton();

    hidePopUpButton->setIcon(closeIcon);
    hidePopUpButton->setStyleSheet(" QPushButton:hover:!pressed{background-color: red;}");

    connect(hidePopUpButton, &QPushButton::pressed, this, [this](){
        removeHighlights(); // if any text was selected before hand, the highlight gets removed
        this->hide();
        editor->setFocus(); // changes the focused widget to the main text box
    });


    searchAndReplaceParent->addWidget(searchTextLineEdit);
    searchAndReplaceParent->addWidget(replaceTextLineEdit);
    searchAndReplaceParent->addWidget(replaceTextButton);

    checkBoxesParent->addWidget(hidePopUpButton);
    checkBoxesParent->addWidget(isCaseSensitive);
    checkBoxesParent->addWidget(isMatchWholeWord);

    fullLayoutParent->addLayout(searchAndReplaceParent);
    fullLayoutParent->addLayout(checkBoxesParent);

    this->setLayout(fullLayoutParent);


    searchTextLineEdit->setPlaceholderText("Search Text");
    searchTextLineEdit->setFixedSize(120, 30);
    replaceTextLineEdit->setPlaceholderText("Replace With");
    replaceTextLineEdit->setFixedSize(120, 30);
    replaceTextButton->setFixedSize(90, 30);


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
    // qDebug() << editor->toPlainText();
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

    // searchAndReplaceStatusLabel->setText(QString("Replaced %1 occurrences").arg(foundOccurrences.size()));
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

    bool caseSensitive = isCaseSensitive->isChecked();
    bool matchWhole = isMatchWholeWord->isChecked();

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
        }
    }

    cursor.endEditBlock();
    // statusBar()->showMessage(QString("Occurrences: %1").arg(foundTextMatchCount));
    // searchAndReplaceStatusLabel->setText(QString("Occurrences: %1").arg(foundOccurrences.size()));
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
    // statusBar()->clearMessage();
    // searchAndReplaceStatusLabel->clear();
}

void searchAndReplaceObject::showWidget(){
    this->show();
    searchTextLineEdit->setFocus();
}
