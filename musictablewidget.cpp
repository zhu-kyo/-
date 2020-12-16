#include "musictablewidget.h"
#include "sqlmanager.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QHeaderView>
#include <QMediaContent>
#include <QFileInfo>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QMediaPlayer>
#include <QDebug>

MusicTableWidget::MusicTableWidget(QWidget *parent)
    : QTableWidget(parent)
    , tableColumnCount(5)
{
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setSectionsClickable(false);
}

void MusicTableWidget::readMusicList(SqlManager *sqlManager)
{
    QList<MusicInfo> musicInfoList = sqlManager->readMusicList(tableName);

    QStringList invalidUrls;
    int index = 0;
    while(index < musicInfoList.size())
    {
        MusicInfo musicInfo = musicInfoList.at(index);
        if(QMediaContent(musicInfo.url).isNull())
        {
            invalidUrls.append(musicInfo.url);
            musicInfoList.removeAt(index);
        }
        else  ++ index;
    }

    clearContents();

    setRowCount(musicInfoList.size());
    setColumnCount(tableColumnCount);
    setColumnHidden(4, true);

    for(int i = 0; i < musicInfoList.size(); ++ i)
    {
        MusicInfo musicInfo = musicInfoList.at(i);
        QTableWidgetItem *nameItem = new QTableWidgetItem(QIcon(itemIcon), musicInfo.name);
        QTableWidgetItem *authorItem = new QTableWidgetItem(musicInfo.author);
        QTableWidgetItem *albumTitleItem = new QTableWidgetItem(musicInfo.albumTitle);
        QTableWidgetItem *durationItem = new QTableWidgetItem(formatDuration(musicInfo.duration));
        QTableWidgetItem *urlItem = new QTableWidgetItem(musicInfo.url);

        setItem(i, 0, nameItem);
        setItem(i, 1, authorItem);
        setItem(i, 2, albumTitleItem);
        setItem(i, 3, durationItem);
        setItem(i, 4, urlItem);
    }   

    setHorizontalHeaderLabels(QStringList() << tr("歌曲") << tr("歌手") << tr("专辑") << tr("时长"));

    sqlManager->deleteInvalidUrl(invalidUrls);
}

MusicInfo MusicTableWidget::getMusicInfo(SqlManager *sqlManager, int row)
{
    return sqlManager->getMusicInfo(item(row, 4)->text());
}

MusicInfo MusicTableWidget::readMusicInfo(int row)
{
    MusicInfo musicInfo;
    musicInfo.name = item(row, 0)->text();
    musicInfo.url = item(row, 4)->text();
    musicInfo.title = QStringLiteral("");
    musicInfo.author = item(row, 1)->text();
    musicInfo.albumTitle = item(row, 2)->text();
    musicInfo.duration = item(row, 3)->text().toLongLong();
    musicInfo.audioBitRate = 0;

    return musicInfo;
}

void MusicTableWidget::updateList(const MusicInfo &musicInfo)
{
    for(int i = 0; i < rowCount(); ++ i)
    {
        if(item(i, 4)->text() == musicInfo.url)
        {
            item(i, 0)->setText(musicInfo.name);
            item(i, 1)->setText(musicInfo.author);
            item(i, 2)->setText(musicInfo.albumTitle);
            item(i, 3)->setText(formatDuration(musicInfo.duration));
        }
    }
}

void MusicTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if(itemAt(event->pos()) != nullptr)
        contextMenu->popup(event->globalPos());

    event->accept();
}

