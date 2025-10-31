#include "mainwindow.h"
#include "welcomewindow.h"
#include "global_objects.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSqlQuery>

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

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("DegreePlanner.db");

    bool db_is_open = db.open();

    MainWindow w;
    WelcomeWindow wm;

    if (db_is_open)
    {
        QSqlQuery query;
        query.exec("Select id from profile");

        if (query.next()){
            //qDebug() << "Profile found, opening it";
            profile_id = query.value(0).toInt();
            w.show();
        } else {
            //qDebug() << "Profile not found, creating one";
            wm.show();
        }
    }
    else{
        //qDebug() << "database is not connected";
    }


    return a.exec();
}

