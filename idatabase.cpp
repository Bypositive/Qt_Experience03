#include "idatabase.h"
#include <QUuid>

QString IDatabase::userLogin(QString userName, QString password)
{
    QSqlQuery query;
    query.prepare("SELECT USERNAME, PASSWORD FROM user WHERE USERNAME = :USER");
    query.bindValue(":USER", userName);

    if(!query.exec()){
        qDebug() << "Query error:" << query.lastError().text();
        return "databaseError";
    }

    if(query.next()){
        QString dbPassword = query.value("PASSWORD").toString();
        qDebug() << "Database password:" << dbPassword;
        qDebug() << "Input password:" << password;

        if(dbPassword == password){
            return "loginOk";
        } else {
            return "wrongPassword";
        }
    } else {
        qDebug() << "No such user:" << userName;
        return "wrongUsername";
    }
}

void IDatabase::initDatabase()
{
    database=QSqlDatabase::addDatabase("QSQLITE");
    QString aFile="D:/a_WujiguoGuowang/homework/three_up/Qt/实验/experiment/Lab3/Lab3a.db";
    database.setDatabaseName(aFile);

    if(!database.open()){
        qDebug()<<"failed to open database";
    }else {
        qDebug()<<"open database is ok";
    }
}

IDatabase::IDatabase(QObject *parent)
    : QObject{parent}
{
    initDatabase();
}

bool IDatabase::initPatientModel()
{
    patientTabModel = new QSqlTableModel(this, database);
    patientTabModel->setTable("Patient"); // 表名首字母大写
    patientTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置字段映射
    patientTabModel->setHeaderData(0, Qt::Horizontal, "ID");
    patientTabModel->setHeaderData(1, Qt::Horizontal, "身份证");
    patientTabModel->setHeaderData(2, Qt::Horizontal, "姓名");
    patientTabModel->setHeaderData(3, Qt::Horizontal, "性别");
    patientTabModel->setHeaderData(4, Qt::Horizontal, "出生日期");
    patientTabModel->setHeaderData(5, Qt::Horizontal, "身高");
    patientTabModel->setHeaderData(6, Qt::Horizontal, "体重");
    patientTabModel->setHeaderData(7, Qt::Horizontal, "手机号");
    patientTabModel->setHeaderData(8, Qt::Horizontal, "年龄");
    patientTabModel->setHeaderData(9, Qt::Horizontal, "创建时间");

    patientTabModel->setSort(patientTabModel->fieldIndex("NAME"), Qt::AscendingOrder);

    if(!(patientTabModel->select())){
        qDebug() << "Failed to select from Patient table";
        return false;
    }

    thePatientSelection = new QItemSelectionModel(patientTabModel);
    return true;
}

