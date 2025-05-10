#include "pythoninstaller.h"

// PythonInstaller::PythonInstaller() {}

QString PythonInstaller::pythonInstallURL(){
#ifdef Q_OS_WIN
    return QString("https://www.python.org/ftp/python/3.13.3/python-3.13.3-amd64.exe");
#elif defined(Q_OS_LINUX)
    return QString("https://www.python.org/ftp/python/3.13.3/Python-3.13.3.tgz");
#elif defined(Q_OS_MACOS)
        return QString("https://www.python.org/ftp/python/3.13.3/python-3.13.3-macos11.pkg");
#else
    return QString("INVALID OS DETECTED");
#endif
}

