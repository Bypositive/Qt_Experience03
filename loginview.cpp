#include "loginview.h"
#include "ui_loginview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>

LoginView::LoginView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginView)
{
    ui->setupUi(this);
    ui->inputPassword->setEchoMode(QLineEdit::Password);
    connect(ui->btSignUp, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "注册", "请联系管理员注册账号");
    });
}

LoginView::~LoginView()
{
    delete ui;
}

void LoginView::on_pushButton_clicked()
{
    QString username = ui->inputUserName->text().trimmed();
    QString password = ui->inputPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "登录失败", "用户名和密码不能为空");
        return;
    }

    QString result = IDatabase::getInstance().userLogin(username, password);

    if (result == "loginOk") {
        QMessageBox::information(this, "登录成功", "欢迎使用无极哥医疗诊断系统");

        // 记录登录操作
        IDatabase::getInstance().addOperationRecord("用户登录", QString("用户：%1").arg(username));

        emit loginSuccess();
    } else if (result == "wrongPassword") {
        QMessageBox::warning(this, "登录失败", "密码错误");
        emit loginFailed();
    } else {
        QMessageBox::warning(this, "登录失败", "用户名不存在");
        emit loginFailed();
    }
}
