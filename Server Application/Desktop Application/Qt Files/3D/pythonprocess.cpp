#include "pythonprocess.h"
#include <QProcess>
#include <QDebug>
#include <QObject>
#include <QString>

PythonProcess::PythonProcess(const QString &file, QObject *parent) : QObject(parent)
{
    process = new QProcess(this);
    pythonFile = file;
}

void PythonProcess::startProcess()
{
    // Run Python in unbuffered mode (-u)
    process->start("python", QStringList() << "-u" << pythonFile);

    // Read standard output in real-time
    connect(process, &QProcess::readyReadStandardOutput, this, &PythonProcess::readOutput);

    // Read standard error in real-time
    connect(process, &QProcess::readyReadStandardError, this, &PythonProcess::readError);

    // Handle process exit
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PythonProcess::processFinished);
}

void PythonProcess::stopProcess()
{
    if (process->state() == QProcess::Running)
    {
        process->terminate(); // Gracefully stop the process
        if (!process->waitForFinished(3000))
        {
            process->kill(); // Force kill if it doesn't stop
        }
    }
}

void PythonProcess::readOutput()
{
    QByteArray output = process->readAllStandardOutput();
    qDebug() << "Python Output:" << QString(output);
}

void PythonProcess::readError()
{
    QByteArray errorOutput = process->readAllStandardError();
    qDebug() << "Python Error:" << QString(errorOutput);
}

void PythonProcess::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Python process finished with exit code:" << exitCode;
    if (exitStatus == QProcess::CrashExit)
    {
        qDebug() << "Python script crashed!";
    }
}

bool PythonProcess::isProcessRunning() const {
    return process->state() == QProcess::Running;
}



