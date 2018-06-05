#include "masternodemanager.h"
#include "ui_masternodemanager.h"
#include "addeditadrenalinenode.h"
#include "adrenalinenodeconfigdialog.h"

#include "sync.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "activemasternode.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "masternode.h"
#include "main.h"
#include "walletdb.h"
#include "wallet.h"
#include "init.h"
#include "rpcserver.h"



#include <boost/lexical_cast.hpp>
#include <fstream>
using namespace json_spirit;
using namespace std;

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QScrollArea>
#include <QScroller>
#include <QDateTime>
#include <QApplication>
#include <QClipboard>
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
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QScrollBar>
#include <openssl/ssl.h>
// We keep track of the last error against each masternode and use them if available
// to provide additional feedback to user.
std::map<std::string, std::string> lastMasternodeErrors;
void setLastMasternodeError(const std::string& masternode, std::string error)
{
    lastMasternodeErrors[masternode] = error;
}
void getLastMasternodeError(const std::string& masternode, std::string& error)
{
    std::map<std::string, std::string>::iterator it = lastMasternodeErrors.find(masternode);
    if (it != lastMasternodeErrors.end()){
        error = (*it).second;
    }
}

int64_t getMnRewardDay(int days)
{
    int64_t ret = 0;
    int64_t Times = GetTargetSpacingWork(pindexBest->nHeight);
  //  int64_t blockValue = GetProofOfWorkReward(pindexBest->nHeight, 0);
  //  int64_t reward = GetMasternodePayment(pindexBest->nHeight, blockValue);
	if (days == 1)
           ret = (86400 / Times) * 0.5 * days;
	if (days == 7)
           ret = (86400 / Times) * 0.5 * days;
	if (days == 30)
           ret = (86400 / Times) * 0.5 * days;
	if (days == 360)
           ret = (86400 / Times) * 0.5 * days;
    return ret;
}
double getMnCoinLocked(int persen)
{
    double ret = 0;
    int64_t MnCounts = mnodeman.size();
    int64_t MnCoin = GetMNCollateral(pindexBest->nHeight);
    double MnSupplypersen = pindexBest->nMoneySupply / 100000000;
    int64_t MnCoinlock = MnCounts * MnCoin;
    if (persen == 0){
	if (MnCounts > 0)
           ret = MnCoinlock;
    }
    if (persen == 1){
	if (MnCounts > 0)
           ret = (MnCoinlock / MnSupplypersen) * 100;
    }

    return ret;
}

MasternodeManager::MasternodeManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MasternodeManager),
    clientModel(0),
    walletModel(0)
{
    ui->setupUi(this);

    ui->editButton->setEnabled(false);
    ui->startButton->setEnabled(false);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	 ui->tableWidget_4->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	networkManager = new QNetworkAccessManager(this);	
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateNodeList()));
    if(!GetBoolArg("-reindexaddr", false))
        timer->start(1000);
    fFilterUpdated = true;
	nTimeFilterUpdated = GetTime();
    updateNodeList();
ui->Maplabel->setStyleSheet("background-image:url(:/images/worldmaps/world)");
}

MasternodeManager::~MasternodeManager()
{
    delete ui;
}

void MasternodeManager::on_tableWidget_2_itemSelectionChanged()
{
    if(ui->tableWidget_2->selectedItems().count() > 0)
    {
        ui->editButton->setEnabled(true);
        ui->startButton->setEnabled(true);
    }
}

