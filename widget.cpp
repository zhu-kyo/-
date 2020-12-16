#include "widget.h"
#include "ui_widget.h"
#include "controlstyle.h"
#include "sqlmanager.h"
#include "addmusicdialog.h"

#include <QStyleOption>
#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>
#include <QSettings>
#include <QPixmap>
#include <QMenu>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <QMediaPlaylist>
#include <QRegularExpression>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    initUI();
    initSettings();
    initSystemTrayIcon();
    initMenu();

    readMusicListData();
    initPlayer();

    qApp->installEventFilter(this);     // 监听整个应用的事件，在eventFilter()中实现，主要目的是在单击其它位置时取消音乐列表的选中状态
}

Widget::~Widget()
{
    delete ui;
}

void Widget::paintEvent(QPaintEvent *event)
{
    //需要添加以下代码，才能正常在主窗口Widget中显示背景图片，原应用是这样做的，不知道原因
    QStyleOption option;
    option.initFrom(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
    QWidget::paintEvent(event);
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if(event->pos().y() < ui->logoLabel->height())
    {
        isMoving = true;
        offset = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if(isMoving)
    {
        move(event->globalPos() - offset);
        event->accept();
    }
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    if(isMoving)
    {
        isMoving = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    }
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress && musicListListMenu->isHidden() && playlistMenu->isHidden()
            && localListMenu->isHidden() && favoriteListMenu->isHidden() && musicListMenu->isHidden())
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *> (event);

        QWidget *widget = ui->musicListListWidget->viewport();
        if(ui->musicListListWidget->itemAt(widget->mapFromGlobal(mouseEvent->globalPos())) == nullptr)
            ui->musicListListWidget->clearSelection();

        widget = ui->playlistWidget->viewport();
        if(ui->playlistWidget->itemAt(widget->mapFromGlobal(mouseEvent->globalPos())) == nullptr)
            ui->playlistWidget->clearSelection();

        widget = ui->localListWidget->viewport();
        if(ui->localListWidget->itemAt(widget->mapFromGlobal(mouseEvent->globalPos())) == nullptr)
            ui->localListWidget->clearSelection();

        widget = ui->favoriteListWidget->viewport();
        if(ui->favoriteListWidget->itemAt(widget->mapFromGlobal(mouseEvent->globalPos())) == nullptr)
            ui->favoriteListWidget->clearSelection();

        widget = ui->musicListWidget->viewport();
        if(ui->musicListWidget->itemAt(widget->mapFromGlobal(mouseEvent->globalPos())) == nullptr)
            ui->musicListWidget->clearSelection();
    }

    return QWidget::eventFilter(watched, event);
}

void Widget::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void Widget::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint);    // 设置无边框后窗口大小不可变
    setAttribute(Qt::WA_TranslucentBackground); //原应用窗口设置了圆角后，会出现留白，需要添加该代码，本应用没有设置圆角，故可不加改行代码

    QString scrollBarStyle = ControlStyle::scrollBarStyle();
    ui->musicListListWidget->verticalScrollBar()->setStyleSheet(scrollBarStyle);
    ui->playlistWidget->verticalScrollBar()->setStyleSheet(scrollBarStyle);
    ui->localListWidget->verticalScrollBar()->setStyleSheet(scrollBarStyle);
    ui->favoriteListWidget->verticalScrollBar()->setStyleSheet(scrollBarStyle);
    ui->musicListWidget->verticalScrollBar()->setStyleSheet(scrollBarStyle);

    ui->playlistWidget->setIcon(QIcon(":/images/musicIcon.png"));
    ui->localListWidget->setIcon(QIcon(":/images/localListIcon.png"));
    ui->favoriteListWidget->setIcon(QIcon(":/images/favIcon.png"));
    ui->musicListWidget->setIcon(QIcon(":/images/musicIcon.png"));
}

void Widget::initSettings()
{
    settings = new QSettings("./settings.ini", QSettings::IniFormat, this);
    settings->setIniCodec("UTF-8");

    QString filePath = settings->value("background").toString();
    QPixmap pixmap(filePath);
    if(pixmap.isNull())
        filePath = QStringLiteral(":/images/background.jpg");

    setStyleSheet(ControlStyle::windowBackgroundStyle(filePath));

    settings->setValue("background", filePath);
}

