#include "patienceview.h"
#include "ui_patienceview.h"

PatienceView::PatienceView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PatienceView)
{
    ui->setupUi(this);
}

PatienceView::~PatienceView()
{
    delete ui;
}
