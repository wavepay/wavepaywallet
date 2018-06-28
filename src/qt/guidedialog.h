#ifndef GUIDEDIALOG_H
#define GUIDEDIALOG_H

#include <QDialog>

namespace Ui {
    class GuideDialog;
}
class ClientModel;

/** "Guide" dialog box */
class GuideDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GuideDialog(QWidget *parent = 0);
    ~GuideDialog();

    void setModel(ClientModel *model);
private:
    Ui::GuideDialog *ui;

private slots:
    void on_buttonBox_accepted();
};

#endif // GUIDEDIALOG_H