void Widget::initMenu()
{
    skinToolBtnMenu = new QMenu(this);
    QAction *changeSkinAction = skinToolBtnMenu->addAction(QIcon(":/images/customize.png"), tr("自定义背景"));
    QAction *defaultSkinAction = skinToolBtnMenu->addAction(QIcon(":/images/restore.png"), tr("恢复默认背景"));
    connect(changeSkinAction, &QAction::triggered, this, &Widget::customizeBackground);
    connect(defaultSkinAction, &QAction::triggered, this, &Widget::restoreBackground);

    menuToolBtnMenu = new QMenu(this);
    QAction *minimizeToTrayAction = menuToolBtnMenu->addAction(QIcon(":/images/toTray.png"), tr("最小化到托盘"));
    QAction *exitAction = menuToolBtnMenu->addAction(QIcon(":/images/exit.png"), tr("退出应用"));
    connect(minimizeToTrayAction, &QAction::triggered, this, &QWidget::hide);
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    ui->musicListListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    musicListListMenu = new QMenu(this);
    QAction *renameAction = musicListListMenu->addAction(QIcon(":/images/rename.png"), tr("重命名"));
    QAction *musicListListDeleteAction = musicListListMenu->addAction(QIcon(":/images/clear.png"), tr("删除"));
    connect(renameAction, &QAction::triggered, this, &Widget::renameMusicListList);
    connect(musicListListDeleteAction, &QAction::triggered, this, &Widget::deleteMusicListList);

    playlistMenu = new QMenu(this);
    QAction *playlistAddToFavAction = playlistMenu->addAction(QIcon(":/images/favIcon.png"), tr("添加到我喜欢"));
    QAction *playlistShowFileAction = playlistMenu->addAction(QIcon(":/images/localMusic.png"), tr("打开所在文件夹"));
    QAction *playlistDeleteAction = playlistMenu->addAction(QIcon(":/images/clear.png"), tr("删除"));
    connect(playlistAddToFavAction, &QAction::triggered, this, &Widget::addToFavorite);
    connect(playlistShowFileAction, &QAction::triggered, ui->playlistWidget, &MusicTableWidget::showInExplorer);
    connect(playlistDeleteAction, &QAction::triggered, this, &Widget::deleteItem);
    ui->playlistWidget->setContextMenu(playlistMenu);

    localListMenu = new QMenu(this);
    QAction *localListAddToPlayAction = localListMenu->addAction(QIcon(":/images/playlist.png"), tr("添加到播放列表"));
    QAction *localListAddToFavAction = localListMenu->addAction(QIcon(":/images/favIcon.png"), tr("添加到我喜欢"));
    QAction *localListShowFileAction = localListMenu->addAction(QIcon(":/images/localMusic.png"), tr("打开所在文件夹"));
    QAction *localListDeleteAction = localListMenu->addAction(QIcon(":/images/clear.png"), tr("删除"));
    connect(localListAddToPlayAction, &QAction::triggered, this, &Widget::addToPlaylist);
    connect(localListAddToFavAction, &QAction::triggered, this, &Widget::addToFavorite);
    connect(localListShowFileAction, &QAction::triggered, ui->localListWidget, &MusicTableWidget::showInExplorer);
    connect(localListDeleteAction, &QAction::triggered, this, &Widget::deleteItem);
    ui->localListWidget->setContextMenu(localListMenu);

    favoriteListMenu = new QMenu(this);
    QAction *favoriteListAddToPlayAction = favoriteListMenu->addAction(QIcon(":/images/playlist.png"), tr("添加到播放列表"));
    QAction *favoriteListShowFileAction = favoriteListMenu->addAction(QIcon(":/images/localMusic.png"), tr("打开所在文件夹"));
    QAction *favoriteListDeleteAction = favoriteListMenu->addAction(QIcon(":/images/clear.png"), tr("删除"));
    connect(favoriteListAddToPlayAction, &QAction::triggered, this, &Widget::addToPlaylist);
    connect(favoriteListShowFileAction, &QAction::triggered, ui->favoriteListWidget, &MusicTableWidget::showInExplorer);
    connect(favoriteListDeleteAction, &QAction::triggered, this, &Widget::deleteItem);
    ui->favoriteListWidget->setContextMenu(favoriteListMenu);

    musicListMenu = new QMenu(this);
    QAction *musicListAddToPlayAction = musicListMenu->addAction(QIcon(":/images/playlist.png"), tr("添加到播放列表"));
    QAction *musicListAddToFavAction = musicListMenu->addAction(QIcon(":/images/favIcon.png"), tr("添加到我喜欢"));
    QAction *musicListShowFileAction = musicListMenu->addAction(QIcon(":/images/localMusic.png"), tr("打开所在文件夹"));
    QAction *musicListDeleteAction = musicListMenu->addAction(QIcon(":/images/clear.png"), tr("删除"));
    connect(musicListAddToPlayAction, &QAction::triggered, this, &Widget::addToPlaylist);
    connect(musicListAddToFavAction, &QAction::triggered, this, &Widget::addToFavorite);
    connect(musicListShowFileAction, &QAction::triggered, ui->musicListWidget, &MusicTableWidget::showInExplorer);
    connect(musicListDeleteAction, &QAction::triggered, this, &Widget::deleteItem);
    ui->musicListWidget->setContextMenu(musicListMenu);
}

void Widget::readMusicListData()
{
    sqlManager = new SqlManager(this);

    QStringList musicList = settings->value("musicListNames").toStringList();
    for(const QString &name : musicList)
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/musicListIcon.png"), name, ui->musicListListWidget);
        ui->musicListListWidget->addItem(item);
    }

    ui->playlistWidget->setTableName("Playlist");
    ui->localListWidget->setTableName("LocalList");
    ui->favoriteListWidget->setTableName("FavoriteList");
    ui->musicListWidget->setTableName("MusicList");

    ui->playlistWidget->readMusicList(sqlManager);
    ui->localListWidget->readMusicList(sqlManager);
    ui->favoriteListWidget->readMusicList(sqlManager);

    ui->stackedWidget->setCurrentWidget(ui->lyricPage);
    showLyric(-1);
}

