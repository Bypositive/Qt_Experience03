#include "idatabase.h"
#include <QUuid>
#include <QDebug>

QString IDatabase::userLogin(QString userName, QString password)
{
    QSqlQuery query;
    query.prepare("SELECT USERNAME, PASSWORD FROM user WHERE USERNAME = :USER");
    query.bindValue(":USER", userName);
    query.exec();

    if(query.first() && query.value("USERNAME").isValid()){
        QString passwd = query.value("PASSWORD").toString();
        qDebug() << "Database password:" << passwd;
        qDebug() << "Input password:" << password;

        if(passwd == password){
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
    database = QSqlDatabase::addDatabase("QSQLITE");
    QString aFile = "D:/a_WujiguoGuowang/homework/three_up/Qt/实验/experiment/Lab3/lab3a.db";
    database.setDatabaseName(aFile);

    if(!database.open()){
        qDebug() << "Failed to open database:" << database.lastError().text();
    } else {
        qDebug() << "Open database is ok";
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
    patientTabModel->setTable("Patient");  // 注意：首字母大写
    patientTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 先执行select获取字段信息
    if(!(patientTabModel->select())){
        qDebug() << "Failed to select from Patient table:" << patientTabModel->lastError().text();
        return false;
    }

    // 打印字段信息用于调试
    qDebug() << "Patient table fields:";
    for(int i = 0; i < patientTabModel->columnCount(); i++){
        qDebug() << "Column" << i << ":" << patientTabModel->headerData(i, Qt::Horizontal).toString();
    }

    // 设置排序字段 - 使用正确的字段名
    patientTabModel->setSort(patientTabModel->fieldIndex("NAME"), Qt::AscendingOrder);

    // 重新select以确保排序生效
    if(!(patientTabModel->select())){
        return false;
    }

    thePatientSelection = new QItemSelectionModel(patientTabModel);
    return true;
}

int IDatabase::addNewPatient()
{
    patientTabModel->insertRow(patientTabModel->rowCount(), QModelIndex());
    QModelIndex curIndex = patientTabModel->index(patientTabModel->rowCount()-1, 1);

    int curRecNo = curIndex.row();
    QSqlRecord curRec = patientTabModel->record(curRecNo);
    curRec.setValue("CREATEDTIMESTAMP", QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    curRec.setValue("ID", QUuid::createUuid().toString(QUuid::WithoutBraces));
    patientTabModel->setRecord(curRecNo, curRec);

    return curIndex.row();
}

bool IDatabase::searchPatient(QString filter)
{
    qDebug() << "SearchPatient called with filter:" << filter;

    // 直接使用传入的filter作为条件，不再重新构建
    if(filter.isEmpty()){
        patientTabModel->setFilter("");
        qDebug() << "Setting empty filter";
    } else {
        patientTabModel->setFilter(filter);
        qDebug() << "Setting filter condition:" << filter;
    }

    bool result = patientTabModel->select();
    qDebug() << "Select result:" << result << ", rows found:" << patientTabModel->rowCount();
    return result;
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
// 科室管理功能
bool IDatabase::initDepartmentModel()
{
    departmentTabModel = new QSqlTableModel(this, database);
    departmentTabModel->setTable("Department");
    departmentTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置字段显示名称
    departmentTabModel->setHeaderData(0, Qt::Horizontal, "科室ID");
    departmentTabModel->setHeaderData(1, Qt::Horizontal, "科室名称");

    // 设置排序
    departmentTabModel->setSort(1, Qt::AscendingOrder); // 按科室名称排序

    if(!(departmentTabModel->select())){
        qDebug() << "Failed to select from Department table:" << departmentTabModel->lastError().text();
        return false;
    }

    theDepartmentSelection = new QItemSelectionModel(departmentTabModel);
    return true;
}

int IDatabase::addNewDepartment()
{
    departmentTabModel->insertRow(departmentTabModel->rowCount(), QModelIndex());
    QModelIndex curIndex = departmentTabModel->index(departmentTabModel->rowCount()-1, 1);

    int curRecNo = curIndex.row();
    QSqlRecord curRec = departmentTabModel->record(curRecNo);

    // 生成新的ID
    curRec.setValue("ID", QUuid::createUuid().toString(QUuid::WithoutBraces));
    departmentTabModel->setRecord(curRecNo, curRec);

    return curIndex.row();
}

bool IDatabase::searchDepartment(QString filter)
{
    if(filter.isEmpty()){
        departmentTabModel->setFilter("");
    } else {
        QString condition = QString("NAME LIKE '%%1%'").arg(filter);
        departmentTabModel->setFilter(condition);
    }

    bool result = departmentTabModel->select();
    qDebug() << "Department search result:" << result << ", rows found:" << departmentTabModel->rowCount();
    return result;
}

bool IDatabase::deleteCurrentDepartment()
{
    QModelIndex curIndex = theDepartmentSelection->currentIndex();
    if(curIndex.isValid()){
        departmentTabModel->removeRow(curIndex.row());
        bool success = departmentTabModel->submitAll();
        departmentTabModel->select();
        return success;
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
// 医生管理功能
bool IDatabase::initDoctorModel()
{
    doctorTabModel = new QSqlTableModel(this, database);
    doctorTabModel->setTable("Doctor");
    doctorTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置字段显示名称
    doctorTabModel->setHeaderData(0, Qt::Horizontal, "ID");
    doctorTabModel->setHeaderData(1, Qt::Horizontal, "工号");
    doctorTabModel->setHeaderData(2, Qt::Horizontal, "姓名");
    doctorTabModel->setHeaderData(3, Qt::Horizontal, "科室ID");

    // 设置排序
    doctorTabModel->setSort(2, Qt::AscendingOrder); // 按姓名排序

    if(!(doctorTabModel->select())){
        qDebug() << "Failed to select from Doctor table:" << doctorTabModel->lastError().text();
        return false;
    }

    theDoctorSelection = new QItemSelectionModel(doctorTabModel);
    return true;
}

int IDatabase::addNewDoctor()
{
    doctorTabModel->insertRow(doctorTabModel->rowCount(), QModelIndex());
    QModelIndex curIndex = doctorTabModel->index(doctorTabModel->rowCount()-1, 1);

    int curRecNo = curIndex.row();
    QSqlRecord curRec = doctorTabModel->record(curRecNo);

    // 生成新的ID
    curRec.setValue("ID", QUuid::createUuid().toString(QUuid::WithoutBraces));
    doctorTabModel->setRecord(curRecNo, curRec);

    return curIndex.row();
}

bool IDatabase::searchDoctor(QString filter)
{
    if(filter.isEmpty()){
        doctorTabModel->setFilter("");
    } else {
        QString condition = QString("NAME LIKE '%%1%' OR EMPLOYEENO LIKE '%%1%'").arg(filter);
        doctorTabModel->setFilter(condition);
    }

    bool result = doctorTabModel->select();
    qDebug() << "Doctor search result:" << result << ", rows found:" << doctorTabModel->rowCount();
    return result;
}

bool IDatabase::deleteCurrentDoctor()
{
    QModelIndex curIndex = theDoctorSelection->currentIndex();
    if(curIndex.isValid()){
        doctorTabModel->removeRow(curIndex.row());
        bool success = doctorTabModel->submitAll();
        doctorTabModel->select();
        return success;
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
