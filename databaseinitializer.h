#ifndef DATABASEINITIALIZER_H
#define DATABASEINITIALIZER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseInitializer
{
public:
    static bool initializeDatabase(const QString& dbPath);

private:
    static bool createTables(QSqlDatabase& db);
    static bool insertAllData(QSqlDatabase& db);
    static bool insertSchools(QSqlDatabase& db);
    static bool insertDepartments(QSqlDatabase& db);
    static bool insertCourses(QSqlDatabase& db);
    static bool insertSemesters(QSqlDatabase& db);
    static bool insertMajors(QSqlDatabase& db);
    static bool insertPrerequisites(QSqlDatabase& db);
    static bool createTrigger(QSqlDatabase& db);
};

struct Course {
    QString code;
    QString title;
    QString description;
    int credits;
    QString classification;
    QString depCode;
    QString type;
    QString level;
};

#endif // DATABASEINITIALIZER_H
