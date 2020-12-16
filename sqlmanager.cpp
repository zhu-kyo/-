#include "sqlmanager.h"
#include "musictablewidget.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QVariant>
#include <QDebug>

SqlManager::SqlManager(QWidget *parent)
    : QObject(parent)
    , mainWidget(parent)
{
    database = QSqlDatabase::database();
    if(!database.isValid())
    {
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName("musicPlayer.db");
        if(!database.open())
            QMessageBox::critical(mainWidget, tr("无法打开数据库文件"), database.lastError().text());
    }

    checkTables();
}

void SqlManager::checkTables()
{
    QSqlQuery query;
    QStringList tables = database.tables();
    if(!tables.contains("MusicInfo", Qt::CaseInsensitive))
    {
        if(!query.exec("CREATE TABLE MusicInfo (name TEXT, url TEXT, title TEXT, author TEXT, albumTitle TEXT, duration BIGINT, audioBitRate INTEGER, UNIQUE (url))"))
            QMessageBox::critical(mainWidget, tr("无法创建数据库表格"), database.lastError().text());
    }

    if(!tables.contains("Playlist", Qt::CaseInsensitive))
    {
        if(!query.exec("CREATE TABLE Playlist (id INTEGER PRIMARY KEY, url TEXT, UNIQUE (url))"))
            QMessageBox::critical(mainWidget, tr("无法创建数据库表格"), database.lastError().text());
    }

    if(!tables.contains("LocalList", Qt::CaseInsensitive))
    {
        if(!query.exec("CREATE TABLE LocalList (id INTEGER PRIMARY KEY, url TEXT, UNIQUE (url))"))
            QMessageBox::critical(mainWidget, tr("无法创建数据库表格"), database.lastError().text());
    }

    if(!tables.contains("FavoriteList", Qt::CaseInsensitive))
    {
        if(!query.exec("CREATE TABLE FavoriteList (id INTEGER PRIMARY KEY, url TEXT, UNIQUE (url))"))
            QMessageBox::critical(mainWidget, tr("无法创建数据库表格"), database.lastError().text());
    }

    if(!tables.contains("MusicList", Qt::CaseInsensitive))
    {
        if(!query.exec("CREATE TABLE MusicList (name TEXT, url TEXT)"))
            QMessageBox::critical(mainWidget, tr("无法创建数据库表格"), database.lastError().text());
    }
}

QList<MusicInfo> SqlManager::readMusicInfo() const
{
    QSqlQuery query;
    query.exec("SELECT * FROM MusicInfo");

    QList<MusicInfo> musicInfoList;
    while(query.next())
    {
        MusicInfo tempInfo;
        tempInfo.name = query.value(0).toString();
        tempInfo.url = query.value(1).toString();
        tempInfo.title = query.value(2).toString();
        tempInfo.author = query.value(3).toString();
        tempInfo.albumTitle = query.value(4).toString();
        tempInfo.duration = query.value(5).toLongLong();
        tempInfo.audioBitRate = query.value(6).toInt();
        musicInfoList.append(tempInfo);
    }

    return musicInfoList;
}

QList<MusicInfo> SqlManager::readMusicInfo(const QStringList &urls) const
{
    QSqlQuery query;
    QList<MusicInfo> musicInfoList;
    for(const QString &url : urls)
    {
        query.prepare("SELECT * FROM MusicInfo WHERE url = ?");
        query.addBindValue(url);
        query.exec();

        if(query.next())
        {
            MusicInfo musicInfo;
            musicInfo.name = query.value(0).toString();
            musicInfo.url = query.value(1).toString();
            musicInfo.title = query.value(2).toString();
            musicInfo.author = query.value(3).toString();
            musicInfo.albumTitle = query.value(4).toString();
            musicInfo.duration = query.value(5).toLongLong();
            musicInfo.audioBitRate = query.value(6).toInt();
            musicInfoList.append(musicInfo);
        }
    }

    return musicInfoList;
}

