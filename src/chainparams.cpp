// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "libzerocoin/Params.h"
#include "chainparams.h"
#include "consensus/merkle.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>
#include <limits>


struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress>& vSeedsOut, const SeedSpec6* data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    for (unsigned int i = 0; i < count; i++) {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//   What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static Checkpoints::MapCheckpoints mapCheckpoints =
    boost::assign::map_list_of
    (0, uint256("41e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"))
    (191884,uint256("45b95a52b673fd9f620bed8d59c5cfb05db0ffc33225bc6778b08d251348b185"))
    (204305,uint256("101cfe17653d7b5ea6bb15531d9344dd65837f96904599ebc054bc8f6fad355d")) //about when spork 15 activated
    (204464,uint256("9d2f9808c10408899cfff44f6a3adc44690912dd0dcb3b530c45cd2ad9a31f0e")) //about when spork 16 activated
    (369647,uint256("82ba21a1afcb59ebdede612516d9c3bfed42e2e18baba172b971fd714d32bdf6"))
    (600000,uint256("c6c87c5f70578ebb270012492e0dd222d6532a864853beefc894840aad73f021"))    
    (800000,uint256("ef48831e1547b45d90ef5360c606f49e35504f38abe383614486753cb9451515"))    
    (1000000,uint256("3a121397bca5552e637c80f981db080c54e1bae2def44fc06071cb4867df7124"))    
    (1402500,uint256("d9544578aae78d2ae2c68599641f66e90829156bb9157c73e807e570f927d6a0"))
    (1529400,uint256("2e2f9fc719f478a532177f57c56345502539aeb37ba45b8147fbdf53f15f628f"))
    (1594800,uint256("35d628c9471ff3d2b98d327a87b69669b219e5c473c0ccd54d6db84198b38819"))
    (1797000,uint256("3222dfdda3b2c842eaff173e3a632964d56c3f5a957d0b444af2df07dd06754d"))
    (2000016,uint256("Fed89c61b9c7e19593c259344eb5d8a3121fd7f516d2cca31499a28f68af73b7"))
    (2050000,uint256("6f89ed36393384b07325b96e54b59abdd8a1803b98b1ccaae14cc80c8912395e"))
    (2078350,uint256("55599f4c11d8e59a8eb94047789123da1eb64d2a4bcce09f499d4e2ad01e3b5a"))
    (2108600,uint256("0bd5d302f078b69a55a1e8caeb643dd8034407d17cb6adc63e867eba5699d7b6"));
    
static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1615666837, // * UNIX timestamp of last checkpoint block
    7626589,     // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
    2000        // * estimated number of transactions per day after checkpoint
};

//testnet
static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of(0, uint256("000007d1b438a4c7dbd6d88546b1cb23d1091f08555262b2e3984aef70e44d6c"));
static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1575145155,
    2971390,
    250};

//regtest
static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of(0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1454124731,
    0,
    100};

libzerocoin::ZerocoinParams* CChainParams::Zerocoin_Params(bool useModulusV1) const
{
    assert(this);
    static CBigNum bnHexModulus = 0;
    if (!bnHexModulus)
        bnHexModulus.SetHex(zerocoinModulus);
    static libzerocoin::ZerocoinParams ZCParamsHex = libzerocoin::ZerocoinParams(bnHexModulus);
    static CBigNum bnDecModulus = 0;
    if (!bnDecModulus)
        bnDecModulus.SetDec(zerocoinModulus);
    static libzerocoin::ZerocoinParams ZCParamsDec = libzerocoin::ZerocoinParams(bnDecModulus);

    if (useModulusV1)
        return &ZCParamsHex;

    return &ZCParamsDec;
}

bool CChainParams::HasStakeMinAgeOrDepth(const int contextHeight, const uint32_t contextTime,
        const int utxoFromBlockHeight, const uint32_t utxoFromBlockTime, const int sporkValue) const
{
    // before stake modifier V2, the age required was 60 * 60 (1 hour). Not required for regtest
    if (!IsStakeModifierV2(contextHeight, sporkValue))
        return NetworkID() == CBaseChainParams::REGTEST || (utxoFromBlockTime + nStakeMinAge <= contextTime);

    // after stake modifier V2, we require the utxo to be nStakeMinDepth deep in the chain
    return (contextHeight - utxoFromBlockHeight >= nStakeMinDepth);
}