void MasternodeManager::updateAdrenalineNode(QString alias, QString addr, QString privkey, QString txHash, QString txIndex, QString donationAddress, QString donationPercentage, QString status)
{
    LOCK(cs_adrenaline);
    bool bFound = false;
    int nodeRow = 0;
    for(int i=0; i < ui->tableWidget_2->rowCount(); i++)
    {
        if(ui->tableWidget_2->item(i, 0)->text() == alias)
        {
            bFound = true;
            nodeRow = i;
            break;
        }
    }

    if(nodeRow == 0 && !bFound)
        ui->tableWidget_2->insertRow(0);

    QTableWidgetItem *aliasItem = new QTableWidgetItem(alias);
    QTableWidgetItem *addrItem = new QTableWidgetItem(addr);
    QTableWidgetItem *donationAddressItem = new QTableWidgetItem(donationAddress);
    QTableWidgetItem *donationPercentageItem = new QTableWidgetItem(donationPercentage);
    QTableWidgetItem *statusItem = new QTableWidgetItem(status);

    ui->tableWidget_2->setItem(nodeRow, 0, aliasItem);
    ui->tableWidget_2->setItem(nodeRow, 1, addrItem);
    ui->tableWidget_2->setItem(nodeRow, 2, donationPercentageItem);
    ui->tableWidget_2->setItem(nodeRow, 3, donationAddressItem);
    ui->tableWidget_2->setItem(nodeRow, 4, statusItem);
}

static QString seconds_to_DHMS(quint32 duration)
{
  QString res;
  int seconds = (int) (duration % 60);
  duration /= 60;
  int minutes = (int) (duration % 60);
  duration /= 60;
  int hours = (int) (duration % 24);
  int days = (int) (duration / 24);
  if((hours == 0)&&(days == 0))
      return res.sprintf("%02dm:%02ds", minutes, seconds);
  if (days == 0)
      return res.sprintf("%02dh:%02dm:%02ds", hours, minutes, seconds);
  return res.sprintf("%dd %02dh:%02dm:%02ds", days, hours, minutes, seconds);
}

void MasternodeManager::updateCountries() {
	ui->tableWidget_4->clearContents();
	ui->tableWidget_4->setRowCount(0);
	std::vector<CMasternode> vMasternodes = mnodeman.GetFullMasternodeVector();
	ui->tableWidget_4->horizontalHeader()->setSortIndicator(ui->tableWidget_4->horizontalHeader()->sortIndicatorSection() ,ui->tableWidget_4->horizontalHeader()->sortIndicatorOrder());
        QString QMntotals = QString::number((int)mnodeman.size());
	QList<QPixmap> PixList;

    if (QMntotals > 0) {
	BOOST_FOREACH(CMasternode& mn, vMasternodes)
	{
	int mnRow = 0;
	QString mnaddr = QString::fromStdString(mn.addr.ToString());
	mnaddr.remove(":5279");
	QString country;
	QString countrycode;
    	QString url = "http://ip-api.com/json/"+mnaddr+"?fields=country,countryCode";
    // create custom temporary event loop on stack
    	QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    	QNetworkAccessManager mgr;
    	QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    // the HTTP request
   	QNetworkRequest req;
    	req.setUrl(QUrl(url));
    	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    	QNetworkReply *replyaddr = mgr.get(req);
    	eventLoop.exec(); // blocks stack until "finished()" has been called

    	if (replyaddr->error() == QNetworkReply::NoError) {

        	QString strReplyaddr = (QString)replyaddr->readAll();

        //parse json
        	qDebug() << "Response:" << strReplyaddr;
        	QJsonDocument jsonResponse = QJsonDocument::fromJson(strReplyaddr.toUtf8());
        	QJsonObject json_object = jsonResponse.object();
		
        	country = json_object.value("country").toString();
		countrycode = json_object.value("countryCode").toString();
        	delete replyaddr;
   	}
    	else {
        //failure 
        	country = replyaddr->errorString();
        	delete replyaddr;
    	}

	ui->tableWidget_4->insertRow(0);
	

	QTableWidgetItem *countryItem = new QTableWidgetItem(country);
	QTableWidgetItem *countryCodeItem = new QTableWidgetItem(countrycode);
        ui->tableWidget_4->setItem(mnRow, 0, countryItem);

	PixList.append(QPixmap(":/images/worldmaps/"+countrycode));

	}
    }
	QPixmap *pixmap=new QPixmap(695,490);
	pixmap->fill(Qt::transparent);

	QPainter *painter=new QPainter(pixmap);

	for(int i=0; i < PixList.size(); ++i)
	{
    		painter->drawPixmap(0,0,695,490, PixList.at(i));
	}
	painter->end();
	ui->Maplabel->setPixmap(*pixmap);
	PixList.clear();

}

