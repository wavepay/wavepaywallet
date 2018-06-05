// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2014-2015 Dash Developers
// Copyright (c) 2017-2018 The Wavepay developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "assert.h"
#include "chainparams.h"
#include "main.h"
#include "util.h"
#include "base58.h"
#include <boost/assign/list_of.hpp>

using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

//
// Main network
//

// Convert the pnSeeds array into usable address objects.
static void convertSeeds(std::vector<CAddress> &vSeedsOut, const unsigned int *data, unsigned int count, int port)
{
     // It'll only connect to one or two seed nodes because once it connects,
     // it'll get a pile of addresses with newer timestamps.
     // Seed nodes are given a random 'last seen time' of between one and two
     // weeks ago.
     const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int k = 0; k < count; ++k)
    {
        struct in_addr ip;
        unsigned int i = data[k], t;
        
        // -- convert to big endian
        t =   (i & 0x000000ff) << 24u
            | (i & 0x0000ff00) << 8u
            | (i & 0x00ff0000) >> 8u
            | (i & 0xff000000) >> 24u;
        
        memcpy(&ip, &t, sizeof(ip));
        
        CAddress addr(CService(ip, port));
        addr.nTime = GetTime()-GetRand(nOneWeek)-nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

// Hardcoded seeds.
static void getHardcodedSeeds(std::vector<CAddress> &vSeedsOut)
{
    std::vector<std::string> ips;

    ips.push_back("125.160.207.235");
    ips.push_back("45.64.254.98");
	
    const int64_t oneWeek = 7 * 24 * 60 * 60;
    for (size_t i = 0; i < ips.size(); ++i)
    {
        CAddress addr(CService(ips[i], 5279));
        addr.nTime = GetTime() - GetRand(oneWeek) - oneWeek;
        vSeedsOut.push_back(addr);
    }
}

class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xef;
        pchMessageStart[1] = 0xbe;
        pchMessageStart[2] = 0xee;
        pchMessageStart[3] = 0xaf;
        vAlertPubKey = ParseHex("04A5902C295770787BA3234C753A846F5FEC7954BEFDB3A6114115C0DA6B09529B64F64C577ADA2B41815D29F73BEAC05E4A13E7CC97145FEBCE37B35F688479AB");
        nDefaultPort = 5279;
        nRPCPort = 5299;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 20); // starting difficulty is 1 / 2^12
		
        const char* pszTimestamp = "wavepay the ultimate choice";
        std::vector<CTxIn> vin;
        vin.resize(1);
        vin[0].scriptSig = CScript() << 486604799 << CBigNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        std::vector<CTxOut> vout;
        vout.resize(1);
        vout[0].SetEmpty();
        vout[0].scriptPubKey = CScript() << ParseHex("04A5902C295770787BA3234C753A846F5FEC7954BEFDB3A6114115C0DA6B09529B64F64C577ADA2B41815D29F73BEAC05E4A13E7CC97145FEBCE37B35F688479AB") << OP_CHECKSIG;
        
        CTransaction txNew(1, 1528207200, vin, vout, 0);

        // LogPrintf("genesis mainnet transaction:  %s\n", txNew.ToString().c_str());

        genesis.vtx.push_back(txNew);

        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = 1528207200; 
        genesis.nBits    = bnProofOfWorkLimit.GetCompact(); 
        genesis.nNonce   = 3250;

        hashGenesisBlock = genesis.GetHash();        

        assert(hashGenesisBlock == uint256("0x825917d024dfbb86f1276f271a0366878a8a9bd3b43497f3c1f7790e07d22185"));
        assert(genesis.hashMerkleRoot == uint256("0x5eec699e36fbf5598e495a2c3bf65ac5a9e36465aef8c9b301a1be6e3249f1d2"));

        
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,73); // W
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,1);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,337);
        base58Prefixes[STEALTH_ADDRESS] = std::vector<unsigned char>(1,83);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();  
	
        vSeeds.push_back(CDNSSeedData("1",  "node.wavepay.org"));
        vSeeds.push_back(CDNSSeedData("2",  "node2.wavepay.org"));
        vSeeds.push_back(CDNSSeedData("3",  "node3.wavepay.org"));
        vSeeds.push_back(CDNSSeedData("4",  "node4.wavepay.org"));
		
		convertSeeds(vFixedSeeds, pnSeed, ARRAYLEN(pnSeed), nDefaultPort); 
		
        getHardcodedSeeds(vFixedSeeds);		

        nPoolMaxTransactions = 3;
        strDarksendPoolDummyAddress = "wMNZVuoP8NiAwupDJrBRdxG8Na6PtmWpVy";

        nLastPOWBlock = 3000000; // 5 years
        nPOSStartBlock = 5000; // BLOCK 5000
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet
//

class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xf4;
        pchMessageStart[1] = 0xae;
        pchMessageStart[2] = 0xf2;
        pchMessageStart[3] = 0xbf;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 16);
        vAlertPubKey = ParseHex("04A5902C295770787BA3234C753A846F5FEC7954BEFDB3A6114115C0DA6B09529B64F64C577ADA2B41815D29F73BEAC05E4A13E7CC97145FEBCE37B35F688479AB");
        nDefaultPort = 5579;
        nRPCPort = 5599;
        strDataDir = "testnet";
        genesis.nTime    = 1527344400;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact(); 
        genesis.nNonce   = 56359;

        hashGenesisBlock = genesis.GetHash();

        assert(hashGenesisBlock == uint256("0xc66613dc024d139a2745ca29d51395e0b494fd536e27bba8f70c536b4c15c763"));

        vFixedSeeds.clear();
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,73); 
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,1);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,337);
        base58Prefixes[STEALTH_ADDRESS] = std::vector<unsigned char>(1,80);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x98)(0x74)(0x44)(0xE1).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x98)(0x72)(0x42)(0xE2).convert_to_container<std::vector<unsigned char> >();

        convertSeeds(vFixedSeeds, pnTestnetSeed, ARRAYLEN(pnTestnetSeed), nDefaultPort);

        nLastPOWBlock = 90000;
    }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;


static CChainParams *pCurrentParams = &mainParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;
        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {
    
    bool fTestNet = GetBoolArg("-testnet", false);
    
    if (fTestNet) {
        SelectParams(CChainParams::TESTNET);
    } else {
        SelectParams(CChainParams::MAIN);
    }
    return true;
}
