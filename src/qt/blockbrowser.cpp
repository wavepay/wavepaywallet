#include "blockbrowser.h"
#include "ui_blockbrowser.h"
#include "main.h"
#include "wallet.h"
#include "base58.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "masternode.h"
#include "rpcconsole.h"
#include "transactionrecord.h"
#include <QtConcurrent/QtConcurrent>
#include <sstream>
#include <string>
double getBlockHardness(int height)
{
    const CBlockIndex* blockindex = getBlockIndex(height);

    int nShift = (blockindex->nBits >> 24) & 0xff;

    double dDiff =
        (double)0x0000ffff / (double)(blockindex->nBits & 0x00ffffff);

    while (nShift < 29)
    {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 29)
    {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;
}

int getBlockHashrate(int height)
{
    int lookup = height;

    double timeDiff = getBlockTime(height) - getBlockTime(1);
    double timePerBlock = timeDiff / lookup;

    return (boost::int64_t)(((double)getBlockHardness(height) * pow(2.0, 32)) / timePerBlock);
}

const CBlockIndex* getBlockIndex(int height)
{
    std::string hex = getBlockHash(height);
    uint256 hash(hex);
    return mapBlockIndex[hash];
}

std::string getBlockHash(int Height)
{
    if(Height > pindexBest->nHeight) { return "825917d024dfbb86f1276f271a0366878a8a9bd3b43497f3c1f7790e07d22185"; }
    if(Height < 0) { return "825917d024dfbb86f1276f271a0366878a8a9bd3b43497f3c1f7790e07d22185"; }
    int desiredheight;
    desiredheight = Height;
    if (desiredheight < 0 || desiredheight > nBestHeight)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hashBestChain];
    while (pblockindex->nHeight > desiredheight)
        pblockindex = pblockindex->pprev;
    return pblockindex->phashBlock->GetHex();
}

int getBlockTime(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->nTime;
}

std::string getBlockMerkle(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->hashMerkleRoot.ToString().substr(0,10).c_str();
}

int getBlocknBits(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->nBits;
}

int getBlockNonce(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->nNonce;
}

std::string getBlockDebug(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->ToString();
}

int blocksInPastHours(int hours)
{
    int wayback = hours * 3600;
    bool check = true;
    int height = pindexBest->nHeight;
    int heightHour = pindexBest->nHeight;
    int utime = (int)time(NULL);
    int target = utime - wayback;

    while(check)
    {
        if(getBlockTime(heightHour) < target)
        {
            check = false;
            return height - heightHour;
        } else {
            heightHour = heightHour - 1;
        }
    }

    return 0;
}

double getTxTotalValue(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return 0;

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;

    double value = 0;
    double buffer = 0;
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];

        buffer = value + convertCoins(txout.nValue);
        value = buffer;
    }

    return value;
}

double convertCoins(int64_t amount)
{
    return (double)amount / (double)COIN;
}

std::string getOutputs(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return "fail";

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;

    std::string str = "";
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];
        CTxDestination source;
        ExtractDestination(txout.scriptPubKey, source);
        CWavepaycoinAddress addressSource(source);
        std::string lol7 = addressSource.ToString();
        double buffer = convertCoins(txout.nValue);
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(4) << buffer;
        std::string amount = ss.str();
        str.append(lol7);
        str.append(": ");
        str.append(amount);
        str.append(" WVP");
        str.append("\n");
    }

    return str;
}

std::string getInputs(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return "fail";

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;

    std::string str = "";
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        uint256 hash;
        const CTxIn& vin = tx.vin[i];
        hash.SetHex(vin.prevout.hash.ToString());
        CTransaction wtxPrev;
        uint256 hashBlock = 0;
        if (!GetTransaction(hash, wtxPrev, hashBlock))
             return "fail";

        CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
        ssTx << wtxPrev;

        CTxDestination source;
        ExtractDestination(wtxPrev.vout[vin.prevout.n].scriptPubKey, source);
        CWavepaycoinAddress addressSource(source);
        std::string lol6 = addressSource.ToString();
        const CScript target = wtxPrev.vout[vin.prevout.n].scriptPubKey;
        double buffer = convertCoins(getInputValue(wtxPrev, target));
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(4) << buffer;
        std::string amount = ss.str();
        str.append(lol6);
        str.append(": ");
        str.append(amount);
        str.append(" WVP");
        str.append("\n");
    }

    return str;
}

