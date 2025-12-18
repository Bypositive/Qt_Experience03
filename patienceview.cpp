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
    initModel();
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

        // 隐藏不需要的列（ID和AGE字段）
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
    if (!filter.isEmpty()) {
        QString condition = QString("name LIKE '%%1%' OR idcard LIKE '%%1%' OR mobile LIKE '%%1%'")
        .arg(filter);
        IDatabase::getInstance().searchPatient(condition);
    } else {
        IDatabase::getInstance().searchPatient("");
    }
}

void PatienceView::on_btAdd_clicked()
{
    // 切换到患者编辑视图

}

void PatienceView::on_btDelete_clicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的患者");
        return;
    }

    int row = currentIndex.row();
    QString patientName = model->data(model->index(row, 1)).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除患者 '%1' 吗？").arg(patientName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        IDatabase::getInstance().deleteCurrentPatient();
        updateTableView();
    }
}

void PatienceView::on_btEdit_clicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的患者");
        return;
    }

    // 切换到患者编辑视图，并传递当前选中的患者ID
    int row = currentIndex.row();
    QString patientId = model->data(model->index(row, 0)).toString();
}

void PatienceView::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        QString patientId = model->data(model->index(index.row(), 0)).toString();

    }
}
