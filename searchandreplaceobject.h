#ifndef SEARCHANDREPLACEOBJECT_H
#define SEARCHANDREPLACEOBJECT_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include "ui_mainwindow.h"
#include <QCheckBox>
#include <QFrame>
// #include <QBoxLayout>


class searchAndReplaceObject : public QDockWidget
{
    Q_OBJECT


public:
    // explicit searchAndReplaceObject(QWidget *parent = nullptr);
    // explicit searchAndReplaceObject(Ui::MainWindow *ui = nullptr);
    searchAndReplaceObject(QPlainTextEdit* editor, QWidget* parent);
    ~searchAndReplaceObject();

signals:

protected slots:
    void closeEvent(QCloseEvent *event) override;


public slots:
    // funcs
    void searchForText(const QString& text);
    void setupUI();
    void connectSignalsAndSlots();
    void onReplaceClicked();
    void removeHighlights();
    void showWidget();
    void goToPreviousSelection();
    void goToNextSelection();




private:
    QLineEdit* searchTextLineEdit;
    QLineEdit* replaceTextLineEdit;
    QVector<QTextCursor> foundOccurrences;
    QCheckBox* isCaseSensitive;
    QCheckBox* isMatchWholeWord;
    QPushButton* replaceTextButton;
    QPlainTextEdit* editor;  // Declare the editor member variable


    QPushButton* nextMatchButton;
    QPushButton* prevMatchButton;
    QLabel* occurenceIteratorLabel;

    int selectedOccurenceIndex;
    // QWidget* searchAndReplaceContainer;
    // QHBoxLayout fullLayoutParent;


};

#endif // SEARCHANDREPLACEOBJECT_H
