#include "masterview.h"
#include "ui_masterview.h"
#include <QDebug>
#include "idatabase.h"

MasterView::MasterView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MasterView)
    , historyView(nullptr)
{
    ui->setupUi(this);

    goLoginView();
    IDatabase::getInstance();
}

MasterView::~MasterView()
{
    delete ui;
}

void MasterView::goLoginView()
{
    qDebug()<<"goLoginView";
    loginView = new LoginView(this);
    pushWidgetToStackView(loginView);

    connect(loginView, SIGNAL(loginSuccess()), this, SLOT(goWelcomeView()));
}

void MasterView::goWelcomeView()
{
    qDebug()<<"goWelcomeView";
    welcomeView = new WelcomeView(this);
    pushWidgetToStackView(welcomeView);

    // 连接欢迎页面的按钮信号
    connect(welcomeView, &WelcomeView::departmentClicked, this, &MasterView::goDepartmentView);
    connect(welcomeView, &WelcomeView::doctorClicked, this, &MasterView::goDoctorView);
    connect(welcomeView, &WelcomeView::patienceClicked, this, &MasterView::goPatientView);
}

void MasterView::goDoctorView()
{
    qDebug()<<"goDoctorView";
    doctorView = new DoctorView(this);
    pushWidgetToStackView(doctorView);
}

void MasterView::goDepartmentView()
{
    qDebug()<<"goDepartmentView";
    departmentView = new DepartmentView(this);
    pushWidgetToStackView(departmentView);
}

void MasterView::goPatientEditView(const QString &patientId)
{
    qDebug()<<"goPatientEditView, patientId:" << patientId;
    patientEditView = new PatientEditView(this, !patientId.isEmpty(), patientId);
    pushWidgetToStackView(patientEditView);

    // 连接编辑完成信号
    connect(patientEditView, &PatientEditView::editFinished, this, &MasterView::goPreviousView);
}

void MasterView::goPatientView()
{
    qDebug()<<"goPatientView";
    patienceView = new PatienceView(this);
    pushWidgetToStackView(patienceView);

    // 连接患者管理页面的信号
    connect(patienceView, &PatienceView::addPatientRequested, this, [this]() {
        goPatientEditView("");
    });

    connect(patienceView, &PatienceView::editPatientRequested, this, [this](const QString &patientId) {
        goPatientEditView(patientId);
    });

    // 连接操作记录查看信号
    connect(patienceView, &PatienceView::showHistoryRequested, this, &MasterView::goHistoryView);
}
void MasterView::goHistoryView()
{
    qDebug() << "goHistoryView";
    historyView = new HistoryView(this);
    pushWidgetToStackView(historyView);
}

void MasterView::goPreviousView()
{
    int count = ui->stackedWidget->count();
    if(count > 1){
        ui->stackedWidget->setCurrentIndex(count - 2);
        ui->labelTitle->setText(ui->stackedWidget->currentWidget()->windowTitle());

        QWidget *widget = ui->stackedWidget->widget(count - 1);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }
}

void MasterView::pushWidgetToStackView(QWidget *widget)
{
    ui->stackedWidget->addWidget(widget);
    int count = ui->stackedWidget->count();
    ui->stackedWidget->setCurrentIndex(count - 1);
    ui->labelTitle->setText(widget->windowTitle());
}

void MasterView::on_btBack_clicked()
{
    goPreviousView();
}

void MasterView::on_stackedWidget_currentChanged(int arg1)
{
    int count = ui->stackedWidget->count();
    if(count > 1)
        ui->btBack->setEnabled(true);
    else
        ui->btBack->setEnabled(false);

    QString title = ui->stackedWidget->currentWidget()->windowTitle();
    if(title == "欢迎登录无极哥诊断系统"){
        ui->btLogout->setEnabled(true);
        ui->btBack->setEnabled(false);
    } else {
        ui->btLogout->setEnabled(false);
    }
}

void MasterView::on_btLogout_clicked()
{
    goPreviousView();
}
