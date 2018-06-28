#include "guidedialog.h"
#include "ui_guidedialog.h"
#include "clientmodel.h"

#include "version.h"
#include <QScrollBar>
#include <QToolBox>
GuideDialog::GuideDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GuideDialog)
{
    ui->setupUi(this);
}

void GuideDialog::setModel(ClientModel *model)
{
    if(model)
    {
        //ui->versionLabel->setText(model->formatFullVersion());
    }
}

GuideDialog::~GuideDialog()
{
    delete ui;
}

void GuideDialog::on_buttonBox_accepted()
{
    close();
}