void Widget::initPlayer()
{
    mediaPlayer = new QMediaPlayer(this);
    mediaPlaylist = new QMediaPlaylist(this);

    initPlaylist();
    initVolume();

    connect(mediaPlayer, &QMediaPlayer::stateChanged, this, &Widget::onPlayerStateChanged);
    connect(mediaPlayer, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &Widget::onMetaDataChanged);
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &Widget::onMediaPositionChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &Widget::onDurationChanged);
    connect(mediaPlaylist, &QMediaPlaylist::currentIndexChanged, this, &Widget::onCurrentIndexChanged);
}

void Widget::initPlaylist()
{
    QString playingUrl = settings->value("playingUrl").toString();
    int index = -1;
    for(int i = 0; i < ui->playlistWidget->rowCount(); ++ i)
    {
        QTableWidgetItem *item = ui->playlistWidget->item(i, 4);
        mediaPlaylist->addMedia(QMediaContent(QUrl(item->text())));

        if(playingUrl == item->text())
            index = i;
    }

    mediaPlaylist->setPlaybackMode(static_cast<QMediaPlaylist::PlaybackMode>(settings->value("playbackMode", QMediaPlaylist::Sequential).toInt()));
    mediaPlayer->setPlaylist(mediaPlaylist);

    switch(mediaPlaylist->playbackMode())
    {
    case QMediaPlaylist::Sequential:
        ui->playModeToolBtn->setIcon(QIcon(":/images/sequential.png"));
        ui->playModeToolBtn->setToolTip(tr("顺序播放"));
        break;

    case QMediaPlaylist::Loop:
        ui->playModeToolBtn->setIcon(QIcon(":/images/loop.png"));
        ui->playModeToolBtn->setToolTip(tr("循环播放"));
        break;

    case QMediaPlaylist::CurrentItemInLoop:
        ui->playModeToolBtn->setIcon(QIcon(":/images/singleLoop.png"));
        ui->playModeToolBtn->setToolTip(tr("单曲循环"));
        break;

    case QMediaPlaylist::Random:
        ui->playModeToolBtn->setIcon(QIcon(":/images/random.png"));
        ui->playModeToolBtn->setToolTip(tr("随机播放"));
        break;

    default:
        ui->playModeToolBtn->setIcon(QIcon(":/images/sequential.png"));
        ui->playModeToolBtn->setToolTip(tr("顺序播放"));
        break;
    }

    if(index != -1)
    {
        QString name = ui->playlistWidget->item(index, 0)->text();
        QString author = ui->playlistWidget->item(index, 1)->text();
        mediaPlaylist->setCurrentIndex(index);
        ui->musicTitleLabel->setText(name);
        systemTrayIcon->setToolTip(tr("已暂停：") + name + " - " + author);

        QPixmap pixmap = settings->value("playingAlbumCover").value<QPixmap>();
        if(!pixmap.isNull())
            ui->albumCoverLabel->setPixmap(pixmap);
    }
    else  ui->musicTitleLabel->setText("");

    ui->musicNameLabel->setText("");
    ui->musicianLabel->setText("");
    ui->musicAlbumLabel->setText("");
}

void Widget::initVolume()
{
    bool isMuted = settings->value("isMuted", false).toBool();
    mediaPlayer->setMuted(isMuted);
    if(isMuted)
        ui->volumeToolBtn->setIcon(QIcon(":/images/mutedVolume.png"));

    int volume = settings->value("volume", 80).toInt();
    mediaPlayer->setVolume(volume);
    ui->volumeSlider->setValue(volume);
}

void Widget::showLyric(qint64 position)
{
    if(position < 0 || lyricLines.isEmpty())
    {
        ui->musicNameLabel->clear();
        ui->musicianLabel->clear();
        ui->musicAlbumLabel->clear();
        ui->lyricLabelN3->clear();
        ui->lyricLabelN2->clear();
        ui->lyricLabelN1->clear();
        ui->lyricLabel->setText("Local Music Player");
        ui->lyricLabelP1->clear();
        ui->lyricLabelP2->clear();
        ui->lyricLabelP3->clear();
    }
    else
    {
        ui->musicNameLabel->setText(ui->playlistWidget->item(mediaPlaylist->currentIndex(), 0)->text());
        ui->musicianLabel->setText(ui->playlistWidget->item(mediaPlaylist->currentIndex(), 1)->text());
        ui->musicAlbumLabel->setText(ui->playlistWidget->item(mediaPlaylist->currentIndex(), 2)->text());
        int index = getLyricIndex(position);
        ui->lyricLabelN3->setText(getLyricText(index - 3));
        ui->lyricLabelN2->setText(getLyricText(index - 2));
        ui->lyricLabelN1->setText(getLyricText(index - 1));
        ui->lyricLabel->setText(getLyricText(index));
        ui->lyricLabelP1->setText(getLyricText(index + 1));
        ui->lyricLabelP2->setText(getLyricText(index + 2));
        ui->lyricLabelP3->setText(getLyricText(index + 3));
    }
}

