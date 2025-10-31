#include "editcoursedetails.h"
#include "qabstractitemview.h"
#include "qcompleter.h"
#include "qmessagebox.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "ui_editcoursedetails.h"

EditCourseDetails::EditCourseDetails(QString course_code ,QString semester, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditCourseDetails)
{
    ui->setupUi(this);

    this->semester = semester;
    this->old_course_code = course_code;

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

    populateCoursesCombobox();
    int index = ui->comboBox->findText(course_code);
    ui->comboBox->setCurrentIndex(index);
}

EditCourseDetails::~EditCourseDetails()
{
    delete ui;
}

void EditCourseDetails::populateCoursesCombobox(){
    QSqlQuery query;
    query.exec("Select course_code from course");
    while(query.next()){
        QString course = query.value(0).toString();
        ui->comboBox->addItem(course);
    }
}

void EditCourseDetails::on_comboBox_currentIndexChanged(int index)
{
    QString course = ui->comboBox->itemText(index);
    this->new_course_code = course;

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
}

void EditCourseDetails::on_buttonBox_accepted()
{
    QSqlQuery query;
    query.prepare("Update course_planning set course_code = ?, is_current_course = ?,"
                  "is_done_course = ?, is_planned_course = ?, grade = ? "
                  "where course_code = ?");
    query.addBindValue(new_course_code);
    query.addBindValue(is_current_course);
    query.addBindValue(is_done_course);
    query.addBindValue(is_planned_course);
    query.addBindValue(grade);
    query.addBindValue(old_course_code);
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


void EditCourseDetails::on_checkBox_3_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        is_done_course = 1;
    } else{
        is_done_course = 0;
    }
}


void EditCourseDetails::on_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        is_current_course = 1;
    } else{
        is_current_course = 0;
    }
}


void EditCourseDetails::on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        is_planned_course = 1;
    } else{
        is_planned_course = 0;
    }
}

course EditCourseDetails::getUpdatedCourse(){
    course new_course;
    new_course.titleLabel = new QLabel();
    new_course.titleLabel->setText(new_course_code);

    new_course.statusLabel = new QLabel();
    auto [status, style_sheet] = getCourseStatus(is_done_course, is_current_course, is_planned_course);
    new_course.statusLabel->setText(status);
    new_course.statusLabel->setStyleSheet(style_sheet);
    new_course.statusLabel->setMinimumHeight(33);

    QSqlQuery query;

    query.prepare("Select course_title from course where course_code = :course_code");
    query.bindValue(":course_code", new_course_code);
    query.exec();
    QString description;
    if(query.next()){
        description = query.value(0).toString();
    }
    new_course.desc = new QLabel();
    new_course.desc->setText(description);
    new_course.desc->setAlignment(Qt::AlignLeft);

    query.prepare("Select course_credits from course where course_code = :course_code");
    query.bindValue(":course_code", new_course_code);
    query.exec();
    QString credits_number;
    if(query.next()){
        credits_number = query.value(0).toString();
    }
    new_course.credits = new QLabel();
    new_course.credits->setText(credits_number + " credits");

    query.prepare("Select course_level from course where course_code = :course_code");
    query.bindValue(":course_code", new_course_code);
    query.exec();
    QString course_level;
    if(query.next()){
        course_level = query.value(0).toString();
    }
    new_course.level = new QLabel();
    new_course.level->setText(course_level);

    new_course.grade = new QLabel();
    new_course.grade->setText("Grade: " + QString::number(grade));

    return new_course;
}

std::tuple<QString, QString> EditCourseDetails::getCourseStatus(bool is_done, bool is_current, bool is_planned){
    QString style_sheet;

    if (is_done){
        style_sheet = "QLabel { background-color: #4CAF50; "
                      "color: white; border-radius: 8px; padding: 6px 12px; "
                      "font-weight: bold; font-size: 14px; "
                      "border: 1px solid #388E3C; "
                      "qproperty-alignment: AlignCenter; }";
        return {"Completed", style_sheet};
    } else if (is_current){
        style_sheet = "QLabel { background-color: #2196F3; "
                      "color: white; border-radius: 8px; padding: 6px 12px; "
                      "font-weight: bold; font-size: 14px; "
                      "border: 1px solid #1976D2; "
                      "qproperty-alignment: AlignCenter; }";
        return {"Current", style_sheet};
    } else if(is_planned){
        style_sheet = "QLabel { background-color: #9C27B0; "
                      "color: white; border-radius: 8px; padding: 6px 12px; "
                      "font-weight: bold; font-size: 14px; "
                      "border: 1px solid #7B1FA2; "
                      "qproperty-alignment: AlignCenter; }";
        return {"Planned", style_sheet};
    }
    else return {"Null", ""};
}

void EditCourseDetails::checkCurrentCourse(QString course_code)
{
    qDebug() << course_code;
}
