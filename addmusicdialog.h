#ifndef ADDMUSICDIALOG_H
#define ADDMUSICDIALOG_H

#include <QDialog>

namespace Ui {
class AddMusicDialog;
}

class AddMusicDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddMusicDialog(QWidget *parent = nullptr, QStringList *selectedUrls = nullptr, const QStringList &urls = QStringList(), const QStringList &names = QStringList());
    ~AddMusicDialog() override;

protected:
    void hideEvent(QHideEvent *event) override;

private:
    Ui::AddMusicDialog *ui;

    QStringList *selectedItems;
};

#endif // ADDMUSICDIALOG_H
