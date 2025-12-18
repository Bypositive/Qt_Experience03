#include "departmentview.h"
#include "ui_departmentview.h"
#include "idatabase.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>

DepartmentView::DepartmentView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DepartmentView)
    , model(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("科室管理");
    initModel();
}

DepartmentView::~DepartmentView()
{
    delete ui;
}

void DepartmentView::initModel()
{
    if (IDatabase::getInstance().initDepartmentModel()) {
        model = IDatabase::getInstance().departmentTabModel;
        ui->tableView->setModel(model);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

        // 调整列宽
        ui->tableView->setColumnWidth(0, 200);   // 科室ID
        ui->tableView->setColumnWidth(1, 250);   // 科室名称
    } else {
        QMessageBox::warning(this, "错误", "无法初始化科室数据模型");
    }
}

void DepartmentView::updateTableView()
{
    if (model) {
        model->select();
    }
}

void DepartmentView::on_btSearch_clicked()
{
    QString filter = ui->txtSearch->text().trimmed();

    if (filter.isEmpty()) {
        IDatabase::getInstance().searchDepartment("");
        updateTableView();
        return;
    }

    if (IDatabase::getInstance().searchDepartment(filter)) {
        int rowCount = model->rowCount();
        if (rowCount > 0) {
            QMessageBox::information(this, "查找成功",
                                     QString("找到 %1 条匹配记录").arg(rowCount));
        } else {
            QMessageBox::information(this, "查找结果",
                                     QString("未找到与 '%1' 匹配的科室").arg(filter));
        }
    } else {
        QMessageBox::warning(this, "查找错误", "搜索失败");
    }
}

void DepartmentView::on_btClear_clicked()
{
    ui->txtSearch->clear();
    IDatabase::getInstance().searchDepartment("");
    updateTableView();
    ui->tableView->clearSelection();
}

void DepartmentView::on_btAdd_clicked()
{
    bool ok;
    QString departmentName = QInputDialog::getText(this, "添加科室",
                                                   "请输入科室名称:",
                                                   QLineEdit::Normal, "", &ok);

    if (ok && !departmentName.trimmed().isEmpty()) {
        int row = IDatabase::getInstance().addNewDepartment();
        model->setData(model->index(row, 1), departmentName.trimmed());

        if (IDatabase::getInstance().submitDepartmentEdit()) {
            QMessageBox::information(this, "成功", "科室已添加");
            updateTableView();
        } else {
            QMessageBox::warning(this, "错误", "添加失败");
            IDatabase::getInstance().revertDepartmentEdit();
        }
    }
}

void DepartmentView::on_btDelete_clicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的科室");
        return;
    }

    int row = currentIndex.row();
    QString departmentName = model->data(model->index(row, 1)).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除科室 '%1' 吗？").arg(departmentName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().deleteCurrentDepartment()) {
            QMessageBox::information(this, "成功", "科室已删除");
            updateTableView();
        } else {
            QMessageBox::warning(this, "错误", "删除失败");
        }
    }
}

void DepartmentView::on_btEdit_clicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的科室");
        return;
    }

    int row = currentIndex.row();
    QString currentName = model->data(model->index(row, 1)).toString();

    bool ok;
    QString newName = QInputDialog::getText(this, "编辑科室",
                                            "请输入新的科室名称:",
                                            QLineEdit::Normal, currentName, &ok);

    if (ok && !newName.trimmed().isEmpty() && newName != currentName) {
        model->setData(model->index(row, 1), newName.trimmed());

        if (IDatabase::getInstance().submitDepartmentEdit()) {
            QMessageBox::information(this, "成功", "科室信息已更新");
            updateTableView();
        } else {
            QMessageBox::warning(this, "错误", "更新失败");
            IDatabase::getInstance().revertDepartmentEdit();
        }
    }
}

void DepartmentView::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        on_btEdit_clicked();
    }
}
