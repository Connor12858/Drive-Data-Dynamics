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

void PythonProcess::sendCommand(const QString &command)
{
    if (process->state() == QProcess::Running)
    {
        process->write(command.toUtf8() + "\n"); // Send command to Python script
        process->waitForBytesWritten();
    }
    else
    {
        qDebug() << "Python process is not running. 1" << command;
    }
}
void PythonProcess::sendByteArray(const QByteArray &command)
{
    if (process->state() == QProcess::Running)
    {
        process->write(command); // Send command to Python script
        process->waitForBytesWritten();
    }
    else
    {
        qDebug() << "Python process is not running. 2";
    }
}

void PythonProcess::startProcess()
{
    // Run Python in unbuffered mode (-u)
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start("python", QStringList() << "-u" << pythonFile);

    // Read standard output in real-time
    connect(process, &QProcess::readyReadStandardOutput, this, &PythonProcess::readOutput);
    connect(process, &QProcess::readyReadStandardError, this, &PythonProcess::readError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PythonProcess::processFinished);
}

void PythonProcess::stopProcess()
{
    if (process->state() == QProcess::Running)
    {
        process->terminate(); // Gracefully stop the process
        if (!process->waitForFinished(1000))
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

bool PythonProcess::isProcessRunning() const
{
    return process->state() == QProcess::Running;
}
