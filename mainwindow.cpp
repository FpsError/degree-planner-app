#include "mainwindow.h"
#include "course.h"
#include "coursedetails.h"
#include "editcoursedetails.h"
#include "global_objects.h"
#include "newsemester.h"
#include "qmessagebox.h"
#include "qpushbutton.h"
#include "qsqlquery.h"
#include "qstyle.h"
#include "ui_mainwindow.h"

#include <QPixmap>
#include <QSqlError>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), scene(new QGraphicsScene(this)) {
    ui->setupUi(this);
    ui->tabWidget->setTabVisible(2, 0);
    ui->tabWidget->setTabVisible(3, 0);
    ui->frame_5->setVisible(0);
    QPixmap pix("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/"
                "trending.png");
    ui->icon_cgpa->setPixmap(pix);

    QPixmap pix1(
        "C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/book.png");
    ui->icon_credits->setPixmap(pix1);

    QPixmap pix2("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/"
                 "calendar.png");
    ui->icon_semesters->setPixmap(pix2);

    QPixmap pix3(
        "C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/target.png");
    ui->icon_track->setPixmap(pix3);

    ui->verticalLayout_12->setObjectName("semesetersLayout");
    if(profile_id != -1){
        updateGPA();
        updateCreditsEarned();
        updateSemsLeft();
        populateSemesters();
        populateGraphics();
    } else return;
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::refreshWindow(){
    updateGPA();
    updateCreditsEarned();
    updateSemsLeft();
    populateSemesters();
    populateGraphics();
}

AcademicStanding MainWindow::getStandingFromGPA(double gpa) {
    if (gpa < 0.0 || gpa > 4.0) {
        throw std::out_of_range("GPA must be between 0.0 and 4.0");
    }

    if (gpa < 2.0) return AcademicStanding::PROBATION;
    if (gpa < 3.5) return AcademicStanding::SATISFACTORY;
    if (gpa < 3.8) return AcademicStanding::GOOD;
    if (gpa < 4.0) return AcademicStanding::EXCELLENT;
    if (gpa == 4.0) return AcademicStanding::HIGHEST_HONORS;
    return AcademicStanding::INVALID;
}

void MainWindow::populateSemesters() {
    QSqlQuery query;

    int starting_year;
    query.exec("Select starting_year from profile");
    query.next();
    starting_year = query.value(0).toInt();

    QString starting_semester;
    query.exec("Select starting_semester from profile");
    query.next();
    starting_semester = query.value(0).toString();

    // Get last year
    int lastYear;
    query.exec("Select graduation_year from profile");
    query.next();
    lastYear = query.value(0).toInt();

    for (int year = starting_year; year <= lastYear; year++) {
        createYearFrame(year, starting_semester);
    }

    QPushButton *add_sem_button = new QPushButton("+ Add Semester", this);
    add_sem_button->setStyleSheet(R"(
    QPushButton {
        background-color: transparent;
        color: #007acc;
        border: 2px dashed #007acc;
        border-radius: 6px;
        padding: 12px 20px;
        font-weight: 500;
        font-size: 14px;
        margin: 10px;
    }

    QPushButton:hover {
        background-color: rgba(0, 122, 204, 0.1);
        border-style: solid;
        border-color: #1c97ea;
        color: #1c97ea;
    }

    QPushButton:pressed {
        background-color: rgba(0, 122, 204, 0.2);
        border-color: #005a9e;
        color: #005a9e;
    }
    )");
    connect(add_sem_button, &QPushButton::clicked, this,
            [=]() { onAddSemButtonClicked(); });

    ui->verticalLayout_12->addWidget(add_sem_button);
}