void MasternodeManager::updateListConc() {
	if (ui->tableWidget->isVisible()) 
	{
		ui->tableWidget_3->clearContents();
		ui->tableWidget_3->setRowCount(0);
		std::vector<CMasternode> vMasternodes = mnodeman.GetFullMasternodeVector();
		ui->tableWidget_3->horizontalHeader()->setSortIndicator(ui->tableWidget->horizontalHeader()->sortIndicatorSection() ,ui->tableWidget->horizontalHeader()->sortIndicatorOrder());

		BOOST_FOREACH(CMasternode& mn, vMasternodes)
		{
			int mnRow = 0;
			ui->tableWidget_3->insertRow(0);
			
			// populate list
			// Address, Rank, Active, Active Seconds, Last Seen, Pub Key
			QTableWidgetItem *activeItem = new QTableWidgetItem(QString::number(mn.IsEnabled()));
			QTableWidgetItem *addressItem = new QTableWidgetItem(QString::fromStdString(mn.addr.ToString()));
			
			QString Rank = QString::number(mnodeman.GetMasternodeRank(mn.vin, pindexBest->nHeight));
			QTableWidgetItem *rankItem = new QTableWidgetItem(Rank.rightJustified(2, '0', false));
			QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(seconds_to_DHMS((qint64)(mn.lastTimeSeen - mn.sigTime)));
			QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat(mn.lastTimeSeen)));

			CScript pubkey;
			pubkey =GetScriptForDestination(mn.pubkey.GetID());
			CTxDestination address1;
			ExtractDestination(pubkey, address1);
			CWavepaycoinAddress address2(address1);
			QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(address2.ToString()));

			ui->tableWidget_3->setItem(mnRow, 0, addressItem);
			ui->tableWidget_3->setItem(mnRow, 1, rankItem);
			ui->tableWidget_3->setItem(mnRow, 2, activeItem);
			ui->tableWidget_3->setItem(mnRow, 3, activeSecondsItem);
			ui->tableWidget_3->setItem(mnRow, 4, lastSeenItem);
			ui->tableWidget_3->setItem(mnRow, 5, pubkeyItem);
		}
		ui->countLabel->setText(QString::number(ui->tableWidget_3->rowCount()));
		on_UpdateButton_clicked();
		ui->tableWidget->setVisible(0);
		ui->tableWidget_3->setVisible(1);
		ui->tableWidget_3->verticalScrollBar()->setSliderPosition(ui->tableWidget->verticalScrollBar()->sliderPosition());
	}
	else
		{
		ui->tableWidget->clearContents();
		ui->tableWidget->setRowCount(0);
		std::vector<CMasternode> vMasternodes = mnodeman.GetFullMasternodeVector();
		ui->tableWidget->horizontalHeader()->setSortIndicator(ui->tableWidget_3->horizontalHeader()->sortIndicatorSection() ,ui->tableWidget_3->horizontalHeader()->sortIndicatorOrder());

		BOOST_FOREACH(CMasternode& mn, vMasternodes)
		{
			int mnRow = 0;
			ui->tableWidget->insertRow(0);

			// populate list
			// Address, Rank, Active, Active Seconds, Last Seen, Pub Key
			QTableWidgetItem *activeItem = new QTableWidgetItem(QString::number(mn.IsEnabled()));
			QTableWidgetItem *addressItem = new QTableWidgetItem(QString::fromStdString(mn.addr.ToString()));
			QString Rank = QString::number(mnodeman.GetMasternodeRank(mn.vin, pindexBest->nHeight));
			QTableWidgetItem *rankItem = new QTableWidgetItem(Rank.rightJustified(2, '0', false));
			QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(seconds_to_DHMS((qint64)(mn.lastTimeSeen - mn.sigTime)));
			QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat(mn.lastTimeSeen)));

			CScript pubkey;
			pubkey =GetScriptForDestination(mn.pubkey.GetID());
			CTxDestination address1;
			ExtractDestination(pubkey, address1);
			CWavepaycoinAddress address2(address1);
			QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(address2.ToString()));

			ui->tableWidget->setItem(mnRow, 0, addressItem);
			ui->tableWidget->setItem(mnRow, 1, rankItem);
			ui->tableWidget->setItem(mnRow, 2, activeItem);
			ui->tableWidget->setItem(mnRow, 3, activeSecondsItem);
			ui->tableWidget->setItem(mnRow, 4, lastSeenItem);
			ui->tableWidget->setItem(mnRow, 5, pubkeyItem);
		}
		ui->countLabel->setText(QString::number(ui->tableWidget->rowCount()));
		on_UpdateButton_clicked();
		ui->tableWidget_3->setVisible(0);
		ui->tableWidget->setVisible(1);
		ui->tableWidget->verticalScrollBar()->setSliderPosition(ui->tableWidget_3->verticalScrollBar()->sliderPosition());
		}
}


