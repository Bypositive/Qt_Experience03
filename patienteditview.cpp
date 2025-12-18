#include "patienteditview.h"
#include "ui_patienteditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>
#include <QDate>

PatientEditView::PatientEditView(QWidget *parent, bool isEditMode, const QString &patientId)
    : QWidget(parent)
    , ui(new Ui::PatientEditView)
    , editMode(isEditMode)
    , currentPatientId(patientId)
    , mapper(nullptr)
{
    ui->setupUi(this);
    initUI();

    if (editMode && !currentPatientId.isEmpty()) {
        setWindowTitle("编辑患者信息");
        loadPatientData();
    } else {
        setWindowTitle("添加新患者");
    }
}

PatientEditView::~PatientEditView()
{
    delete ui;
}

void PatientEditView::initUI()
{
    ui->sex->addItem("男");
    ui->sex->addItem("女");
    QDate currentDate = QDate::currentDate();
    for (int year = 1900; year <= currentDate.year(); year++) {
        ui->birthday->addItem(QString::number(year) + "-01-01");
    }
    ui->birthday->setEditable(true);
    for (int i = 140; i <= 220; i++) {
        ui->Height->addItem(QString::number(i));
    }
    ui->Height->setCurrentText("170");
    ui->Height->setEditable(true);
    for (int i = 30; i <= 150; i++) {
        ui->Weigh->addItem(QString::number(i));
    }
    ui->Weigh->setCurrentText("65");
    ui->Weigh->setEditable(true);
    connect(ui->save, &QPushButton::clicked, this, &PatientEditView::on_save_clicked);
    connect(ui->cancel, &QPushButton::clicked, this, &PatientEditView::on_cancel_clicked);
}

void PatientEditView::setPatientData(const QString &patientId)
{
    currentPatientId = patientId;
    loadPatientData();
}

void PatientEditView::loadPatientData()
{
    if (!IDatabase::getInstance().initPatientModel()) {
        QMessageBox::warning(this, "错误", "无法加载患者数据");
        return;
    }

    QSqlTableModel *model = IDatabase::getInstance().patientTabModel;
    for (int i = 0; i < model->rowCount(); i++) {
        if (model->data(model->index(i, 0)).toString() == currentPatientId) {
            mapper = new QDataWidgetMapper(this);
            mapper->setModel(model);
            mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
            mapper->addMapping(ui->ID, 0);              // ID
            mapper->addMapping(ui->shenfenzheng, 1);   // ID_CARD
            mapper->addMapping(ui->Name, 2);           // NAME
            mapper->addMapping(ui->sex, 3);            // SEX
            mapper->addMapping(ui->birthday, 4);       // DOB
            mapper->addMapping(ui->Height, 5);         // HEIGHT
            mapper->addMapping(ui->Weigh, 6);          // WEIGHT
            mapper->addMapping(ui->MobilePhone, 7);    // MOBILEPHONE

            mapper->setCurrentIndex(i);
            break;
        }
    }
}

bool PatientEditView::validateInput()
{
    if (ui->Name->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入患者姓名");
        ui->Name->setFocus();
        return false;
    }

    if (ui->shenfenzheng->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入身份证号码");
        ui->shenfenzheng->setFocus();
        return false;
    }

    if (ui->MobilePhone->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入手机号码");
        ui->MobilePhone->setFocus();
        return false;
    }

    return true;
}

void PatientEditView::savePatientData()
{
    if (!validateInput()) {
        return;
    }

    QString patientName = ui->Name->text().trimmed();
    QString patientId = ui->ID->text().trimmed();

    if (editMode && mapper) {
        if (mapper->submit()) {
            if (IDatabase::getInstance().submitPatientEdit()) {
                QMessageBox::information(this, "成功", "患者信息已更新");
                IDatabase::getInstance().addOperationRecord(
                    "修改患者信息",
                    QString("患者：%1 (ID:%2)").arg(patientName).arg(patientId)
                    );

                emit editFinished();
            } else {
                QMessageBox::warning(this, "错误", "更新失败: " +
                                                       IDatabase::getInstance().patientTabModel->lastError().text());
            }
        }
    } else {
        int row = IDatabase::getInstance().addNewPatient();
        QSqlTableModel *model = IDatabase::getInstance().patientTabModel;
        model->setData(model->index(row, 2), ui->Name->text().trimmed());      // NAME
        model->setData(model->index(row, 1), ui->shenfenzheng->text().trimmed()); // ID_CARD
        model->setData(model->index(row, 3), ui->sex->currentText());          // SEX
        model->setData(model->index(row, 4), ui->birthday->currentText());     // DOB
        model->setData(model->index(row, 5), ui->Height->currentText());       // HEIGHT
        model->setData(model->index(row, 6), ui->Weigh->currentText());        // WEIGHT
        model->setData(model->index(row, 7), ui->MobilePhone->text().trimmed()); // MOBILEPHONE

        if (IDatabase::getInstance().submitPatientEdit()) {
            QMessageBox::information(this, "成功", "患者信息已添加");

            QString newPatientId = model->data(model->index(row, 0)).toString();

            IDatabase::getInstance().addOperationRecord(
                "添加新患者",
                QString("患者：%1 (ID:%2)").arg(patientName).arg(newPatientId)
                );

            emit editFinished();
        } else {
            QMessageBox::warning(this, "错误", "保存失败: " + model->lastError().text());
            IDatabase::getInstance().revertPatientEdit();
        }
    }
}

void PatientEditView::on_save_clicked()
{
    savePatientData();
}

void PatientEditView::on_cancel_clicked()
{
    if (mapper) {
        mapper->revert();
    }
    IDatabase::getInstance().revertPatientEdit();
    emit editFinished();
}
