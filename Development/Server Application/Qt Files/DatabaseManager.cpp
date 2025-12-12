#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

// Open the database to connect to
bool DatabaseManager::openDatabase(const QString &dbPath)
{
    // Set up the database connection
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        //qCritical() << "Error opening database: " << db.lastError();
        return false;
    }
    //qDebug() << "Database opened successfully!";
    return true;
}

// Add a log gile
bool DatabaseManager::insertLogFile(const QString &filename, const QString &filepath, const QString &clientName, const QString &eventName)
{
    // Insert a new log file record into the database
    QSqlQuery query;
    query.prepare("INSERT INTO log_files (filename, filepath, client_name, event_name, timestamp) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(filename);
    query.addBindValue(filepath);
    query.addBindValue(clientName);
    query.addBindValue(eventName);
    query.addBindValue(QDateTime::currentSecsSinceEpoch()); // current timestamp

    if (!query.exec()) {
        qCritical() << "Failed to insert log file: " << query.lastError();
        return false;
    }
    qDebug() << "Log file inserted successfully!";
    return true;
}

// Get all the log files
void DatabaseManager::fetchLogFiles()
{
    // Fetch all log files from the database
    QSqlQuery query("SELECT * FROM log_files");

    while (query.next()) {
        QString filename = query.value(1).toString();
        QString filepath = query.value(2).toString();
        QString clientName = query.value(3).toString();
        QString eventName = query.value(4).toString();
        qDebug() << "File: " << filename << " Path: " << filepath
                 << " Client: " << clientName << " Event: " << eventName;
    }
}

// Check if connection to database is open
bool DatabaseManager::isOpen() {
    return db.isOpen();
}
