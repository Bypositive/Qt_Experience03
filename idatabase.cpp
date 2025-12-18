#include "idatabase.h"
#include <QUuid>
#include <QDebug>
#include <QDateTime>

QString IDatabase::userLogin(QString userName, QString password)
{
    QSqlQuery query;
    query.prepare("SELECT USERNAME, PASSWORD, FULLNAME FROM user WHERE USERNAME = :USER");
    query.bindValue(":USER", userName);
    query.exec();

    if(query.first() && query.value("USERNAME").isValid()){
        QString passwd = query.value("PASSWORD").toString();
        QString fullName = query.value("FULLNAME").toString();
        qDebug() << "Database password:" << passwd;
        qDebug() << "Input password:" << password;

        if(passwd == password){
            currentUserName = fullName.isEmpty() ? userName : fullName; // 保存当前用户信息
            qDebug() << "Current user set to:" << currentUserName;
            return "loginOk";
        } else {
            return "wrongPassword";
        }
    } else {
        qDebug() << "No such user:" << userName;
        return "wrongUsername";
    }
}
QString IDatabase::getCurrentUserName()
{
    return currentUserName;
}
bool IDatabase::initHistoryModel()
{
    historyTabModel = new QSqlTableModel(this, database);
    historyTabModel->setTable("History");
    historyTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    historyTabModel->setHeaderData(0, Qt::Horizontal, "记录ID");
    historyTabModel->setHeaderData(1, Qt::Horizontal, "操作用户");
    historyTabModel->setHeaderData(2, Qt::Horizontal, "操作事件");
    historyTabModel->setHeaderData(3, Qt::Horizontal, "操作时间");
    historyTabModel->setSort(3, Qt::DescendingOrder);

    if(!(historyTabModel->select())){
        qDebug() << "Failed to select from History table:" << historyTabModel->lastError().text();
        return false;
    }

    return true;
}
bool IDatabase::addOperationRecord(const QString &event, const QString &details)
{
    QSqlQuery query;
    QString fullEvent = event;
    if (!details.isEmpty()) {
        fullEvent += "：" + details;
    }

    query.prepare("INSERT INTO History (ID, USER_ID, EVENT, TIMESTAMP) "
                  "VALUES (:id, :user_id, :event, :timestamp)");

    query.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    query.bindValue(":user_id", currentUserName);
    query.bindValue(":event", fullEvent);
    query.bindValue(":timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    if (query.exec()) {
        qDebug() << "Operation record added:" << fullEvent;
        return true;
    } else {
        qDebug() << "Failed to add operation record:" << query.lastError().text();
        return false;
    }
}
bool IDatabase::searchOperationRecord(const QString &filter)
{
    if (filter.isEmpty()) {
        historyTabModel->setFilter("");
    } else {
        QString condition = QString("EVENT LIKE '%%1%' OR USER_ID LIKE '%%1%'").arg(filter);
        historyTabModel->setFilter(condition);
    }

    bool result = historyTabModel->select();
    qDebug() << "History search result:" << result << ", rows found:" << historyTabModel->rowCount();
    return result;
}

QSqlTableModel *IDatabase::getHistoryModel()
{
    return historyTabModel;
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
    if(!(patientTabModel->select())){
        qDebug() << "Failed to select from Patient table:" << patientTabModel->lastError().text();
        return false;
    }
    qDebug() << "Patient table fields:";
    for(int i = 0; i < patientTabModel->columnCount(); i++){
        qDebug() << "Column" << i << ":" << patientTabModel->headerData(i, Qt::Horizontal).toString();
    }

    patientTabModel->setSort(patientTabModel->fieldIndex("NAME"), Qt::AscendingOrder);

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
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    curRec.setValue("ID", newId);

    patientTabModel->setRecord(curRecNo, curRec);
    addOperationRecord("创建新患者", QString("新患者(ID:%1)").arg(newId));

    return curIndex.row();
}

bool IDatabase::searchPatient(QString filter)
{
    qDebug() << "SearchPatient called with filter:" << filter;
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
        QString patientName = patientTabModel->data(patientTabModel->index(curIndex.row(), 2)).toString();
        QString patientId = patientTabModel->data(patientTabModel->index(curIndex.row(), 0)).toString();

        patientTabModel->removeRow(curIndex.row());
        bool success = patientTabModel->submitAll();
        patientTabModel->select();
        if (success) {
            addOperationRecord("删除患者", QString("%1(ID:%2)").arg(patientName).arg(patientId));
        }

        return success;
    }
    return false;
}

bool IDatabase::submitPatientEdit()
{
    bool success = patientTabModel->submitAll();
    if (success) {
        int row = patientTabModel->rowCount() - 1;
        if (row >= 0) {
            QString patientName = patientTabModel->data(patientTabModel->index(row, 2)).toString();
            QString patientId = patientTabModel->data(patientTabModel->index(row, 0)).toString();
            addOperationRecord("保存患者信息", QString("%1(ID:%2)").arg(patientName).arg(patientId));
        }
    }
    return success;
}
void IDatabase::revertPatientEdit()
{
    patientTabModel->revertAll();
}
bool IDatabase::initDepartmentModel()
{
    departmentTabModel = new QSqlTableModel(this, database);
    departmentTabModel->setTable("Department");
    departmentTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    departmentTabModel->setHeaderData(0, Qt::Horizontal, "科室ID");
    departmentTabModel->setHeaderData(1, Qt::Horizontal, "科室名称");
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
bool IDatabase::initDoctorModel()
{
    doctorTabModel = new QSqlTableModel(this, database);
    doctorTabModel->setTable("Doctor");
    doctorTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    doctorTabModel->setHeaderData(0, Qt::Horizontal, "ID");
    doctorTabModel->setHeaderData(1, Qt::Horizontal, "工号");
    doctorTabModel->setHeaderData(2, Qt::Horizontal, "姓名");
    doctorTabModel->setHeaderData(3, Qt::Horizontal, "科室ID");
    doctorTabModel->setSort(2, Qt::AscendingOrder);

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
