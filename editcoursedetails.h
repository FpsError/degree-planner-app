#ifndef EDITCOURSEDETAILS_H
#define EDITCOURSEDETAILS_H

#include "course.h"
#include <QDialog>

namespace Ui {
class EditCourseDetails;
}

class EditCourseDetails : public QDialog
{
    Q_OBJECT

public:
    explicit EditCourseDetails(QString course_code ,QString semester, QWidget *parent = nullptr);
    ~EditCourseDetails();

    course getUpdatedCourse();

private slots:
    void on_comboBox_currentIndexChanged(int index);

    //void on_checkBox_3_checkStateChanged(const Qt::CheckState &arg1);

    //void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    //void on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1);

    void on_buttonBox_accepted();

    void on_checkBox_3_checkStateChanged(const Qt::CheckState &arg1);

    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::EditCourseDetails *ui;
    QString old_course_code;
    QString new_course_code;
    QString semester;
    int is_current_course = 0;
    int is_done_course = 0;
    int is_planned_course = 0;
    int grade = 0;

    void populateCoursesCombobox();
    std::tuple<QString, QString> getCourseStatus(bool is_done, bool is_current, bool is_planned);
    void checkCurrentCourse(QString course_code);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // EDITCOURSEDETAILS_H
