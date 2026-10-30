#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

namespace Ui {
class ReplaceDialog;
}

class ReplaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog() override;

    void setSearchText(const QString &text);
    QString searchText() const;
    QString replacementText() const;
    bool caseSensitive() const;
    bool searchForward() const;

signals:
    void findRequested(const QString &text, bool caseSensitive, bool forward);
    void replaceRequested(const QString &text, const QString &replacement,
                          bool caseSensitive, bool forward);
    void replaceAllRequested(const QString &text, const QString &replacement,
                             bool caseSensitive);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onButtonFindClicked();
    void onButtonReplaceClicked();
    void onButtonReplaceAllClicked();
    void onButtonCancelClicked();

private:
    bool validateSearchText();
    Ui::ReplaceDialog *ui;
};

#endif // REPLACEDIALOG_H
