#include "newsemester.h"
#include "ui_newsemester.h"

newSemester::newSemester(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::newSemester)
{
    ui->setupUi(this);
}

newSemester::~newSemester()
{
    delete ui;
}

void newSemester::on_summer_semester_button_clicked()
{
    semester_type = 1;
}


void newSemester::on_regular_semester_button_clicked()
{
    semester_type = 0;
}

int newSemester::get_sem_type(){
    return semester_type;
}
