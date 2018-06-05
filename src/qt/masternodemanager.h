#ifndef MASTERNODEMANAGER_H
#define MASTERNODEMANAGER_H

#include "util.h"
#include "sync.h"

#include <QGraphicsScene>
#include <QWidget>
#include <QTimer>
#include <QFuture>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#define MASTERNODELIST_UPDATE_SECONDS 60
#define MASTERNODELIST_FILTER_COOLDOWN_SECONDS 60

namespace Ui {
    class MasternodeManager;

}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Masternode Manager page widget */
class MasternodeManager : public QWidget
{
    Q_OBJECT

public:
    explicit MasternodeManager(QWidget *parent = 0);
    ~MasternodeManager();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);
    void updateListConc();
    void updateCountries();
    QNetworkAccessManager *networkManager;
    QNetworkReply *NetReply;

public slots:
    void updateNodeList();
    void updateAdrenalineNode(QString alias, QString addr, QString privkey, QString txHash, QString txIndex, QString donationAddress, QString donationPercentage, QString status);
    void on_UpdateButton_clicked();



signals:

private:
    QTimer *timer;
    Ui::MasternodeManager *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    CCriticalSection cs_adrenaline;
    int64_t nTimeFilterUpdated;
	bool fFilterUpdated;
	QFuture<void> f1;
	QFuture<void> f2;

private slots:
    void on_createButton_clicked();
    void on_startButton_clicked();
    void on_startAllButton_clicked();
    void on_tableWidget_2_itemSelectionChanged();
};
#endif // MASTERNODEMANAGER_H
