#include "mainwindow.h"
#include "welcomewindow.h"
#include "global_objects.h"
#include "DatabaseInitializer.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSqlQuery>
#include <QDir>
#include <QMessageBox>

// check if database is already initialized
bool isDatabaseInitialized() {
    QSqlQuery query;

    // Check if 'course' table exists (or any other key table)
    query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='course'");

    return query.next(); // Returns true if table exists
}


// Main database initialization function
bool initializeDatabase() {
    // Get database path (next to executable)
    QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("DegreePlanner.db");

    qDebug() << "Database path:" << dbPath;

    // Initialize database with ALL data
    if (!DatabaseInitializer::initializeDatabase(dbPath)) {
        QMessageBox::critical(nullptr, "Database Error",
                                "Failed to initialize the database.\n"
                                "The application cannot continue.");
        return 1;
    }

    qDebug() << "Database initialized successfully with all tables and data!";

    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "degreePlan_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    initializeDatabase();

    QSqlQuery query;
    query.exec("Select id from profile");
    if (query.next()){
        qDebug() << "Profile found, opening it";
        profile_id = query.value(0).toInt();
    } else {
        qDebug() << "Profile not found, creating one";
    }

    MainWindow w;
    WelcomeWindow wm;

    //has to be this way for the profile_id not to be -1 later on
    if (profile_id != -1){
        w.show();
    } else {
        wm.show();
    }

    return a.exec();
}
