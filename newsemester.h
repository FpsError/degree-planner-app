#ifndef NEWSEMESTER_H
#define NEWSEMESTER_H

#include <QDialog>

namespace Ui {
class newSemester;
}

class newSemester : public QDialog
{
    Q_OBJECT

public:
    explicit newSemester(QWidget *parent = nullptr);
    ~newSemester();

    int get_sem_type();

private slots:
    void on_summer_semester_button_clicked();

    void on_regular_semester_button_clicked();

private:
    Ui::newSemester *ui;

    int semester_type; // 0 for regular, 1 for summer
};

#endif // NEWSEMESTER_H
