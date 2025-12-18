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

    // 设置密码输入框为密码模式
    ui->inputPassword->setEchoMode(QLineEdit::Password);

    // 连接按钮信号
    connect(ui->pushButton, &QPushButton::clicked, this, &LoginView::on_pushButton_clicked);
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
        emit loginSuccess();
    } else if (result == "wrongPassword") {
        QMessageBox::warning(this, "登录失败", "密码错误");
        emit loginFailed();
    } else {
        QMessageBox::warning(this, "登录失败", "用户名不存在");
        emit loginFailed();
    }
}