QString MusicTableWidget::formatDuration(qint64 duration)
{
    qint64 minutes = duration / 60000;
    qint64 seconds = duration / 1000 - minutes * 60;

    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

void MusicTableWidget::updatePlayingIcon(const QIcon &icon, int row)
{
    for(int i = 0; i < rowCount(); ++ i)
    {
        if(i != row)  item(i, 0)->setIcon(itemIcon);
        else  item(i, 0)->setIcon(icon);
    }
}

bool MusicTableWidget::addMusic(SqlManager *sqlManager, QList<QUrl> urls)
{
    QStringList validUrls;
    int index = 0;
    while(index < urls.size())
    {
        if(!urls.at(index).isValid())  urls.removeAt(index);
        else
        {
            validUrls.append(urls.at(index).toString());
            ++ index;
        }
    }

    QList<MusicInfo> musicInfoList = sqlManager->readMusicInfo();
    QList<MusicInfo> updateMusicInfoList;
    QList<MusicInfo> newMusicInfoList;

    QProgressDialog dialog(tr("正在添加"), tr("取消"), 0, urls.size(), sqlManager->getMainWidget());
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setWindowTitle(tr("添加本地音乐文件"));
    dialog.setWindowIcon(QIcon(":/images/add.png"));
    dialog.setMinimumSize(350, 150);
    dialog.show();

    for(int i = 0; i < urls.size(); ++ i)
    {
        dialog.setValue(i);

        QUrl url = urls.at(i);
        QMediaPlayer tempPlayer;
        tempPlayer.setMedia(QMediaContent(url));

        while(!tempPlayer.isMetaDataAvailable())
            QCoreApplication::processEvents();

        MusicInfo tempInfo;
        tempInfo.name = QFileInfo(url.path()).baseName();
        tempInfo.url = url.toString();
        tempInfo.title = tempPlayer.metaData("Title").toString();
        tempInfo.author = tempPlayer.metaData("Author").toStringList().join(" / ");
        tempInfo.albumTitle = tempPlayer.metaData("AlbumTitle").toString();
        tempInfo.duration = tempPlayer.duration();
        tempInfo.audioBitRate = tempPlayer.metaData("AudioBitRate").toInt();

        bool isExisted = false;
        index = 0;
        while(index < musicInfoList.size())
        {
            if(musicInfoList.at(index).url == url.toString())
            {
                updateMusicInfoList.append(tempInfo);
                musicInfoList.removeAt(index);
                isExisted = true;
                break;
            }

            ++ index;
        }

        if(!isExisted)
            newMusicInfoList.append(tempInfo);

        if(dialog.wasCanceled())
            return false;
    }

    sqlManager->updateMusicInfo(updateMusicInfoList);
    sqlManager->addMusicInfo(newMusicInfoList);

    addToList(sqlManager, validUrls);

    dialog.setValue(urls.size());   

    return true;
}

void MusicTableWidget::deleteMusic(SqlManager *sqlManager)
{
    QStringList urls;
    QList<int> rows;
    for(int i = 0; i < rowCount(); ++ i)
    {
        if(item(i, 0)->isSelected())
        {
            urls.append(item(i, 4)->text());
            rows.prepend(i);
        }
    }

    if(tableName == "LocalList")  sqlManager->deleteInvalidUrl(urls);
    else  sqlManager->deleteMusic(urls, tableName, listName);

    for(int row : rows)
        removeRow(row);
}

void MusicTableWidget::showInExplorer() const
{
    QString url = item(currentRow(), 4)->text();
    QDesktopServices::openUrl(QFileInfo(url).path());
}

void MusicTableWidget::addToList(SqlManager *sqlManager, QStringList urls)
{
    QStringList existUrls = sqlManager->getListUrls(tableName);

    int index = 0;
    while(index < urls.size())
    {
        if(existUrls.contains(urls.at(index)))  urls.removeAt(index);
        else  ++ index;

    }

    sqlManager->addToList(tableName, urls);

    int row = rowCount();
    QList<MusicInfo> musicInfoList = sqlManager->readMusicInfo(urls);
    for(const MusicInfo &musicInfo : musicInfoList)
    {
        insertRow(row);


        QTableWidgetItem *nameItem = new QTableWidgetItem(itemIcon, musicInfo.name);
        QTableWidgetItem *authorItem = new QTableWidgetItem(musicInfo.author);
        QTableWidgetItem *albumTitleItem = new QTableWidgetItem(musicInfo.albumTitle);
        QTableWidgetItem *durationItem = new QTableWidgetItem(formatDuration(musicInfo.duration));
        QTableWidgetItem *urlItem = new QTableWidgetItem(musicInfo.url);
        setItem(row, 0, nameItem);
        setItem(row, 1, authorItem);
        setItem(row, 2, albumTitleItem);
        setItem(row, 3, durationItem);
        setItem(row, 4, urlItem);

        ++ row;
    }
}

void MusicTableWidget::readMusicListData(SqlManager *sqlManager, const QString &name)
{
    setColumnCount(tableColumnCount);
    setColumnHidden(4, true);

    setListName(name);

    QList<MusicInfo> musicInfoList = sqlManager->getMusicListInfo(name);
    setRowCount(musicInfoList.size());
    for(int i = 0; i < musicInfoList.size(); ++ i)
    {
        MusicInfo musicInfo = musicInfoList.at(i);
        QTableWidgetItem *nameItem = new QTableWidgetItem(itemIcon, musicInfo.name);
        QTableWidgetItem *authorItem = new QTableWidgetItem(musicInfo.author);
        QTableWidgetItem *albumTitleItem = new QTableWidgetItem(musicInfo.albumTitle);
        QTableWidgetItem *durationItem = new QTableWidgetItem(formatDuration(musicInfo.duration));
        QTableWidgetItem *urlItem = new QTableWidgetItem(musicInfo.url);
        setItem(i, 0, nameItem);
        setItem(i, 1, authorItem);
        setItem(i, 2, albumTitleItem);
        setItem(i, 3, durationItem);
        setItem(i, 4, urlItem);
    }

    setHorizontalHeaderLabels(QStringList() << tr("歌曲") << tr("歌手") << tr("专辑") << tr("时长"));
}

void MusicTableWidget::sortByMusician(SqlManager *sqlManager)
{
    sortItems(1);

    QStringList urls;
    for(int i = 0; i < rowCount(); ++ i)
        urls.append(item(i, 4)->text());

    sqlManager->deleteMusic(urls, tableName, listName);
    sqlManager->addToList(tableName, urls);
}

void MusicTableWidget::sortBySongs(SqlManager *sqlManager)
{
    sortItems(0);

    QStringList urls;
    for(int i = 0; i < rowCount(); ++ i)
        urls.append(item(i, 4)->text());

    sqlManager->deleteMusic(urls, tableName, listName);
    sqlManager->addToList(tableName, urls);
}

void MusicTableWidget::sortByDuration(SqlManager *sqlManager)
{
    sortItems(3);

    QStringList urls;
    for(int i = 0; i < rowCount(); ++ i)
        urls.append(item(i, 4)->text());

    sqlManager->deleteMusic(urls, tableName, listName);

    if(listName.isEmpty())  sqlManager->addToList(tableName, urls);
    else  sqlManager->addToMusicList(urls, listName);
}

void MusicTableWidget::addToMusicList(SqlManager *sqlManager, QStringList urls)
{
    QList<MusicInfo> musicInfoList = sqlManager->getMusicListInfo(listName);
    QStringList existUrls;
    for(const MusicInfo &musicInfo : musicInfoList)
        existUrls.append(musicInfo.url);

    int index = 0;
    while(index < urls.size())
    {
        if(existUrls.contains(urls.at(index)))  urls.removeAt(index);
        else  ++ index;
    }

    sqlManager->addToMusicList(urls, listName);

    int row = rowCount();
    musicInfoList = sqlManager->readMusicInfo(urls);
    for(const MusicInfo &musicInfo : musicInfoList)
    {
        insertRow(row);

        QTableWidgetItem *nameItem = new QTableWidgetItem(itemIcon, musicInfo.name);
        QTableWidgetItem *authorItem = new QTableWidgetItem(musicInfo.author);
        QTableWidgetItem *albumTitleItem = new QTableWidgetItem(musicInfo.albumTitle);
        QTableWidgetItem *durationItem = new QTableWidgetItem(formatDuration(musicInfo.duration));
        QTableWidgetItem *urlItem = new QTableWidgetItem(musicInfo.url);
        setItem(row, 0, nameItem);
        setItem(row, 1, authorItem);
        setItem(row, 2, albumTitleItem);
        setItem(row, 3, durationItem);
        setItem(row, 4, urlItem);

        ++ row;
    }
}

void MusicTableWidget::deleteUrls(const QStringList &urls)
{
    for(int i = rowCount() - 1; i >= 0; -- i)
    {
        if(urls.contains(item(i, 4)->text()))
            removeRow(i);
    }
}
