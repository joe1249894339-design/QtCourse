#include "replacedialog.h"
#include "ui_replacedialog.h"

#include <QMessageBox>

ReplaceDialog::ReplaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    connect(ui->buttonFind, &QPushButton::clicked,
            this, &ReplaceDialog::onButtonFindClicked);
    connect(ui->buttonReplace, &QPushButton::clicked,
            this, &ReplaceDialog::onButtonReplaceClicked);
    connect(ui->buttonReplaceAll, &QPushButton::clicked,
            this, &ReplaceDialog::onButtonReplaceAllClicked);
    connect(ui->buttonCancel, &QPushButton::clicked,
            this, &ReplaceDialog::onButtonCancelClicked);
    connect(ui->lineEditFind, &QLineEdit::returnPressed,
            this, &ReplaceDialog::onButtonFindClicked);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::setSearchText(const QString &text)
{
    if (!text.isEmpty())
        ui->lineEditFind->setText(text);
    ui->lineEditFind->selectAll();
}

QString ReplaceDialog::searchText() const
{
    return ui->lineEditFind->text();
}

QString ReplaceDialog::replacementText() const
{
    return ui->lineEditReplace->text();
}

bool ReplaceDialog::caseSensitive() const
{
    return ui->checkCase->isChecked();
}

bool ReplaceDialog::searchForward() const
{
    return ui->radioDown->isChecked();
}

void ReplaceDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    ui->lineEditFind->setFocus();
    ui->lineEditFind->selectAll();
}

bool ReplaceDialog::validateSearchText()
{
    if (ui->lineEditFind->text().isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入查找内容。"));
        ui->lineEditFind->setFocus();
        return false;
    }
    return true;
}

void ReplaceDialog::onButtonFindClicked()
{
    if (!validateSearchText())
        return;
    emit findRequested(searchText(), caseSensitive(), searchForward());
}

void ReplaceDialog::onButtonReplaceClicked()
{
    if (!validateSearchText())
        return;
    emit replaceRequested(searchText(), replacementText(),
                          caseSensitive(), searchForward());
}

void ReplaceDialog::onButtonReplaceAllClicked()
{
    if (!validateSearchText())
        return;
    emit replaceAllRequested(searchText(), replacementText(), caseSensitive());
}

void ReplaceDialog::onButtonCancelClicked()
{
    hide();
}
