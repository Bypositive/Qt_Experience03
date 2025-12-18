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

        model->setSort(0, Qt::AscendingOrder);
        model->select();

        ui->tableView->setModel(model);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

        for (int i = 0; i < model->columnCount(); ++i) {
            ui->tableView->setColumnHidden(i, false);
        }

        ui->tableView->setColumnWidth(0, 200);
        ui->tableView->setColumnWidth(1, 250);
    } else {
        QMessageBox::warning(this, "错误", "无法初始化科室数据模型");
    }
}

void DepartmentView::updateTableView()
{
    if (model) {
        model->setSort(0, Qt::AscendingOrder);
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
    bool okId;
    QString departmentId = QInputDialog::getText(this, "添加科室",
                                                 "请输入科室ID:",
                                                 QLineEdit::Normal, "", &okId);

    if (!okId || departmentId.trimmed().isEmpty()) {
        return;
    }
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM Department WHERE ID = :id");
    query.bindValue(":id", departmentId.trimmed());

    if (query.exec() && query.next()) {
        if (query.value(0).toInt() > 0) {
            QMessageBox::warning(this, "错误",
                                 QString("科室ID '%1' 已存在，请使用其他ID").arg(departmentId.trimmed()));
            return;
        }
    } else {
        QMessageBox::warning(this, "错误", "检查ID时发生错误");
        return;
    }

    // 获取科室名称
    bool okName;
    QString departmentName = QInputDialog::getText(this, "添加科室",
                                                   "请输入科室名称:",
                                                   QLineEdit::Normal, "", &okName);

    if (okName && !departmentName.trimmed().isEmpty()) {
        int row = IDatabase::getInstance().addNewDepartment();
        model->setData(model->index(row, 0), departmentId.trimmed());    // 设置ID到第0列
        model->setData(model->index(row, 1), departmentName.trimmed());  // 设置名称到第1列

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
void DepartmentView::on_btStats_clicked()
{
    QSqlQuery query;
    query.prepare("SELECT d.NAME as 科室名称, COUNT(doc.ID) as 医生数量 "
                  "FROM Department d "
                  "LEFT JOIN Doctor doc ON d.ID = doc.DEPARTMENT_ID "
                  "GROUP BY d.ID, d.NAME "
                  "ORDER BY COUNT(doc.ID) DESC");

    if (query.exec()) {
        QString stats = "科室医生数量统计:\n\n";
        int totalDoctors = 0;

        while (query.next()) {
            QString deptName = query.value("科室名称").toString();
            int doctorCount = query.value("医生数量").toInt();
            totalDoctors += doctorCount;
            stats += QString("%1: %2 名医生\n").arg(deptName).arg(doctorCount);
        }

        stats += QString("\n总计: %1 名医生").arg(totalDoctors);
        QMessageBox::information(this, "统计信息", stats);
    } else {
        QMessageBox::warning(this, "错误", "统计失败");
    }
}
