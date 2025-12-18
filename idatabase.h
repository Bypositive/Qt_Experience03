#ifndef IDATABASE_H
#define IDATABASE_H

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>
#include <QDataWidgetMapper>

class IDatabase : public QObject
{
    Q_OBJECT
public:
    static IDatabase &getInstance(){
        static IDatabase instance;
        return instance;
    }

    QString userLogin(QString userName, QString password);
    QString getCurrentUserName();

private:
    void initDatabase();
    explicit IDatabase(QObject *parent = nullptr);
    IDatabase(IDatabase const &)=delete;
    void operator=(IDatabase const &)=delete;
    QSqlDatabase database;
    QString currentUserName;

public:
    // 操作记录管理
    bool initHistoryModel();
    bool addOperationRecord(const QString &event, const QString &details = "");
    bool searchOperationRecord(const QString &filter);
    QSqlTableModel *getHistoryModel();

    // 医生管理
    bool initDoctorModel();
    int addNewDoctor();
    bool searchDoctor(QString filter);
    bool deleteCurrentDoctor();
    bool submitDoctorEdit();
    void revertDoctorEdit();

    // 科室管理
    bool initDepartmentModel();
    int addNewDepartment();
    bool searchDepartment(QString filter);
    bool deleteCurrentDepartment();
    bool submitDepartmentEdit();
    void revertDepartmentEdit();

    QSqlTableModel *doctorTabModel;
    QSqlTableModel *departmentTabModel;
    QSqlTableModel *historyTabModel;
    QItemSelectionModel *theDoctorSelection;
    QItemSelectionModel *theDepartmentSelection;

public:
    bool initPatientModel();
    int addNewPatient();
    bool searchPatient(QString filter);
    bool deleteCurrentPatient();
    bool submitPatientEdit();
    void revertPatientEdit();

    QSqlTableModel *patientTabModel;
    QItemSelectionModel *thePatientSelection;

signals:
};

#endif // IDATABASE_H
