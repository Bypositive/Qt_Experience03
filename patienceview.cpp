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

    // 连接按钮信号
    connect(ui->btSearch, &QPushButton::clicked, this, &PatienceView::on_btSearch_clicked);
    connect(ui->btClear, &QPushButton::clicked, this, &PatienceView::on_btClear_clicked);
    connect(ui->btAdd, &QPushButton::clicked, this, &PatienceView::on_btAdd_clicked);
    connect(ui->btDelete, &QPushButton::clicked, this, &PatienceView::on_btDelete_clicked);
    connect(ui->btEdit, &QPushButton::clicked, this, &PatienceView::on_btEdit_clicked);
    connect(ui->tableView, &QTableView::doubleClicked, this, &PatienceView::on_tableView_doubleClicked);
    connect(ui->txtSearch, &QLineEdit::returnPressed, this, &PatienceView::on_btSearch_clicked);

    // 按Enter键搜索
    connect(ui->txtSearch, &QLineEdit::returnPressed, this, &PatienceView::on_btSearch_clicked);
}

PatienceView::~PatienceView()
{
    delete ui;
}

void PatienceView::initModel()
{
    if (IDatabase::getInstance().initPatientModel()) {
        model = IDatabase::getInstance().patientTabModel;
        ui->tableView->setModel(model);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

        // 隐藏不需要的列
        ui->tableView->setColumnHidden(0, true);    // 隐藏ID列
        ui->tableView->setColumnHidden(8, true);    // 隐藏AGE列
        ui->tableView->setColumnHidden(9, true);    // 隐藏CREATEDTIMESTAMP列

        // 调整列宽
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

    if (filter.isEmpty()) {
        // 清除搜索，显示所有患者
        IDatabase::getInstance().searchPatient("");
        updateTableView();
        ui->tableView->clearSelection();
        return;
    }

    // 检查输入内容类型，提供更精确的搜索
    QString condition;

    // 如果是纯数字，可能是身份证号或手机号
    bool isNumber;
    filter.toLongLong(&isNumber);

    if (isNumber) {
        // 纯数字：检查是身份证号(18位)还是手机号(11位)
        if (filter.length() == 18) {
            // 身份证号
            condition = QString("ID_CARD LIKE '%1%'").arg(filter);
        } else if (filter.length() == 11) {
            // 手机号
            condition = QString("MOBILEPHONE LIKE '%1%'").arg(filter);
        } else {
            // 其他数字，在所有数字字段中查找
            condition = QString("ID_CARD LIKE '%1%' OR MOBILEPHONE LIKE '%1%'")
                            .arg(filter);
        }
    } else {
        // 包含非数字，主要是姓名
        condition = QString("NAME LIKE '%%1%'").arg(filter);
    }

    qDebug() << "Search condition:" << condition;

    if (IDatabase::getInstance().searchPatient(condition)) {
        int rowCount = model->rowCount();
        if (rowCount == 0) {
            QMessageBox::information(this, "查找结果", "未找到匹配的患者");
        } else {
            QMessageBox::information(this, "查找结果",
                                     QString("找到 %1 条匹配记录").arg(rowCount));

            // 选中第一条记录
            if (rowCount > 0) {
                ui->tableView->selectRow(0);
            }
        }
    } else {
        QMessageBox::warning(this, "查找错误", "搜索失败");
    }
}
void PatienceView::on_btClear_clicked()
{
    // 清除搜索框
    ui->txtSearch->clear();

    // 显示所有患者
    IDatabase::getInstance().searchPatient("");
    updateTableView();

    // 清除选择
    ui->tableView->clearSelection();
}

void PatienceView::on_btAdd_clicked()
{
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
    QString patientName = model->data(model->index(row, 2)).toString(); // NAME在第2列

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除患者 '%1' 吗？").arg(patientName),
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
    QString patientId = model->data(model->index(row, 0)).toString(); // ID在第0列
    emit editPatientRequested(patientId);
}

void PatienceView::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        int row = index.row();
        QString patientId = model->data(model->index(row, 0)).toString(); // ID在第0列
        emit editPatientRequested(patientId);
    }
}