void MasternodeManager::updateNodeList()
{
	
    TRY_LOCK(cs_masternodes, lockMasternodes);
    if(!lockMasternodes)
        return;
	static int64_t nTimeListUpdated = GetTime();

    // to prevent high cpu usage update only once in MASTERNODELIST_UPDATE_SECONDS seconds
    // or MASTERNODELIST_FILTER_COOLDOWN_SECONDS seconds after filter was last changed
    int64_t nSecondsToWait = fFilterUpdated ? nTimeFilterUpdated - GetTime() + MASTERNODELIST_FILTER_COOLDOWN_SECONDS : nTimeListUpdated - GetTime() + MASTERNODELIST_UPDATE_SECONDS;

    if (fFilterUpdated) ui->countLabel->setText(QString::fromStdString(strprintf("Please wait... %d", nSecondsToWait)));
    if (nSecondsToWait > 0) return;

    nTimeListUpdated = GetTime();
    
	nTimeListUpdated = GetTime();
    fFilterUpdated = false;
	if (f1.isFinished())
		f1 = QtConcurrent::run(this,&MasternodeManager::updateListConc);

	if (f2.isFinished())
		f2 = QtConcurrent::run(this,&MasternodeManager::updateCountries);   
	

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
    req.setUrl(QUrl("https://api.crex24.com/CryptoExchangeService/BotPublic/ReturnTicker?request=[NamePairs=BTC_WVP]"));
    //req.setUrl(QUrl("https://graviex.net/api/v2/tickers/andbtc"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = mgr_.get(req);
    eventLoop_.exec(); // blocks stack until "finished()" has been called

    QString QMnpricebtc;
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
	QJsonArray jsonArray = jsonObject["Tickers"].toArray();
	 

	foreach (const QJsonValue & value, jsonArray) {
        QJsonObject obj = value.toObject();
        double pricebtc = obj["Last"].toDouble();
	QMnpricebtc = QString::number(pricebtc, 'f', 8);
	}


	if (QMnpricebtc.isEmpty()) {
	QMnpricebtc = "0.00000500";
	ui->countPricebtc->setToolTip(tr("Not Listed in Exchange Yet, result: depend on masternode sell price"));
	}
	else {
	ui->countPricebtc->setToolTip(tr("result: on market Crex24"));

	}
	
        delete reply;
    }
    else {
        //failure 
	QMnpricebtc = "0.00000500";
	ui->countPricebtc->setToolTip(tr("Not Listed in Exchange Yet, Result: depend on masternode sell price"));
        delete reply;
    }

        ui->countPricebtc->setText(QMnpricebtc + " BTC");

        QString QMncounts = QString::number((int)mnodeman.size());
        QString QMncollateral = QString::number((int)GetMNCollateral(pindexBest->nHeight));
        QString QMncoinlocked = QString::number((int)getMnCoinLocked(0));
        QString QMncoinlockedpersen = QString::number((double)getMnCoinLocked(1));
        QString QMncoinsupply = QString::number((double)pindexBest->nMoneySupply / 100000000, 'f', 8);
        QString QMrewardday = QString::number((double)getMnRewardDay(1));
        QString QMrewardweek = QString::number((double)getMnRewardDay(7));
        QString QMrewardmonth = QString::number((double)getMnRewardDay(30));
        QString QMrewardyear = QString::number((double)getMnRewardDay(360));

        ui->mncountLabel->setText(QMncounts);
        ui->countCollateral->setText(QMncollateral + " WVP");
        ui->countCoinlocked->setText(QMncoinlocked + " WVP / " + QMncoinlockedpersen + " %");
        ui->countCoinsupply->setText(QMncoinsupply + " WVP");
        ui->countMnrewardday->setText(QMrewardday + " WVP");
        ui->countMnrewardweek->setText(QMrewardweek + " WVP");
        ui->countMnrewardmonth->setText(QMrewardmonth + " WVP");
        ui->countMnrewardyear->setText(QMrewardyear + " WVP");
	
	int MnColateral = GetMNCollateral(pindexBest->nHeight);

	double MInvest = MnColateral * QMnpricebtc.toDouble();

	int Mnperday = getMnRewardDay(1);
	double Mday = Mnperday * QMnpricebtc.toDouble();
	double Mweek = (Mnperday * QMnpricebtc.toDouble()) * 7;
	double Mmonth = (Mnperday * QMnpricebtc.toDouble()) * 30;
	double Myear = (Mnperday * QMnpricebtc.toDouble()) * 360;
	double Mdaypersen = (Mday / MInvest) * 100;
	double Mweekpersen = (Mweek / MInvest) * 100;
	double Mmonthpersen = (Mmonth / MInvest) * 100;
	double Myearpersen = (Myear / MInvest) * 100;
	
	QString MnInvest = QString::number((double)MInvest);
	QString Mnday = QString::number((double)Mday);
	QString Mndaypersen = QString::number((double)Mdaypersen);
	QString Mnweek = QString::number((double)Mweek);
	QString Mnweekpersen = QString::number((double)Mweekpersen);
	QString Mnmonth = QString::number((double)Mmonth);
	QString Mnmonthpersen = QString::number((double)Mmonthpersen);
	QString Mnyear = QString::number((double)Myear);
	QString Mnyearpersen = QString::number((double)Myearpersen);
	

	ui->countInvestment->setText(MnInvest + " BTC");
	ui->countMnroiday->setText(Mnday + " BTC / " + Mndaypersen + " %");
	ui->countMnroiweek->setText(Mnweek + " BTC / " + Mnweekpersen + " %");
	ui->countMnroimonth->setText(Mnmonth + " BTC / " + Mnmonthpersen + " %");
	ui->countMnroiyear->setText(Mnyear + " BTC / " + Mnyearpersen + " %");
        
	
}

