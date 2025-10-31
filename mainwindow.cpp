#include "mainwindow.h"
#include "course.h"
#include "coursedetails.h"
#include "editcoursedetails.h"
#include "global_objects.h"
#include "qdatetime.h"
#include "qsqlquery.h"
#include "qstyle.h"
#include "ui_mainwindow.h"
#include "qpushbutton.h"

#include <QPixmap>
#include <QSqlError>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QPixmap pix("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/trending.png");
    ui->icon_cgpa->setPixmap(pix);

    QPixmap pix1("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/book.png");
    ui->icon_credits->setPixmap(pix1);

    QPixmap pix2("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/calendar.png");
    ui->icon_semesters->setPixmap(pix2);

    QPixmap pix3("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/target.png");
    ui->icon_track->setPixmap(pix3);

    populateSemesters();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::populateSemesters(){
    QSqlQuery query;

    int starting_year;
    query.exec("Select starting_year from profile");
    query.next();
    starting_year = query.value(0).toInt();

    QString starting_semester;
    query.exec("Select starting_semester from profile");
    query.next();
    starting_semester = query.value(0).toString();

    // Get current year
    int currentYear = QDate::currentDate().year();

    query.prepare("Select sem_term, sem_year from semester "
               "where sem_year >= :starting_year and sem_term != \"SUMMER\" order by sem_year");
    query.bindValue(":starting_year", starting_year);
    query.exec();

    for(int year = starting_year; year <= currentYear ;year++){
        query.next();
        int sem_year = query.value("sem_year").toInt();
        QString sem_term = query.value("sem_term").toString();

        if (sem_year < year){
            year-=1;
            createSemesterFrame(year, sem_term);
        }
        else createSemesterFrame(year, sem_term);
    }

    query.exec("Select sem_code from course_enrolled");

    while(query.next()){
        QString sem = query.value(0).toString();
        qDebug() << "test: " + sem;
    }
}

void MainWindow::createSemesterFrame(int year, QString semester){
    // * Main semester frame
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::StyledPanel);
    frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    frame->setObjectName("semesterFrame");

    // ** Vertical Layout for the main frame
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frameLayout->setObjectName("semesterFrameLayout");

    // *** Frame for the semester title
    QFrame *titleFrame = new QFrame();
    titleFrame->setMinimumHeight(80);
    titleFrame->setMinimumWidth(1122);
    titleFrame->setMaximumHeight(80);
    titleFrame->setMaximumHeight(1122);

    // Horizontal Layout for the semester title frame
    QHBoxLayout *titleFrameLayout = new QHBoxLayout(titleFrame);

    // **** Frame for the semester name and courses status
    QFrame *semNameFrame = new QFrame();
    QVBoxLayout *semNameFrameLayout = new QVBoxLayout(semNameFrame);
    QLabel *semesterTitle = new QLabel();
    semesterTitle->setObjectName("semesterTitle");
    semesterTitle->setMinimumHeight(30);
    semesterTitle->setText(semester + " " + QString::number(year));
    semesterTitle->setFont(QFont("Segoe UI", 16, QFont::Bold));

    QLabel *semesterStatus = new QLabel();
    semesterStatus->setObjectName("semesterStatus");

    //QObject *semesterParent = findParent(semesterStatus, "semesterFrame");


    semNameFrameLayout->addWidget(semesterTitle);
    semNameFrameLayout->addWidget(semesterStatus);

    // **** Button to add courses
    QPushButton *button = new QPushButton("+ Add Course");
    button->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "    background-color: #2E2E2E;       /* Deep charcoal base */"
        "    color: #FFFFFF;                  /* Crisp white text */"
        "    border: 1px solid #555555;       /* Subtle border */"
        "    border-radius: 6px;              /* Smooth rounded corners */"
        "    padding: 6px 12px;               /* Comfortable spacing */"
        "    font-size: 14px;                 /* Clean, readable font */"
        "}"
        ""
        "QPushButton:hover {"
        "    background-color: #3E3E3E;       /* Slightly lighter on hover */"
        "    border: 1px solid #777777;       /* Highlight border */"
        "}"
        ""
        "QPushButton:pressed {"
        "    background-color: #1E1E1E;       /* Darker when pressed */"
        "    border: 1px solid #999999;       /* Stronger border */"
        "}"
        ""
        "QPushButton:disabled {"
        "    background-color: #444444;       /* Muted tone */"
        "    color: #AAAAAA;                  /* Dimmed text */"
        "    border: 1px solid #555555;"
        "}"
        ));

    connect(button, &QPushButton::clicked, this, [=]() {
        onAddCourseButtonClicked(frame, frameLayout, semester + " " + QString::number(year));
    });
    // *** Add semester frame name and button and horizontal spacer to the whole semester title frame
    titleFrameLayout->addWidget(semNameFrame);
    titleFrameLayout->addStretch();
    titleFrameLayout->addWidget(button);

    // *** Frame for the no courses yet
    QFrame *noCoursesFrame = new QFrame();
    noCoursesFrame->setObjectName("noCoursesFrame");
    noCoursesFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    noCoursesFrame->setStyleSheet("background-color: #363636; border-radius: 8px; border: 1px #4a4a4a;");

    // **** Add Vertical Layout for the elements inside the no courses frame
    QVBoxLayout *noCoursesFrameLayout = new QVBoxLayout(noCoursesFrame);

    // Elements
    QLabel *icon = new QLabel();
    QPixmap pix4("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/closed_book.png");
    icon->setPixmap(pix4);
    icon->setAlignment(Qt::AlignCenter);
    QLabel *textLabel = new QLabel();
    textLabel->setText("No courses planned for this semester");
    textLabel->setFont(QFont("Segoe UI", 13));
    textLabel->setAlignment(Qt::AlignCenter);
    QPushButton *addFirstCourseButton = new QPushButton();
    addFirstCourseButton->setText("Add your first course");
    addFirstCourseButton->setStyleSheet(R"(
    QPushButton {
        background-color: transparent;
        border: none;
        color: #4fc3f7;           /* Light blue - good visibility on dark background */
        padding: 2px 4px;
        text-decoration: underline;
    }

    QPushButton:hover {
        color: #29b6f6;           /* Slightly brighter blue on hover */
        text-decoration: none;
    }

    QPushButton:pressed {
        color: #81d4fa;           /* Even lighter blue when pressed */
    })");
    addFirstCourseButton->setFont(QFont("Segoe UI", 13));

    connect(addFirstCourseButton, &QPushButton::clicked, this, [=]() {
        onFirstAddCourseButtonClicked(frame, frameLayout, noCoursesFrame, semester + QString::number(year));
    });

    // **** Add elements to layout
    noCoursesFrameLayout->addWidget(icon);
    noCoursesFrameLayout->addWidget(textLabel);
    noCoursesFrameLayout->addWidget(addFirstCourseButton);

    // ** Add title Frame to the Vertical Layout
    frameLayout->addWidget(titleFrame);
    frameLayout->addWidget(noCoursesFrame);

    QSqlQuery query;
    // Add existing courses
    query.prepare("Select course_code from course_planning "
                  "inner join semester on semester.sem_code = course_planning.sem_code "
                  "where sem_year = :year and sem_term = :semester");
    query.bindValue(":year", year);
    query.bindValue(":semester", semester.toUpper());
    query.exec();
    while (query.next()) {
        QString course_code = query.value(0).toString();
        addCoursesFromDatabase(frame, frameLayout, course_code);
    }

    // * Add the semester frame to the vertical layout of scrollable area
    ui->verticalLayout_12->addWidget(frame);
    updateSemesterStatus(frame, semester + QString::number(year));
}