void SqlManager::updateMusicInfo(const QList<MusicInfo> &musicInfoList)
{
    database.transaction();

    QVariantList names, titles, authors, albumTitles, durations, audioBitRates, urls;
    for(const MusicInfo &musicInfo : musicInfoList)
    {
        names << musicInfo.name;
        titles << musicInfo.title;
        authors << musicInfo.author;
        albumTitles << musicInfo.albumTitle;
        durations << musicInfo.duration;
        audioBitRates << musicInfo.audioBitRate;
        urls << musicInfo.url;
    }

    QSqlQuery query;
    query.prepare("UPDATE MusicInfo SET name = ?, title = ?, author = ?, albumTitle = ?, duration = ?, audioBitRate = ? WHERE url = ?");
    query.addBindValue(names);
    query.addBindValue(titles);
    query.addBindValue(authors);
    query.addBindValue(albumTitles);
    query.addBindValue(durations);
    query.addBindValue(audioBitRates);
    query.addBindValue(urls);
    query.execBatch();

    query.prepare("UPDATE Playlist SET name = ?, title = ?, author = ?, albumTitle = ?, duration = ?, audioBitRate = ? WHERE url = ?");
    query.addBindValue(names);
    query.addBindValue(titles);
    query.addBindValue(authors);
    query.addBindValue(albumTitles);
    query.addBindValue(durations);
    query.addBindValue(audioBitRates);
    query.addBindValue(urls);
    query.execBatch();

    query.prepare("UPDATE LocalList SET name = ?, title = ?, author = ?, albumTitle = ?, duration = ?, audioBitRate = ? WHERE url = ?");
    query.addBindValue(names);
    query.addBindValue(titles);
    query.addBindValue(authors);
    query.addBindValue(albumTitles);
    query.addBindValue(durations);
    query.addBindValue(audioBitRates);
    query.addBindValue(urls);
    query.execBatch();

    query.prepare("UPDATE FavoriteList SET name = ?, title = ?, author = ?, albumTitle = ?, duration = ?, audioBitRate = ? WHERE url = ?");
    query.addBindValue(names);
    query.addBindValue(titles);
    query.addBindValue(authors);
    query.addBindValue(albumTitles);
    query.addBindValue(durations);
    query.addBindValue(audioBitRates);
    query.addBindValue(urls);
    query.execBatch();

    query.prepare("UPDATE MusicList SET name = ?, title = ?, author = ?, albumTitle = ?, duration = ?, audioBitRate = ? WHERE url = ?");
    query.addBindValue(names);
    query.addBindValue(titles);
    query.addBindValue(authors);
    query.addBindValue(albumTitles);
    query.addBindValue(durations);
    query.addBindValue(audioBitRates);
    query.addBindValue(urls);
    query.execBatch();

    database.commit();
}

