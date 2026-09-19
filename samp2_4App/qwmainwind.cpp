#include "qwmainwind.h"
#include "ui_qwmainwind.h"
#include <QMessageBox>
#include <QToolBar>

QWMainWind::QWMainWind(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QWMainWind)
{
    ui->setupUi(this);

    // 创建【关于】按钮
    actAbout = new QAction("关于", this);
    // 把按钮加到工具栏
    ui->mainToolBar->addAction(actAbout);
    // 绑定点击事件
    connect(actAbout, &QAction::triggered, this, &QWMainWind::on_actAbout_triggered);
}

QWMainWind::~QWMainWind()
{
    delete ui;
}

void QWMainWind::on_actAbout_triggered()
{
    QMessageBox::about(this, "samp2_4",
                       "开发人员信息：黄俊豪\n学号：2021463050509");
}