int IDatabase::addNewPatient()
{
    patientTabModel->insertRow(patientTabModel->rowCount(), QModelIndex());
    QModelIndex curIndex = patientTabModel->index(patientTabModel->rowCount() - 1, 1);

    int curRecNo = curIndex.row();
    QSqlRecord curRec = patientTabModel->record(curRecNo);

    curRec.setValue("CREATEDTIMESTAMP", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    curRec.setValue("ID", QUuid::createUuid().toString(QUuid::WithoutBraces));

    patientTabModel->setRecord(curRecNo, curRec);

    return curIndex.row();
}

bool IDatabase::searchPatient(QString filter)
{
    if(filter.isEmpty()){
        patientTabModel->setFilter("");
    } else {
        QString condition = QString("NAME LIKE '%%1%' OR ID_CARD LIKE '%%1%' OR MOBILEPHONE LIKE '%%1%'")
        .arg(filter);
        patientTabModel->setFilter(condition);
    }
    return patientTabModel->select();
}

bool IDatabase::deleteCurrentPatient()
{
    QModelIndex curIndex = thePatientSelection->currentIndex();
    if(curIndex.isValid()){
        patientTabModel->removeRow(curIndex.row());
        bool success = patientTabModel->submitAll();
        patientTabModel->select();
        return success;
    }
    return false;
}

bool IDatabase::submitPatientEdit()
{
    return patientTabModel->submitAll();
}

void IDatabase::revertPatientEdit()
{
    patientTabModel->revertAll();
}
// 医生管理功能
bool IDatabase::initDoctorModel()
{
    doctorTabModel = new QSqlTableModel(this, database);
    doctorTabModel->setTable("Doctor");
    doctorTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    doctorTabModel->setHeaderData(0, Qt::Horizontal, "ID");
    doctorTabModel->setHeaderData(1, Qt::Horizontal, "工号");
    doctorTabModel->setHeaderData(2, Qt::Horizontal, "姓名");
    doctorTabModel->setHeaderData(3, Qt::Horizontal, "科室ID");

    doctorTabModel->setSort(doctorTabModel->fieldIndex("NAME"), Qt::AscendingOrder);

    if(!(doctorTabModel->select())){
        qDebug() << "Failed to select from Doctor table";
        return false;
    }

    theDoctorSelection = new QItemSelectionModel(doctorTabModel);
    return true;
}

int IDatabase::addNewDoctor()
{
    doctorTabModel->insertRow(doctorTabModel->rowCount(), QModelIndex());
    QModelIndex curIndex = doctorTabModel->index(doctorTabModel->rowCount() - 1, 1);

    int curRecNo = curIndex.row();
    QSqlRecord curRec = doctorTabModel->record(curRecNo);

    curRec.setValue("ID", QUuid::createUuid().toString(QUuid::WithoutBraces));
    doctorTabModel->setRecord(curRecNo, curRec);

    return curIndex.row();
}

bool IDatabase::searchDoctor(QString filter)
{
    if(filter.isEmpty()){
        doctorTabModel->setFilter("");
    } else {
        QString condition = QString("NAME LIKE '%%1%' OR EMPLOYEENO LIKE '%%1%'")
        .arg(filter);
        doctorTabModel->setFilter(condition);
    }
    return doctorTabModel->select();
}

bool IDatabase::deleteCurrentDoctor()
{
    QModelIndex curIndex = theDoctorSelection->currentIndex();
    if(curIndex.isValid()){
        doctorTabModel->removeRow(curIndex.row());
        return doctorTabModel->submitAll();
    }
    return false;
}

bool IDatabase::submitDoctorEdit()
{
    return doctorTabModel->submitAll();
}

void IDatabase::revertDoctorEdit()
{
    doctorTabModel->revertAll();
}

// 科室管理功能
bool IDatabase::initDepartmentModel()
{
    departmentTabModel = new QSqlTableModel(this, database);
    departmentTabModel->setTable("Department");
    departmentTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    departmentTabModel->setHeaderData(0, Qt::Horizontal, "ID");
    departmentTabModel->setHeaderData(1, Qt::Horizontal, "科室名称");

    departmentTabModel->setSort(departmentTabModel->fieldIndex("NAME"), Qt::AscendingOrder);

    if(!(departmentTabModel->select())){
        qDebug() << "Failed to select from Department table";
        return false;
    }

    theDepartmentSelection = new QItemSelectionModel(departmentTabModel);
    return true;
}

int IDatabase::addNewDepartment()
{
    departmentTabModel->insertRow(departmentTabModel->rowCount(), QModelIndex());
    QModelIndex curIndex = departmentTabModel->index(departmentTabModel->rowCount() - 1, 1);

    int curRecNo = curIndex.row();
    QSqlRecord curRec = departmentTabModel->record(curRecNo);

    curRec.setValue("ID", QUuid::createUuid().toString(QUuid::WithoutBraces));
    departmentTabModel->setRecord(curRecNo, curRec);

    return curIndex.row();
}

bool IDatabase::searchDepartment(QString filter)
{
    if(filter.isEmpty()){
        departmentTabModel->setFilter("");
    } else {
        QString condition = QString("NAME LIKE '%%1%'")
        .arg(filter);
        departmentTabModel->setFilter(condition);
    }
    return departmentTabModel->select();
}

bool IDatabase::deleteCurrentDepartment()
{
    QModelIndex curIndex = theDepartmentSelection->currentIndex();
    if(curIndex.isValid()){
        departmentTabModel->removeRow(curIndex.row());
        return departmentTabModel->submitAll();
    }
    return false;
}

bool IDatabase::submitDepartmentEdit()
{
    return departmentTabModel->submitAll();
}

void IDatabase::revertDepartmentEdit()
{
    departmentTabModel->revertAll();
}
