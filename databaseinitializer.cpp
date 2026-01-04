#include "DatabaseInitializer.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QTextStream>

bool DatabaseInitializer::initializeDatabase(const QString& dbPath) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Cannot open database:" << db.lastError().text();
        return false;
    }

    // Check if database is already initialized
    QSqlQuery checkQuery(db);
    checkQuery.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='course'");

    if (checkQuery.next()) {
        qDebug() << "Database already initialized";
        return true;
    }

    qDebug() << "Initializing new database...";

    // Create all tables
    if (!createTables(db)) {
        qDebug() << "Failed to create tables";
        return false;
    }

    // Insert all data
    if (!insertAllData(db)) {
        qDebug() << "Warning: Some data insertion failed, but continuing...";
    }

    qDebug() << "Database initialization complete!";
    return true;
}

bool DatabaseInitializer::createTables(QSqlDatabase& db) {
    QSqlQuery query(db);

    // Enable foreign keys
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qDebug() << "Failed to enable foreign keys:" << query.lastError().text();
        return false;
    }

    // Create tables one by one
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS school ("
            "school_code text not null primary key, "
            "school_name text not null)"
            )) {
        qDebug() << "Failed to create school table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS department ("
            "dep_code text not null primary key, "
            "dep_title text not null, "
            "school_code text not null references school)"
            )) {
        qDebug() << "Failed to create department table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS course ("
            "course_code varchar not null primary key, "
            "course_title text not null, "
            "course_description text, "
            "course_credits integer not null, "
            "course_classification varchar, "
            "dep_code varchar not null references department, "
            "course_type varchar, "
            "course_level text)"
            )) {
        qDebug() << "Failed to create course table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS semester ("
            "sem_code text not null primary key, "
            "sem_year integer not null, "
            "sem_term text not null)"
            )) {
        qDebug() << "Failed to create semester table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS profile ("
            "id integer primary key autoincrement, "
            "fname text not null, "
            "lname text not null, "
            "graduation_year integer not null, "
            "starting_year integer not null, "
            "starting_semester integer not null, "
            "major text not null, "
            "minor text)"
            )) {
        qDebug() << "Failed to create profile table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS course_planning ("
            "course_id integer primary key autoincrement, "
            "course_code text not null unique references course, "
            "sem_code text not null references semester, "
            "is_current_course integer not null, "
            "is_planned_course integer not null, "
            "is_done_course integer not null, "
            "grade text, "
            "profile_id integer not null references profile)"
            )) {
        qDebug() << "Failed to create course_planning table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS course_prereq ("
            "course_code text not null references course, "
            "prereq text not null references course, "
            "prereq_is_corequisite integer default 0 not null, "
            "primary key (course_code, prereq))"
            )) {
        qDebug() << "Failed to create course_prereq table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS major ("
            "major_id integer primary key autoincrement, "
            "major_code text not null, "
            "major_name text not null, "
            "department_id integer not null references department, "
            "credit_requirements integer not null)"
            )) {
        qDebug() << "Failed to create major table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseInitializer::insertAllData(QSqlDatabase& db) {
    bool success = true;

    if (!insertSchools(db)) success = false;
    if (!insertDepartments(db)) success = false;
    if (!insertCourses(db)) success = false;
    if (!insertSemesters(db)) success = false;
    if (!insertMajors(db)) success = false;
    if (!insertPrerequisites(db)) success = false;
    if (!createTrigger(db)) success = false;

    return success;
}

bool DatabaseInitializer::insertSchools(QSqlDatabase& db) {
    QSqlQuery query(db);
    query.prepare("INSERT OR IGNORE INTO school (school_code, school_name) VALUES (?, ?)");

    // Use QList instead of QVector for better compatibility
    QList<QPair<QString, QString> > schools;
    schools.append(qMakePair(QString("SHSS"), QString("School of Humanities and Social Sciences")));
    schools.append(qMakePair(QString("SBA"), QString("School of Business Administration")));
    schools.append(qMakePair(QString("SSE"), QString("School of Science and Engineering")));

    for (int i = 0; i < schools.size(); ++i) {
        const QPair<QString, QString>& school = schools.at(i);
        query.addBindValue(school.first);
        query.addBindValue(school.second);
        if (!query.exec()) {
            qDebug() << "Failed to insert school" << school.first << ":" << query.lastError().text();
            return false;
        }
        query.finish();
    }

    return true;
}

bool DatabaseInitializer::insertDepartments(QSqlDatabase& db) {
    QSqlQuery query(db);
    query.prepare("INSERT OR IGNORE INTO department (dep_code, dep_title, school_code) VALUES (?, ?, ?)");

    // Use QList of QStringList instead of QVector<QVector<QString>>
    QList<QStringList> departments;
    departments << (QStringList() << "CSC" << "Computer Science" << "SSE");
    departments << (QStringList() << "ACC" << "Accounting" << "SBA");
    departments << (QStringList() << "MTH" << "Mathematics" << "SSE");
    departments << (QStringList() << "FYE" << "First Year Experience" << "SHSS");
    departments << (QStringList() << "FAS" << "Foundations of Academic Success" << "SHSS");
    departments << (QStringList() << "ARB" << "Arabic" << "SHSS");
    departments << (QStringList() << "ENG" << "English" << "SHSS");
    departments << (QStringList() << "Basic Sciences" << "Basic Sciences" << "SSE");
    departments << (QStringList() << "EGR" << "General Engineering" << "SSE");
    departments << (QStringList() << "COM" << "Communication" << "SHSS");
    departments << (QStringList() << "HUM" << "Humanities" << "SHSS");
    departments << (QStringList() << "LIT" << "Literature" << "SHSS");
    departments << (QStringList() << "PHI" << "Philosophy" << "SHSS");
    departments << (QStringList() << "ART" << "Art" << "SHSS");
    departments << (QStringList() << "HIS" << "History" << "SHSS");
    departments << (QStringList() << "PSC" << "Political Science" << "SHSS");
    departments << (QStringList() << "ECO" << "Economics" << "SBA");
    departments << (QStringList() << "GEO" << "Geography" << "SHSS");
    departments << (QStringList() << "PSY" << "Psychology" << "SHSS");
    departments << (QStringList() << "SOC" << "Sociology" << "SHSS");
    departments << (QStringList() << "SSC" << "Social Sciences" << "SHSS");
    departments << (QStringList() << "FIN" << "Finance" << "SBA");
    departments << (QStringList() << "MGT" << "Management" << "SBA");
    departments << (QStringList() << "GBU" << "General Business" << "SBA");
    departments << (QStringList() << "MIS" << "Management Information Systems" << "SBA");
    departments << (QStringList() << "MKT" << "Marketing" << "SBA");
    departments << (QStringList() << "SCM" << "Supply Chain Management" << "SBA");
    departments << (QStringList() << "SPN" << "Spanish" << "SHSS");
    departments << (QStringList() << "FRN" << "French" << "SHSS");
    departments << (QStringList() << "GER" << "German" << "SHSS");

    for (int i = 0; i < departments.size(); ++i) {
        const QStringList& dept = departments.at(i);
        if (dept.size() == 3) {
            query.addBindValue(dept[0]);
            query.addBindValue(dept[1]);
            query.addBindValue(dept[2]);
            if (!query.exec()) {
                qDebug() << "Failed to insert department" << dept[0] << ":" << query.lastError().text();
            }
            query.finish();
        }
    }

    return true;
}

