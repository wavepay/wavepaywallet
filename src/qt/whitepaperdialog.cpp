#include "whitepaperdialog.h"
#include "ui_whitepaperdialog.h"
#include "clientmodel.h"

#include "version.h"
#include <QScrollBar>
#include <QToolBox>
WhitepaperDialog::WhitepaperDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WhitepaperDialog)
{
    ui->setupUi(this);
}

void WhitepaperDialog::setModel(ClientModel *model)
{
    if(model)
    {
        //ui->versionLabel->setText(model->formatFullVersion());
    }
}

WhitepaperDialog::~WhitepaperDialog()
{
    delete ui;
}

void WhitepaperDialog::on_buttonBox_accepted()
{
    close();
}
