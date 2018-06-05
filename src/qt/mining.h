#ifndef MINING_H
#define MINING_H

#include "walletmodel.h"
#include <QWidget>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QProcess>
#include <QTime>
#include <QTimer>
#include <QStringList>
#include <QMap>
#include <QSettings>
#include <QEvent>
#include <QMenu>
#include <QDialog>
#include <QIcon>

#include <QFile>

#include <QSystemTrayIcon>

#include "json.h"

// Log types
#define STARTED 0
#define SHARE_SUCCESS 1
#define SHARE_FAIL 2
#define ERROR 3
#define LONGPOLL 4
#define FATAL_ERROR 5

namespace Ui {
    class Mining;
}
class WalletModel;

class Mining : public QWidget
{
    Q_OBJECT

public:
    explicit Mining(QWidget *parent = 0);
    ~Mining();

    void setModel(WalletModel *model);
    QString appDirPath;

    QIcon inactiveIcon;
    QIcon activeIcon;

    QSystemTrayIcon *trayIcon;

    bool minerActive;

    QNetworkAccessManager *networkManager;

    QProcess *minerProcess;

    QMap<int, double> threadSpeed;

    QTimer *readTimer;
    QTimer *poolTimer;

    QFile *outputFile;

    int acceptedShares;
    int rejectedShares;

    int roundAcceptedShares;
    int roundRejectedShares;

    int initThreads;

public slots:
    //
    void changeEvent(QEvent *);
    void setAppDirPath();

    void startPressed();
    void createTrayActions();

    QStringList getArgs();

    void startMining();
    void stopMining();

    // Window manipulation stuff
    void resizeElements();
    void showMainWindow();
    void hideMainWindow();

    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void updateSpeed();

    void updatePoolData();

    void checkSettings();
    void saveSettings();

    void recordOutput(QString text);

    void reportToList(QString, int, QString);

    void minerStarted();

    void minerError(QProcess::ProcessError);
    void minerFinished(int, QProcess::ExitStatus);

    void poolDataLoaded(QNetworkReply*);

    void readProcessOutput();

    QString getTime(QString);

private:
    Ui::Mining *ui;
    WalletModel *model;
};

#endif // MINING_H
