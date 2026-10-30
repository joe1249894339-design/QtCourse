#include "finddialog.h"
#include "ui_finddialog.h"

#include <QMessageBox>

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    connect(ui->buttonFind, &QPushButton::clicked,
            this, &FindDialog::onButtonFindClicked);
    connect(ui->buttonCancel, &QPushButton::clicked,
            this, &FindDialog::onButtonCancelClicked);
    connect(ui->lineEditFind, &QLineEdit::returnPressed,
            this, &FindDialog::onButtonFindClicked);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setSearchText(const QString &text)
{
    if (!text.isEmpty())
        ui->lineEditFind->setText(text);
    ui->lineEditFind->selectAll();
}

QString FindDialog::searchText() const
{
    return ui->lineEditFind->text();
}

bool FindDialog::caseSensitive() const
{
    return ui->checkCase->isChecked();
}

bool FindDialog::searchForward() const
{
    return ui->radioDown->isChecked();
}

void FindDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    ui->lineEditFind->setFocus();
    ui->lineEditFind->selectAll();
}

void FindDialog::onButtonFindClicked()
{
    const QString text = ui->lineEditFind->text();
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("查找"), tr("请输入查找内容。"));
        ui->lineEditFind->setFocus();
        return;
    }

    emit findRequested(text, caseSensitive(), searchForward());
}

void FindDialog::onButtonCancelClicked()
{
    hide();
}