void MainWindow::onAddCourseButtonClicked(QFrame *source, QLayout *sourceLayout, QString semester){
    CourseDetails w(semester,this);
    bool activated = w.exec();

    if (activated) {
        QString course_code = w.getCourseCode();
        addCoursesFromDatabase(source, sourceLayout, course_code);
        updateSemesterStatus(source, semester.remove(' '));
    }
}

void MainWindow::onFirstAddCourseButtonClicked(QFrame *frame, QLayout *sourceLayout, QFrame *noCoursesFrame, QString semester){
    onAddCourseButtonClicked(frame, sourceLayout, semester);
    noCoursesFrame->hide();
}

void MainWindow::addCoursesFromDatabase(QFrame *source, QLayout *sourceLayout, QString course_code){
    // Check if no courses frame is showen
    QFrame *noCoursesFrame = source->findChild<QFrame*>("noCoursesFrame");
    if (noCoursesFrame && !noCoursesFrame->isHidden()){
        noCoursesFrame->hide();
    }

    // Frame for each course
    QFrame *courseFrame = new QFrame(source);
    courseFrame->setObjectName("courseFrame");

    courseFrame->setFrameStyle(QFrame::StyledPanel);
    courseFrame->setMinimumHeight(143);

    // Create a layout for the frame
    QVBoxLayout *crouseFrameLayout = new QVBoxLayout(courseFrame);
    crouseFrameLayout->setObjectName("crouseFrameLayout");

    // Frame for title card
    QFrame *titleFrame = new QFrame();
    titleFrame->setMinimumHeight(48);
    titleFrame->setObjectName("titleFrame");

    QHBoxLayout *titleFrameLayout = new QHBoxLayout(titleFrame);

    //Course labels structure
    course course_labels;

    // Course Title
    course_labels.titleLabel = new QLabel(titleFrame);
    course_labels.titleLabel->setObjectName("titleLabel");

    titleFrameLayout->addWidget(course_labels.titleLabel);
    course_labels.titleLabel->setText(course_code);
    course_labels.titleLabel->setFont(QFont("Segoe UI", 19, QFont::Bold));

    crouseFrameLayout->addWidget(titleFrame);

    // Status for each semester
    course_labels.statusLabel = new QLabel();

    titleFrameLayout->addWidget(course_labels.statusLabel);

    // The edit button
    QPushButton *edit_button = new QPushButton;
    edit_button->setObjectName("edit_button");
    edit_button->setStyleSheet(R"(
    QPushButton {
        background: transparent;
        border: none;
        color: black;
    }
    QPushButton:hover {
        background-color: rgba(0, 0, 0, 30);
        border: 1px solid rgba(0, 0, 0, 50);
        border-radius: 4px;
    }
    QPushButton:pressed {
        background-color: rgba(0, 0, 0, 50);
    }
    )");
    QPixmap edit_button_icon("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/edit.png");
    edit_button->setIcon(edit_button_icon);
    edit_button->setHidden(1);

    // The delete button
    QPushButton *delete_button = new QPushButton;
    delete_button->setObjectName("delete_button");
    delete_button->setStyleSheet(R"(
    QPushButton {
        background: transparent;
        border: none;
        color: black;
    }
    QPushButton:hover {
        background-color: rgba(0, 0, 0, 30);
        border: 1px solid rgba(0, 0, 0, 50);
        border-radius: 4px;
    }
    QPushButton:pressed {
        background-color: rgba(0, 0, 0, 50);
    }
    )");
    QPixmap delete_button_icon("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/delete.png");
    delete_button->setIcon(delete_button_icon);
    delete_button->setHidden(1);

    connect(delete_button, &QPushButton::clicked, this, [=]() {
        onDeleteCourseButtonClicked(courseFrame, course_code);
    });

    // add a horizontal spacer first between the buttons and the label
    titleFrameLayout->addStretch();
    titleFrameLayout->addWidget(edit_button);
    titleFrameLayout->addWidget(delete_button);

    QString semester;
    if (edit_button->parent()->parent()->parent()){
        semester = edit_button->parent()->parent()->parent()->findChild<QLabel*>("semesterTitle")->text();
    }

    QSqlQuery query;
    query.prepare("Select course_code, is_current_course, is_planned_course, is_done_course from course_planning "
                  "where course_code = :course_code");
    query.bindValue(":course_code", course_code);
    query.exec();
    int c_is_complete = 0;
    int c_is_planned = 0;
    int c_is_current = 0;

    if (query.next()) {
        c_is_current = query.value("is_current_course").toInt();
        c_is_planned = query.value("is_planned_course").toInt();
        c_is_complete = query.value("is_done_course").toInt();
    } else {
        // Handle case where no results found
        qDebug() << "No course found with code:" << course_code;
    }

    auto [status, style_sheet] = getCourseStatus(c_is_complete, c_is_current, c_is_planned);
    course_labels.statusLabel->setText(status);
    course_labels.statusLabel->setStyleSheet(style_sheet);
    course_labels.statusLabel->setMinimumHeight(33);

    // Course desc
    QFrame *c_desc_frame = new QFrame();

    QVBoxLayout *descFrameLayout = new QVBoxLayout(c_desc_frame);
    course_labels.desc = new QLabel();
    descFrameLayout->addWidget(course_labels.desc);
    course_labels.desc->setAlignment(Qt::AlignCenter);
    query.prepare("Select course_title from course where course_code = :course_code");
    query.bindValue(":course_code", course_code);
    query.exec();
    QString description;
    if(query.next()){
        description = query.value(0).toString();
    }
    course_labels.desc->setText(description);
    course_labels.desc->setAlignment(Qt::AlignLeft);

    crouseFrameLayout->addWidget(c_desc_frame);

    // Frame for course status
    QFrame *statusFrame = new QFrame();
    statusFrame->setMaximumHeight(44);
    statusFrame->setMaximumWidth(214);

    // Create a layout for the frame
    QHBoxLayout *statusframeLayout = new QHBoxLayout(statusFrame);

    query.prepare("Select course_credits from course where course_code = :course_code");
    query.bindValue(":course_code", course_code);
    query.exec();
    QString credits_number;
    if(query.next()){
        credits_number = query.value(0).toString();
    }

    course_labels.credits = new QLabel();
    course_labels.credits->setText(credits_number + " credits");

    query.prepare("Select course_level from course where course_code = :course_code");
    query.bindValue(":course_code", course_code);
    query.exec();
    QString course_level;
    if(query.next()){
        course_level = query.value(0).toString();
    }

    course_labels.level = new QLabel();
    course_labels.level->setText(course_level);

    query.prepare("Select grade from course_planning where course_code = :course_code");
    query.bindValue(":course_code", course_code);
    query.exec();
    QString course_grade;
    if(query.next()){
        course_grade = query.value(0).toString();
    }

    course_labels.grade = new QLabel();
    course_labels.grade->setText("Grade: " + course_grade);

    statusframeLayout->addWidget(course_labels.credits);
    statusframeLayout->addWidget(course_labels.level);
    statusframeLayout->addWidget(course_labels.grade);

    crouseFrameLayout->addWidget(statusFrame);

    courseFrame->installEventFilter(this);

    sourceLayout->addWidget(courseFrame);
    QString rawSemester = semester.remove(' ');
    connect(edit_button, &QPushButton::clicked, this, [=]() {
        onEditCourseButtonClicked(courseFrame,course_labels ,semester);
        updateSemesterStatus(source, rawSemester);
    });
}

