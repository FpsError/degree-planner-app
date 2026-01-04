#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "course.h"
#include "qboxlayout.h"
#include "qframe.h"
#include "qgraphicsscene.h"
#include "qpushbutton.h"
#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QDebug>

enum AcademicStanding {
    PROBATION,      // < 2.0
    SATISFACTORY,   // 2.0 - 3.49
    GOOD,           // 3.5 - 3.79
    EXCELLENT,      // 3.8 - 4.0
    HIGHEST_HONORS,  // = 4.0
    INVALID
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    std::tuple<QString, QString> getCourseStatus(bool is_done, bool is_current, bool is_planned);
    void refreshWindow();

private slots:   
    //void on_pushButton_clicked();
    bool onAddCourseButtonClicked(QFrame *frame, QLayout *sourceLayout, QString semester);
    void onFirstAddCourseButtonClicked(QFrame *frame, QLayout *sourceLayout, QFrame *noCoursesFrame, QString semester);
    void onDeleteCourseButtonClicked(QFrame *parent);
    void onEditCourseButtonClicked(QFrame* source, course course_label, QString semester);
    void onAddSemButtonClicked();
    void onAddSemButtonClicked(QHBoxLayout* layout,int year, QPushButton* button);

    void on_tabWidget_tabBarClicked(int index);

private:
    course course_labels_temp;
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    void populateSemesters();
    void populateGraphics();

    void createYearFrame(int year, QString starting_semester);
    void createSemesterFrame(int year, QString semester);
    void createSemesterFrame(QHBoxLayout* yearFrame, int year, QString semester);
    void addCoursesFromDatabase(QFrame *frame, QLayout *sourceLayout, QString course_code);
    void addAddSummerButton(QHBoxLayout* layout, int year);
    QObject* findParent(QObject* child, const QString& parent_name);
    void updateSemesterStatus(QObject *semesterFrame, QString semester);
    void updateSemesterStatus(QObject *semesterFrame);

    std::string extractYear(const std::string& semester);
    std::string extractSeason(const std::string& semester);
    void swapTwoItemsInLayout(QVBoxLayout* layout, int index_1, int index_2);

    void updateGPA();
    AcademicStanding getStandingFromGPA(double gpa);
    int getCreditsEarned();
    void updateCreditsEarned();
    void updateSemsLeft();
    bool hasSummerSemester(int year);
    float PointsFromGradeLetters(QString grade);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // MAINWINDOW_H