void Widget::getLyric(QString &url)
{
    lyricLines.clear();

    int index = url.indexOf(QChar('.'));
    if(index != -1)
    {
        QFile file(QUrl(url.replace(index, url.size() - index, ".lrc")).toLocalFile());
        if(file.open(QFile::ReadOnly))
        {
            QString content = QString::fromUtf8(file.readAll());
            file.close();

            QRegularExpression regExp("\\[(\\d+):(\\d+(\\.\\d+)?)\\]");
            QRegularExpressionMatch match;
            int offset = 0;
            QList<qint64> timeList;
            while(true)
            {
                match = regExp.match(content, offset);
                if(match.hasMatch())
                {
                    if(!timeList.isEmpty())
                    {
                        QString text = content.mid(offset, match.capturedStart() - offset).trimmed();
                        if(!text.isEmpty())
                        {
                            for(qint64 time : timeList)
                                lyricLines.insert(time, text);

                            timeList.clear();
                        }
                    }

                    timeList<< static_cast<qint64>((static_cast<float>(match.captured(1).toInt() * 60) + match.captured(2).toFloat()) * 1000);
                    offset = match.capturedEnd();
                }
                else
                {
                    if(!timeList.isEmpty())
                    {
                        QString text = content.mid(offset).trimmed();
                        if(!text.isEmpty())
                        {
                            for(qint64 time: timeList)
                                lyricLines.insert(time, text);
                        }
                        timeList.clear();
                    }

                    break;
                }

            }
        }
    }
}

int Widget::getLyricIndex(qint64 position)
{
    QMap<qint64, QString>::const_iterator iterator = lyricLines.constBegin();
    int index = -1;
    while(iterator != lyricLines.constEnd())
    {
        if(iterator.key() > position)
            break;

        ++ index;
        ++ iterator;
    }

    return index;
}

QString Widget::getLyricText(int index)
{
    if(index < 0 || index >= lyricLines.size())  return QStringLiteral("");
    else  return lyricLines.values().at(index);
}

void Widget::initSystemTrayIcon()
{
    systemTrayIcon = new QSystemTrayIcon(QIcon(":/images/tray.png"),this);
    systemTrayIcon->setToolTip(tr("Local Music Player"));
    connect(systemTrayIcon, &QSystemTrayIcon::activated, this, &Widget::onSystemTrayIconActivated);

    QMenu *contextMenu = new QMenu(this);
    QAction *previousAction = contextMenu->addAction(QIcon(":/images/trayPrevious.png"), tr("上一首"));
    trayPlayAction = contextMenu->addAction(QIcon(":/images/trayPlay.png"), tr("播放"));
    QAction *nextAction = contextMenu->addAction(QIcon(":/images/trayNext.png"), tr("下一首"));
    QAction *exitAction = contextMenu->addAction(QIcon(":/images/exit.png"), tr("退出应用"));

    connect(previousAction, &QAction::triggered, this, &Widget::on_previousToolBtn_clicked);
    connect(trayPlayAction, &QAction::triggered, this, &Widget::on_playToolBtn_clicked);
    connect(nextAction, &QAction::triggered, this, &Widget::on_nextToolBtn_clicked);
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    systemTrayIcon->setContextMenu(contextMenu);
    systemTrayIcon->show();
}

void Widget::deletePlaylist(QStringList urls)
{
    if(ui->stackedWidget->currentWidget() == ui->playlistPage)
    {
        for(int i = 0; i < ui->playlistWidget->rowCount(); ++ i)
        {
            if(ui->playlistWidget->item(i, 0)->isSelected())
                urls.append(ui->playlistWidget->item(i, 4)->text());
        }
    }

    for(const QString &url : urls)
    {
        for(int i = 0; i < mediaPlaylist->mediaCount(); ++ i)
        {
            if(QMediaContent(url) == mediaPlaylist->media(i))
            {
                mediaPlaylist->removeMedia(i);
                break;
            }
        }
    }
}

void Widget::customizeBackground()
{
    QString dir = settings->value("backgroundPath").toString();
    if(dir.isEmpty())
        dir = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).value(0, QDir::currentPath());

    QString filePath = QFileDialog::getOpenFileName(this, tr("选择背景图片"), dir, tr("Images (*.png *.jpg)"));
    QPixmap pixmap(filePath);
    if(!pixmap.isNull())
    {
        setStyleSheet(ControlStyle::windowBackgroundStyle(filePath));
        settings->setValue("background", filePath);
        settings->setValue("backgroundPath", QFileInfo(filePath).canonicalPath());
    }
}

void Widget::restoreBackground()
{
    QString filePath(":/images/background.jpg");
    setStyleSheet(ControlStyle::windowBackgroundStyle(filePath));
    settings->setValue("background", filePath);
}

void Widget::renameMusicListList()
{
    QString name = QInputDialog::getText(this, tr("重命名歌单"), tr("输入歌单名称："), QLineEdit::Normal, ui->musicListListWidget->currentItem()->text());
    if(!name.isEmpty() && name != ui->musicListListWidget->currentItem()->text())
    {
        QStringList musicList = settings->value("musicListNames").toStringList();
        if(musicList.contains(name))
        {
            ui->warningLabel->setText(tr("已有同名歌单，重命名失败"));
            QTimer::singleShot(2000, this, [&]{ ui->warningLabel->setText(""); });
        }
        else
        {
            musicList[ui->musicListListWidget->currentRow()] = name;
            settings->setValue("musicListNames", musicList);

            ui->musicListListWidget->currentItem()->setText(name);
            ui->musicListTextLabel2->setText(name);
        }
    }
}

