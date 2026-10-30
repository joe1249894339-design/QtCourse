#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QPixmap>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    const QPixmap logo(QStringLiteral(":/icons/app.png"));
    if (!logo.isNull()) {
        ui->logoLabel->setPixmap(logo.scaled(ui->logoLabel->size(),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
    }

    connect(ui->buttonOK, &QPushButton::clicked, this, &QDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