void MainWindow::createYearFrame(int year, QString starting_semester){
    QFrame *frame = new QFrame(this);
    frame->setObjectName("yearFrame");

    // ** Horizontal Layout for the main frame
    QHBoxLayout *frameLayout = new QHBoxLayout(frame);
    frameLayout->setObjectName("yearFrameLayout");

    bool hasSummer = hasSummerSemester(year);

    if(!hasSummer){
        //no summer session, don't print it
        if(starting_semester == "SPRING"){
            createSemesterFrame(frameLayout, year+1, "SPRING");
            addAddSummerButton(frameLayout, year);
        } else{
            createSemesterFrame(frameLayout, year, "FALL");
            createSemesterFrame(frameLayout, year+1, "SPRING");
            addAddSummerButton(frameLayout, year);
        }
    } else{
        if(starting_semester == "SPRING"){
            createSemesterFrame(frameLayout, year+1, "SPRING");
            createSemesterFrame(frameLayout, year+1, "SUMMER");
        } else if(starting_semester == "SUMMER"){
            createSemesterFrame(frameLayout, year+1, "SUMMER");
        } else{
            createSemesterFrame(frameLayout, year, "FALL");
            createSemesterFrame(frameLayout, year+1, "SPRING");
            createSemesterFrame(frameLayout, year+1, "SUMMER");
        }
    }

    ui->verticalLayout_12->addWidget(frame);
}

bool MainWindow::hasSummerSemester(int year){
    //check if year has a summer session
    QSqlQuery query;
    query.prepare("SELECT DISTINCT s.sem_term, s.sem_year "
                  "FROM course_planning cp INNER JOIN semester s "
                  "on cp.sem_code = s.sem_code "
                  "WHERE s.sem_term = :sem_term and sem_year = :year");
    query.bindValue(":sem_term", "SUMMER");
    query.bindValue(":year", year+1);
    query.exec();

    if(!query.next()) return false;
    else return true;
}

void MainWindow::addAddSummerButton(QHBoxLayout* layout, int year){
    QPushButton *addSummerButton = new QPushButton("+");
    addSummerButton->setObjectName("addSummerButton");
    addSummerButton->setToolTip("Add Summer Semester");
    addSummerButton->setFixedSize(30, 60);

    // Apply the stylesheet
    addSummerButton->setStyleSheet(R"(
    QPushButton {
        background-color: #2196F3;
        border: 2px solid #1976D2;
        border-radius: 10px;
        color: white;
        font-weight: bold;
        font-size: 20px;
    }
    QPushButton:hover {
        background-color: #1976D2;
        border: 2px solid #0D47A1;
    }
    QPushButton:pressed {
        background-color: #0D47A1;
    }
    )");

    layout->addWidget(addSummerButton);
    connect(addSummerButton, &QPushButton::clicked, this,[=]() {
        onAddSemButtonClicked(layout, year, addSummerButton); });
}

void MainWindow::createSemesterFrame(QHBoxLayout* yearFrame, int year, QString semester) {
    QFrame *frame_with_spacers = new QFrame();
    QVBoxLayout *SpacersFrameLayout = nullptr;
    frame_with_spacers->setFrameStyle(QFrame::NoFrame);
    // * Main semester frame
    QFrame *frame;
    if(semester == "SUMMER") {
        frame = new QFrame(frame_with_spacers);
        SpacersFrameLayout = new QVBoxLayout(frame_with_spacers);
    }
    else frame = new QFrame();
    frame->setFrameStyle(QFrame::StyledPanel);
    frame->setObjectName("semesterFrame");

    // ** Vertical Layout for the main frame
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frameLayout->setObjectName("semesterFrameLayout");

    // *** Frame for the semester title
    QFrame *titleFrame = new QFrame();

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

    // QObject *semesterParent = findParent(semesterStatus, "semesterFrame");

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
        "}"));

    connect(button, &QPushButton::clicked, this, [=]() {
        onAddCourseButtonClicked(frame, frameLayout,
                                 semester + " " + QString::number(year));
    });
    // *** Add semester frame name and button and horizontal spacer to the whole
    // semester title frame
    titleFrameLayout->addWidget(semNameFrame);
    titleFrameLayout->addStretch();
    titleFrameLayout->addWidget(button);

    // *** Frame for the no courses yet
    QFrame *noCoursesFrame = new QFrame();
    noCoursesFrame->setObjectName("noCoursesFrame");
    noCoursesFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    noCoursesFrame->setStyleSheet(
        "background-color: #363636; border-radius: 8px; border: 1px #4a4a4a;");

    // **** Add Vertical Layout for the elements inside the no courses frame
    QVBoxLayout *noCoursesFrameLayout = new QVBoxLayout(noCoursesFrame);

    // Elements
    QLabel *icon = new QLabel();
    QPixmap pix4("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/"
                 "closed_book.png");
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
        onFirstAddCourseButtonClicked(frame, frameLayout, noCoursesFrame,
                                      semester + QString::number(year));
    });

    // **** Add elements to layout
    noCoursesFrameLayout->addWidget(icon);
    noCoursesFrameLayout->addWidget(textLabel);
    noCoursesFrameLayout->addWidget(addFirstCourseButton);

    // ** Add title Frame to the Vertical Layout
    frameLayout->addWidget(titleFrame);
    frameLayout->addWidget(noCoursesFrame);

    if(semester == "SUMMER"){
        SpacersFrameLayout->addStretch();
        SpacersFrameLayout->addWidget(frame);
        SpacersFrameLayout->addStretch();
    }

    QSqlQuery query;
    // Add existing courses
    query.prepare(
        "Select course_code from course_planning "
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
    //ui->verticalLayout_12->addWidget(frame);
    if(semester == "SUMMER") yearFrame->addWidget(frame_with_spacers);
    else yearFrame->addWidget(frame);

    updateSemesterStatus(frame, semester + QString::number(year));
}