void Widget::deleteMusicListList()
{
    QStringList musicList = settings->value("musicListNames").toStringList();
    musicList.removeOne(ui->musicListListWidget->currentItem()->text());
    settings->setValue("musicListNames", musicList);

    QStringList urls;
    QList<MusicInfo> musicInfoList = sqlManager->getMusicListInfo(ui->musicListListWidget->currentItem()->text());
    for(const MusicInfo &musicInfo : musicInfoList)
        urls.append(musicInfo.url);

    QString listName = ui->musicListListWidget->currentItem()->text();
    sqlManager->deleteMusic(urls, "MusicList" ,listName);

    if(ui->stackedWidget->currentWidget() == ui->musicListPage && ui->musicListWidget->getListName() == listName)
        ui->stackedWidget->setCurrentWidget(ui->lyricPage);

    delete ui->musicListListWidget->takeItem(ui->musicListListWidget->currentRow());
}

void Widget::deleteItem()
{
    MusicTableWidget *widget = nullptr;
    if(ui->stackedWidget->currentWidget() == ui->playlistPage)  widget = ui->playlistWidget;
    else if(ui->stackedWidget->currentWidget() == ui->localListPage)  widget = ui->localListWidget;
    else if(ui->stackedWidget->currentWidget() == ui->favoriteListPage)  widget = ui->favoriteListWidget;
    else if(ui->stackedWidget->currentWidget() == ui->musicListPage)  widget = ui->musicListWidget;

    if(widget != nullptr)
    {
        if(widget == ui->localListWidget)
        {
            QStringList urls;
            for(int i = 0; i < widget->rowCount(); ++ i)
            {
                if(widget->item(i, 0)->isSelected())
                    urls.append(widget->item(i, 4)->text());
            }

            ui->playlistWidget->deleteUrls(urls);
            ui->favoriteListWidget->deleteUrls(urls);
            ui->musicListWidget->deleteUrls(urls);

            deletePlaylist(urls);
        }

        if(widget == ui->playlistWidget)
            deletePlaylist();

        widget->deleteMusic(sqlManager);
    }
}

void Widget::addToFavorite()
{
    MusicTableWidget *widget = nullptr;
    if(ui->stackedWidget->currentWidget() == ui->playlistPage)  widget = ui->playlistWidget;
    else if(ui->stackedWidget->currentWidget() == ui->localListPage)  widget = ui->localListWidget;
    else if(ui->stackedWidget->currentWidget() == ui->musicListPage)  widget = ui->musicListWidget;

    if(widget != nullptr)
    {
        QStringList urls;
        for(int i = 0; i < widget->rowCount(); ++ i)
        {
            if(widget->item(i, 0)->isSelected())
                urls.append(widget->item(i, 4)->text());
        }

        ui->favoriteListWidget->addToList(sqlManager, urls);
    }
}

void Widget::addToPlaylist()
{
    MusicTableWidget *widget = nullptr;
    if(ui->stackedWidget->currentWidget() == ui->localListPage)  widget = ui->localListWidget;
    else if(ui->stackedWidget->currentWidget() == ui->favoriteListPage)  widget = ui->favoriteListWidget;
    else if(ui->stackedWidget->currentWidget() == ui->musicListPage)  widget = ui->musicListWidget;

    if(widget != nullptr)
    {
        QStringList urls;
        for(int i = 0; i < widget->rowCount(); ++ i)
        {
            if(widget->item(i, 0)->isSelected())
                urls.append(widget->item(i, 4)->text());
        }       

        QStringList existUrls = sqlManager->getListUrls("Playlist");

        int index = 0;
        while(index < urls.size())
        {
            if(existUrls.contains(urls.at(index)))  urls.removeAt(index);
            else  ++ index;
        }

        ui->playlistWidget->addToList(sqlManager, urls);

        for(const QString &url : urls)
            mediaPlaylist->addMedia(QMediaContent(url));
    }
}

void Widget::onPlayerStateChanged(QMediaPlayer::State state)
{
    switch(state)
    {
    case QMediaPlayer::PlayingState:        
        ui->playToolBtn->setIcon(QIcon(":/images/pause.png"));
        trayPlayAction->setIcon(QIcon(":/images/trayPause.png"));
        ui->playToolBtn->setToolTip(tr("暂停"));
        trayPlayAction->setText(tr("暂停"));
        break;

    case QMediaPlayer::StoppedState:
        onDurationChanged(0);
        ui->playToolBtn->setIcon(QIcon(":/images/play.png"));
        trayPlayAction->setIcon(QIcon(":/images/trayPlay.png"));
        ui->playToolBtn->setToolTip(tr("播放"));
        trayPlayAction->setText(tr("播放"));
        break;

    case QMediaPlayer::PausedState:        
        ui->playToolBtn->setIcon(QIcon(":/images/play.png"));
        trayPlayAction->setIcon(QIcon(":/images/trayPlay.png"));
        ui->playToolBtn->setToolTip(tr("播放"));
        trayPlayAction->setText(tr("播放"));
        break;
    }
}

