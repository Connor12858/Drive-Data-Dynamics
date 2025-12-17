#include "PythonProcess.h"
#include <QProcess>
#include <QDebug>
#include <QObject>
#include <QString>

// Class to run python scripts
PythonProcess::PythonProcess(const QString &file, QObject *parent) : QObject(parent)
{
    process = new QProcess(this);
    pythonFile = file;
}

// Send a message to the python script
void PythonProcess::sendCommand(const QString &command) {
    if (process->state() == QProcess::Running) {
        process->write(command.toUtf8() + "\n");  // Send command to Python script
        process->waitForBytesWritten();
    } else {
        qDebug() << "Python process is not running.";
    }
}

// Start the python script
void PythonProcess::startProcess()
{
    // Run Python
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start("python", QStringList() << pythonFile);

    // Read standard output in real-time
    connect(process, &QProcess::readyReadStandardOutput, this, &PythonProcess::readOutput);
    connect(process, &QProcess::readyReadStandardError, this, &PythonProcess::readError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PythonProcess::processFinished);
}

// Close the python script
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

// Read the output that python generates
void PythonProcess::readOutput()
{
    QByteArray output = process->readAllStandardOutput();
    qDebug() << "Python Output:" << QString(output);
}

// Read the error that python generates
void PythonProcess::readError()
{
    QByteArray errorOutput = process->readAllStandardError();
    qDebug() << "Python Error:" << QString(errorOutput);
}

// Let the user know the script finished
void PythonProcess::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Python process finished with exit code:" << exitCode;
    if (exitStatus == QProcess::CrashExit)
    {
        qDebug() << "Python script crashed!";
    }
}

// Check if the python script is running
bool PythonProcess::isProcessRunning() const {
    return process->state() == QProcess::Running;
}



