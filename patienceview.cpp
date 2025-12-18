#include "patienceview.h"
#include "ui_patienceview.h"
#include "idatabase.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>

PatienceView::PatienceView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PatienceView)
    , model(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("患者管理");
    initModel();
    connect(ui->txtSearch, &QLineEdit::returnPressed, this, &PatienceView::on_btSearch_clicked);
    connect(ui->btHistory, &QPushButton::clicked, this, &PatienceView::on_btHistory_clicked);
}

PatienceView::~PatienceView()
{
    delete ui;
}
void PatienceView::on_btHistory_clicked()
{
    emit showHistoryRequested();
}

void PatienceView::initModel()
{
    if (IDatabase::getInstance().initPatientModel()) {
        model = IDatabase::getInstance().patientTabModel;
        ui->tableView->setModel(model);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableView->setColumnHidden(0, true);
        ui->tableView->setColumnHidden(8, true);
        ui->tableView->setColumnHidden(9, true);
        ui->tableView->setColumnWidth(1, 150);   // 身份证
        ui->tableView->setColumnWidth(2, 100);   // 姓名
        ui->tableView->setColumnWidth(3, 60);    // 性别
        ui->tableView->setColumnWidth(4, 100);   // 出生日期
        ui->tableView->setColumnWidth(5, 60);    // 身高
        ui->tableView->setColumnWidth(6, 60);    // 体重
        ui->tableView->setColumnWidth(7, 120);   // 手机号
    } else {
        QMessageBox::warning(this, "错误", "无法初始化患者数据模型");
    }
}

void PatienceView::updateTableView()
{
    if (model) {
        model->select();
    }
}

void PatienceView::on_btSearch_clicked()
{
    QString filter = ui->txtSearch->text().trimmed();

    qDebug() << "=== 搜索开始 ===";
    qDebug() << "搜索关键词:" << filter;

    if (filter.isEmpty()) {
        if (IDatabase::getInstance().searchPatient("")) {
            updateTableView();
        }
        return;
    }
    QString condition;

    bool isNumber;
    filter.toLongLong(&isNumber);

    if (isNumber && filter.length() == 18) {
        // 身份证号
        condition = QString("ID_CARD LIKE '%%1%'").arg(filter);
    } else if (isNumber && filter.length() == 11) {
        // 手机号
        condition = QString("MOBILEPHONE LIKE '%%1%'").arg(filter);
    } else if (isNumber) {
        // 其他数字，搜索身份证和手机号
        condition = QString("ID_CARD LIKE '%%1%' OR MOBILEPHONE LIKE '%%1%'").arg(filter);
    } else {
        // 中文姓名
        condition = QString("NAME LIKE '%%1%'").arg(filter);
    }

    qDebug() << "构建的搜索条件:" << condition;
    QSqlQuery testQuery;
    QString testSql = QString("SELECT COUNT(*) as count FROM Patient WHERE %1").arg(condition);
    qDebug() << "测试SQL:" << testSql;

    if (testQuery.exec(testSql) && testQuery.next()) {
        int count = testQuery.value("count").toInt();
        qDebug() << "直接SQL查询找到" << count << "条记录";

        if (count > 0) {
            testQuery.exec(QString("SELECT NAME, ID_CARD, MOBILEPHONE FROM Patient WHERE %1").arg(condition));
            while (testQuery.next()) {
                qDebug() << "找到: 姓名=" << testQuery.value("NAME").toString()
                    << ", 身份证=" << testQuery.value("ID_CARD").toString()
                    << ", 手机=" << testQuery.value("MOBILEPHONE").toString();
            }
        }
    }
    if (IDatabase::getInstance().searchPatient(condition)) {
        int rowCount = model->rowCount();
        qDebug() << "QSqlTableModel找到" << rowCount << "条记录";

        if (rowCount > 0) {
            QMessageBox::information(this, "查找成功",
                                     QString("找到 %1 条匹配记录").arg(rowCount));
            if (rowCount > 0) {
                ui->tableView->selectRow(0);
            }
        } else {
            QMessageBox::information(this, "查找结果",
                                     QString("未找到与 '%1' 匹配的患者").arg(filter));
        }
    } else {
        QMessageBox::warning(this, "查找错误",
                             "搜索失败，请检查数据库连接");
    }

    qDebug() << "=== 搜索结束 ===";
}
void PatienceView::on_btClear_clicked()
{
    ui->txtSearch->clear();

    IDatabase::getInstance().searchPatient("");
    updateTableView();
    ui->tableView->clearSelection();
}

void PatienceView::on_btAdd_clicked()
{
    IDatabase::getInstance().addOperationRecord("开始添加患者");
    emit addPatientRequested();
}

void PatienceView::on_btDelete_clicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的患者");
        return;
    }

    int row = currentIndex.row();
    QString patientName = model->data(model->index(row, 2)).toString();
    QString patientId = model->data(model->index(row, 0)).toString();
    QString patientCard = model->data(model->index(row, 1)).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除患者 '%1' 吗？\n身份证：%2").arg(patientName).arg(patientCard),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().deleteCurrentPatient()) {
            QMessageBox::information(this, "成功", "患者已删除");
            updateTableView();
        } else {
            QMessageBox::warning(this, "错误", "删除失败");
        }
    }
}

void PatienceView::on_btEdit_clicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的患者");
        return;
    }

    int row = currentIndex.row();
    QString patientId = model->data(model->index(row, 0)).toString();
    emit editPatientRequested(patientId);
}

void PatienceView::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        int row = index.row();
        QString patientId = model->data(model->index(row, 0)).toString();
        emit editPatientRequested(patientId);
    }
}
