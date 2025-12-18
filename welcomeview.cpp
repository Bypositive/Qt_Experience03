#include "welcomeview.h"
#include "ui_welcomeview.h"
#include <QDebug>

WelcomeView::WelcomeView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WelcomeView)
{
    qDebug()<<"create WelcomeView";
    ui->setupUi(this);
    setWindowTitle("欢迎登录无极哥诊断系统");
    connect(ui->department, &QPushButton::clicked, this, &WelcomeView::on_btDepartment_clicked);
    connect(ui->doctor, &QPushButton::clicked, this, &WelcomeView::on_btDoctor_clicked);
    connect(ui->patience, &QPushButton::clicked, this, &WelcomeView::on_btPatience_clicked);
}

WelcomeView::~WelcomeView()
{
    qDebug()<<"destroy WelcomeView";
    delete ui;
}

void WelcomeView::on_btDepartment_clicked()
{
    qDebug() << "Department button clicked";
    emit departmentClicked();
}

void WelcomeView::on_btDoctor_clicked()
{
    qDebug() << "Doctor button clicked";
    emit doctorClicked();
}

void WelcomeView::on_btPatience_clicked()
{
    qDebug() << "Patience button clicked";
    emit patienceClicked();
}