void Widget::onMetaDataChanged()
{
    if(mediaPlayer->isMetaDataAvailable())
    {
        QStringList availableData = mediaPlayer->availableMetaData();
        MusicInfo musicInfo = ui->playlistWidget->getMusicInfo(sqlManager, mediaPlaylist->currentIndex());
        QList<MusicInfo> musicInfoList;
        musicInfo.title = mediaPlayer->metaData("Title").toString();
        musicInfo.author = mediaPlayer->metaData("Author").toStringList().join(" / ");
        musicInfo.albumTitle = mediaPlayer->metaData("AlbumTitle").toString();
        musicInfo.duration = mediaPlayer->metaData("Duration").toLongLong();
        musicInfo.audioBitRate = mediaPlayer->metaData("AudioBitRate").toInt();
        musicInfoList.append(musicInfo);

        sqlManager->updateMusicInfo(musicInfoList);
        ui->playlistWidget->updateList(musicInfo);
        ui->localListWidget->updateList(musicInfo);
        ui->favoriteListWidget->updateList(musicInfo);
    }
}

void Widget::onMediaPositionChanged(qint64 position)
{
    ui->progressLabel->setText(ui->playlistWidget->formatDuration(position) + " / " + ui->playlistWidget->formatDuration(mediaPlayer->duration()));
    ui->progressSlider->setValue(static_cast<int>(position));
    showLyric(position);
}

void Widget::onDurationChanged(qint64 duration)
{
    ui->progressSlider->setRange(0, static_cast<int>(duration));
    ui->progressSlider->setEnabled(duration > 0);

    if(duration > 0)
    {
        QString url = ui->playlistWidget->item(mediaPlaylist->currentIndex(), 4)->text();
        getLyric(url);
        ui->musicTitleLabel->setText(ui->playlistWidget->item(mediaPlaylist->currentIndex(), 0)->text());
    }
    if(!lyricLines.isEmpty() && mediaPlayer->state() != QMediaPlayer::StoppedState)
    {
        ui->musicNameLabel->setText(ui->playlistWidget->item(mediaPlaylist->currentIndex(), 0)->text());
        ui->musicianLabel->setText(ui->playlistWidget->item(mediaPlaylist->currentIndex(), 1)->text());
        ui->musicAlbumLabel->setText(ui->playlistWidget->item(mediaPlaylist->currentIndex(), 2)->text());
    }
    else
    {
        ui->musicNameLabel->setText("");
        ui->musicianLabel->setText("");
        ui->musicAlbumLabel->setText("");
    }

    ui->playlistWidget->updatePlayingIcon(QIcon(":/images/musicPlayingIcon.png"), mediaPlaylist->currentIndex());
}

void Widget::onCurrentIndexChanged(int index)
{
    if(index >= 0)
    {
        QString url = ui->playlistWidget->item(index, 4)->text();
        settings->setValue("playingUrl", url);

        QImage albumCover = mediaPlayer->metaData("ThumbnailImage").value<QImage>();
        QPixmap pixmap = QPixmap::fromImage(albumCover);
        if(pixmap.isNull())  ui->albumCoverLabel->setPixmap(QPixmap(":/images/albumCover.png"));
        else  ui->albumCoverLabel->setPixmap(pixmap);

        settings->setValue("playingAlbumCover", pixmap);

        MusicInfo musicInfo = ui->playlistWidget->readMusicInfo(index);
        ui->musicTitleLabel->setText(musicInfo.name);
    }
    else
    {
        settings->setValue("playingUrl", "");
        settings->setValue("playingAlbumCover", QPixmap());
        ui->musicTitleLabel->clear();
        ui->musicNameLabel->clear();
        ui->musicianLabel->clear();
        ui->musicAlbumLabel->clear();

        lyricLines.clear();

        systemTrayIcon->setToolTip(tr("Local Music Player"));
    }

    ui->playlistWidget->updatePlayingIcon(QIcon(":/images/musicPlayingIcon.png"), index);
}

void Widget::onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Context:
        return ;

    default:
        show();
    }
}


void Widget::on_skinToolBtn_clicked()
{
    skinToolBtnMenu->popup(ui->skinToolBtn->mapToGlobal(QPoint(0, ui->skinToolBtn->height())));
}

void Widget::on_menuToolBtn_clicked()
{
    menuToolBtnMenu->popup(ui->menuToolBtn->mapToGlobal(QPoint(0, ui->menuToolBtn->height())));
}

void Widget::on_musicListListWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->musicListListWidget->indexAt(pos).isValid())
        musicListListMenu->popup(ui->musicListListWidget->mapToGlobal(pos));
}

void Widget::on_progressSlider_valueChanged(int value)
{
    if(qAbs(mediaPlayer->position() - value) > 99)
        mediaPlayer->setPosition(static_cast<qint64>(value));
}

void Widget::on_volumeToolBtn_clicked()
{
    if(mediaPlayer->isMuted())
    {
        ui->volumeToolBtn->setIcon(QIcon(":/images/volume.png"));
        mediaPlayer->setMuted(false);
    }
    else
    {
        ui->volumeToolBtn->setIcon(QIcon(":/images/mutedVolume.png"));
        mediaPlayer->setMuted(true);
    }

    settings->setValue("isMuted", mediaPlayer->isMuted());
}

void Widget::on_volumeSlider_valueChanged(int value)
{
    mediaPlayer->setVolume(value);
    settings->setValue("volume", value);
}

