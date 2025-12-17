#ifndef PATIENCEVIEW_H
#define PATIENCEVIEW_H

#include <QWidget>

namespace Ui {
class PatienceView;
}

class PatienceView : public QWidget
{
    Q_OBJECT

public:
    explicit PatienceView(QWidget *parent = nullptr);
    ~PatienceView();

private:
    Ui::PatienceView *ui;
};

#endif // PATIENCEVIEW_H
