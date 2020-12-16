#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; class Dialog; }
QT_END_NAMESPACE

class QSettings;
class QMenu;
class SqlManager;
class QMediaPlaylist;
class QListWidgetItem;
class AddMusicDialog;
class QTableWidgetItem;

struct MusicInfo;
struct LyricLine;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Widget *ui;
    Ui::Dialog *addMusicDialog;

    QPoint offset;
    bool isMoving = false;

    QSettings *settings;

    QMenu *skinToolBtnMenu;
    QMenu *menuToolBtnMenu;

    QMenu *musicListListMenu;
    QMenu *playlistMenu;
    QMenu *localListMenu;
    QMenu *favoriteListMenu;
    QMenu *musicListMenu;

    QSystemTrayIcon *systemTrayIcon;
    QAction *trayPlayAction;

    SqlManager *sqlManager;

    QMediaPlayer *mediaPlayer;
    QMediaPlaylist *mediaPlaylist;

    QMap<qint64, QString> lyricLines;

    void initUI();
    void initSettings();
    void initMenu();
    void readMusicListData();
    void initPlayer();
    void initPlaylist();
    void initVolume();
    void showLyric(qint64 position);
    void getLyric(QString &url);
    int getLyricIndex(qint64 position);
    QString getLyricText(int index);
    void initSystemTrayIcon();
    void deletePlaylist(QStringList urls = QStringList());

private slots:
    void customizeBackground();
    void restoreBackground();
    void renameMusicListList();
    void deleteMusicListList();
    void deleteItem();
    void addToFavorite();
    void addToPlaylist();

    void onPlayerStateChanged(QMediaPlayer::State state);
    void onMetaDataChanged();
    void onMediaPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onCurrentIndexChanged(int index);
    void onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void on_skinToolBtn_clicked();
    void on_menuToolBtn_clicked();
    void on_musicListListWidget_customContextMenuRequested(const QPoint &pos);
    void on_progressSlider_valueChanged(int value);
    void on_volumeToolBtn_clicked();
    void on_volumeSlider_valueChanged(int value);
    void on_previousToolBtn_clicked();
    void on_nextToolBtn_clicked();
    void on_playToolBtn_clicked();
    void on_playModeToolBtn_clicked();
    void on_lyricToolBtn_clicked();
    void on_addMusicToolBtn_clicked();
    void on_aboutToolBtn_clicked();
    void on_stackedWidget_currentChanged(int arg1);
    void on_playlistPushBtn_clicked();
    void on_localPushBtn_clicked();
    void on_favoritePushBtn_clicked();
    void on_playlistClearToolBtn_clicked();
    void on_musicListAddToolBtn_clicked();
    void on_musicListListWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_localListMusicianToolBtn_clicked();
    void on_localListSongsToolBtn_clicked();
    void on_localListDurationToolBtn_clicked();
    void on_localListClearToolBtn_clicked();
    void on_favoriteListMusicianToolBtn_clicked();
    void on_favoriteListSongsToolBtn_clicked();
    void on_favoriteListDurationToolBtn_clicked();
    void on_favoriteListClearToolBtn_clicked();
    void on_musicListClearToolBtn_clicked();
    void on_musicListAddToolBtn2_clicked();
    void on_musicListMusicianToolBtn_clicked();
    void on_musicListSongsToolBtn_clicked();
    void on_musicListDurationToolBtn_clicked();
    void on_playlistWidget_itemDoubleClicked(QTableWidgetItem *item);
    void on_localListWidget_itemDoubleClicked(QTableWidgetItem *item);
    void on_favoriteListWidget_itemDoubleClicked(QTableWidgetItem *item);
    void on_musicListWidget_itemDoubleClicked(QTableWidgetItem *item);
};

//struct LyricLine
//{
//    qint64 time;
//    QString lyric;

//    LyricLine(qint64 t, const QString &text) : time(t), lyric(text) {}
//};
#endif // WIDGET_H
