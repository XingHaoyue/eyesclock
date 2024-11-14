#include "watchdial.h"
#include <QPainter>
#include <QTime>
#include <QTimerEvent>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QContextMenuEvent>
#include <QAction>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>

watchdial::watchdial(QWidget* parent)
    : QWidget(parent), showMinuteHand(true), isWorking(true), second(0), minute(0), isResting(false), restSecond(0)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    timer = new QTime(0, 0, 0);
    countdownLabel = new QLabel(this);
    countdownLabel->setAlignment(Qt::AlignCenter);
    countdownLabel->setStyleSheet("QLabel {"
                                  "font-family: '黑体';"
                                  "font-style: italic;"
                                  "color: #3dc5c4;"
                                  "font-size: 18px;}"
                                  );

    QVBoxLayout* layout = new QVBoxLayout(this);
    QSpacerItem* topSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QSpacerItem* bottomSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

    layout->addItem(topSpacer);
    layout->addWidget(countdownLabel);
    layout->addItem(bottomSpacer);
    layout->setStretch(0, 3); // 上部空间占3份
    layout->setStretch(1, 1); // QLabel占1份
    layout->setStretch(2, 1); // 下部空间占1份
    setLayout(layout);

    dialImage.load(":/images/dial.png");
    secondHandImage.load(":/images/second_hand.png");
    minuteHandImage.load(":/images/minute_hand.png");

    QTimer* updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &watchdial::updateDial);
    updateTimer->start(1000); // 每秒更新一次

    updateDial(); // 初始化显示

    contextMenu = new QMenu(this);
    pauseAction = new QAction(tr("暂停"), this);
    QAction* resetAction = new QAction(tr("重置"), this);
    minimizeAction = new QAction(tr("最小化"), this);
    QAction* exitAction = new QAction(tr("退出"), this);
    connect(pauseAction, &QAction::triggered, this, &watchdial::pause);
    connect(resetAction, &QAction::triggered, this, &watchdial::reset);
    connect(minimizeAction, &QAction::triggered, this, &watchdial::minimize);
    connect(exitAction, &QAction::triggered, this, &watchdial::exit);
    contextMenu->addAction(pauseAction);
    contextMenu->addAction(resetAction);
    contextMenu->addAction(minimizeAction);
    contextMenu->addAction(exitAction);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(pauseAction);
    trayIconMenu->addAction(resetAction);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(exitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/images/dial.png"));
    connect(trayIcon, &QSystemTrayIcon::activated, this, &watchdial::restore);
    trayIcon->show();
}

void watchdial::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int side = qMin(width(), height());
    painter.setViewport((width() - side) / 2, (height() - side) / 2, side, side);
    painter.setWindow(-50, -50, 100, 100);

    painter.drawPixmap(-50, -50, 100, 100, dialImage);

    painter.save();
    if (isResting) {
        painter.rotate(18.0 * restSecond); // 休息时间内每秒旋转18度
    } else {
        painter.rotate(6.0 * second); // 工作时间内每秒旋转6度
    }
    painter.drawPixmap(-1, -35, 2, 30, secondHandImage); // 缩短秒针长度
    painter.restore();

    if (showMinuteHand) {
        painter.save();
        painter.rotate(18.0 * minute); // 每分钟旋转6度
        painter.drawPixmap(-1, -25, 2, 20, minuteHandImage);
        painter.restore();
    }
}

void watchdial::updateDial()
{
    if (isWorking) {
        if (isResting) {
            restSecond++;
            if (restSecond == 20) {
                restSecond = 0;
                isResting = false;
                showMinuteHand = true;
            }
        } else {
            second++;
            if (second == 60) {
                second = 0;
                minute++;
                if (minute == 20) { // 恢复为20分钟
                    minute = 0;
                    showMinuteHand = false;
                    isResting = true;
                    trayIcon->showMessage(tr("提示"), tr("20分钟到了，放松一下眼睛吧！"));
                }
            }
        }

        if (isResting) {
            int remainingSeconds = 20 - restSecond;
            countdownLabel->setText(QString("00:%1").arg(remainingSeconds, 2, 10, QChar('0')));
        } else {
            int remainingMinutes = 19 - minute; // 20分钟倒计时
            int remainingSeconds = 59 - second;
            countdownLabel->setText(QString("%1:%2")
                .arg(remainingMinutes, 2, 10, QChar('0'))
                .arg(remainingSeconds, 2, 10, QChar('0')));
        }

        update();
    }
}

void watchdial::pause()
{
    isWorking = !isWorking;
    pauseAction->setText(isWorking ? tr("暂停") : tr("启动"));
}

void watchdial::reset()
{
    second = 0;
    minute = 0;
    restSecond = 0;
    isResting = false;
    showMinuteHand = true;
    isWorking = true;
    pauseAction->setText(tr("暂停"));
    update();
}

void watchdial::exit()
{
    QApplication::quit();
}

void watchdial::minimize()
{
    hide();
    trayIcon->showMessage(tr("提示"), tr("表盘已最小化到托盘。"));
}

void watchdial::restore()
{
    show();
    activateWindow();
}

void watchdial::contextMenuEvent(QContextMenuEvent* event)
{
    contextMenu->exec(event->globalPos());
}

void watchdial::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void watchdial::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}