void MasternodeManager::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
    }
}

void MasternodeManager::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model && model->getOptionsModel())
    {
    }

}

void MasternodeManager::on_createButton_clicked()
{
    AddEditAdrenalineNode* aenode = new AddEditAdrenalineNode();
    aenode->exec();
}

void MasternodeManager::on_startButton_clicked()
{
    // start the node
    QItemSelectionModel* selectionModel = ui->tableWidget_2->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows();
    if(selected.count() == 0)
        return;

    QModelIndex index = selected.at(0);
    int r = index.row();
    std::string sAlias = ui->tableWidget_2->item(r, 0)->text().toStdString();



    if(pwalletMain->IsLocked()) {
    }

    std::string statusObj;
    statusObj += "<center>Alias: " + sAlias;

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        if(mne.getAlias() == sAlias) {
            std::string errorMessage;
            std::string strDonateAddress = mne.getDonationAddress();
            std::string strDonationPercentage = mne.getDonationPercentage();

            bool result = activeMasternode.Register(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strDonateAddress, strDonationPercentage, errorMessage);
            setLastMasternodeError(mne.getTxHash() +  mne.getOutputIndex(), errorMessage);

            if(result) {
                statusObj += "<br>Successfully started masternode." ;
            } else {
                statusObj += "<br>Failed to start masternode.<br>Error: " + errorMessage;
            }
            break;
        }
    }
    statusObj += "</center>";
    pwalletMain->Lock();

    QMessageBox msg;
    msg.setText(QString::fromStdString(statusObj));

    msg.exec();

    on_UpdateButton_clicked();
}

