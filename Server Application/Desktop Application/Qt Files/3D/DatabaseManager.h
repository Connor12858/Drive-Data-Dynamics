#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QDir>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool openDatabase(const QString &dbPath);
    bool insertLogFile(const QString &filename, const QString &filepath, const QString &clientName, const QString &eventName);
    void fetchLogFiles();

    bool isOpen();

private:
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
