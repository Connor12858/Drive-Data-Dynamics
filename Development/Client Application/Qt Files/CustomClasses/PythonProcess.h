#ifndef PYTHONPROCESS_H
#define PYTHONPROCESS_H

#include <QProcess>
#include <QDebug>
#include <QObject>
#include <QString>

class PythonProcess : public QObject
{
    Q_OBJECT

public:
    PythonProcess(const QString &pythonFile, QObject *parent = nullptr);

    void startProcess();

    void stopProcess();

    void sendCommand(const QString &command);

    void sendByteArray(const QByteArray &command);

    bool isProcessRunning() const;

    bool isGood() const;

    void setToGood(bool state);

private slots:

    void readOutput();

    void readError();

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString pythonFile;
    QProcess *process;
    bool _good;
};

#endif // PYTHONPROCESS_H
