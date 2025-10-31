#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QMainWindow>
#include "mainwindow.h"

namespace Ui {
class WelcomeWindow;
}

class WelcomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = nullptr);
    ~WelcomeWindow();

private slots:
    void on_pushButton_clicked();
    void populateGraduationYears();

    void go_back_page();

    void on_continueButton_1_clicked();

    void on_backButton_2_clicked();

    void on_confirm_button_clicked();

    void on_backButton_1_clicked();

    void on_graduation_ycombobox_currentIndexChanged(int index);

    void on_fname_line_textChanged(const QString &arg1);

    void on_lname_line_textChanged(const QString &arg1);

    void on_total_credits_line_textChanged(const QString &arg1);

    void populateMajor();
    void on_comboBox_2_currentIndexChanged(int index);

    void on_continueButton_3_clicked();

    void on_continueButton_4_clicked();

private:
    Ui::WelcomeWindow *ui;
    MainWindow w;
};

#endif // WELCOMEWINDOW_H