int64_t getInputValue(CTransaction tx, CScript target)
{
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];
        if(txout.scriptPubKey == target)
        {
            return txout.nValue;
        }
    }
    return 0;
}

double getTxFees(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);


    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return 0;

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;

    double value = 0;
    double buffer = 0;
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];

        buffer = value + convertCoins(txout.nValue);
        value = buffer;
    }

    double value0 = 0;
    double buffer0 = 0;
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        uint256 hash0;
        const CTxIn& vin = tx.vin[i];
        hash0.SetHex(vin.prevout.hash.ToString());
        CTransaction wtxPrev;
        uint256 hashBlock0 = 0;
        if (!GetTransaction(hash0, wtxPrev, hashBlock0))
             return 0;
        CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
        ssTx << wtxPrev;
        const CScript target = wtxPrev.vout[vin.prevout.n].scriptPubKey;
        buffer0 = value0 + convertCoins(getInputValue(wtxPrev, target));
        value0 = buffer0;
    }

    return value0 - value;
}


BlockBrowser::BlockBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BlockBrowser)
{
    ui->setupUi(this);

    setFixedSize(400, 420);
        
    connect(ui->blockButton, SIGNAL(pressed()), this, SLOT(blockClicked()));
    connect(ui->txButton, SIGNAL(pressed()), this, SLOT(txClicked()));
        timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTimes()));
    if(!GetBoolArg("-reindexaddr", false))
        timer->start(1000);
	              
	ui->heightLabel->hide();
        ui->heightLabel_2->hide();
        ui->hashLabel->hide();
        ui->hashBox->hide();
        ui->merkleLabel->hide();
        ui->merkleBox->hide();
        ui->nonceLabel->hide();
        ui->nonceBox->hide();
        ui->bitsLabel->hide();
        ui->bitsBox->hide();
        ui->timeLabel->hide();
        ui->timeBox->hide();
        ui->hardLabel->hide();
        ui->hardBox->hide();
  
	ui->txID->hide();
        ui->txLabel->hide();
        ui->valueLabel->hide();
        ui->valueBox->hide();
        ui->inputLabel->hide();
        ui->inputBox->hide();
        ui->outputLabel->hide();
        ui->outputBox->hide();
        ui->feesLabel->hide();
        ui->feesBox->hide();
    nTimeFilterUpdated = GetTime();
    updateTimes();

}