void MasternodeManager::on_startAllButton_clicked()
{
    if(pwalletMain->IsLocked()) {
    }

    std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;

    int total = 0;
    int successful = 0;
    int fail = 0;
    std::string statusObj;

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        total++;

        std::string errorMessage;
        std::string strDonateAddress = mne.getDonationAddress();
        std::string strDonationPercentage = mne.getDonationPercentage();

        bool result = activeMasternode.Register(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strDonateAddress, strDonationPercentage, errorMessage);
        setLastMasternodeError(mne.getTxHash() +  mne.getOutputIndex(), errorMessage);

        if(result) {
            successful++;
        } else {
            fail++;
            statusObj += "\nFailed to start " + mne.getAlias() + ". Error: " + errorMessage;
        }
    }
    pwalletMain->Lock();

    std::string returnObj;
    returnObj = "Successfully started " + boost::lexical_cast<std::string>(successful) + " masternodes, failed to start " +
            boost::lexical_cast<std::string>(fail) + ", total " + boost::lexical_cast<std::string>(total);
    if (fail > 0)
        returnObj += statusObj;

    QMessageBox msg;
    msg.setText(QString::fromStdString(returnObj));
    msg.exec();

    on_UpdateButton_clicked();
}

void MasternodeManager::on_UpdateButton_clicked()
{
    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        std::string errorMessage;
        std::string strDonateAddress = mne.getDonationAddress();
        std::string strDonationPercentage = mne.getDonationPercentage();

        std::vector<CMasternode> vMasternodes = mnodeman.GetFullMasternodeVector();

        getLastMasternodeError(mne.getTxHash() +  mne.getOutputIndex(), errorMessage);

        // If an error is available we use it. Otherwise we print the update text as we are searching for the MN in the Wavepay network.
        if (errorMessage == ""){
            updateAdrenalineNode(QString::fromStdString(mne.getAlias()), QString::fromStdString(mne.getIp()), QString::fromStdString(mne.getPrivKey()), QString::fromStdString(mne.getTxHash()),
                QString::fromStdString(mne.getOutputIndex()), QString::fromStdString(strDonateAddress), QString::fromStdString(strDonationPercentage), QString::fromStdString("Updating Network List."));
        }
        else {
            updateAdrenalineNode(QString::fromStdString(mne.getAlias()), QString::fromStdString(mne.getIp()), QString::fromStdString(mne.getPrivKey()), QString::fromStdString(mne.getTxHash()),
                QString::fromStdString(mne.getOutputIndex()), QString::fromStdString(strDonateAddress), QString::fromStdString(strDonationPercentage), QString::fromStdString(errorMessage));
        }

        BOOST_FOREACH(CMasternode& mn, vMasternodes) {
            if (mn.addr.ToString().c_str() == mne.getIp()){
                updateAdrenalineNode(QString::fromStdString(mne.getAlias()), QString::fromStdString(mne.getIp()), QString::fromStdString(mne.getPrivKey()), QString::fromStdString(mne.getTxHash()),
                QString::fromStdString(mne.getOutputIndex()), QString::fromStdString(strDonateAddress), QString::fromStdString(strDonationPercentage), QString::fromStdString("Masternode is Running."));
            }
        }
    }
}
