#ifndef SEARCHANDREPLACEOBJECT_H
#define SEARCHANDREPLACEOBJECT_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include "ui_mainwindow.h"
#include <QCheckBox>
// #include <QBoxLayout>


class searchAndReplaceObject : public QWidget
{
    Q_OBJECT

public:
    // explicit searchAndReplaceObject(QWidget *parent = nullptr);
    // explicit searchAndReplaceObject(Ui::MainWindow *ui = nullptr);
    explicit searchAndReplaceObject(QPlainTextEdit* editor, QWidget* parent);

signals:

public slots:
    // funcs
    void searchForText(const QString& text);
    void setupUI();
    void connectSignalsAndSlots();
    void onReplaceClicked();
    void removeHighlights();
    void showWidget();

private:
    QLineEdit* searchTextLineEdit;
    QLineEdit* replaceTextLineEdit;
    QVector<QTextCursor> foundOccurrences;
    QCheckBox* isCaseSensitive;
    QCheckBox* isMatchWholeWord;
    QPushButton* replaceTextButton;
    QPlainTextEdit* editor;  // Declare the editor member variable
    // QWidget* searchAndReplaceContainer;
    // QHBoxLayout fullLayoutParent;


};

#endif // SEARCHANDREPLACEOBJECT_H