void BlockBrowser::updateExplorer(bool block)
{    
    if(block)
    {
	// block
        ui->heightLabel->show();
        ui->heightLabel_2->show();
        ui->hashLabel->show();
        ui->hashBox->show();
        ui->merkleLabel->show();
        ui->merkleBox->show();
        ui->nonceLabel->show();
        ui->nonceBox->show();
        ui->bitsLabel->show();
        ui->bitsBox->show();
        ui->timeLabel->show();
        ui->timeBox->show();
        ui->hardLabel->show();
        ui->hardBox->show();
	// tx
	ui->txID->hide();
        ui->txLabel->hide();
        ui->valueLabel->hide();
        ui->valueBox->hide();
        ui->inputLabel->hide();
        ui->inputBox->hide();
        ui->outputLabel->hide();
        ui->outputBox->hide();
        ui->feesLabel->hide();
        ui->feesBox->hide();

        int height = ui->heightBox->value();
        if (height > pindexBest->nHeight)
        {
            ui->heightBox->setValue(pindexBest->nHeight);
            height = pindexBest->nHeight;
        }
        std::string hash = getBlockHash(height);
        std::string merkle = getBlockMerkle(height);
        int nBits = getBlocknBits(height);
        int nNonce = getBlockNonce(height);
        int atime = getBlockTime(height);
        double hardness = getBlockHardness(height);
        QString QHeight = QString::number(height);
        QString QHash = QString::fromUtf8(hash.c_str());
        QString QMerkle = QString::fromUtf8(merkle.c_str());
        QString QBits = QString::number(nBits);
        QString QNonce = QString::number(nNonce);
        QString QTime = QString::number(atime);
        QString QHardness = QString::number(hardness, 'f', 6);
        ui->heightLabel->setText(QHeight);
        ui->hashBox->setText(QHash);
        ui->merkleBox->setText(QMerkle);
        ui->bitsBox->setText(QBits);
        ui->nonceBox->setText(QNonce);
        ui->timeBox->setText(QTime);     
        ui->hardBox->setText(QHardness);
    } 
    
    if(block == false) {
	
        ui->heightLabel->hide();
        ui->heightLabel_2->hide();
        ui->hashLabel->hide();
        ui->hashBox->hide();
        ui->merkleLabel->hide();
        ui->merkleBox->hide();
        ui->nonceLabel->hide();
        ui->nonceBox->hide();
        ui->bitsLabel->hide();
        ui->bitsBox->hide();
        ui->timeLabel->hide();
        ui->timeBox->hide();
        ui->hardLabel->hide();
        ui->hardBox->hide();

        ui->txID->show();
        ui->txLabel->show();
        ui->valueLabel->show();
        ui->valueBox->show();
        ui->inputLabel->show();
        ui->inputBox->show();
        ui->outputLabel->show();
        ui->outputBox->show();
        ui->feesLabel->show();
        ui->feesBox->show();
        std::string txid = ui->txBox->text().toUtf8().constData();
        double value = getTxTotalValue(txid);
        double fees = getTxFees(txid);
        std::string outputs = getOutputs(txid);
        std::string inputs = getInputs(txid);
        QString QValue = QString::number(value, 'f', 6);
        QString QID = QString::fromUtf8(txid.c_str());
        QString QOutputs = QString::fromUtf8(outputs.c_str());
        QString QInputs = QString::fromUtf8(inputs.c_str());
        QString QFees = QString::number(fees, 'f', 6);
        ui->valueBox->setText(QValue + " WVP");
        ui->txID->setText(QID);
        ui->outputBox->setText(QOutputs);
        ui->inputBox->setText(QInputs);
        ui->feesBox->setText(QFees + " WVP");
    }
}