std::tuple<QString, QString> MainWindow::getCourseStatus(bool is_done, bool is_current, bool is_planned){
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

void MainWindow::onDeleteCourseButtonClicked(QFrame *parent, QString course_code){
    QObject *semesterFrame = parent->parent();
    QFrame *noCoursesFrame = semesterFrame->findChild<QFrame*>("noCoursesFrame");

    if (noCoursesFrame && semesterFrame->findChildren<QFrame*>("courseFrame").size() == 1){
        noCoursesFrame->show();
    }
    if (parent){
        QSqlQuery query;
        query.prepare("Delete from course_planning where course_code = :course_code");
        query.bindValue(":course_code", course_code);
        query.exec();
        updateSemesterStatus(semesterFrame);
        parent->deleteLater();
    }
}

void MainWindow::onEditCourseButtonClicked(QFrame* courseFrame, course course_label, QString semester){
    QString course_code = courseFrame->findChild<QLabel*>("titleLabel")->text();
    EditCourseDetails w(course_code ,semester, this);
    bool activated = w.exec();

    if (activated) {
        course new_updated_course = w.getUpdatedCourse();
        course_label.updateText(new_updated_course);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // Cast the watched object to QFrame to check if it's a courseFrame
    QFrame *courseFrame = qobject_cast<QFrame*>(watched);

    // Check if the watched object is a courseFrame
    if (courseFrame && courseFrame->objectName().contains("courseFrame")) {
        if (event->type() == QEvent::Enter) {
            // Find the edit and delete buttons within THIS specific courseFrame
            QPushButton *editButton = courseFrame->findChild<QPushButton*>("edit_button");
            QPushButton *deleteButton = courseFrame->findChild<QPushButton*>("delete_button");

            if (editButton) {
                editButton->show();
            }
            if (deleteButton) {
                deleteButton->show();
            }
            return true;
        }
        else if (event->type() == QEvent::Leave) {
            // Find the edit and delete buttons within THIS specific courseFrame
            QPushButton *editButton = courseFrame->findChild<QPushButton*>("edit_button");
            QPushButton *deleteButton = courseFrame->findChild<QPushButton*>("delete_button");

            if (editButton) {
                editButton->hide();
            }
            if (deleteButton) {
                deleteButton->hide();
            }
            return true;
        }
    }

    // Call the base class implementation for other events
    return QMainWindow::eventFilter(watched, event);
}

QObject* MainWindow::findParent(QObject* child, const QString& parent_name){
    if(!child->parent())
        return nullptr;
    else if(child->parent()->objectName() == parent_name){
        return child->parent();
    } else{
        return findParent(child->parent(), parent_name);
    }
}

void MainWindow::updateSemesterStatus(QObject *semesterFrame, QString semester){
    QLabel* semesterStatus = semesterFrame->findChild<QLabel*>("semesterStatus", Qt::FindChildrenRecursively);

    QSqlQuery query;
    query.prepare("SELECT sum(course_credits) "
                  "FROM course_planning INNER JOIN course on course_planning.course_code = course.course_code "
                  "WHERE sem_code = :semester");
    query.bindValue(":semester", semester);
    query.exec();
    QString credits = "0"; // Default to 0
    if (query.next()) {
        QVariant result = query.value(0);
        if (!result.isNull()) {
            credits = result.toString();
        }
    }

    query.prepare("SELECT count(course_planning.course_code) "
                  "FROM course_planning INNER JOIN course on course_planning.course_code = course.course_code "
                  "WHERE sem_code = :semester");
    query.bindValue(":semester", semester);
    query.exec();
    query.next();
    QString n_of_courses = query.value(0).toString();
    semesterStatus->setText("test");
    semesterStatus->setText(credits + " credits • " + n_of_courses +" courses");
}

void MainWindow::updateSemesterStatus(QObject *semesterFrame){
    QLabel* semesterTitle = semesterFrame->findChild<QLabel*>("semesterTitle", Qt::FindChildrenRecursively);

    updateSemesterStatus(semesterFrame, semesterTitle->text().remove(' '));
}
