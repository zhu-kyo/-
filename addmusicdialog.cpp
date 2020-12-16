#include "addmusicdialog.h"
#include "ui_addmusicdialog.h"

#include <QIcon>
#include <QCloseEvent>

AddMusicDialog::AddMusicDialog(QWidget *parent, QStringList *selectedUrls, const QStringList &urls, const QStringList &names) :
    QDialog(parent),
    ui(new Ui::AddMusicDialog)
{
    ui->setupUi(this);

    for(int i = 0; i < urls.size(); ++ i)
    {
        ui->listWidget->addItem(names.at(i));
        ui->listWidget->item(i)->setIcon(QIcon(":/images/localListIcon.png"));
        ui->listWidget->item(i)->setData(Qt::UserRole, urls.at(i));
    }

    selectedItems = selectedUrls;
}

AddMusicDialog::~AddMusicDialog()
{
    delete ui;
}

void AddMusicDialog::hideEvent(QHideEvent *event)
{
    for(QListWidgetItem *item : ui->listWidget->selectedItems())
        selectedItems->append(item->data(Qt::UserRole).toString());

    event->accept();
}
