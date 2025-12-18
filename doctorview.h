#ifndef DOCTORVIEW_H
#define DOCTORVIEW_H

#include <QWidget>
#include <QSqlTableModel>

namespace Ui {
class DoctorView;
}

class DoctorView : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorView(QWidget *parent = nullptr);
    ~DoctorView();

private slots:
    void on_btSearch_clicked();
    void on_btClear_clicked();
    void on_btAdd_clicked();
    void on_btDelete_clicked();
    void on_btEdit_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::DoctorView *ui;
    QSqlTableModel *model;
    void initModel();
    void updateTableView();
};

#endif // DOCTORVIEW_H
