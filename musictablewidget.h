#ifndef MUSICLISTWIDGET_H
#define MUSICLISTWIDGET_H

#include <QTableWidget>

class QIcon;
class SqlManager;


struct MusicInfo;

class MusicTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MusicTableWidget(QWidget *parent = nullptr);

    void setIcon(const QIcon &icon) { itemIcon = icon; }
    void setTableName(const QString &name) { tableName = name; }
    void setListName(const QString &name) { listName = name; }
    QString getListName() { return listName; }
    void setContextMenu(QMenu *menu) { contextMenu = menu; }

    void readMusicList(SqlManager *sqlManager);
    MusicInfo getMusicInfo(SqlManager *sqlManager, int row);
    MusicInfo readMusicInfo(int row);
    void updateList(const MusicInfo &musicInfo);
    QString formatDuration(qint64 duration);
    void updatePlayingIcon(const QIcon &icon, int row);
    bool addMusic(SqlManager *sqlManager, QList<QUrl> urls);
    void deleteMusic(SqlManager *sqlManager);
    void addToList(SqlManager *sqlManager, QStringList urls);
    void readMusicListData(SqlManager *sqlManager, const QString &name);
    void sortByMusician(SqlManager *sqlManager);
    void sortBySongs(SqlManager *sqlManager);
    void sortByDuration(SqlManager *sqlManager);
    void addToMusicList(SqlManager *sqlManager, QStringList urls);
    void deleteUrls(const QStringList &urls);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QIcon itemIcon = QIcon(":/images/musicIcon.png");
    QString tableName;
    QString listName = QStringLiteral("");
    QMenu *contextMenu;
    int tableColumnCount = 4;

public slots:
    void showInExplorer() const;
};

struct MusicInfo
{
    QString name;
    QString url;
    QString title;
    QString author;
    QString albumTitle;
    qint64 duration;
    int audioBitRate;
};

#endif // MUSICLISTWIDGET_H
