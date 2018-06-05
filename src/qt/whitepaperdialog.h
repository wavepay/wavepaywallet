#ifndef WHITEPAPERDIALOG_H
#define WHITEPAPERDIALOG_H

#include <QDialog>

namespace Ui {
    class WhitepaperDialog;
}
class ClientModel;

/** "Whitepaper" dialog box */
class WhitepaperDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WhitepaperDialog(QWidget *parent = 0);
    ~WhitepaperDialog();

    void setModel(ClientModel *model);
private:
    Ui::WhitepaperDialog *ui;

private slots:
    void on_buttonBox_accepted();
};

#endif // WHITEPAPERDIALOG_H
