#ifndef DEPARTMENTVIEW_H
#define DEPARTMENTVIEW_H

#include <QWidget>
#include <QSqlTableModel>

namespace Ui {
class DepartmentView;
}

class DepartmentView : public QWidget
{
    Q_OBJECT

public:
    explicit DepartmentView(QWidget *parent = nullptr);
    ~DepartmentView();

signals:
    void goBack();

private slots:
    void on_btStats_clicked();
    void on_btSearch_clicked();
    void on_btClear_clicked();
    void on_btAdd_clicked();
    void on_btDelete_clicked();
    void on_btEdit_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::DepartmentView *ui;
    QSqlTableModel *model;
    void initModel();
    void updateTableView();
};

#endif // DEPARTMENTVIEW_H
