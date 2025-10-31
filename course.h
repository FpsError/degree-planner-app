#ifndef COURSE_H
#define COURSE_H

#include "qlabel.h"

struct course {
    QLabel *titleLabel;
    QLabel *statusLabel;
    QLabel *desc;
    QLabel *credits;
    QLabel *level;
    QLabel *grade;

    void updateText(course new_course){
        titleLabel->setText(new_course.titleLabel->text());
        statusLabel->setText(new_course.statusLabel->text());
        statusLabel->setStyleSheet(new_course.statusLabel->styleSheet());
        desc->setText(new_course.desc->text());
        credits->setText(new_course.credits->text());
        level->setText(new_course.level->text());
        grade->setText(new_course.grade->text());
    }
};

#endif // COURSE_H
