#ifndef MARKETS_H
#define MARKETS_H

#include "clientmodel.h"
#include "walletmodel.h"
#include <QWidget>
#include <QTimer>
#include <QFuture>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace Ui {
    class Markets;

}

class WalletModel;

/** Markets page widget */
class Markets : public QWidget
{
    Q_OBJECT

public:
    explicit Markets(QWidget *parent = 0);
    ~Markets();

    void setModel(WalletModel *model);
    void updateMarkets();
    QNetworkAccessManager *networkManager;
    QNetworkReply *NetReply;

public slots:
    void addcoinClicked();
    void updateMarketTimes();



signals:

private:
    QTimer *timer;
    Ui::Markets *ui;

    WalletModel *model;

    int64_t nTimeFilterUpdated;
	//bool fFilterUpdated;
	QFuture<void> f5;


private slots:
    //void on_addcoinButton_clicked();

};
#endif // MARKETS_H