int CChainParams::FutureBlockTimeDrift(const int nHeight, const int sporkValue) const
{
    if (IsTimeProtocolV2(nHeight, sporkValue))
        // PoS (TimeV2): 14 seconds
        return TimeSlotLength() - 1;

    // PoS (TimeV1): 3 minutes
    // PoW: 2 hours
    return (nHeight > LAST_POW_BLOCK()) ? nFutureTimeDriftPoS : nFutureTimeDriftPoW;
}

bool CChainParams::IsValidBlockTimeStamp(const int64_t nTime, const int nHeight, const int sporkValue) const
{
    // Before time protocol V2, blocks can have arbitrary timestamps
    if (!IsTimeProtocolV2(nHeight, sporkValue))
        return true;

    // Time protocol v2 requires time in slots
    return (nTime % TimeSlotLength()) == 0;
}

int CChainParams::BlockStartTimeProtocolV2(const int sporkValue) const
{
    return sporkValue;
    //return GetSporkValue(SPORK_23_TIME_PROTOCOL_V2_BLOCK);
    //return nBlockTimeProtocolV2;
}

bool CChainParams::IsTimeProtocolV2(const int nHeight, const int sporkValue) const
{
    // The caller must pass sporkValue with GetSporkValue(SPORK_23_TIME_PROTOCOL_V2_BLOCK)
    // CChainParams can't call any spork stuff because library-common doesn't include any spork related files.
    return nHeight >= sporkValue;
    //return nHeight >= BlockStartTimeProtocolV2();
}

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0x03;
        pchMessageStart[1] = 0xc4;
        pchMessageStart[2] = 0xfd;
        pchMessageStart[3] = 0x13;
        vAlertPubKey = ParseHex("0000098d3ba6ba6e7423fa5cbd6a89e0a9a5348f88d332b44a5cb1a8b7ed2c1eaa335fc8dc4f012cb8241cc0bdafd6ca70c5f5448916e4e6f511bcd746ed57dc50");
        nDefaultPort = 8765;
        bnProofOfWorkLimit = ~uint256(0) >> 20; // VITAE starting difficulty is 1 / 2^12
        bnProofOfStakeLimit = ~uint256(0) >> 24;
        bnProofOfStakeLimit_V2 = ~uint256(0) >> 20; // 60/4 = 15 ==> use 2**4 higher limit
        nSubsidyHalvingInterval = 210000;
        nMaxReorganizationDepth = 100;
        nEnforceBlockUpgradeMajority = 10800;  // 75% ... ((60*60*24)/45)*7.5 = 14400 or about 7 days
        nRejectBlockOutdatedMajority = 13680;  // 95%
        nToCheckBlockUpgradeMajority = 14400;  // Approximate expected amount of blocks in 7 days (1920*7.5)
        nMinerThreads = 0;
        nTargetSpacing = 1 * 45;                    // 1 minute
        nTargetTimespan = 40 * 60;                  // 40 minutes
        nTimeSlotLength = 15;                       // 15 seconds
        nTargetTimespan_V2 = 2 * nTimeSlotLength * 60;  // 15 minutes
        nMaturity = 8;
        nStakeMinAge = 60 * 60;                         // 1 hour
        nStakeMinDepth = 80;
        nFundamentalnodeCountDrift = 20;
        nFutureTimeDriftPoW = 7200;
        nFutureTimeDriftPoS = 180;
        nMasternodeCountDrift = 20;
        nMaxMoneyOut = 21000000 * COIN;
        nTimeSlotLength = 15;           // 15 seconds
        nMinColdStakingAmount = 1 * COIN;

        /** Height or Time Based Activations **/
        nLastPOWBlock = 200;
        nVitaeBadBlockTime = 1471401614; // Skip nBit validation of Block 259201 per PR #915
        nVitaeBadBlocknBits = 0x1c056dac; // Skip nBit validation of Block 259201 per PR #915
        nModifierUpdateBlock = 615800;
        nZerocoinStartHeight = 209467;
        nZerocoinStartTime = 1536314400; // september 7, 2018 12:00:00 UTC
        nBlockEnforceSerialRange = 895400; //Enforce serial range starting this block
        nBlockRecalculateAccumulators = 908000; //Trigger a recalculation of accumulators
        nBlockFirstFraudulent = 891737; //First block that bad serials emerged
        nBlockLastGoodCheckpoint = 891730; //Last valid accumulator checkpoint
        nBlockEnforceInvalidUTXO = 902850; //Start enforcing the invalid UTXO's
        nInvalidAmountFiltered = 268200*COIN; //Amount of invalid coins filtered through exchanges, that should be considered valid
        nBlockZerocoinV2 = 999999999; //The block that zerocoin v2 becomes active
        nEnforceNewSporkKey = 1596240000; //!> Sporks signed after (GMT): August 1, 2020 12:00:00 AM must use the new spork key
        nRejectOldSporkKey = 1604188800; //!> Fully reject old spork key after (GMT): November 1, 2020 12:00:00 AM
        nBlockStakeModifierlV2 = 1967000;
        nBIP65ActivationHeight = 1808634;
        // Activation height for TimeProtocolV2, Blocks V7 and newMessageSignatures
        nBlockTimeProtocolV2 = 2153200;

        // Public coin spend enforcement
        nPublicZCSpends = 1880000;

        // New P2P messages signatures
        nBlockEnforceNewMessageSignatures = nBlockTimeProtocolV2;

        // Blocks v7
        nBlockLastAccumulatorCheckpoint = 1686240;
        nBlockV7StartHeight = nBlockTimeProtocolV2;

        // Fake Serial Attack
        nFakeSerialBlockheightEnd = 1686229;
        nSupplyBeforeFakeSerial = 4131563 * COIN;   // zerocoin supply at block nFakeSerialBlockheightEnd

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         *
         * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
         *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
         *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
         *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
         *   vMerkleTree: e0028e
         */
        const char* pszTimestamp = "U.S. News & World Report Jan 28 2016 With His Absence, Trump Dominates Another Debate";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 250 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04c10e83b2703ccf322f7dbd62dd5855ac7c10bd055814ce121ba32607d573b8810c02c0582aed05b4deb9c4b77b26d92428c61256cd42774babea0a073b2ed0c9") << OP_CHECKSIG;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
        genesis.nVersion = 1;
        genesis.nTime = 1454124731;
        genesis.nBits = 0x1e0ffff0;
        genesis.nNonce = 2402015;

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x0000041e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"));
        assert(genesis.hashMerkleRoot == uint256("0x1b2ef6e2f28be914103a277377ae7729dcd125dfeb8bf97bd5964ba72b6dc39b"));


        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("dns0", "dns0.vitae.phore.io")); // Primary DNS seeder
        vSeeds.push_back(CDNSSeedData("dns1", "dns1.vitae.phore.io")); // Secondary DNS seeder
        vSeeds.push_back(CDNSSeedData("seednode1.vitae.co", "seednode1.vitae.co"));
        vSeeds.push_back(CDNSSeedData("seednode2.vitae.co", "seednode2.vitae.co"));
        vSeeds.push_back(CDNSSeedData("seednode1.vitaetoken.io", "seednode1.vitaetoken.io"));
        vSeeds.push_back(CDNSSeedData("seednode2.vitaetoken.io", "seednode2.vitaetoken.io"));
        vSeeds.push_back(CDNSSeedData("seednode1.vitaeinfo.co", "seednode1.vitaeinfo.co"));
        vSeeds.push_back(CDNSSeedData("seednode2.vitaeinfo.co", "seednode2.vitaeinfo.co"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 71);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 13);
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 63);     // starting with 'S'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 212);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x02)(0x2D)(0x25)(0x33).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x02)(0x21)(0x31)(0x2B).convert_to_container<std::vector<unsigned char> >();
        // BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x77).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;
        fHeadersFirstSyncingActive = false;

        nPoolMaxTransactions = 3;
        nBudgetCycleBlocks = 43200; //!< Amount of blocks in a months period of time (using 1 minutes per) = (60*24*30)
        strSporkKey = "042c1257f8e148675cdb62b86a9a86625c00a2330957d7d2b6a1d9b685c7e0705014dfb70bf6358c272da0258481902a813197a6bddfddf86f46c48b4f37de9732";
        strSporkKeyOld = "04fd2375653a3064623b8a9e179c34a4ffa9ee9afbc13e2218b37f5fa6cbe2f94ef874a216cbfddbcbf06b5951a9011d65dae988fb4469fabcfa29b9c8daf23c7e";
        strObfuscationPoolDummyAddress = "VjVqgZbamLZ3KmEKBZZzmZgvtqDWw7jsrL";
        nStartFundamentalnodePayments = 1524487214;

        /** Zerocoin */
        zerocoinModulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
            "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
            "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
            "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
            "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
            "31438167899885040445364023527381951378636564391212010397122822120720357";
        nMaxZerocoinSpendsPerTransaction = 7; // Assume about 20kb each
        nMaxZerocoinPublicSpendsPerTransaction = 637; // Assume about 220 bytes each input
        nMinZerocoinMintFee = 1 * CENT; //high fee required for zerocoin mints
        nMintRequiredConfirmations = 20; //the maximum amount of confirmations until accumulated in 19
        nRequiredAccumulation = 1;
        nDefaultSecurityLevel = 100; //full security level for accumulators
        nZerocoinHeaderVersion = 5; //Block headers must be this version once zerocoin is active
        nZerocoinRequiredStakeDepth = 200; //The required confirmations for a zVIT to be stakable
        nBudget_Fee_Confirmations = 6; // Number of confirmations for the finalization fee
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }

};
static CMainParams mainParams;

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.nVersion = nVersion;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.hashMerkleRoot = genesis.BuildMerkleTree();
    return genesis;
}

