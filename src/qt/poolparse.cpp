#include "poolparse.h"

PoolParse::PoolParse(QObject *parent) :
    QObject(parent)
{
}

QString PoolParse::getURL(QString poolName, QString apiKey)
{
    QString url;

    if (poolName == "hash4.life")
        url = "http://hash4.life/api/walletEx?address=";
    else if (poolName == "hashpool.eu")
        url = "http://hashpool.eu/api/walletEx?address=";
    else if (poolName == "-")
        url = "-";

    url.append(apiKey);

    return url;
}

QString PoolParse::parseData(QString poolName, QVariantMap apidata)
{
    QString message;
    double userAccepted = -1;
    double userRejected = -1;
    double userBalance = -1;
     double userUnpaid = -1;   
     double userPaid24h = -1;
      double userTotal = -1;    
    QVariantMap userStatus;

	if (poolName == "hash4.life")
    {
    	userBalance = apidata.value("balance").toDouble();
    	userUnpaid = apidata.value("unpaid").toDouble();
    	userPaid24h = apidata.value("paid24h").toDouble();
    	userTotal = apidata.value("total").toDouble();
    	userStatus = apidata.value("miners").toMap();
    	userAccepted = userStatus.value("accepted").toDouble();
    	userRejected = userStatus.value("rejected").toDouble();
    	        
    }
    else if (poolName == "hashpool.eu")
    {
    	userBalance = apidata.value("balance").toDouble();
    	userUnpaid = apidata.value("unpaid").toDouble();
    	userPaid24h = apidata.value("paid24h").toDouble();
    	userTotal = apidata.value("total").toDouble();
    	userStatus = apidata.value("miners").toMap();
    	userAccepted = userStatus.value("accepted").toDouble();
    	userRejected = userStatus.value("rejected").toDouble();
    	        
    }
    else if (poolName == "-")
    {
    
    }    

    	QString userBalanceString = QString("<b>Balance:</b> %1 WVP<br/>").arg(userBalance);
    	message.append(userBalanceString);
    	QString userUnpaidString = QString("<b>Unpaid:</b> %1 WVP<br/>").arg(userUnpaid);
    	message.append(userUnpaidString);
    	QString userPaid24hString = QString("<b>Paid24h:</b> %1 WVP<br/>").arg(userPaid24h);
    	message.append(userPaid24hString);
    	QString totalString = QString("<b>Total:</b> %1 WVP<br/>").arg(userTotal);
        message.append(totalString);
      /*  QString userAcceptedString = QString("<b>Accepted shares:</b> %1<br/>").arg(userAccepted);
        message.append(userAcceptedString);
        QString userRejectedString = QString("<b>Rejected shares:</b> %1<br/>").arg(userRejected);
        message.append(userRejectedString); */
    

    return message;
}
