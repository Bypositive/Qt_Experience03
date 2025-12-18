#include "historyview.h"
#include "ui_historyview.h"
#include "idatabase.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QLabel>

HistoryView::HistoryView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HistoryView)
    , model(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("操作记录");
    initModel();
    QLabel *developerLabel = new QLabel("操作记录查看 | 开发者：张三 学号：20230001", this);
    developerLabel->setGeometry(QRect(20, this->height() - 40, 400, 30));
    developerLabel->setStyleSheet("color: gray; font-size: 12px;");
}

HistoryView::~HistoryView()
{
    delete ui;
}

void HistoryView::initModel()
{
    if (IDatabase::getInstance().initHistoryModel()) {
        model = IDatabase::getInstance().getHistoryModel();
        ui->tableView->setModel(model);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableView->setColumnWidth(0, 200);   // 记录ID
        ui->tableView->setColumnWidth(1, 100);   // 操作用户
        ui->tableView->setColumnWidth(2, 300);   // 操作事件
        ui->tableView->setColumnWidth(3, 150);   // 操作时间
        ui->tableView->setColumnHidden(0, true);
    } else {
        QMessageBox::warning(this, "错误", "无法初始化操作记录数据模型");
    }
}

void HistoryView::updateTableView()
{
    if (model) {
        model->select();
    }
}

void HistoryView::on_btSearch_clicked()
{
    QString filter = ui->txtSearch->text().trimmed();

    if (filter.isEmpty()) {
        IDatabase::getInstance().searchOperationRecord("");
        updateTableView();
        return;
    }

    if (IDatabase::getInstance().searchOperationRecord(filter)) {
        int rowCount = model->rowCount();
        if (rowCount > 0) {
            QMessageBox::information(this, "查找成功",
                                     QString("找到 %1 条匹配记录").arg(rowCount));
        } else {
            QMessageBox::information(this, "查找结果",
                                     QString("未找到与 '%1' 匹配的操作记录").arg(filter));
        }
    } else {
        QMessageBox::warning(this, "查找错误", "搜索失败");
    }
}

void HistoryView::on_btClear_clicked()
{
    ui->txtSearch->clear();
    IDatabase::getInstance().searchOperationRecord("");
    updateTableView();
    ui->tableView->clearSelection();
}

void HistoryView::on_btRefresh_clicked()
{
    updateTableView();
    QMessageBox::information(this, "刷新", "操作记录已刷新");
}

void HistoryView::on_btExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出操作记录",
                                                    "操作记录_" + QDateTime::currentDateTime().toString("yyyyMMdd") + ".csv",
                                                    "CSV文件 (*.csv)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建文件");

        return;
    }

    QTextStream out(&file);
    out << "操作用户,操作事件,操作时间\n";
    for (int row = 0; row < model->rowCount(); row++) {
        QString userName = model->data(model->index(row, 1)).toString();
        QString event = model->data(model->index(row, 2)).toString();
        QString timestamp = model->data(model->index(row, 3)).toString();

        out << "\"" << userName << "\",\"" << event << "\",\"" << timestamp << "\"\n";
    }

    file.close();
    QMessageBox::information(this, "成功", "操作记录已导出到: " + fileName);
}
