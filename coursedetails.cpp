#include "coursedetails.h"
#include "global_objects.h"
#include "qpushbutton.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "ui_coursedetails.h"

#include <QCompleter>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QMouseEvent>

CourseDetails::CourseDetails(QString semester, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CourseDetails)
{
    ui->setupUi(this);

    this->semester = semester;

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(1);

    ui->label_2->setText(semester);
    ui->course_title->setReadOnly(1);
    ui->spinBox->setReadOnly(1);

    ui->comboBox->setEditable(1);
    ui->comboBox->itemDelegate();
    ui->comboBox->setMaxVisibleItems(5);
    ui->comboBox->setPlaceholderText("Search by course code (e.g., CSC1401)");
    QCompleter *completer = new QCompleter(ui->comboBox->model(), this);
    ui->comboBox->setCompleter(completer);
    completer->setCaseSensitivity(Qt::CaseInsensitive); // Make suggestions case-insensitive
    completer->setFilterMode(Qt::MatchContains); // Suggest items that *contain* the text, not just start with it

    QAbstractItemView *popup = completer->popup();
    popup->setStyleSheet(R"(
    QListView {
        background-color: #252526;
        border: 1px solid #3e3e42;
        border-radius: 4px;
        padding: 2px;
        color: #cccccc;
        outline: 0;
    }
    QListView::item {
        padding: 5px;
        border-radius: 2px;
    }
    QListView::item:selected {
        background-color: #007acc;
        color: #ffffff;
    }
    QListView::item:hover {
        background-color: #2a2d2e;
    }
    QListView::item:hover:selected {
        background-color: #1c97ea;
    }
    QScrollBar:vertical {
        background-color: #252526;
        width: 12px;
        margin: 0px;
    }
    QScrollBar::handle:vertical {
        background-color: #424245;
        border-radius: 4px;
        min-height: 20px;
    }
    QScrollBar::handle:vertical:hover {
        background-color: #515155;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0px;
    }
)");
    //ui->comboBox->installEventFilter(this);
    //ui->comboBox->lineEdit()->installEventFilter(this);
    populateCoursesCombobox();
}

CourseDetails::~CourseDetails()
{
    delete ui;
}

void CourseDetails::populateCoursesCombobox(){
    QSqlQuery query;
    query.exec("Select course_code from course "
               "where course_code NOT IN (select cp.course_code from course_planning cp)");
    while(query.next()){
        QString course = query.value(0).toString();
        ui->comboBox->addItem(course);
    }
}

void CourseDetails::on_comboBox_currentIndexChanged(int index)
{
    QString course = ui->comboBox->itemText(index);
    this->course_code = course;

    QSqlQuery query;
    query.prepare("Select course_title, course_credits from course where course_code = :course");
    query.bindValue(":course", course);
    query.exec();
    query.next();

    QString course_title;
    int course_credits;

    course_title = query.value("course_title").toString();
    course_credits = query.value("course_credits").toInt();

    ui->course_title->setText(course_title);
    ui->spinBox->setValue(course_credits);

    if(ui->checkBox->isChecked() or ui->checkBox_2->isChecked() or ui->checkBox_3->isChecked()){
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(1);
    }
}

QString CourseDetails::getCourseCode(){
    return this->course_code;
}

void CourseDetails::on_buttonBox_accepted()
{
    QSqlQuery query;
    query.prepare("Insert into course_planning (course_code, sem_code, is_current_course, is_planned_course, is_done_course, grade, profile_id) "
                  "VALUES (:course_code, :sem_code, :is_current_course, :is_planned_course, :is_done_course, :grade, :profile_id)");
    query.bindValue(":course_code", course_code);
    query.bindValue(":sem_code", semester.remove(" "));
    query.bindValue(":is_current_course", is_current_course);
    query.bindValue(":is_planned_course", is_planned_course);
    query.bindValue(":is_done_course", is_done_course);
    query.bindValue(":grade", grade);
    query.bindValue(":profile_id", profile_id);
    query.exec();

    if(query.lastError().isValid()){
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText(query.lastError().text());
        msgBox.setIcon(QMessageBox::Critical);

        // Apply style sheet
        msgBox.setStyleSheet(
            "QMessageBox { "
            "   background-color: #2b2b2b; "
            "   color: white; "
            "}"
            "QMessageBox QLabel { "
            "   color: white; "
            "   font-size: 14px; "
            "}"
            "QMessageBox QPushButton { "
            "   background-color: #0078d4; "
            "   color: white; "
            "   border: none; "
            "   padding: 8px 16px; "
            "   border-radius: 4px; "
            "   min-width: 80px; "
            "}"
            "QMessageBox QPushButton:hover { "
            "   background-color: #106ebe; "
            "}"
            );

        msgBox.exec();

        reject();
    }
}

std::tuple<bool, bool, bool> CourseDetails::getCourseStatus(){
    return {is_current_course, is_planned_course, is_done_course};
}


void CourseDetails::on_checkBox_3_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        is_done_course = 1;
    } else{
        is_done_course = 0;
    }

    if(ui->comboBox->currentIndex() != -1){
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(1);
    }
}


void CourseDetails::on_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        is_current_course = 1;
    } else{
        is_current_course = 0;
    }
    if(ui->comboBox->currentIndex() != -1){
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(1);
    }
}


void CourseDetails::on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        is_planned_course = 1;
    } else{
        is_planned_course = 0;
    }
    if(ui->comboBox->currentIndex() != -1){
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(1);
    }
}

QString CourseDetails::getCourseGrade(){
    return grade;
}

bool CourseDetails::eventFilter(QObject *obj, QEvent *event) // to show list after click anywhere on the combobox
{
    // Check if the event is from either the combobox or its line edit
    if ((obj == ui->comboBox || obj == ui->comboBox->lineEdit()) &&
        event->type() == QEvent::MouseButtonPress) {

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // Show the dropdown when clicking anywhere on the combobox or line edit
            ui->comboBox->showPopup();
            return true; // Event handled - prevent default processing
        }
    }

    // Pass the event to the parent class for normal processing
    return QWidget::eventFilter(obj, event);
}

void CourseDetails::on_lineEdit_2_textChanged(const QString &arg1)
{
    grade = ui->lineEdit_2->text();
}