void BlockBrowser::updateTimes()
{
	
    TRY_LOCK(cs_masternodes, lockMasternodes);
    if(!lockMasternodes)
        return;
	static int64_t nTimeListUpdated = GetTime();

    // to prevent high cpu usage update only once in MASTERNODELIST_UPDATE_SECONDS seconds
    // or MASTERNODELIST_FILTER_COOLDOWN_SECONDS seconds after filter was last changed
    int64_t nSecondsToWait = nTimeFilterUpdated - GetTime() + 60;

    ui->countDownlabel->setText(QString::fromStdString(strprintf("Auto Refresh In %d", nSecondsToWait)));
    if (nSecondsToWait > 0) return;

    nTimeListUpdated = GetTime();
    nTimeFilterUpdated = GetTime();
	updateBlockExp();
   // fFilterUpdated = false;
	if (f3.isFinished())
		f3 = QtConcurrent::run(this,&BlockBrowser::updateBlockExp);
}
void BlockBrowser::updateBlockExp()
{
	// block
        ui->heightLabel->show();
        ui->heightLabel_2->show();
        ui->hashLabel->show();
        ui->hashBox->show();
        ui->merkleLabel->show();
        ui->merkleBox->show();
        ui->nonceLabel->show();
        ui->nonceBox->show();
        ui->bitsLabel->show();
        ui->bitsBox->show();
        ui->timeLabel->show();
        ui->timeBox->show();
        ui->hardLabel->show();
        ui->hardBox->show();
	// tx
	ui->txID->hide();
        ui->txLabel->hide();
        ui->valueLabel->hide();
        ui->valueBox->hide();
        ui->inputLabel->hide();
        ui->inputBox->hide();
        ui->outputLabel->hide();
        ui->outputBox->hide();
        ui->feesLabel->hide();
        ui->feesBox->hide();

        int height = pindexBest->nHeight;
        if (height > pindexBest->nHeight)
        {
            ui->heightBox->setValue(pindexBest->nHeight);
            height = pindexBest->nHeight;
        }
        if (height < 10)
	   return;

        std::string hash = getBlockHash(height);
        std::string merkle = getBlockMerkle(height);
        int nBits = getBlocknBits(height);
        int nNonce = getBlockNonce(height);
        int atime = getBlockTime(height);
        double hardness = getBlockHardness(height);
        QString QHeight = QString::number(height);
        QString QHash = QString::fromUtf8(hash.c_str());
        QString QMerkle = QString::fromUtf8(merkle.c_str());
        QString QBits = QString::number(nBits);
        QString QNonce = QString::number(nNonce);
        QString QTime = QString::number(atime);
        QString QHardness = QString::number(hardness, 'f', 6);
        ui->heightLabel->setText(QHeight);
        ui->hashBox->setText(QHash);
        ui->merkleBox->setText(QMerkle);
        ui->bitsBox->setText(QBits);
        ui->nonceBox->setText(QNonce);
        ui->timeBox->setText(QTime);     
        ui->hardBox->setText(QHardness);

        ui->Helabel_1->setText(QHeight);
        ui->Hashlabel_1->setText(QHash);
        std::string hash2 = getBlockHash(height -1);
        QString QHeight2 = QString::number(height -1);
        QString QHash2 = QString::fromUtf8(hash2.c_str());
        ui->Helabel_2->setText(QHeight2);
        ui->Hashlabel_2->setText(QHash2);
        std::string hash3 = getBlockHash(height -2);
        QString QHeight3 = QString::number(height -2);
        QString QHash3 = QString::fromUtf8(hash3.c_str());
        ui->Helabel_3->setText(QHeight3);
        ui->Hashlabel_3->setText(QHash3);
        std::string hash4 = getBlockHash(height -3);
        QString QHeight4 = QString::number(height -3);
        QString QHash4 = QString::fromUtf8(hash4.c_str());
        ui->Helabel_4->setText(QHeight4);
        ui->Hashlabel_4->setText(QHash4);
        std::string hash5 = getBlockHash(height -4);
        QString QHeight5 = QString::number(height -4);
        QString QHash5 = QString::fromUtf8(hash5.c_str());
        ui->Helabel_5->setText(QHeight5);
        ui->Hashlabel_5->setText(QHash5);
        std::string hash6 = getBlockHash(height -5);
        QString QHeight6 = QString::number(height -5);
        QString QHash6 = QString::fromUtf8(hash6.c_str());
        ui->Helabel_6->setText(QHeight6);
        ui->Hashlabel_6->setText(QHash6);	
        std::string hash7 = getBlockHash(height -6);
        QString QHeight7 = QString::number(height -6);
        QString QHash7 = QString::fromUtf8(hash7.c_str());
        ui->Helabel_7->setText(QHeight7);
        ui->Hashlabel_7->setText(QHash7);
        std::string hash8 = getBlockHash(height -7);
        QString QHeight8 = QString::number(height -7);
        QString QHash8 = QString::fromUtf8(hash8.c_str());
        ui->Helabel_8->setText(QHeight8);
        ui->Hashlabel_8->setText(QHash8);
        std::string hash9 = getBlockHash(height -8);
        QString QHeight9 = QString::number(height -8);
        QString QHash9 = QString::fromUtf8(hash9.c_str());
        ui->Helabel_9->setText(QHeight9);
        ui->Hashlabel_9->setText(QHash9);
        std::string hash10 = getBlockHash(height -9);
        QString QHeight10 = QString::number(height -9);
        QString QHash10 = QString::fromUtf8(hash10.c_str());
        ui->Helabel_10->setText(QHeight10);
        ui->Hashlabel_10->setText(QHash10);
}
void BlockBrowser::txClicked()
{
    updateExplorer(false);
}

void BlockBrowser::blockClicked()
{
    updateExplorer(true);
}

void BlockBrowser::setModel(WalletModel *model)
{
    this->model = model;
}

BlockBrowser::~BlockBrowser()
{
    delete ui;
}
