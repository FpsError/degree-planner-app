#ifndef COURSEDETAILS_H
#define COURSEDETAILS_H

#include <QDialog>

namespace Ui {
class CourseDetails;
}

class CourseDetails : public QDialog
{
    Q_OBJECT

public:
    explicit CourseDetails(QString semester, QWidget *parent = nullptr);
    ~CourseDetails();

    QString getCourseCode();
    std::tuple<bool, bool, bool> getCourseStatus();
    QString getCourseGrade();

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_buttonBox_accepted();

    void on_checkBox_3_checkStateChanged(const Qt::CheckState &arg1);

    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1);

    void on_lineEdit_2_textChanged(const QString &arg1);

private:
    Ui::CourseDetails *ui;
    QString course_code;
    QString semester;
    int is_current_course = 0;
    int is_done_course = 0;
    int is_planned_course = 0;
    QString grade;

    void populateCoursesCombobox();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // COURSEDETAILS_H
