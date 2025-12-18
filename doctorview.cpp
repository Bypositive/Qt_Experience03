#include "doctorview.h"
#include "ui_doctorview.h"
#include "idatabase.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>

DoctorView::DoctorView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorView)
    , model(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("医生管理");
    initModel();
}

DoctorView::~DoctorView()
{
    delete ui;
}

void DoctorView::initModel()
{
    if (IDatabase::getInstance().initDoctorModel()) {
        model = IDatabase::getInstance().doctorTabModel;
        model->setSort(0, Qt::AscendingOrder);
        model->select();
        ui->tableView->setModel(model);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

        ui->tableView->setColumnWidth(1, 100);   // 工号
        ui->tableView->setColumnWidth(2, 120);   // 姓名
        ui->tableView->setColumnWidth(3, 150);   // 科室ID
    } else {
        QMessageBox::warning(this, "错误", "无法初始化医生数据模型");
    }
}

void DoctorView::updateTableView()
{
    if (model) {
        model->setSort(0, Qt::AscendingOrder);
        model->select();
    }
}

void DoctorView::on_btSearch_clicked()
{
    QString filter = ui->txtSearch->text().trimmed();

    if (filter.isEmpty()) {
        IDatabase::getInstance().searchDoctor("");
        updateTableView();
        return;
    }

    if (IDatabase::getInstance().searchDoctor(filter)) {
        int rowCount = model->rowCount();
        if (rowCount > 0) {
            QMessageBox::information(this, "查找成功",
                                     QString("找到 %1 条匹配记录").arg(rowCount));
        } else {
            QMessageBox::information(this, "查找结果",
                                     QString("未找到与 '%1' 匹配的医生").arg(filter));
        }
    } else {
        QMessageBox::warning(this, "查找错误", "搜索失败");
    }
}

void DoctorView::on_btClear_clicked()
{
    ui->txtSearch->clear();
    IDatabase::getInstance().searchDoctor("");
    updateTableView();
    ui->tableView->clearSelection();
}

void DoctorView::on_btAdd_clicked()
{
    bool ok1, ok2, ok3;
    QString employeeNo = QInputDialog::getText(this, "添加医生",
                                               "请输入工号:",
                                               QLineEdit::Normal, "", &ok1);

    if (!ok1 || employeeNo.trimmed().isEmpty()) return;

    QString doctorName = QInputDialog::getText(this, "添加医生",
                                               "请输入医生姓名:",
                                               QLineEdit::Normal, "", &ok2);

    if (!ok2 || doctorName.trimmed().isEmpty()) return;

    QString departmentId = QInputDialog::getText(this, "添加医生",
                                                 "请输入科室ID:",
                                                 QLineEdit::Normal, "", &ok3);

    if (ok1 && ok2 && ok3) {
        int row = IDatabase::getInstance().addNewDoctor();
        model->setData(model->index(row, 1), employeeNo.trimmed());
        model->setData(model->index(row, 2), doctorName.trimmed());
        model->setData(model->index(row, 3), departmentId.trimmed());

        if (IDatabase::getInstance().submitDoctorEdit()) {
            QMessageBox::information(this, "成功", "医生信息已添加");
            updateTableView();
        } else {
            QMessageBox::warning(this, "错误", "添加失败");
            IDatabase::getInstance().revertDoctorEdit();
        }
    }
}

void DoctorView::on_btDelete_clicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的医生");
        return;
    }

    int row = currentIndex.row();
    QString doctorName = model->data(model->index(row, 2)).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除医生 '%1' 吗？").arg(doctorName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().deleteCurrentDoctor()) {
            QMessageBox::information(this, "成功", "医生信息已删除");
            updateTableView();
        } else {
            QMessageBox::warning(this, "错误", "删除失败");
        }
    }
}

void DoctorView::on_btEdit_clicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的医生");
        return;
    }

    int row = currentIndex.row();
    QString currentEmployeeNo = model->data(model->index(row, 1)).toString();
    QString currentName = model->data(model->index(row, 2)).toString();
    QString currentDeptId = model->data(model->index(row, 3)).toString();
    bool ok1, ok2, ok3;
    QString employeeNo = QInputDialog::getText(this, "编辑医生信息",
                                               "工号:",
                                               QLineEdit::Normal, currentEmployeeNo, &ok1);

    QString doctorName = QInputDialog::getText(this, "编辑医生信息",
                                               "姓名:",
                                               QLineEdit::Normal, currentName, &ok2);

    QString departmentId = QInputDialog::getText(this, "编辑医生信息",
                                                 "科室ID:",
                                                 QLineEdit::Normal, currentDeptId, &ok3);

    if (ok1 && ok2 && ok3) {
        model->setData(model->index(row, 1), employeeNo.trimmed());
        model->setData(model->index(row, 2), doctorName.trimmed());
        model->setData(model->index(row, 3), departmentId.trimmed());

        if (IDatabase::getInstance().submitDoctorEdit()) {
            QMessageBox::information(this, "成功", "医生信息已更新");
            updateTableView();
        } else {
            QMessageBox::warning(this, "错误", "更新失败");
            IDatabase::getInstance().revertDoctorEdit();
        }
    }
}

void DoctorView::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        on_btEdit_clicked();
    }
}
