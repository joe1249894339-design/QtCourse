#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog() override;

    void setSearchText(const QString &text);
    QString searchText() const;
    bool caseSensitive() const;
    bool searchForward() const;

signals:
    void findRequested(const QString &text, bool caseSensitive, bool forward);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onButtonFindClicked();
    void onButtonCancelClicked();

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
