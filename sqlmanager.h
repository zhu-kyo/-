#ifndef SQLMANAGER_H
#define SQLMANAGER_H

#include <QObject>
#include <QSqlDatabase>

struct MusicInfo;

class SqlManager : public QObject
{
    Q_OBJECT
public:
    explicit SqlManager(QWidget *parent = nullptr);

    QList<MusicInfo> readMusicInfo() const;
    QList<MusicInfo> readMusicInfo(const QStringList &urls) const;
    QList<MusicInfo> readMusicList(const QString &tableName) const;
    void deleteInvalidUrl(const QStringList &urls);
    MusicInfo getMusicInfo(const QString &url);
    void updateMusicInfo(const QList<MusicInfo> &musicInfoList);
    QWidget *getMainWidget() { return mainWidget; }
    void addMusicInfo(const QList<MusicInfo> &musicInfoList);
    void deleteMusic(const QStringList urls, const QString &tableName, const QString &listName);
    void updateId(const QString &tableName);
    QStringList getListUrls(const QString &tableName);
    void addToList(const QString &tableName, const QStringList &urls);
    int getNextId(const QString &tableName);
    QList<MusicInfo> getMusicListInfo(const QString &name);
    void addToMusicList(const QStringList &urls, const QString &listName);

private:
    QSqlDatabase database;
    QWidget *mainWidget;

    void checkTables();
};

#endif // SQLMANAGER_H