bool DatabaseInitializer::insertCourses(QSqlDatabase& db) {
    QSqlQuery query(db);
    query.prepare("INSERT OR IGNORE INTO course (course_code, course_title, course_description, course_credits, course_classification, dep_code, course_type, course_level) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    // Use QList to store all courses
    QList<Course> courses;
    Course course;

    // 1. MTH1303
    course.code = "MTH1303";
    course.title = "Calculus I: Differential and Integral Calculus";
    course.description = "The emphasis of this course is on problem solving, not on the presentation of theoretical considerations. While the course necessarily includes some discussion of theoretical notions, its primary objective is not the production of theorem-provers. The syllabus for MTH 1303 includes most of the elementary topics in the theory of real-valued functions of a real variable: limits, continuity, derivatives, maxima and minima, integration, trigonometric, logarithmic, and exponential functions and techniques of integration.";
    course.credits = 3;
    course.classification = "Freshman";
    course.depCode = "MTH";
    course.type = "Mathematics";
    course.level = "";
    courses.append(course);

    // 2. CSC1401
    course.code = "CSC1401";
    course.title = "Computer Programming";
    course.description = "This course covers algorithms and problem solving, basic algorithmic analysis, fundamental programming constructs and data structures, and basic algorithmic strategies. Students will learn the basic skills of constructing a problem solution and will illustrate this in labs with a high-level language (currently C). Students will also be introduced to different aspects of ethics in Computer Science.";
    course.credits = 4;
    course.classification = "Freshman";
    course.depCode = "CSC";
    course.type = "Core";
    course.level = "Beginner";
    courses.append(course);

    // 3. FYE1101
    course.code = "FYE1101";
    course.title = "First Year Experience";
    course.description = "Throughout their first year, all incoming students will enroll in a Pass/Fail, one-credit Seminar (FYE 1101 during the first semester and FYE 1102 during the second one) and will also be paired with a peer mentor for their first semester to provide them with guidance, advice, feedback and support during this new journey. The FYE Seminars consist of a series of informative forums developed through collaboration between the Division of Student Affairs and Academic Affairs. Incoming students will be charged 2500 MAD to cover orientation and program fees. Additionally, they will pay 2100 MAD for each of the one-credit Seminars.";
    course.credits = 1;
    course.classification = "Freshman";
    course.depCode = "FYE";
    course.type = "General Education";
    course.level = "";
    courses.append(course);

    // 4. FAS0210
    course.code = "FAS0210";
    course.title = "Foundations for Academic Success: Strategic Academic Skills";
    course.description = "This course offers basic study skills and information literacy skills preparing students for academic success at AUI. Students are trained to take responsibility for their own learning through tasks that require them to reflect on, as well as evaluate, their current study habits, and to implement new strategies that improve their learning. Students are also introduced to the Information Literacy Skills necessary to communicate effectively in online, classroom, academic, and group contexts. These skills include being able to locate, evaluate, synthesize, and present information efficiently and effectively. They learn and practice the principles of effective academic research and demonstrate their mastery in a final project. This course can be waived upon successful completion of a placement test or upon meeting portfolio requirements.";
    course.credits = 2;
    course.classification = "Freshman";
    course.depCode = "FAS";
    course.type = "General Education";
    course.level = "";
    courses.append(course);

    // 5. ARB1241
    course.code = "ARB1241";
    course.title = "Arabic Literature";
    course.description = "The aim of this course, taught in Arabic, is to promote the knowledge of different literary genres and to enable students to differentiate between them and to appreciate beauty in a literary work of art. Literary concepts are introduced to students and followed by reading, analysis, and discussion of works drawn from various genres. The texts are either originally written in Arabic or translated into Arabic from other languages. In addition to its literary and aesthetic value, the collection of texts used belongs to a variety of literary genres and is written by authors of diverse nationalities. This course is designed exclusively for students with a Moroccan baccalaureate or an Arabic-based high school degree from any country. However, other students whose placement test scores indicate that they have the capability to attend this course may enroll in it.";
    course.credits = 2;
    course.classification = "Freshman";
    course.depCode = "ARB";
    course.type = "General Education";
    course.level = "";
    courses.append(course);

    // 6. ENG1301
    course.code = "ENG1301";
    course.title = "English Composition I";
    course.description = "This course introduces students to the essay genres of reflecting, reporting, explaining, and arguing. This involves writing several genres of academic essay following the process approach to construction, including four out-of-class essays. The expository and argumentative essays are sourced from a course reader. At least two additional essays are written in class on topics not announced in advance. Additional exercises deemed important for learning fluent and accurate writing are assigned from a course rhetoric/grammar text and other supplementary material. A semester-long project of keeping a writer's journal may be part of the instruction.";
    course.credits = 3;
    course.classification = "Freshman";
    course.depCode = "ENG";
    course.type = "General Education";
    course.level = "";
    courses.append(course);

    // 7. CSC2302
    course.code = "CSC2302";
    course.title = "Data Structures";
    course.description = "This course will build on the knowledge and skills acquired in the introductory programming course. It covers different types of data structures (lists, queues, graphs, etc.) and the associated algorithms, emphasizing memory vs. CPU tradeoffs.";
    course.credits = 3;
    course.classification = "Freshman";
    course.depCode = "CSC";
    course.type = "Core";
    course.level = "";
    courses.append(course);

    // 8. MTH2301
    course.code = "MTH2301";
    course.title = "Calculus II: Multivariable Calculus";
    course.description = "Multivariable calculus covers vectors and surfaces, partial differentiation, multiple integration, and vector calculus, including Green's Theorem and Stokes' Theorem.";
    course.credits = 3;
    course.classification = "Freshman";
    course.depCode = "MTH";
    course.type = "Mathematics";
    course.level = "";
    courses.append(course);

    // 9. MTH2320
    course.code = "MTH2320";
    course.title = "Linear and Matrix Algebra";
    course.description = "The course covers differential equations, including basic concepts, first-order differential equations and applications, linear differential equations of higher order, series solutions, solutions using Laplace transforms, and solutions using numerical methods and applications.";
    course.credits = 3;
    course.classification = "Freshman";
    course.depCode = "MTH";
    course.type = "Mathematics";
    course.level = "";
    courses.append(course);

    // 10. MTH3301
    course.code = "MTH3301";
    course.title = "Probability and Statistics for Engineers";
    course.description = "This course is a calculus-based introduction to probability and statistics with a focus on techniques and applications most relevant to engineering. Topics include basic probability, conditional probability, independence of events, random variables, cumulative distribution functions, density functions, expectation, variance and covariance, independence of events and random variables, common discrete and continuous distributions, Law of Large Numbers, the Central Limit Theorem, and an introduction to descriptive statistics and sampling distributions";
    course.credits = 3;
    course.classification = "";
    course.depCode = "MTH";
    course.type = "Mathematics";
    course.level = "";
    courses.append(course);

    // 11. MTH1304
    course.code = "MTH1304";
    course.title = "Discrete Mathematics for Engineers";
    course.description = "This course is an introduction to the fundamental ideas of discrete mathematics. Topics covered in this course include logic, sets, relations, function, number representation, graphs, and counting techniques and their applications to probabilistic reasoning. The aim of the course is to give the students a foundation for the development of more advanced mathematical concepts that are used in computer science and engineering.";
    course.credits = 3;
    course.classification = "";
    course.depCode = "MTH";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 12. CHE1401
    course.code = "CHE1401";
    course.title = "General Chemistry I";
    course.description = "This course starts with the basics of chemistry and chemical reactions. It then gives an insight into the structure of matter and the origin of chemical periodicity. The course provides an introduction to the fundamentals of atomic structure, the periodic table, nomenclature, chemical equations and solutions. The concepts of chemical bonding and molecular structure are also developed. The laboratory sessions are designed to reinforce the lecture material and to allow students to become familiar with the experimental procedures used in chemistry.";
    course.credits = 4;
    course.classification = "";
    course.depCode = "Basic Sciences";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 13. PHY1401
    course.code = "PHY1401";
    course.title = "Physics I";
    course.description = "This course provides engineering students with the basic knowledge of mechanics and vibration required for the advanced engineering course. Theoretical and experimental concepts covered include physical quantities, linear motion, Newton's law of motion, work, energy, momentum, systems in equilibrium, periodic motion, elasticity, fluid mechanics, the universal law of gravitation, the general form of gravitational potential energy, temperature and thermal expansion.";
    course.credits = 4;
    course.classification = "";
    course.depCode = "Basic Sciences";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 14. PHY1402
    course.code = "PHY1402";
    course.title = "Physics II";
    course.description = "This course is a continuation of General Physics I (Phy1401), it provides you with fundamental knowledge of: Electricity, and magnetism required in follow up-courses in electric circuits, electromagnetism, communications, and sensor systems. Sound waves required in conversion to electrical signals. Optics required in follow-up courses in electromagnetism, communications, and sensor systems. Theoretical and experimental topics include wave productions and propagation, standing waves, wave effects, sound, electric fields and forces, electromagnetic induction, AC circuits, light, optical systems, interference, and diffraction";
    course.credits = 4;
    course.classification = "";
    course.depCode = "Basic Sciences";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 15. CSC2306
    course.code = "CSC2306";
    course.title = "Object Oriented Programming";
    course.description = "This course will build on the broad understanding acquired in the previous programming courses to allow students to scale-up and be able to solve more complex problems through object-oriented methodology. The object-oriented paradigm encapsulates code complexity within objects and integrates features such as abstraction, cohesion, encapsulation, information hiding, inheritance, and polymorphism. The object-oriented approach emphasizes software qualities such as robustness, maintainability, extensibility, and reusability principles into software development. Students will gain experience building object-oriented software solutions using the Unified Modeling Language (UML) and the Java programming language.";
    course.credits = 3;
    course.classification = "";
    course.depCode = "CSC";
    course.type = "Required";
    course.level = "Undergraduate";
    courses.append(course);

    // 16. CSC2305
    course.code = "CSC2305";
    course.title = "Computer Organization and Architecture";
    course.description = "This course provides a discussion of the fundamentals of computer organization and architecture and relates it to contemporary design issues. Students will gain an understanding of the basic structure and operation of a digital computer. Topics include digital logic, instruction set architecture, computer arithmetic, architectural CPU design, and functional computer organization. Besides emphasizing the fundamental concepts, the course will discuss the critical role of performance in driving computer design.";
    course.credits = 3;
    course.classification = "";
    course.depCode = "CSC";
    course.type = "Required";
    course.level = "Undergraduate";
    courses.append(course);

    // 17. CSC3315
    course.code = "CSC3315";
    course.title = "Languages and Compilers";
    course.description = "This course examines computer languages from the perspective of translation and the computational models they implement. The first half of the course reviews the evolution of languages, language design principles, and evaluation criteria, then presents the processing stages, theoretical concepts, and tools associated with language translation. The second half of the course introduces languages representing different programming paradigms and examines runtime handling of selected language features.";
    course.credits = 3;
    course.classification = "Junior";
    course.depCode = "CSC";
    course.type = "Required";
    course.level = "Undergraduate";
    courses.append(course);

    // 18. CSC3351
    course.code = "CSC3351";
    course.title = "Operating Systems";
    course.description = "This course introduces students to fundamental concepts of operating systems. It emphasizes structures, key design issues, as well as the fundamental principles behind modern operating systems. It covers concurrency, scheduling, memory and device management, file systems, and scripting. The course uses implementations of Unix-like and Windows systems to illustrate some concepts.";
    course.credits = 3;
    course.classification = "";
    course.depCode = "CSC";
    course.type = "Required";
    course.level = "Undergraduate";
    courses.append(course);

    // 19. CSC3374
    course.code = "CSC3374";
    course.title = "Advanced and Distributed Programming Paradigms";
    course.description = "This course provides theoretical knowledge about, and practical skills in advanced programming paradigms. It builds on the background acquired in introductory programming courses and other prerequisite courses to tackle programming models used in professional, enterprise-grade software development. Covered topics are organized into five parts, where each build on previous ones: Programming for communication, integration, performance, extensibility, and scalability.";
    course.credits = 3;
    course.classification = "";
    course.depCode = "CSC";
    course.type = "Required";
    course.level = "Undergraduate";
    courses.append(course);

    // 20. CSC3323
    course.code = "CSC3323";
    course.title = "Analysis of Algorithms";
    course.description = "This course covers the study of algorithm design, including introductory concepts: sorting and searching algorithms; string, graph, and set algorithms; complexity classes; branch and bound algorithms; dynamic programming, and advanced topics.";
    course.credits = 3;
    course.classification = "Junior";
    course.depCode = "CSC";
    course.type = "Required";
    course.level = "Undergraduate";
    courses.append(course);

    // 21. CSC3324
    course.code = "CSC3324";
    course.title = "Software Engineering";
    course.description = "Software Engineering is the application of engineering principles and techniques to the process of software development, delivery, maintenance, and decommission. Its purpose is to make the building of software systems formal and predictable, while delivering high quality software products that comply with client requirements and expectations. The main goal of this course is to learn how to build Professional Software, in a Business Environment, using a formal development framework/methodology, and to develop the necessary skills of Modeling, Analysis and Design, including System Architecture and Technology Stack issues, necessary to this end.";
    course.credits = 3;
    course.classification = "";
    course.depCode = "CSC";
    course.type = "Required";
    course.level = "Undergraduate";
    courses.append(course);

    // 22. CSC3371
    course.code = "CSC3371";
    course.title = "Computer Communications and Networks";
    course.description = "This course covers topics in data communications of interest to computing majors, such as types of LAN, VLAN, MAN, and WAN networks, with their corresponding access methods and link layer protocols. The course then moves on to cover the TCP/IP protocol stack, including IP functions such as addressing, routing, congestion control, and Transport layer function, such as reliable transmission and flow control. The course culminates into the coverage of major Internet enabling services and application layer protocols, namely DNS, SMTP, and HTTP.";
    course.credits = 3;
    course.classification = "";
    course.depCode = "CSC";
    course.type = "Required";
    course.level = "Undergraduate";
    courses.append(course);

    // 23. CSC3326
    course.code = "CSC3326";
    course.title = "Database Systems";
    course.description = "The course covers the design and implementation of Relational databases. While significant focus is placed on Entity-Relationship and Relational modeling, other models and current trends in database are introduced. Relational Algebra and SQL are covered, including procedural SQL. A Relational DBMS is used along with database application development as lab work and class projects. Transactions and concurrency issues are also addressed.";
    course.credits = 3;
    course.classification = "Junior";
    course.depCode = "CSC";
    course.type = "Required";
    course.level = "Undergraduate";
    courses.append(course);

    // 24. EGR2302
    course.code = "EGR2302";
    course.title = "Engineering Economics";
    course.description = "This course covers the time value of economic resources, engineering project investments analysis, and the effect of taxes on engineering project decisions. Students learn about societal, technical, economic, environmental, political, legal, and ethical analyses of project alternatives";
    course.credits = 3;
    course.classification = "";
    course.depCode = "EGR";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 25. FYE1102
    course.code = "FYE1102";
    course.title = "First Year Experience II";
    course.description = "Continuation of the first-year experience program focusing on academic success and campus integration.";
    course.credits = 1;
    course.classification = "freshman";
    course.depCode = "FYE";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 26. FAS1220
    course.code = "FAS1220";
    course.title = "Introduction to Critical Thinking";
    course.description = "Designed to develop students understanding of the critical thinking process, with a view to improving their own reasoning and analysis skills.";
    course.credits = 2;
    course.classification = "freshman";
    course.depCode = "FAS";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 27. COM1301
    course.code = "COM1301";
    course.title = "Public Speaking";
    course.description = "Develops public speaking and group discussion skills. Students engage in public speaking activities designed to promote competency in delivery of speeches, organization of ideas, methods of argumentation, and utilization of supporting materials.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "COM";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 28. ENG2303
    course.code = "ENG2303";
    course.title = "Technical Writing";
    course.description = "Develops writing skills necessary for advanced undergraduate writing in SSE courses and for future writing tasks in professional contexts. Students write resumes, cover letters, memorandums, reports, professional emails, and a final research paper.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "ENG";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 29. HUM2305
    course.code = "HUM2305";
    course.title = "Science and Society";
    course.description = "Provides understanding of science as an intellectual endeavor and the contemporary world shaped by science. Examines the nature of scientific knowledge and its relationship to other kinds of knowledge, technology, and power.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "HUM";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 30. HUM2306
    course.code = "HUM2306";
    course.title = "Comparative Religion";
    course.description = "Introduction to the world major religions: Islam, Christianity, Judaism, Buddhism, and Hinduism. Covers ideas and concepts that have shaped religions and themes shared by all of them.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "HUM";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 31. HUM2307
    course.code = "HUM2307";
    course.title = "History of Islamic Technology";
    course.description = "Survey of technological advances in the Middle East and North Africa in the pre-modern era, showing intellectual and technological exchanges between Muslims and their neighbors.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "HUM";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 32. LIT2301
    course.code = "LIT2301";
    course.title = "Major Works of World Literature";
    course.description = "Gives students a generous sampling of major works of World Literature using a multi-disciplinary approach, focusing on reading, analyzing, and discussing works from various historical periods and genres.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "LIT";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 33. PHI2301
    course.code = "PHI2301";
    course.title = "Philosophical Thought";
    course.description = "Introduction to Philosophy examining principal themes of philosophical inquiry from both historical and multicultural perspective. Contributes to understanding ideas that shape human life.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "PHI";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 34. PHI2302
    course.code = "PHI2302";
    course.title = "History of Ideas";
    course.description = "Examines influential ideas in distinct areas of thought and from diverse intellectual traditions, analyzing them within historical context and tracing their origin and development.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "PHI";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 35. ART1301
    course.code = "ART1301";
    course.title = "Introduction to Drawing";
    course.description = "Fundamental course in drawing techniques and principles for beginners.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "ART";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 36. ART1302
    course.code = "ART1302";
    course.title = "Introduction to Painting";
    course.description = "Basic painting techniques and color theory for beginning artists.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "ART";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 37. ART1303
    course.code = "ART1303";
    course.title = "Introduction to Sculpture";
    course.description = "Fundamental principles and techniques of three-dimensional art forms.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "ART";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 38. ART1304
    course.code = "ART1304";
    course.title = "Introduction to Digital Art";
    course.description = "Basic digital art creation using contemporary software and tools.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "ART";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 39. ART1305
    course.code = "ART1305";
    course.title = "Art History Survey";
    course.description = "Comprehensive overview of major movements and periods in art history.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "ART";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 40. ART3399
    course.code = "ART3399";
    course.title = "Special Topics in Art";
    course.description = "Specially scheduled courses on significant issues or topics relevant to the study of art. Content varies according to faculty expertise.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "ART";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 41. ENG2320
    course.code = "ENG2320";
    course.title = "Creative Writing";
    course.description = "Students immerse themselves in reading and writing, creating their own community of writers. Examines how craft transforms source material and brings instinct and critical understanding into equilibrium.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "ENG";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 42. HUM2301
    course.code = "HUM2301";
    course.title = "Islamic Art and Architecture";
    course.description = "Introduction to Islamic art and architecture with emphasis on the Maghreb and Al-Andalus. Covers the period of formation of Islamic art and surveys art and architecture of major dynasties.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "HUM";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 43. COM2327
    course.code = "COM2327";
    course.title = "Art and Design Production";
    course.description = "Introduces fundamental art concepts and histories important in all aspects of contemporary design from print and photography to website interface. Covers vocabulary and working terms of artists and designers.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "COM";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 44. LIT3370
    course.code = "LIT3370";
    course.title = "Aesthetics";
    course.description = "Examines aesthetic theory, basic themes such as notion of aesthetic, definition of art, taste, value of art, beauty, interpretation, and artistic imagination. Focuses on four categories of art forms: literature, dramatic arts, music, and visual arts.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "LIT";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 45. HIS1301
    course.code = "HIS1301";
    course.title = "History of the Arab World";
    course.description = "Covers history of the Arab world from the rise of Islam to the present, taking social and cultural approach to understanding different histories of Arab society. Balances political history with long-term social transformations.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "HIS";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 46. HIS2301
    course.code = "HIS2301";
    course.title = "Contemporary World History";
    course.description = "Provides broad overview of major developments affecting different parts of the world in the modern era. Focuses on political, social, and cultural history of Europe and interactions with Africa, Asia, and Latin America.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "HIS";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 47. HUM1310
    course.code = "HUM1310";
    course.title = "History and Culture of the Amazigh";
    course.description = "Provides insight into specifics of Amazigh culture and institutions, covering origins, distribution, and common history of Amazigh peoples of North Africa from ancient history to medieval times.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "HUM";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 48. HUM2302
    course.code = "HUM2302";
    course.title = "Islamic Societies";
    course.description = "Introduction to historical and contemporary study of Islamic societies using interdisciplinary approach. Explores topics including history, theology, mysticism, architecture, political theory, literature, and fine arts.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "HUM";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 49. PSC2301
    course.code = "PSC2301";
    course.title = "Comparative Political Systems";
    course.description = "Provides solid introduction to major concepts, theories, and debates relevant to contemporary politics. Includes analysis of origins and development of the state, with particular emphasis on the democratic state.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "PSC";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 50. ECO1300
    course.code = "ECO1300";
    course.title = "Introduction to Economics";
    course.description = "Introduces essentials of both microeconomics and macroeconomics. Topics include supply and demand, price elasticity, producer and consumer choices, market structures, GDP, inflation, labor market, and international trade.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "ECO";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 51. GEO1301
    course.code = "GEO1301";
    course.title = "Introduction to Geography";
    course.description = "Introductory social science course providing survey of principles of human geography. Initiates students in basic concepts required for spatial analysis of social phenomena, including notions of scale and cartographic representation.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "GEO";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 52. PSY1301
    course.code = "PSY1301";
    course.title = "Introduction to Psychology";
    course.description = "Introductory survey of major content areas and theoretical models of psychology emphasizing scientific approach to critically understand human behavior. Examines behavior as product of interaction between biology, culture, and individual abilities.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "PSY";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 53. SOC1301
    course.code = "SOC1301";
    course.title = "Principles of Sociology";
    course.description = "Introduction to sociology covering range of objects of sociological investigation, including socialization, deviance, social stratification, family, gender, race, and ethnicity. Introduces scope, approach, research methods, and ethical concerns.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "SOC";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 54. SSC1310
    course.code = "SSC1310";
    course.title = "Introduction to Anthropology";
    course.description = "Introduction to principles and processes of Cultural Anthropology with emphasis on Applied Anthropology. Has strong theoretical base linking social, economic, and political organizations to wider considerations of environment and culture.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "SSC";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 55. EGR4300
    course.code = "EGR4300";
    course.title = "Internship";
    course.description = "On-the-job education and training in a public or private sector agency or business related to the student's major. Requires consultation with internship director and three-party contract.";
    course.credits = 3;
    course.classification = "senior";
    course.depCode = "EGR";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 56. EGR4402
    course.code = "EGR4402";
    course.title = "Capstone Design";
    course.description = "Project-design course to consolidate information from all previous courses, identifying appropriate standards and incorporating realistic constraints. Taken during last regular semester of enrollment.";
    course.credits = 4;
    course.classification = "senior";
    course.depCode = "EGR";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 57. ACC2301
    course.code = "ACC2301";
    course.title = "Accounting Principles I";
    course.description = "Introduction to fundamental concepts of financial accounting, double entry accounting theory, recording procedures, and financial statements preparation and analysis.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "ACC";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 58. FIN3301
    course.code = "FIN3301";
    course.title = "Principles of Finance";
    course.description = "Financial management for business and the corporation emphasizes the conceptual framework and principles of financial management for business organizations. Integrates knowledge from introductory courses in accounting and economics.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "FIN";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 59. MGT3301
    course.code = "MGT3301";
    course.title = "Principles of Management";
    course.description = "Overview of the art and science of Management. Broad categories covered are evolution of management theory, decision-making procedures, foundations of leading and managing, and strategic management.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "MGT";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 60. ACC2302
    course.code = "ACC2302";
    course.title = "Accounting Principles II";
    course.description = "Covers basic cost relationships, cost behavior, cash flow statements, financial statement analysis including ratio analysis, cost of products for managerial decision-making, forecasting, budgeting, and profitability analysis.";
    course.credits = 3;
    course.classification = "sophomore";
    course.depCode = "ACC";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 61. GBU3311
    course.code = "GBU3311";
    course.title = "Quantitative Methods in Business";
    course.description = "Provides foundations of statistical methods for managerial decision-making. Offers extensive coverage of inferential statistics: hypothesis testing, ANOVA, chi-square tests, regression analysis, and time series forecasting.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "GBU";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 62. GBU4308
    course.code = "GBU4308";
    course.title = "Foundations of E-Commerce";
    course.description = "Introduces concepts and methods surrounding electronic commerce practices. Student gains command of current e-commerce business models, opportunities, and related barriers. Focus on innovative thinking for turning e-commerce technologies into value.";
    course.credits = 3;
    course.classification = "senior";
    course.depCode = "GBU";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 63. MGT3302
    course.code = "MGT3302";
    course.title = "Entrepreneurship";
    course.description = "Introduces role of entrepreneur in economic system and covers how to start, finance, and operate successful business. Focuses on developing business plan, sources of capital, recruiting, forecasting, and financial planning.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "MGT";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 64. MGT4303
    course.code = "MGT4303";
    course.title = "Operations Management";
    course.description = "Introduces field of operations management and state-of-the-art view of primary activities of operations function in manufacturing and service organizations. Provides basic principles of designing, analyzing, and controlling production systems.";
    course.credits = 3;
    course.classification = "senior";
    course.depCode = "MGT";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 65. MIS3302
    course.code = "MIS3302";
    course.title = "Managing Information Systems and Technology in the Global Marketplace";
    course.description = "Covers managerial and organizational issues involved in operating a firm. Reviews role of automated information processing, automated support processes, and information systems. Examines planning development, direction, and control of computer information systems.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "MIS";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 66. MKT3301
    course.code = "MKT3301";
    course.title = "Principles of Marketing";
    course.description = "Gives outline of common marketing concepts and models with purpose of developing responsive marketing strategies that meet customers' needs. Focuses on use of marketing in organizations and society.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "MKT";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 67. MKT4304
    course.code = "MKT4304";
    course.title = "Marketing Research";
    course.description = "Focuses on use of marketing research as aid to making marketing decisions. Addresses how information used to make marketing decisions is gathered and analyzed. Topics include marketing research process, research design, methodologies, data collection, analysis, and interpretation.";
    course.credits = 3;
    course.classification = "senior";
    course.depCode = "MKT";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 68. MKT4305
    course.code = "MKT4305";
    course.title = "Marketing Management";
    course.description = "Concerned with development, evaluation, and implementation of marketing management in complex environments. Focuses on formulating and implementing marketing management strategies and policies at strategic business unit level.";
    course.credits = 3;
    course.classification = "senior";
    course.depCode = "MKT";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 69. SCM4301
    course.code = "SCM4301";
    course.title = "Introduction to Supply Chain Management";
    course.description = "Supply chain management is integration of all activities associated with flow and transformation of goods. Provides introduction to logistics and supply chain concepts, structures, design, and control.";
    course.credits = 3;
    course.classification = "senior";
    course.depCode = "SCM";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 70. CSC4307
    course.code = "CSC4307";
    course.title = "Agile Software Engineering and DevOps";
    course.description = "Focuses on agility in software engineering: its values, principles, roles, practices, frameworks, and tools. Compares agile software development against traditional phase-based methodologies. Emphasizes Continuous Integration and Continuous Delivery.";
    course.credits = 3;
    course.classification = "senior";
    course.depCode = "CSC";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 71. CSC4309
    course.code = "CSC4309";
    course.title = "Enterprise Cloud and Mobile Application Architecture, Design and Development";
    course.description = "Provides solid background on architecture, design and full stack development of enterprise-class, cloud-based, mobile-first progressive applications, based on well-defined security, performance, scalability, extensibility and integration requirements.";
    course.credits = 3;
    course.classification = "senior";
    course.depCode = "CSC";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 72. CSC3359
    course.code = "CSC3359";
    course.title = "Software Testing and Analysis";
    course.description = "Presents overview of principles and practices of software testing. Covers different concepts and techniques on how to test software. Topics include various testing methods such as structural and functional testing.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "CSC";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 73. CSC3309
    course.code = "CSC3309";
    course.title = "Introduction to Artificial Intelligence";
    course.description = "Provides students with concepts and knowledge to design intelligent agents by leveraging main skills of AI programming, namely searching for goals and planning, knowledge representation, logical and probabilistic inferencing, and learning from experience.";
    course.credits = 3;
    course.classification = "junior";
    course.depCode = "CSC";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 74. SPN1301
    course.code = "SPN1301";
    course.title = "Beginning Spanish I";
    course.description = "Intended for students with no basic knowledge in Spanish. Students apply, develop, and integrate different language skills: listening, speaking, reading comprehension, and writing. Upon completion, students can express themselves in spoken and written Spanish.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "SPN";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 75. FRN2210
    course.code = "FRN2210";
    course.title = "Intermediate French";
    course.description = "Consolidates and extends vocabulary, speaking and comprehension abilities. Uses variety of reading materials including complex narratives and short poetry. Offers insight into culture and history of Francophone world.";
    course.credits = 2;
    course.classification = "sophomore";
    course.depCode = "FRN";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 76. FRN3210
    course.code = "FRN3210";
    course.title = "Advanced French Communication";
    course.description = "Develops advanced French communication skills for academic and professional contexts. Focuses on complex grammatical structures, advanced vocabulary, and nuanced expression in both written and spoken French.";
    course.credits = 2;
    course.classification = "junior";
    course.depCode = "FRN";
    course.type = "";
    course.level = "";
    courses.append(course);

    // 77. GER1301
    course.code = "GER1301";
    course.title = "Beginning German I";
    course.description = "Introductory course in German language for beginners. Focuses on basic communication skills, grammar, vocabulary, and cultural understanding.";
    course.credits = 3;
    course.classification = "freshman";
    course.depCode = "GER";
    course.type = "";
    course.level = "";
    courses.append(course);

    // Insert all courses
    int inserted = 0;
    int failed = 0;

    for (int i = 0; i < courses.size(); ++i) {
        const Course& course = courses.at(i);
        query.addBindValue(course.code);
        query.addBindValue(course.title);
        query.addBindValue(course.description);
        query.addBindValue(course.credits);
        query.addBindValue(course.classification);
        query.addBindValue(course.depCode);
        query.addBindValue(course.type);
        query.addBindValue(course.level);

        if (query.exec()) {
            inserted++;
        } else {
            failed++;
            qDebug() << "Failed to insert course" << course.code << ":" << query.lastError().text();
        }

        query.finish();
    }

    qDebug() << "Inserted" << inserted << "courses," << failed << "failed";
    return failed == 0;
}

bool DatabaseInitializer::insertSemesters(QSqlDatabase& db) {
    QSqlQuery query(db);
    query.prepare("INSERT OR IGNORE INTO semester (sem_code, sem_year, sem_term) VALUES (?, ?, ?)");

    // Simple loop without range-based for
    QStringList terms;
    terms << "FALL" << "SPRING" << "SUMMER";

    // Generate semesters from 2023 to 2035
    for (int year = 2023; year <= 2035; ++year) {
        for (int i = 0; i < terms.size(); ++i) {
            QString term = terms.at(i);
            QString semCode = term + QString::number(year);
            query.addBindValue(semCode);
            query.addBindValue(year);
            query.addBindValue(term);

            if (!query.exec()) {
                qDebug() << "Failed to insert semester" << semCode << ":" << query.lastError().text();
            }
            query.finish();
        }
    }

    return true;
}

bool DatabaseInitializer::insertMajors(QSqlDatabase& db) {
    QSqlQuery query(db);
    query.prepare("INSERT OR IGNORE INTO major (major_id, major_code, major_name, department_id, credit_requirements) VALUES (?, ?, ?, ?, ?)");

    // Computer Science major
    query.addBindValue(1);
    query.addBindValue("CS");
    query.addBindValue("Computer Science");
    query.addBindValue("CSC");
    query.addBindValue(134);

    if (!query.exec()) {
        qDebug() << "Failed to insert major:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseInitializer::insertPrerequisites(QSqlDatabase& db) {
    QSqlQuery query(db);
    query.prepare("INSERT OR IGNORE INTO course_prereq (course_code, prereq, prereq_is_corequisite) VALUES (?, ?, ?)");

    // Use QList to store all prerequisites
    QList<QStringList> prereqs;

    // Add ALL prerequisites from your SQL file
    prereqs << (QStringList() << "CSC2302" << "CSC1401" << "0");
    prereqs << (QStringList() << "MTH2301" << "MTH1303" << "0");
    prereqs << (QStringList() << "MTH2320" << "MTH2301" << "0");
    prereqs << (QStringList() << "MTH3301" << "MTH2301" << "0");
    prereqs << (QStringList() << "PHY1401" << "MTH1303" << "0");
    prereqs << (QStringList() << "PHY1402" << "PHY1401" << "0");
    prereqs << (QStringList() << "CSC2306" << "CSC2302" << "0");
    prereqs << (QStringList() << "CSC2306" << "MTH1304" << "0");
    prereqs << (QStringList() << "CSC2305" << "CSC2302" << "0");
    prereqs << (QStringList() << "CSC2305" << "PHY1402" << "0");
    prereqs << (QStringList() << "CSC3315" << "CSC2306" << "0");
    prereqs << (QStringList() << "CSC3351" << "CSC2305" << "0");
    prereqs << (QStringList() << "CSC3374" << "CSC3326" << "0");
    prereqs << (QStringList() << "CSC3374" << "CSC3351" << "0");
    prereqs << (QStringList() << "CSC3323" << "CSC2302" << "0");
    prereqs << (QStringList() << "CSC3324" << "CSC2306" << "0");
    prereqs << (QStringList() << "CSC3371" << "CSC2305" << "0");
    prereqs << (QStringList() << "CSC3326" << "CSC2306" << "0");
    prereqs << (QStringList() << "EGR2302" << "MTH1303" << "0");
    prereqs << (QStringList() << "FAS1220" << "FAS0210" << "0");
    prereqs << (QStringList() << "ENG2303" << "ENG1301" << "0");
    prereqs << (QStringList() << "ENG2303" << "COM1301" << "0");
    prereqs << (QStringList() << "HUM2305" << "FAS1220" << "0");
    prereqs << (QStringList() << "HUM2306" << "FAS1220" << "0");
    prereqs << (QStringList() << "HUM2307" << "FAS1220" << "0");
    prereqs << (QStringList() << "LIT2301" << "FAS1220" << "0");
    prereqs << (QStringList() << "LIT2301" << "ENG1301" << "0");
    prereqs << (QStringList() << "PHI2301" << "FAS1220" << "0");
    prereqs << (QStringList() << "PHI2301" << "ENG1301" << "0");
    prereqs << (QStringList() << "PHI2302" << "FAS1220" << "0");
    prereqs << (QStringList() << "PHI2302" << "ENG1301" << "0");
    prereqs << (QStringList() << "ENG2320" << "ENG1301" << "0");
    prereqs << (QStringList() << "HUM2301" << "FAS1220" << "0");
    prereqs << (QStringList() << "HIS2301" << "FAS1220" << "0");
    prereqs << (QStringList() << "HIS2301" << "ENG1301" << "0");
    prereqs << (QStringList() << "HUM1310" << "FAS1220" << "0");
    prereqs << (QStringList() << "HUM2302" << "FAS1220" << "0");
    prereqs << (QStringList() << "PSC2301" << "FAS1220" << "0");
    prereqs << (QStringList() << "PSC2301" << "ENG1301" << "0");
    prereqs << (QStringList() << "HIS1301" << "FAS1220" << "1");
    prereqs << (QStringList() << "GEO1301" << "FAS1220" << "1");
    prereqs << (QStringList() << "SOC1301" << "FAS1220" << "1");
    prereqs << (QStringList() << "SSC1310" << "FAS1220" << "1");
    prereqs << (QStringList() << "FIN3301" << "ACC2301" << "0");
    prereqs << (QStringList() << "FIN3301" << "EGR2302" << "0");
    prereqs << (QStringList() << "MGT3301" << "ACC2301" << "0");
    prereqs << (QStringList() << "MGT3301" << "EGR2302" << "0");
    prereqs << (QStringList() << "GBU3311" << "MTH3301" << "0");
    prereqs << (QStringList() << "MKT3301" << "ACC2301" << "0");
    prereqs << (QStringList() << "MKT3301" << "EGR2302" << "0");
    prereqs << (QStringList() << "CSC4307" << "CSC3326" << "0");
    prereqs << (QStringList() << "CSC4307" << "CSC3351" << "0");
    prereqs << (QStringList() << "CSC3309" << "CSC3323" << "0");
    prereqs << (QStringList() << "CSC3309" << "CSC2306" << "0");
    prereqs << (QStringList() << "CSC3309" << "MTH3301" << "0");
    prereqs << (QStringList() << "EGR4300" << "ENG2303" << "0");
    prereqs << (QStringList() << "EGR4402" << "ENG2303" << "0");
    prereqs << (QStringList() << "ACC2302" << "ACC2301" << "0");
    prereqs << (QStringList() << "GBU4308" << "MGT3301" << "0");
    prereqs << (QStringList() << "MGT3302" << "MGT3301" << "0");
    prereqs << (QStringList() << "MGT3302" << "MKT3301" << "0");
    prereqs << (QStringList() << "MGT3302" << "FIN3301" << "0");
    prereqs << (QStringList() << "MGT4303" << "MGT3301" << "0");
    prereqs << (QStringList() << "MGT4303" << "GBU3311" << "0");
    prereqs << (QStringList() << "MKT4304" << "MKT3301" << "0");
    prereqs << (QStringList() << "MKT4304" << "GBU3311" << "0");
    prereqs << (QStringList() << "MKT4305" << "MGT3301" << "0");
    prereqs << (QStringList() << "CSC4309" << "CSC3374" << "0");
    prereqs << (QStringList() << "CSC3359" << "CSC2306" << "0");

    int inserted = 0;
    int failed = 0;

    for (int i = 0; i < prereqs.size(); ++i) {
        const QStringList& prereq = prereqs.at(i);
        if (prereq.size() == 3) {
            query.addBindValue(prereq[0]);  // course_code
            query.addBindValue(prereq[1]);  // prereq
            query.addBindValue(prereq[2].toInt());  // is_corequisite

            if (query.exec()) {
                inserted++;
            } else {
                failed++;
                qDebug() << "Failed to insert prerequisite" << prereq[0] << "->" << prereq[1] << ":" << query.lastError().text();
            }
            query.finish();
        }
    }

    qDebug() << "Inserted" << inserted << "prerequisites," << failed << "failed";
    return failed == 0;
}

bool DatabaseInitializer::createTrigger(QSqlDatabase& db) {
    QSqlQuery query(db);

    QString triggerSql =
        "CREATE TRIGGER IF NOT EXISTS prereq_check_trigger "
        "BEFORE INSERT ON course_planning "
        "FOR EACH ROW "
        "BEGIN "
        "    SELECT CASE "
        "        WHEN EXISTS ( "
        "            SELECT 1  "
        "            FROM course_prereq  "
        "            WHERE course_code = NEW.course_code "
        "            AND prereq NOT IN ( "
        "                SELECT course_code  "
        "                FROM course_planning  "
        "                WHERE profile_id = NEW.profile_id "
        "            ) "
        "        ) THEN "
        "            RAISE(FAIL, 'You dont have the required prerequisites') "
        "    END; "
        "END";

    if (!query.exec(triggerSql)) {
        qDebug() << "Failed to create trigger:" << query.lastError().text();
        return false;
    }

    return true;
}
