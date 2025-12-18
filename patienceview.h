#ifndef PATIENCEVIEW_H
#define PATIENCEVIEW_H

#include <QWidget>
#include <QSqlTableModel>

namespace Ui {
class PatienceView;
}

class PatienceView : public QWidget
{
    Q_OBJECT

public:
    explicit PatienceView(QWidget *parent = nullptr);
    ~PatienceView();

signals:
    void addPatientRequested();
    void editPatientRequested(const QString &patientId);
    void goBack();

private slots:
    void on_btSearch_clicked();
    void on_btAdd_clicked();
    void on_btDelete_clicked();
    void on_btEdit_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::PatienceView *ui;
    QSqlTableModel *model;
    void initModel();
    void updateTableView();
};

#endif // PATIENCEVIEW_H
