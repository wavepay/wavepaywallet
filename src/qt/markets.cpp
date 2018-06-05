#include "markets.h"
#include "ui_markets.h"
#include "walletmodel.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QSslConfiguration>
#include <QVariant>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>
#include <QMessageBox>
#include <QDesktopServices>

#include <QtConcurrent/QtConcurrent>
#include <QScrollBar>
#include <openssl/ssl.h>

Markets::Markets(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Markets)
{
    ui->setupUi(this);

	connect(ui->addcoinButton, SIGNAL(pressed()), this, SLOT(addcoinClicked()));
	networkManager = new QNetworkAccessManager(this);	
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateMarketTimes()));
    if(!GetBoolArg("-reindexaddr", false))
        timer->start(1000);
   
	nTimeFilterUpdated = GetTime();
 
}
Markets::~Markets()
{
    delete ui;
}
void Markets::updateMarketTimes()
{
        
	static int64_t nTimeListUpdated = GetTime();

        int64_t nSecondsToWait = nTimeFilterUpdated - GetTime() + 60;

    ui->CountRelabel->setText(QString::fromStdString(strprintf("Auto Refresh In %d", nSecondsToWait)));
    if (nSecondsToWait > 0) return;

    nTimeListUpdated = GetTime();
    nTimeFilterUpdated = GetTime();
    updateMarkets();
	if (f5.isFinished())
		f5 = QtConcurrent::run(this,&Markets::updateMarkets);

}

void Markets::updateMarkets()
{
	    // create custom temporary event loop on stack
    QEventLoop eventLoop_;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr_;
    QObject::connect(&mgr_, SIGNAL(finished(QNetworkReply*)), &eventLoop_, SLOT(quit()));

    // the HTTP request
    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
    sslConfiguration.setProtocol(QSsl::TlsV1_2);
    QNetworkRequest req;
    req.setSslConfiguration(sslConfiguration);
    req.setUrl(QUrl("https://api.coinmarketcap.com/v2/ticker/660/?convert=BTC"));

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = mgr_.get(req);
    eventLoop_.exec(); // blocks stack until "finished()" has been called

    QString Qtsupply;
    QString QusdPrice;
    QString QusdVol;
    QString QusdCap;
    QString QusdChange;
    QString QbtcPrice;
    QString QbtcVol;
    QString QbtcCap;
    QString QbtcChange;

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(statusCode == 302)
    {
       QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

       QNetworkRequest newRequest(newUrl);
       mgr_.get(newRequest);
       return;
    }
    if (reply->error() == QNetworkReply::NoError) {

        QString strReply = (QString)reply->readAll();
        QJsonDocument jsdoc = QJsonDocument::fromJson(strReply.toUtf8());
	QJsonObject jsonObject = jsdoc.object();
	QJsonObject jsondata = jsonObject.value("data").toObject();
	QJsonObject jsonquote = jsondata.value("quotes").toObject(); 
	QJsonObject jsonusd = jsonquote.value("USD").toObject();
	QJsonObject jsonbtc = jsonquote.value("BTC").toObject();
	double tsupply = jsondata.value("total_supply").toDouble();
	double usdPrice = jsonusd.value("price").toDouble();
	double usdVol = jsonusd.value("volume_24h").toDouble();
	double usdCap = jsonusd.value("market_cap").toDouble();
	double usdChange = jsonusd.value("percent_change_24h").toDouble();
	double btcPrice = jsonbtc.value("price").toDouble();
	double btcVol = jsonbtc.value("volume_24h").toDouble();
	double btcCap = jsonbtc.value("market_cap").toDouble();
	double btcChange = jsonbtc.value("percent_change_24h").toDouble();
	Qtsupply = QString::number(tsupply, 'f', 8);
	QusdPrice = QString::number(usdPrice, 'f', 8);
	QusdVol = QString::number(usdVol, 'f', 8);
	QusdCap = QString::number(usdCap, 'f', 8);
	QusdChange = QString::number(usdChange, 'f', 2);
	QbtcPrice = QString::number(btcPrice, 'f', 8);
	QbtcVol = QString::number(btcVol, 'f', 8);
	QbtcCap = QString::number(btcCap, 'f', 8);
	QbtcChange = QString::number(btcChange, 'f', 2);

        delete reply;
    }
    else {
        //failure 

        delete reply;
    }
	ui->Dsupplylabel->setText(Qtsupply + " DASH");
	ui->DusdPricelabel->setText(QusdPrice + " USD");
	ui->DusdVollabel->setText(QusdVol + " USD");
	ui->DusdMarketlabel->setText(QusdCap + " USD");
	ui->DusdChangelabel->setText(QusdChange + " %");
	ui->DbtcPricelabel->setText(QbtcPrice + " BTC");
	ui->DbtcVollabel->setText(QbtcVol + " BTC");
	ui->DbtcMarketlabel->setText(QbtcCap + " BTC");
	ui->DbtcChangelabel->setText(QbtcChange + " %");
}

void Markets::addcoinClicked()
{
    QString link = "https://docs.google.com/forms/d/e/1FAIpQLSeqpM6DVuxNRJ43oD90fCsm8UltR9oIiFaVxxqt3ch_eJ5Dlw/viewform";
QDesktopServices::openUrl(QUrl(link));
}
void Markets::setModel(WalletModel *model)
{
    this->model = model;
}