void MainWindow::createSemesterFrame(int year, QString semester) {
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

    // QObject *semesterParent = findParent(semesterStatus, "semesterFrame");

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
        "}"));

    connect(button, &QPushButton::clicked, this, [=]() {
        onAddCourseButtonClicked(frame, frameLayout,
                                 semester + " " + QString::number(year));
    });
    // *** Add semester frame name and button and horizontal spacer to the whole
    // semester title frame
    titleFrameLayout->addWidget(semNameFrame);
    titleFrameLayout->addStretch();
    titleFrameLayout->addWidget(button);

    // *** Frame for the no courses yet
    QFrame *noCoursesFrame = new QFrame();
    noCoursesFrame->setObjectName("noCoursesFrame");
    noCoursesFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    noCoursesFrame->setStyleSheet(
        "background-color: #363636; border-radius: 8px; border: 1px #4a4a4a;");

    // **** Add Vertical Layout for the elements inside the no courses frame
    QVBoxLayout *noCoursesFrameLayout = new QVBoxLayout(noCoursesFrame);

    // Elements
    QLabel *icon = new QLabel();
    QPixmap pix4("C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/"
                 "closed_book.png");
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
        onFirstAddCourseButtonClicked(frame, frameLayout, noCoursesFrame,
                                      semester + QString::number(year));
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
    query.prepare(
        "Select course_code from course_planning "
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

void MainWindow::onAddCourseButtonClicked(QFrame *source, QLayout *sourceLayout,
                                          QString semester) {
    CourseDetails w(semester, this);
    bool activated = w.exec();

    if (activated) {
        QString course_code = w.getCourseCode();
        addCoursesFromDatabase(source, sourceLayout, course_code);
        updateSemesterStatus(source, semester.remove(' '));
    }
}

void MainWindow::onFirstAddCourseButtonClicked(QFrame *frame,
                                               QLayout *sourceLayout,
                                               QFrame *noCoursesFrame,
                                               QString semester) {
    onAddCourseButtonClicked(frame, sourceLayout, semester);
    noCoursesFrame->hide();
}

void MainWindow::addCoursesFromDatabase(QFrame *source, QLayout *sourceLayout,
                                        QString course_code) {
    // Check if no courses frame is showen
    QFrame *noCoursesFrame = source->findChild<QFrame *>("noCoursesFrame");
    if (noCoursesFrame && !noCoursesFrame->isHidden()) {
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

    // Course labels structure
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
    QPixmap edit_button_icon(
        "C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/edit.png");
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
    QPixmap delete_button_icon(
        "C:/Users/FpsError/Documents/QtDesignStudio/degreePlan/icons/delete.png");
    delete_button->setIcon(delete_button_icon);
    delete_button->setHidden(1);

    connect(delete_button, &QPushButton::clicked, this,
            [=]() { onDeleteCourseButtonClicked(courseFrame); });

    // add a horizontal spacer first between the buttons and the label
    titleFrameLayout->addStretch();
    titleFrameLayout->addWidget(edit_button);
    titleFrameLayout->addWidget(delete_button);

    QString semester;
    if (edit_button->parent()->parent()->parent()) {
        semester = edit_button->parent()
                       ->parent()
                       ->parent()
                       ->findChild<QLabel *>("semesterTitle")
                       ->text();
    }

    QSqlQuery query;
    query.prepare("Select course_code, is_current_course, is_planned_course, "
                  "is_done_course from course_planning "
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

    auto [status, style_sheet] =
        getCourseStatus(c_is_complete, c_is_current, c_is_planned);
    course_labels.statusLabel->setText(status);
    course_labels.statusLabel->setStyleSheet(style_sheet);
    course_labels.statusLabel->setMinimumHeight(33);

    // Course desc
    QFrame *c_desc_frame = new QFrame();

    QVBoxLayout *descFrameLayout = new QVBoxLayout(c_desc_frame);
    course_labels.desc = new QLabel();
    descFrameLayout->addWidget(course_labels.desc);
    course_labels.desc->setAlignment(Qt::AlignCenter);
    query.prepare(
        "Select course_title from course where course_code = :course_code");
    query.bindValue(":course_code", course_code);
    query.exec();
    QString description;
    if (query.next()) {
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

    query.prepare(
        "Select course_credits from course where course_code = :course_code");
    query.bindValue(":course_code", course_code);
    query.exec();
    QString credits_number;
    if (query.next()) {
        credits_number = query.value(0).toString();
    }

    course_labels.credits = new QLabel();
    course_labels.credits->setText(credits_number + " credits");

    query.prepare(
        "Select course_level from course where course_code = :course_code");
    query.bindValue(":course_code", course_code);
    query.exec();
    QString course_level;
    if (query.next()) {
        course_level = query.value(0).toString();
    }

    course_labels.level = new QLabel();
    course_labels.level->setText(course_level);

    query.prepare(
        "Select grade from course_planning where course_code = :course_code");
    query.bindValue(":course_code", course_code);
    query.exec();
    QString course_grade;
    if (query.next()) {
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
        onEditCourseButtonClicked(courseFrame, course_labels, semester);
        updateSemesterStatus(source, rawSemester);
    });
}

std::tuple<QString, QString>
MainWindow::getCourseStatus(bool is_done, bool is_current, bool is_planned) {
    QString style_sheet;

    if (is_done) {
        style_sheet = "QLabel { background-color: #4CAF50; "
                      "color: white; border-radius: 8px; padding: 6px 12px; "
                      "font-weight: bold; font-size: 14px; "
                      "border: 1px solid #388E3C; "
                      "qproperty-alignment: AlignCenter; }";
        return {"Completed", style_sheet};
    } else if (is_current) {
        style_sheet = "QLabel { background-color: #2196F3; "
                      "color: white; border-radius: 8px; padding: 6px 12px; "
                      "font-weight: bold; font-size: 14px; "
                      "border: 1px solid #1976D2; "
                      "qproperty-alignment: AlignCenter; }";
        return {"Current", style_sheet};
    } else if (is_planned) {
        style_sheet = "QLabel { background-color: #9C27B0; "
                      "color: white; border-radius: 8px; padding: 6px 12px; "
                      "font-weight: bold; font-size: 14px; "
                      "border: 1px solid #7B1FA2; "
                      "qproperty-alignment: AlignCenter; }";
        return {"Planned", style_sheet};
    } else
        return {"Null", ""};
}

void MainWindow::onDeleteCourseButtonClicked(QFrame *parent) {
    QObject *semesterFrame = parent->parent();
    QFrame *noCoursesFrame = semesterFrame->findChild<QFrame *>("noCoursesFrame");

    if (noCoursesFrame &&
        semesterFrame->findChildren<QFrame *>("courseFrame").size() == 1) {
        noCoursesFrame->show();
    }
    if (parent) {
        QSqlQuery query;
        query.prepare(
            "Delete from course_planning where course_code = :course_code");
        query.bindValue(":course_code", parent->findChild<QLabel*>("titleLabel", Qt::FindChildrenRecursively)->text());
        query.exec();
        updateSemesterStatus(semesterFrame);
        parent->deleteLater();
    }
}

void MainWindow::onEditCourseButtonClicked(QFrame *courseFrame,
                                           course course_label,
                                           QString semester) {
    QString course_code = courseFrame->findChild<QLabel *>("titleLabel")->text();
    EditCourseDetails w(course_code, semester, this);
    bool activated = w.exec();

    if (activated) {
        course new_updated_course = w.getUpdatedCourse();
        course_label.updateText(new_updated_course);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    // Cast the watched object to QFrame to check if it's a courseFrame
    QFrame *courseFrame = qobject_cast<QFrame *>(watched);

    // Check if the watched object is a courseFrame
    if (courseFrame && courseFrame->objectName().contains("courseFrame")) {
        if (event->type() == QEvent::Enter) {
            // Find the edit and delete buttons within THIS specific courseFrame
            QPushButton *editButton =
                courseFrame->findChild<QPushButton *>("edit_button");
            QPushButton *deleteButton =
                courseFrame->findChild<QPushButton *>("delete_button");

            if (editButton) {
                editButton->show();
            }
            if (deleteButton) {
                deleteButton->show();
            }
            return true;
        } else if (event->type() == QEvent::Leave) {
            // Find the edit and delete buttons within THIS specific courseFrame
            QPushButton *editButton =
                courseFrame->findChild<QPushButton *>("edit_button");
            QPushButton *deleteButton =
                courseFrame->findChild<QPushButton *>("delete_button");

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

QObject *MainWindow::findParent(QObject *child, const QString &parent_name) {
    if (!child->parent())
        return nullptr;
    else if (child->parent()->objectName() == parent_name) {
        return child->parent();
    } else {
        return findParent(child->parent(), parent_name);
    }
}

void MainWindow::updateSemesterStatus(QObject *semesterFrame,
                                      QString semester) {
    QLabel *semesterStatus = semesterFrame->findChild<QLabel *>(
        "semesterStatus", Qt::FindChildrenRecursively);

    QSqlQuery query;
    query.prepare("SELECT sum(course_credits) "
                  "FROM course_planning INNER JOIN course on "
                  "course_planning.course_code = course.course_code "
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
                  "FROM course_planning INNER JOIN course on "
                  "course_planning.course_code = course.course_code "
                  "WHERE sem_code = :semester");
    query.bindValue(":semester", semester);
    query.exec();
    query.next();
    QString n_of_courses = query.value(0).toString();
    semesterStatus->setText("test");
    semesterStatus->setText(credits + " credits • " + n_of_courses + " courses");
}

void MainWindow::updateSemesterStatus(QObject *semesterFrame) {
    QLabel *semesterTitle = semesterFrame->findChild<QLabel *>(
        "semesterTitle", Qt::FindChildrenRecursively);

    updateSemesterStatus(semesterFrame, semesterTitle->text().remove(' '));
}

void MainWindow::onAddSemButtonClicked() {
    newSemester w(this);
    bool activated = w.exec();
    QVBoxLayout *layout = this->findChild<QVBoxLayout *>("semesetersLayout");
    int index_1;
    int index_2;

    if (activated) {
        int semesters = layout->count() - 1;
        QLabel* previous_sem = layout->itemAt(semesters - 1)->widget()->findChild<QLabel *>("semesterTitle");
        QString semester;
        if(previous_sem){
            semester = previous_sem->text();
        } else semester = layout->itemAt(semesters - 2)->widget()->findChild<QLabel *>("semesterTitle")->text();

        int last_sem_year = std::stoi(extractYear(semester.toStdString()));
        int semester_type = w.get_sem_type();
        std::string season = extractSeason(semester.toStdString());

        if (semester_type == 0) {
            if (season == "FALL") {
                // add SPRING and last_sem_year + 1
                createSemesterFrame(last_sem_year + 1, "SPRING");
                index_1 = layout->count() - 1;
                index_2 = layout->count() - 2;
                swapTwoItemsInLayout(layout, index_1, index_2);
            } else {
                // add FALL and last_sem_year
                createSemesterFrame(last_sem_year, "FALL");
                index_1 = layout->count() - 1;
                index_2 = layout->count() - 2;
                swapTwoItemsInLayout(layout, index_1, index_2);
            }
        } else {
            if (season == "SPRING") {
                // add SUMMER and last_sem_year
                createSemesterFrame(last_sem_year, "SUMMER");
                index_1 = layout->count() - 1;
                index_2 = layout->count() - 2;
                swapTwoItemsInLayout(layout, index_1, index_2);
            } else {
                // add SUMMER and last_sem_year + 1
                createSemesterFrame(last_sem_year + 1, "SUMMER");
                index_1 = layout->count() - 1;
                index_2 = layout->count() - 2;
                swapTwoItemsInLayout(layout, index_1, index_2);
            }
        }
    }
}

// Swap positions of two items at index1 and index2
void MainWindow::swapTwoItemsInLayout(QVBoxLayout *layout, int index_1,
                                      int index_2) {
    if (index_1 < 0 || index_2 < 0 || index_1 >= layout->count() ||
        index_2 >= layout->count()) {
        return;
    }

    QLayoutItem *item_1 = layout->takeAt(index_1);
    QLayoutItem *item_2 = layout->takeAt(index_2 - (index_2 > index_1 ? 1 : 0));

    layout->insertItem(index_2, item_1);
    layout->insertItem(index_1, item_2);
}

std::string MainWindow::extractYear(const std::string &semester) {
    // Assuming format: "SEASON YEAR"
    return semester.substr(semester.find(' ') + 1);
}

std::string MainWindow::extractSeason(const std::string &semester) {
    // Assuming format: "SEASON YEAR"
    return semester.substr(0, semester.find(' '));
}

void MainWindow::onAddSemButtonClicked(QHBoxLayout* layout, int year, QPushButton* button_src){
    int index = layout->indexOf(button_src);
    createSemesterFrame(layout , year+1, "SUMMER");
    button_src->setStyleSheet("QPushButton{background-color: transparent}");
    QLayoutItem *button = layout->takeAt(index);
    layout->removeItem(button);
}

void MainWindow::updateCreditsEarned(){
    QSqlQuery query;
    int credits_earned = getCreditsEarned();

    query.prepare("select credit_requirements from profile inner join major on profile.major = major.major_name "
                  "where id = :profile_id");
    query.bindValue(":profile_id", profile_id);
    query.exec();
    query.next();
    int credits_requirement = query.value(0).toInt();

    ui->credits_earned_label->setText(QString::number(credits_earned) + "/" + QString::number(credits_requirement));


    int credits_progress = (static_cast<float>(credits_earned)/static_cast<float>(credits_requirement))*100;
    ui->credit_precentage_label->setText(QString::number(credits_progress) + "% complete");
}

int MainWindow::getCreditsEarned(){
    QSqlQuery query;
    query.exec("select sum(course_credits) "
               "from course_planning cp inner JOIN course c on cp.course_code = c.course_code "
               "where is_done_course = 1");
    if(query.next()){
        return query.value(0).toInt();
    } else return 0;
}

void MainWindow::populateGraphics(){
    scene->addText("Hello, world!");
    scene->addEllipse(1, 1, 1, 1);

    ui->graphicsView->setScene(scene);
}

void MainWindow::updateSemsLeft() {
    // Get all profile data in one query
    QSqlQuery query;
    query.prepare(
        "SELECT starting_semester, starting_year, graduation_year "
        "FROM profile WHERE id = :profile_id"
        );
    query.bindValue(":profile_id", profile_id);
    query.exec();
    if (!query.next()) {
        qDebug() << "Failed to get profile data:" << query.lastError().text();
        return;
    }

    QString startingSemester = query.value("starting_semester").toString();
    int startingYear = query.value("starting_year").toInt();
    int endingYear = query.value("graduation_year").toInt();
    QString endingSemester = "SPRING";  // Assuming graduation is always spring

    ui->label_12->setText("Expected graduation: Spring " + QString::number(endingYear));

    QString expectedSemsCountQuery = "SELECT count(sem_code) FROM semester "
                                     "WHERE sem_code NOT LIKE '%SUMMER%' "
                                     "AND ( (sem_year > :starting_year AND sem_year < :ending_year) "
                                     "OR (sem_year = :starting_year AND CASE :starting_semester "
                                     "WHEN 'FALL' THEN sem_term IN ('FALL') "
                                     "WHEN 'SPRING' THEN sem_term IN ('SPRING', 'FALL') END) "
                                     "OR (sem_year = :ending_year AND CASE :ending_semester "
                                     "WHEN 'FALL' THEN sem_term IN ('SPRING', 'FALL') "
                                     "WHEN 'SPRING' THEN sem_term IN ('SPRING') END) ) "
                                     "ORDER BY sem_year, "
                                     "CASE sem_term WHEN 'SPRING' THEN 1 WHEN 'FALL' THEN 2 END";

    query.prepare(expectedSemsCountQuery);

    query.bindValue(":starting_year", startingYear);
    query.bindValue(":ending_year", endingYear);
    query.bindValue(":starting_semester", startingSemester);
    query.bindValue(":ending_semester", endingSemester);

    query.exec();
    query.next();
    int expectedSemesterCount = query.value(0).toInt();

    query.prepare("SELECT COUNT(*) FROM (SELECT sem_code FROM course_planning WHERE sem_code NOT LIKE '%SUMMER%' GROUP BY sem_code HAVING SUM(is_done_course) != 0)");
    query.exec();
    query.next();
    int completedSemestersCount = query.value(0).toInt();

    int num_of_sems_left = expectedSemesterCount - completedSemestersCount;
    ui->num_sems_left->setText(QString::number(num_of_sems_left));
}

void MainWindow::updateGPA() {
    int credits_earned = getCreditsEarned();

    QSqlQuery query;
    query.prepare("SELECT grade, c.course_credits "
                  "FROM course_planning CP INNER JOIN course C on CP.course_code = c.course_code "
                  "WHERE is_done_course = 1");
    query.exec();
    float sum = 0;
    while(query.next()){
        float points = PointsFromGradeLetters(query.value(0).toString());
        sum += points*query.value(1).toFloat();
    }

    float gpa = sum/static_cast<float>(credits_earned);

    ui->label_6->setText(QString::number(gpa));

    AcademicStanding standing = getStandingFromGPA(gpa);

    switch(standing) {
    case AcademicStanding::PROBATION:
        ui->label_5->setText("Probation");
        break;
    case AcademicStanding::SATISFACTORY:
        ui->label_5->setText("Satisfactory");
        break;
    case AcademicStanding::GOOD:
        ui->label_5->setText("Good");
        break;
    case AcademicStanding::EXCELLENT:
        ui->label_5->setText("Excellent");
        break;
    case AcademicStanding::HIGHEST_HONORS:
        ui->label_5->setText("Highest Honors");
        break;
    case AcademicStanding::INVALID:
        ui->label_5->setText("More info needed to find GPA");
        break;
    }

}

float MainWindow::PointsFromGradeLetters(QString grade) {
    if (grade == "A+") return 4.3;
    if (grade == "A") return 4;
    if (grade == "A-") return 3.7;
    if (grade == "B+") return 3.3;
    if (grade == "B") return 3;
    if (grade == "B-") return 2.7;
    if (grade == "C+") return 2.3;
    if (grade == "C") return 2;
    if (grade == "C-") return 1.7;
    if (grade == "D+") return 1.3;
    if (grade == "D") return 1;
    if (grade == "D-") return 0.7;
    if (grade == "F") return 0;
    return 0;
}

void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    if(index==0) refreshWindow();
}
