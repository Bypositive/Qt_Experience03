#include "masterview.h"
#include "ui_masterview.h"
#include <QDebug>

MasterView::MasterView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MasterView)
{
    ui->setupUi(this);

    goLoginView();
}

MasterView::~MasterView()
{
    delete ui;
}

void MasterView::goLoginView()
{
    qDebug()<<"goLoginView";
    loginView =new LoginView(this);
    ui->stackedWidget->addWidget(loginView);

}

void MasterView::goWelcomeView()
{

}

void MasterView::goDoctorView()
{

}

void MasterView::goDepartmentView()
{

}

void MasterView::goPatientEditView()
{

}

void MasterView::goPatientView()
{

}

void MasterView::goPreviousView()
{

}