/**
 * Build the genesis block. Note that the output of the genesis coinbase cannot
 * be spent as it did not originally exist in the database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "U.S. News & World Report Jan 28 2016 With His Absence, Trump Dominates Another Debate";
    const CScript genesisOutputScript = CScript() << ParseHex("04c10e83b2703ccf322f7dbd62dd5855ac7c10bd055814ce121ba32607d573b8810c02c0582aed05b4deb9c4b77b26d92428c61256cd42774babea0a073b2ed0c9") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Testnet (v4)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0x45;
        pchMessageStart[1] = 0x76;
        pchMessageStart[2] = 0x65;
        pchMessageStart[3] = 0xba;
        vAlertPubKey = ParseHex("000010e83b2703ccf322f7dbd62dd5855ac7c10bd055814ce121ba32607d573b8810c02c0582aed05b4deb9c4b77b26d92428c61256cd42774babea0a073b2ed0c9");
        nDefaultPort = 8763;
        bnProofOfWorkLimit = ~uint256(0) >> 20;
        bnProofOfStakeLimit = ~uint256(0) >> 24;
        bnProofOfStakeLimit_V2 = ~uint256(0) >> 20; // 60/4 = 15 ==> use 2**4 higher limit
        nEnforceBlockUpgradeMajority = 6480;
        nRejectBlockOutdatedMajority = 8208;
        nToCheckBlockUpgradeMajority = 8640; // ((60*60*24)/45)*4.5 = 8640 or about 4 days
        nMinerThreads = 0;
        nTargetTimespan = 40 * 45; // VITAE: 1 day
        nTargetTimespan_V2 = 2 * nTimeSlotLength * 60;  // 30 minutes
        nTargetSpacing = 1 * 45;  // VITAE: 45 seconds
        nLastPOWBlock = 200;
        nVitaeBadBlockTime = 1489001494; // Skip nBit validation of Block 259201 per PR #915
        nVitaeBadBlocknBits = 0x1e0a20bd; // Skip nBit validation of Block 201 per PR #915
        nMaturity = 15;
        nTimeSlotLength = 15;                       // 15 seconds
        nStakeMinAge = 60 * 60;                         // 1 hour
        nStakeMinDepth = 80;
        nFutureTimeDriftPoW = 7200;
        nFutureTimeDriftPoS = 180;

        nFundamentalnodeCountDrift = 4;
        nStakeMinDepth = 100;
        nMasternodeCountDrift = 4;
        nModifierUpdateBlock = 51197; //approx Mon, 17 Apr 2017 04:00:00 GMT
        nMaxMoneyOut = 43199500 * COIN;
        nZerocoinStartHeight = 130400;
        nZerocoinStartTime = 1608878580; // Friday, December 25, 2020 6:43:00 AM GMT  
        nBlockEnforceSerialRange = 1; //Enforce serial range starting this block
        nBlockRecalculateAccumulators = 9908000; //Trigger a recalculation of accumulators
        nBlockFirstFraudulent = 9891737; //First block that bad serials emerged
        nBlockLastGoodCheckpoint = 9891730; //Last valid accumulator checkpoint
        nBlockEnforceInvalidUTXO = 9902850; //Start enforcing the invalid UTXO's
        nInvalidAmountFiltered = 0; //Amount of invalid coins filtered through exchanges, that should be considered valid
        nBlockZerocoinV2 = 999999999; //!> The block that zerocoin v2 becomes active
        nEnforceNewSporkKey = 1596240000; //!> Sporks signed after (GMT): August 1, 2020 12:00:00 AM must use the new spork key
        nRejectOldSporkKey = 1601510400; //!> Fully reject old spork key after (GMT): October 1, 2020 12:00:00 AM
        nBlockStakeModifierlV2 = 1214000;
        nBIP65ActivationHeight = 851019;
        // Activation height for TimeProtocolV2, Blocks V7 and newMessageSignatures
        nBlockTimeProtocolV2 = 1347000;

        // Public coin spend enforcement
        nPublicZCSpends = 1106100;

        // New P2P messages signatures
        nBlockEnforceNewMessageSignatures = nBlockTimeProtocolV2;

        // Blocks v7
        nBlockLastAccumulatorCheckpoint = nPublicZCSpends - 10;
        nBlockV7StartHeight = nBlockTimeProtocolV2;

        // Fake Serial Attack
        nFakeSerialBlockheightEnd = -1;
        nSupplyBeforeFakeSerial = 0;

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1589445785;
        genesis.nNonce = 1346982;
        
        genesis = CreateGenesisBlock(1589445785, 1346982, 0x1e0ffff0, 1, 250 * COIN);
        //genesis = CreateGenesisBlock(1589445785, 296110, 0x1e0ffff0, 1, 250 * COIN);
        
        const uint256 checkHash("000007d1b438a4c7dbd6d88546b1cb23d1091f08555262b2e3984aef70e44d6c");
        //const uint256 checkHash("000001b272c8b0558fff38d94cb05f821d4f67b47bad407826e231a5553c64a3");

        // To mine new genesis block, change the 'false' to 'true', and change hashGenesisBlock to "0x001"
        hashGenesisBlock = checkHash;
        if(true && genesis.GetHash() != hashGenesisBlock)
        {
            printf("recalculating params for testnet.\n");
            printf("old testnet genesis nonce: %s\n", std::to_string(genesis.nNonce).c_str());
            printf("old testnet genesis hash:  %s\n", hashGenesisBlock.ToString().c_str());
            // deliberately empty for loop finds nonce value.
            for(genesis.nNonce = 0; genesis.GetHash() > bnProofOfWorkLimit; genesis.nNonce++){ } 
            printf("new testnet genesis merkle root: %s\n", genesis.hashMerkleRoot.ToString().c_str());
            printf("new testnet genesis nonce: %s\n", std::to_string(genesis.nNonce).c_str());
            printf("new testnet genesis hash: %s\n", genesis.GetHash().ToString().c_str());
            exit(0);
        }        

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == checkHash);

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("209.182.216.144", "209.182.216.144")); // vitae fn
        vSeeds.push_back(CDNSSeedData("209.182.216.187", "209.182.216.187")); // vitae fn
        vSeeds.push_back(CDNSSeedData("198.13.50.121", "198.13.50.121"));     //rasalghul
        vSeeds.push_back(CDNSSeedData("104.238.183.75", "104.238.183.75"));   //rasalghul

        vSeeds.push_back(CDNSSeedData("fuzzbawls.pw", "vitae-testnet.seed.fuzzbawls.pw"));
        vSeeds.push_back(CDNSSeedData("fuzzbawls.pw", "vitae-testnet.seed2.fuzzbawls.pw"));
        vSeeds.push_back(CDNSSeedData("warrows.dev", "testnet.dnsseed.vitae.warrows.dev"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 139); // Testnet vitae addresses start with 'x' or 'y'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);  // Testnet vitae script addresses start with '8' or '9'
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 73);     // starting with 'W'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        // Testnet vitae BIP32 pubkeys start with 'DRKV'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x3a)(0x80)(0x61)(0xa0).convert_to_container<std::vector<unsigned char> >();
        // Testnet vitae BIP32 prvkeys start with 'DRKP'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x3a)(0x80)(0x58)(0x37).convert_to_container<std::vector<unsigned char> >();
        // Testnet vitae BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = true;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 2;
        nBudgetCycleBlocks = 144; //!< Ten cycles per day on testnet
        strSporkKey = "0416a999f63f7f20d76e5f2d75d23987902aeb372c44ce275e5f6c07b99155a666ef9c96a6d5cc8232fd4eeb6546caa2b35b4b7f336daedbb337b55392ecf69744";
        strSporkKeyOld = "04cef2ceafa824fa3e5777989e032cf4d48ab3b5ccb83897c7892dd9fd72e69676355e18082e795b67d051b487c6852105db03160e547eeb81b20a608560974cb9";
        strObfuscationPoolDummyAddress = "y57cqfGRkekRyDRNeJiLtYVEbvhXrNbmox";
        nStartFundamentalnodePayments = 1420837558; //Fri, 09 Jan 2015 21:05:58 GMT
        nBudget_Fee_Confirmations = 3; // Number of confirmations for the finalization fee. We have to make this very short
                                       // here because we only have a 8 block finalization window on testnet

        nZerocoinHeaderVersion = 5; //Block headers must be this version once zerocoin is active
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0xa1;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0x7e;
        pchMessageStart[3] = 0xac;
        nDefaultPort = 51476;
        nSubsidyHalvingInterval = 150;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        bnProofOfStakeLimit = ~uint256(0) >> 24;
        bnProofOfStakeLimit_V2 = ~uint256(0) >> 20; // 60/4 = 15 ==> use 2**4 higher limit
        nLastPOWBlock = 250;
        nMaturity = 100;
        nStakeMinAge = 0;
        nStakeMinDepth = 0;
        nMasternodeCountDrift = 4;
        nModifierUpdateBlock = 0;       //approx Mon, 17 Apr 2017 04:00:00 GMT
        nMaxMoneyOut = 43199500 * COIN;
        nZerocoinStartHeight = 300;
        nBlockZerocoinV2 = 300;
        nZerocoinStartTime = 1501776000;
        nBlockEnforceSerialRange = 1;               // Enforce serial range starting this block
        nBlockRecalculateAccumulators = 999999999;  // Trigger a recalculation of accumulators
        nBlockFirstFraudulent = 999999999;          // First block that bad serials emerged
        nBlockLastGoodCheckpoint = 999999999;       // Last valid accumulator checkpoint
        nBlockStakeModifierlV2 = nLastPOWBlock + 1; // start with modifier V2 on testnet
        nBlockTimeProtocolV2 = 999999999;

        nMintRequiredConfirmations = 10;
        nZerocoinRequiredStakeDepth = nMintRequiredConfirmations;

        // Public coin spend enforcement
        nPublicZCSpends = 400;

        // Blocks v7
        nBlockV7StartHeight = nPublicZCSpends + 1;
        nBlockLastAccumulatorCheckpoint = nPublicZCSpends - 10;

        // New P2P messages signatures
        nBlockEnforceNewMessageSignatures = 1;

        // Fake Serial Attack
        nFakeSerialBlockheightEnd = -1;

        //! Modify the regtest genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1454124731;
        genesis.nNonce = 2402015;

        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 51476;
        assert(hashGenesisBlock == uint256("0x0000041e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"));
        //assert(hashGenesisBlock == uint256("0x4f023a2120d9127b21bbad01724fdb79b519f593f2a85b60d3d79160ec5f29df"));

        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fSkipProofOfWorkCheck = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        /* Spork Key for RegTest:
        WIF private key: 932HEevBSujW2ud7RfB1YF91AFygbBRQj3de3LyaCRqNzKKgWXi
        private key hex: bd4960dcbd9e7f2223f24e7164ecb6f1fe96fc3a416f5d3a830ba5720c84b8ca
        Address: yCvUVd72w7xpimf981m114FSFbmAmne7j9
        */
        strSporkPubKey = "043969b1b0e6f327de37f297a015d37e2235eaaeeb3933deecd8162c075cee0207b13537618bde640879606001a8136091c62ec272dd0133424a178704e6e75bb7";
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams
{
public:
    CUnitTestParams()
    {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 51478;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Unit test mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fAllowMinDifficultyBlocks = false;
        fMineBlocksOnDemand = true;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setSubsidyHalvingInterval(int anSubsidyHalvingInterval) { nSubsidyHalvingInterval = anSubsidyHalvingInterval; }
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority) { nEnforceBlockUpgradeMajority = anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority) { nRejectBlockOutdatedMajority = anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority) { nToCheckBlockUpgradeMajority = anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks) { fDefaultConsistencyChecks = afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) { fAllowMinDifficultyBlocks = afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;

static CChainParams* pCurrentParams = 0;

CModifiableParams* ModifiableParams()
{
    assert(pCurrentParams);
    assert(pCurrentParams == &unitTestParams);
    return (CModifiableParams*)&unitTestParams;
}

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    case CBaseChainParams::UNITTEST:
        return unitTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}

