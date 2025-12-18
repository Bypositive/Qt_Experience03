#ifndef HISTORYVIEW_H
#define HISTORYVIEW_H

#include <QWidget>
#include <QSqlTableModel>

namespace Ui {
class HistoryView;
}

class HistoryView : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryView(QWidget *parent = nullptr);
    ~HistoryView();

private slots:
    void on_btSearch_clicked();
    void on_btClear_clicked();
    void on_btRefresh_clicked();
    void on_btExport_clicked();

private:
    Ui::HistoryView *ui;
    QSqlTableModel *model;
    void initModel();
    void updateTableView();
};

#endif // HISTORYVIEW_H
