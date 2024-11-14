#ifndef WATCHDIAL_H
#define WATCHDIAL_H

#include <QObject>
#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QPixmap>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QPoint>

class watchdial : public QWidget
{
    Q_OBJECT

public:
    watchdial(QWidget* parent = nullptr);
    void paintEvent(QPaintEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void updateDial();
    void pause();
    void reset();
    void exit();
    void minimize();
    void restore();

private:
    QTime* timer;
    QLabel* countdownLabel;
    QPixmap dialImage;
    QPixmap secondHandImage;
    QPixmap minuteHandImage;
    bool showMinuteHand;
    bool isWorking;
    int second;
    int minute;
    QMenu* contextMenu;
    QAction* pauseAction;
    QAction* minimizeAction;
    QSystemTrayIcon* trayIcon;
    QMenu* trayIconMenu;
    QPoint dragPosition;
};

#endif // WATCHDIAL_H