void Widget::on_previousToolBtn_clicked()
{
    mediaPlaylist->previous();
    mediaPlayer->play();
}

void Widget::on_nextToolBtn_clicked()
{
    mediaPlaylist->next();
    mediaPlayer->play();
}

void Widget::on_playToolBtn_clicked()
{    
    int index = mediaPlaylist->currentIndex();
    if(!mediaPlaylist->isEmpty())
    {
        if(index < 0)
        {
            mediaPlaylist->setCurrentIndex(0);
            index = 0;
        }

        QString name = ui->playlistWidget->item(index, 0)->text();
        QString author = ui->playlistWidget->item(index, 1)->text();

        if(mediaPlayer->state() == QMediaPlayer::PlayingState)
        {
            mediaPlayer->pause();
            systemTrayIcon->setToolTip(tr("已暂停：") + name + " - " + author);
        }
        else
        {
            mediaPlayer->play();
            systemTrayIcon->setToolTip(tr("正在播放：") + name + " - " + author);
        }
    }
}

void Widget::on_playModeToolBtn_clicked()
{
    switch(mediaPlaylist->playbackMode())
    {
    case QMediaPlaylist::Sequential:
        mediaPlaylist->setPlaybackMode(QMediaPlaylist::Loop);
        ui->playModeToolBtn->setIcon(QIcon(":/images/loop.png"));
        ui->playModeToolBtn->setToolTip(tr("循环播放"));
        break;

    case QMediaPlaylist::Loop:
        mediaPlaylist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
        ui->playModeToolBtn->setIcon(QIcon(":/images/singleLoop.png"));
        ui->playModeToolBtn->setToolTip(tr("单曲循环"));
        break;

    case QMediaPlaylist::CurrentItemInLoop:
        mediaPlaylist->setPlaybackMode(QMediaPlaylist::Random);
        ui->playModeToolBtn->setIcon(QIcon(":/images/random.png"));
        ui->playModeToolBtn->setToolTip(tr("随机播放"));
        break;

    default:
        mediaPlaylist->setPlaybackMode(QMediaPlaylist::Sequential);
        ui->playModeToolBtn->setIcon(QIcon(":/images/sequential.png"));
        ui->playModeToolBtn->setToolTip(tr("顺序播放"));
        break;
    }

    settings->setValue("playbackMode", mediaPlaylist->playbackMode());
}

void Widget::on_lyricToolBtn_clicked()
{
    if(!ui->lyricPage->isVisible())
        ui->stackedWidget->setCurrentWidget(ui->lyricPage);
}

void Widget::on_addMusicToolBtn_clicked()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setWindowTitle(tr("添加本地音乐"));

    QString filePath = settings->value("localMusicPath").toString();
    if(!QDir(filePath).exists())
        filePath = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::currentPath());

    fileDialog.setDirectory(filePath);
    fileDialog.setNameFilter("Music (*.mp3 *.wav)");

    if(fileDialog.exec() == QDialog::Accepted)
    {
        if(ui->localListWidget->addMusic(sqlManager, fileDialog.selectedUrls()))
            settings->setValue("localMusicPath", filePath);

        ui->stackedWidget->setCurrentWidget(ui->localListPage);
    }
}

void Widget::on_aboutToolBtn_clicked()
{
    QMessageBox::about(this, tr("关于"), tr("Local Music Player\n"
                                          "【快捷键】\n"
                                          "播放 / 暂停：空格\n"
                                          "上  一  曲 ：Alt + ←\n"
                                          "下  一  曲 ：Alt + →\n"
                                          "添加本地音乐：Ctrl + O\n"));
}

void Widget::on_stackedWidget_currentChanged(int arg1)
{
    Q_UNUSED(arg1)

    ui->playlistPushBtn->setChecked(false);
    ui->localPushBtn->setChecked(false);
    ui->favoritePushBtn->setChecked(false);

    QWidget *currentWidget = ui->stackedWidget->currentWidget();
    if(currentWidget == ui->playlistPage)  ui->playlistPushBtn->setChecked(true);
    else if(currentWidget == ui->localListPage)  ui->localPushBtn->setChecked(true);
    else if(currentWidget == ui->favoriteListPage)  ui->favoritePushBtn->setChecked(true);
}

void Widget::on_playlistPushBtn_clicked()
{
    if(ui->stackedWidget->currentWidget() != ui->playlistPage)
        ui->stackedWidget->setCurrentWidget(ui->playlistPage);
}

void Widget::on_localPushBtn_clicked()
{
    if(ui->stackedWidget->currentWidget() != ui->localListPage)
        ui->stackedWidget->setCurrentWidget(ui->localListPage);
}

void Widget::on_favoritePushBtn_clicked()
{
    if(ui->stackedWidget->currentWidget() != ui->favoriteListPage)
        ui->stackedWidget->setCurrentWidget(ui->favoriteListPage);
}

void Widget::on_playlistClearToolBtn_clicked()
{
    mediaPlaylist->setCurrentIndex(-1);
    mediaPlaylist->clear();
    ui->playlistWidget->selectAll();
    ui->playlistWidget->deleteMusic(sqlManager);
}

