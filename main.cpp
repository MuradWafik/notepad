#include "mainwindow.h"

#include <QApplication>
#include "pythoninstaller.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    qDebug() << PythonInstaller::pythonInstallURL();
    return a.exec();
}