void SqlManager::addMusicInfo(const QList<MusicInfo> &musicInfoList)
{
    database.transaction();

    QVariantList names, urls, titles, authors, albumTitles, durations, audioBitRates;
    for(const MusicInfo &musicInfo : musicInfoList)
    {
        names << musicInfo.name;
        urls << musicInfo.url;
        titles << musicInfo.title;
        authors << musicInfo.author;
        albumTitles << musicInfo.albumTitle;
        durations << musicInfo.duration;
        audioBitRates << musicInfo.audioBitRate;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO MusicInfo VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(names);
    query.addBindValue(urls);
    query.addBindValue(titles);
    query.addBindValue(authors);
    query.addBindValue(albumTitles);
    query.addBindValue(durations);
    query.addBindValue(audioBitRates);
    query.execBatch();

    database.commit();
}

void SqlManager::deleteMusic(const QStringList urls, const QString &tableName, const QString &listName)
{
    database.transaction();

    QSqlQuery query;

    if(tableName == "MusicList")
    {
        QStringList names;
        for(int i = 0; i < urls.size(); ++ i)
            names.append(listName);

        query.prepare("DELETE FROM MusicList WHERE url = ? AND name = ?");
        query.addBindValue(urls);
        query.addBindValue(names);
        query.execBatch();
    }
    else
    {
        query.prepare(QStringLiteral("DELETE FROM %1 WHERE url = ?").arg(tableName));
        query.addBindValue(urls);
        query.execBatch();

        updateId(tableName);
    }

    database.commit();
}

void SqlManager::updateId(const QString &tableName)
{
    QVariantList ids, urls;
    QList<MusicInfo> musicInfoList = readMusicList(tableName);
    for(int i = 0; i < musicInfoList.size(); ++ i)
    {
        ids.append(i + 1);
        urls.append(musicInfoList.at(i).url);
    }

    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE %1 SET id = ? WHERE url = ?").arg(tableName));
    query.addBindValue(ids);
    query.addBindValue(urls);
    query.execBatch();
}

QStringList SqlManager::getListUrls(const QString &tableName)
{
    QSqlQuery query;
    query.exec(QStringLiteral("SELECT url FROM %1").arg(tableName));

    QStringList urls;
    while(query.next())
        urls.append(query.value(0).toString());

    return urls;
}

void SqlManager::addToList(const QString &tableName, const QStringList &urls)
{
    database.transaction();

    QSqlQuery query;

    int id = getNextId(tableName);
    QVariantList ids;
    for(int i = 0; i < urls.size(); ++ i)
        ids << ++ id;

    query.prepare(QStringLiteral("INSERT INTO %1 VALUES (?, ?)").arg(tableName));
    query.addBindValue(ids);
    query.addBindValue(urls);
    query.execBatch();

    database.commit();
}

int SqlManager::getNextId(const QString &tableName)
{
    QSqlQuery query;
    query.exec(QStringLiteral("SELECT id FROM %1 ORDER BY id DESC").arg(tableName));

    if(query.next())
        return query.value(0).toInt();

    return 0;
}

QList<MusicInfo> SqlManager::getMusicListInfo(const QString &name)
{
    QSqlQuery query;

    QStringList urls;
    query.prepare("SELECT url FROM MusicList WHERE name = ?");
    query.addBindValue(name);
    query.exec();
    while(query.next())
        urls.append(query.value(0).toString());

    return readMusicInfo(urls);
}

void SqlManager::addToMusicList(const QStringList &urls, const QString &listName)
{
    database.transaction();

    QSqlQuery query;
    QStringList names;
    for(int i = 0; i < urls.size(); ++ i)
        names.append(listName);

    query.prepare("INSERT INTO MusicList VALUES (?, ?)");
    query.addBindValue(names);
    query.addBindValue(urls);
    query.execBatch();

    database.commit();
}

QList<MusicInfo> SqlManager::readMusicList(const QString &tableName) const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT * FROM MusicInfo INNER JOIN %1 AS T ON MusicInfo.url = T.url ORDER BY T.id").arg(tableName));
    query.exec();

    QList<MusicInfo> musicInfoList;
    while(query.next())
    {
        MusicInfo tempInfo;
        tempInfo.name = query.value(0).toString();
        tempInfo.url = query.value(1).toString();
        tempInfo.title = query.value(2).toString();
        tempInfo.author = query.value(3).toString();
        tempInfo.albumTitle = query.value(4).toString();
        tempInfo.duration = query.value(5).toLongLong();
        tempInfo.audioBitRate = query.value(6).toInt();
        musicInfoList.append(tempInfo);
    }

    return musicInfoList;
}

void SqlManager::deleteInvalidUrl(const QStringList &urls)
{
    database.transaction();

    QSqlQuery query;
    query.prepare("DELETE FROM MusicInfo WHERE url = ?");
    query.addBindValue(urls);
    query.execBatch();

    query.prepare("DELETE FROM Playlist WHERE url = ?");
    query.addBindValue(urls);
    query.execBatch();

    updateId("Playlist");

    query.prepare("DELETE FROM LocalList WHERE url = ?");
    query.addBindValue(urls);
    query.execBatch();

    updateId("LocalList");

    query.prepare("DELETE FROM FavoriteList WHERE url = ?");
    query.addBindValue(urls);
    query.execBatch();

    updateId("FavoriteList");

    query.prepare("DELETE FROM MusicList WHERE url = ?");
    query.addBindValue(urls);
    query.execBatch();

    database.commit();
}

MusicInfo SqlManager::getMusicInfo(const QString &url)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM MusicInfo WHERE url = ?");
    query.addBindValue(url);
    query.exec();

    MusicInfo tempInfo;
    if(query.next())
    {
        tempInfo.name = query.value(0).toString();
        tempInfo.url = query.value(1).toString();
        tempInfo.title = query.value(2).toString();
        tempInfo.author = query.value(3).toString();
        tempInfo.albumTitle = query.value(4).toString();
        tempInfo.duration = query.value(5).toLongLong();
        tempInfo.audioBitRate = query.value(6).toInt();
    }

    return tempInfo;
}