void Widget::on_musicListAddToolBtn_clicked()
{
    QString name = QInputDialog::getText(this, tr("新建歌单"), tr("输入歌单名称："));
    if(!name.isEmpty())
    {
        QStringList musicList = settings->value("musicListNames").toStringList();
        if(musicList.contains(name))
        {
            ui->warningLabel->setText(tr("已有同名歌单，创建失败"));
            QTimer::singleShot(2000, this, [&]{ ui->warningLabel->setText(""); });
        }
        else
        {
            musicList.append(name);
            settings->setValue("musicListNames", musicList);

            QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/musicListIcon.png"), name, ui->musicListListWidget);
            ui->musicListListWidget->addItem(item);

            on_musicListListWidget_itemDoubleClicked(item);
        }
    }
}

void Widget::on_musicListListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if(ui->musicListWidget->getListName() != item->text())
    {
        ui->musicListWidget->setListName(item->text());
        ui->musicListWidget->clearContents();
        ui->musicListTextLabel2->setText(item->text());
        ui->musicListWidget->readMusicListData(sqlManager, item->text());
    }

    ui->stackedWidget->setCurrentWidget(ui->musicListPage);
}

void Widget::on_localListMusicianToolBtn_clicked()
{
    ui->localListWidget->sortByMusician(sqlManager);
}

void Widget::on_localListSongsToolBtn_clicked()
{
    ui->localListWidget->sortBySongs(sqlManager);
}

void Widget::on_localListDurationToolBtn_clicked()
{
    ui->localListWidget->sortByDuration(sqlManager);
}

void Widget::on_localListClearToolBtn_clicked()
{
    ui->localListWidget->selectAll();

    QStringList urls;
    for(int i = 0; i < ui->localListWidget->rowCount(); ++ i)
        urls.append(ui->localListWidget->item(i, 4)->text());

    ui->playlistWidget->deleteUrls(urls);
    ui->favoriteListWidget->deleteUrls(urls);
    ui->musicListWidget->deleteUrls(urls);

    ui->localListWidget->deleteMusic(sqlManager);
}

void Widget::on_favoriteListMusicianToolBtn_clicked()
{
    ui->favoriteListWidget->sortByMusician(sqlManager);
}

void Widget::on_favoriteListSongsToolBtn_clicked()
{
    ui->favoriteListWidget->sortBySongs(sqlManager);
}

void Widget::on_favoriteListDurationToolBtn_clicked()
{
    ui->favoriteListWidget->sortByDuration(sqlManager);
}

void Widget::on_favoriteListClearToolBtn_clicked()
{
    ui->favoriteListWidget->selectAll();
    ui->favoriteListWidget->deleteMusic(sqlManager);
}

void Widget::on_musicListClearToolBtn_clicked()
{
    ui->musicListWidget->selectAll();
    ui->musicListWidget->deleteMusic(sqlManager);
}

void Widget::on_musicListAddToolBtn2_clicked()
{
    QStringList urls, names;
    for(int i = 0; i < ui->localListWidget->rowCount(); ++ i)
    {
        urls.append(ui->localListWidget->item(i, 4)->text());
        names.append(ui->localListWidget->item(i, 0)->text());
    }

    QStringList selectedUrls;
    AddMusicDialog dialog(this, &selectedUrls, urls, names);
    if(dialog.exec() == QDialog::Accepted)
        ui->musicListWidget->addToMusicList(sqlManager, selectedUrls);
}

void Widget::on_musicListMusicianToolBtn_clicked()
{
    ui->musicListWidget->sortByMusician(sqlManager);
}

void Widget::on_musicListSongsToolBtn_clicked()
{
    ui->musicListWidget->sortBySongs(sqlManager);
}

void Widget::on_musicListDurationToolBtn_clicked()
{
    ui->musicListWidget->sortByDuration(sqlManager);
}

void Widget::on_playlistWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    if(mediaPlayer->state() != QMediaPlayer::PlayingState)
    {
        mediaPlaylist->setCurrentIndex(item->row());
        mediaPlayer->play();
    }
    else if(mediaPlaylist->currentIndex() != item->row())
    {
        mediaPlaylist->setCurrentIndex(item->row());
        mediaPlayer->play();
    }
}

void Widget::on_localListWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    addToPlaylist();
    for(int i = 0; i < ui->playlistWidget->rowCount(); ++ i)
    {
        if(ui->playlistWidget->item(i, 4)->text() == ui->localListWidget->item(item->row(), 4)->text())
        {
            mediaPlaylist->setCurrentIndex(i);
            mediaPlayer->play();
        }
    }
}

void Widget::on_favoriteListWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    addToPlaylist();
    for(int i = 0; i < ui->playlistWidget->rowCount(); ++ i)
    {
        if(ui->playlistWidget->item(i, 4)->text() == ui->favoriteListWidget->item(item->row(), 4)->text())
        {
            mediaPlaylist->setCurrentIndex(i);
            mediaPlayer->play();
        }
    }
}

void Widget::on_musicListWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    addToPlaylist();
    for(int i = 0; i < ui->playlistWidget->rowCount(); ++ i)
    {
        if(ui->playlistWidget->item(i, 4)->text() == ui->musicListWidget->item(item->row(), 4)->text())
        {
            mediaPlaylist->setCurrentIndex(i);
            mediaPlayer->play();
        }
    }
}
