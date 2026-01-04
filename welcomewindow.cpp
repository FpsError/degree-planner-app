#include "welcomewindow.h"
#include "global_objects.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "ui_welcomewindow.h"

#include <QGraphicsDropShadowEffect>
#include <QDate>

WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WelcomeWindow)
{
    ui->setupUi(this);

    populateGraduationYears();
    populateMajor();

    ui->stackedWidget->setCurrentIndex(0);

    ui->progressBar->hide();
    ui->step_lable->hide();

    ui->continueButton_1->setDisabled(1);
    ui->confirm_button->setDisabled(1);
}

WelcomeWindow::~WelcomeWindow()
{
    delete ui;
}

void WelcomeWindow::go_back_page(){
    int currentPage;
    currentPage = ui->stackedWidget->currentIndex();

    ui->stackedWidget->setCurrentIndex(currentPage-1);
}

void WelcomeWindow::on_pushButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);

    ui->progressBar->setValue(50);
    ui->progressBar->show();
    ui->step_lable->show();
}

void WelcomeWindow::populateGraduationYears()
{
    // Get current year
    int currentYear = QDate::currentDate().year();

    // Add years from current year to current year + 10
    for (int year = currentYear; year <= currentYear + 10; ++year) {
        ui->graduation_ycombobox->addItem(QString::number(year));
    }
}

void WelcomeWindow::populateMajor(){

    QSqlQuery query;
    query.exec("Select major_name from major");

    while(query.next()){
        QString name = query.value(0).toString();
        ui->comboBox_2->addItem(name);
    }
}

void WelcomeWindow::on_continueButton_1_clicked()
{
    int currentPage;
    currentPage = ui->stackedWidget->currentIndex();

    ui->stackedWidget->setCurrentIndex(currentPage+1);
    ui->progressBar->setValue(100);
    ui->step_lable->setText("Step 2 of 2");
}


void WelcomeWindow::on_backButton_2_clicked()
{
    go_back_page();
}


void WelcomeWindow::on_confirm_button_clicked()
{
    std::string fname = ui->fname_line->text().toStdString();
    std::string lname = ui->lname_line->text().toStdString();
    int graduation_year = ui->graduation_ycombobox->currentText().toInt();
    int starting_year = ui->starting_year->text().toInt();
    std::string minor;

    std::string starting_semester;

    if(ui->continueButton_3->isChecked()){
        starting_semester = "Fall";
    } else if (ui->continueButton_4->isChecked()) {
            starting_semester = "Spring";
    }
    std::string major = ui->comboBox_2->currentText().toStdString();

    //qDebug() << fname + " " + lname + " " + std::to_string(total_credits) + " " + std::to_string(graduation_year) + " " + std::to_string(starting_year) + " " + starting_semester + " " + major;

    QSqlQuery query;
    query.prepare("INSERT INTO profile (fname, lname, graduation_year, starting_year, starting_semester, major, minor)"
                  "VALUES (:fname, :lname, :graduation_year, :starting_year, :starting_semester, :major, :minor)");
    query.bindValue(":fname", QString::fromStdString(fname));
    query.bindValue(":lname", QString::fromStdString(lname));
    query.bindValue(":graduation_year", graduation_year);
    query.bindValue(":starting_year", starting_year);
    query.bindValue(":starting_semester", QString::fromStdString(starting_semester));
    query.bindValue(":major", QString::fromStdString(major));
    if (!minor.empty()){
        query.bindValue(":major", QString::fromStdString(minor));
    }
    int profileId = -1;
    if (query.exec()) {
        // Get the last inserted ID
        QVariant lastId = query.lastInsertId();
        if (lastId.isValid()) {
            profileId = lastId.toInt();
            qDebug() << "Inserted profile with ID:" << profileId;
        } else {
            qDebug() << "Failed to get last inserted ID";
        }
    } else {
        qDebug() << "Insert failed:" << query.lastError().text();
    }
    profile_id = profileId;
    w.refreshWindow();
    w.show();
    close();
}


void WelcomeWindow::on_backButton_1_clicked()
{
    go_back_page();
}


void WelcomeWindow::on_graduation_ycombobox_currentIndexChanged(int index)
{
    if(!ui->fname_line->text().isEmpty() && !ui->lname_line->text().isEmpty()
        && index != -1){
        ui->continueButton_1->setEnabled(1);
    } else {
        ui->continueButton_1->setDisabled(1);
    }
}


void WelcomeWindow::on_fname_line_textChanged(const QString &arg1)
{
    if(!arg1.isEmpty() && !ui->lname_line->text().isEmpty()
        && ui->graduation_ycombobox->currentIndex() != -1){
        ui->continueButton_1->setEnabled(1);
    }else {
        ui->continueButton_1->setDisabled(1);
    }
}


void WelcomeWindow::on_lname_line_textChanged(const QString &arg1)
{
    if(!ui->fname_line->text().isEmpty() && !arg1.isEmpty()
        && ui->graduation_ycombobox->currentIndex() != -1){
        ui->continueButton_1->setEnabled(1);
    }else {
        ui->continueButton_1->setDisabled(1);
    }
}


void WelcomeWindow::on_total_credits_line_textChanged(const QString &arg1)
{
    if(arg1.isEmpty())
        ;
    if(!ui->fname_line->text().isEmpty() && !ui->lname_line->text().isEmpty()
        && ui->graduation_ycombobox->currentIndex() != -1){
        ui->continueButton_1->setEnabled(1);
    }else {
        ui->continueButton_1->setDisabled(1);
    }
}


void WelcomeWindow::on_comboBox_2_currentIndexChanged(int index)
{
    if(index == 1)
        ;
    if(ui->continueButton_3->isChecked() || ui->continueButton_4->isChecked()){
        ui->confirm_button->setEnabled(1);
    }else {
        ui->confirm_button->setDisabled(1);
    }
}


void WelcomeWindow::on_continueButton_3_clicked()
{
    if(ui->comboBox_2->currentIndex() != -1){
        ui->confirm_button->setEnabled(1);
    }else {
        ui->confirm_button->setDisabled(1);
    }
}


void WelcomeWindow::on_continueButton_4_clicked()
{
    if(ui->comboBox_2->currentIndex() != -1){
        ui->confirm_button->setEnabled(1);
    }else {
        ui->confirm_button->setDisabled(1);
    }
}

