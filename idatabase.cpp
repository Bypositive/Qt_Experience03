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
