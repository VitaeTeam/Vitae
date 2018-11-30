// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "fundamentalnode-payments.h"
#include "addrman.h"
#include "fundamentalnode-budget.h"
#include "fundamentalnode-sync.h"
#include "fundamentalnodeman.h"
#include "obfuscation.h"
#include "spork.h"
#include "sync.h"
#include "util.h"
#include "utilmoneystr.h"

#include "masternode-pos.h"
#include "masternode.h"
#include "masternodeman.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

/** Object for who's going to get paid on which blocks */
CFundamentalnodePayments fundamentalnodePayments;

CCriticalSection cs_vecPayments;
CCriticalSection cs_mapFundamentalnodeBlocks;
CCriticalSection cs_mapFundamentalnodePayeeVotes;

//
// CFundamentalnodePaymentDB
//

CFundamentalnodePaymentDB::CFundamentalnodePaymentDB()
{
    pathDB = GetDataDir() / "fnpayments.dat";
    strMagicMessage = "FundamentalnodePayments";
}

bool CFundamentalnodePaymentDB::Write(const CFundamentalnodePayments& objToSave)
{
    int64_t nStart = GetTimeMillis();

    // serialize, checksum data up to that point, then append checksum
    CDataStream ssObj(SER_DISK, CLIENT_VERSION);
    ssObj << strMagicMessage;                   // fundamentalnode cache file specific magic message
    ssObj << FLATDATA(Params().MessageStart()); // network specific magic number
    ssObj << objToSave;
    uint256 hash = Hash(ssObj.begin(), ssObj.end());
    ssObj << hash;

    // open output file, and associate with CAutoFile
    FILE* file = fopen(pathDB.string().c_str(), "wb");
    CAutoFile fileout(file, SER_DISK, CLIENT_VERSION);
    if (fileout.IsNull())
        return error("%s : Failed to open file %s", __func__, pathDB.string());

    // Write and commit header, data
    try {
        fileout << ssObj;
    } catch (std::exception& e) {
        return error("%s : Serialize or I/O error - %s", __func__, e.what());
    }
    fileout.fclose();

    LogPrint("fundamentalnode","Written info to mnpayments.dat  %dms\n", GetTimeMillis() - nStart);

    return true;
}

CFundamentalnodePaymentDB::ReadResult CFundamentalnodePaymentDB::Read(CFundamentalnodePayments& objToLoad, bool fDryRun)
{
    int64_t nStart = GetTimeMillis();
    // open input file, and associate with CAutoFile
    FILE* file = fopen(pathDB.string().c_str(), "rb");
    CAutoFile filein(file, SER_DISK, CLIENT_VERSION);
    if (filein.IsNull()) {
        error("%s : Failed to open file %s", __func__, pathDB.string());
        return FileError;
    }

    // use file size to size memory buffer
    int fileSize = boost::filesystem::file_size(pathDB);
    int dataSize = fileSize - sizeof(uint256);
    // Don't try to resize to a negative number if file is small
    if (dataSize < 0)
        dataSize = 0;
    vector<unsigned char> vchData;
    vchData.resize(dataSize);
    uint256 hashIn;

    // read data and checksum from file
    try {
        filein.read((char*)&vchData[0], dataSize);
        filein >> hashIn;
    } catch (std::exception& e) {
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return HashReadError;
    }
    filein.fclose();

    CDataStream ssObj(vchData, SER_DISK, CLIENT_VERSION);

    // verify stored checksum matches input data
    uint256 hashTmp = Hash(ssObj.begin(), ssObj.end());
    if (hashIn != hashTmp) {
        error("%s : Checksum mismatch, data corrupted", __func__);
        return IncorrectHash;
    }

    unsigned char pchMsgTmp[4];
    std::string strMagicMessageTmp;
    try {
        // de-serialize file header (fundamentalnode cache file specific magic message) and ..
        ssObj >> strMagicMessageTmp;

        // ... verify the message matches predefined one
        if (strMagicMessage != strMagicMessageTmp) {
            error("%s : Invalid fundamentalnode payement cache magic message", __func__);
            return IncorrectMagicMessage;
        }


        // de-serialize file header (network specific magic number) and ..
        ssObj >> FLATDATA(pchMsgTmp);

        // ... verify the network matches ours
        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp))) {
            error("%s : Invalid network magic number", __func__);
            return IncorrectMagicNumber;
        }

        // de-serialize data into CFundamentalnodePayments object
        ssObj >> objToLoad;
    } catch (std::exception& e) {
        objToLoad.Clear();
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return IncorrectFormat;
    }

    LogPrint("fundamentalnode","Loaded info from mnpayments.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrint("fundamentalnode","  %s\n", objToLoad.ToString());
    if (!fDryRun) {
        LogPrint("fundamentalnode","Fundamentalnode payments manager - cleaning....\n");
        objToLoad.CleanPaymentList();
        LogPrint("fundamentalnode","Fundamentalnode payments manager - result:\n");
        LogPrint("fundamentalnode","  %s\n", objToLoad.ToString());
    }

    return Ok;
}

void DumpFundamentalnodePayments()
{
    int64_t nStart = GetTimeMillis();

    CFundamentalnodePaymentDB paymentdb;
    CFundamentalnodePayments tempPayments;

    LogPrint("fundamentalnode","Verifying mnpayments.dat format...\n");
    CFundamentalnodePaymentDB::ReadResult readResult = paymentdb.Read(tempPayments, true);
    // there was an error and it was not an error on file opening => do not proceed
    if (readResult == CFundamentalnodePaymentDB::FileError)
        LogPrint("fundamentalnode","Missing budgets file - mnpayments.dat, will try to recreate\n");
    else if (readResult != CFundamentalnodePaymentDB::Ok) {
        LogPrint("fundamentalnode","Error reading mnpayments.dat: ");
        if (readResult == CFundamentalnodePaymentDB::IncorrectFormat)
            LogPrint("fundamentalnode","magic is ok but data has invalid format, will try to recreate\n");
        else {
            LogPrint("fundamentalnode","file format is unknown or invalid, please fix it manually\n");
            return;
        }
    }
    LogPrint("fundamentalnode","Writting info to mnpayments.dat...\n");
    paymentdb.Write(fundamentalnodePayments);

    LogPrint("fundamentalnode","Budget dump finished  %dms\n", GetTimeMillis() - nStart);
}

bool IsBlockValueValid(const CBlock& block, CAmount nExpectedValue, CAmount nMinted)
{
    CBlockIndex* pindexPrev = chainActive.Tip();
    if (pindexPrev == NULL) return true;

    int nHeight = 0;
    if (pindexPrev->GetBlockHash() == block.hashPrevBlock) {
        nHeight = pindexPrev->nHeight + 1;
    } else { //out of order
        BlockMap::iterator mi = mapBlockIndex.find(block.hashPrevBlock);
        if (mi != mapBlockIndex.end() && (*mi).second)
            nHeight = (*mi).second->nHeight + 1;
    }

    if (nHeight == 0) {
        LogPrint("fundamentalnode","IsBlockValueValid() : WARNING: Couldn't find previous block\n");
    }

    //LogPrintf("XX69----------> IsBlockValueValid(): nMinted: %d, nExpectedValue: %d\n", FormatMoney(nMinted), FormatMoney(nExpectedValue));

    if (!fundamentalnodeSync.IsSynced()) { //there is no budget data to use to check anything
        //super blocks will always be on these blocks, max 100 per budgeting
        if (nHeight % GetBudgetPaymentCycleBlocks() < 100) {
            return true;
        } else {
            if (nMinted > nExpectedValue) {
                return false;
            }
        }
    } else { // we're synced and have data so check the budget schedule

        //are these blocks even enabled
        if (!IsSporkActive(SPORK_13_ENABLE_SUPERBLOCKS)) {
            return nMinted <= nExpectedValue;
        }

        if (budget.IsBudgetPaymentBlock(nHeight)) {
            //the value of the block is evaluated in CheckBlock
            return true;
        } else {
            if (nMinted > nExpectedValue) {
                return false;
            }
        }
    }

    return true;
}

bool IsBlockPayeeValid(const CBlock& block, int nBlockHeight)
{
    if (!fundamentalnodeSync.IsSynced()) { //there is no budget data to use to check anything -- find the longest chain
        LogPrint("mnpayments", "Client not synced, skipping block payee checks\n");
        return true;
    }

	bool MasternodePayments = false;


    if(block.nTime > START_MASTERNODE_PAYMENTS) MasternodePayments = true;

    if(!IsMNSporkActive(MN_SPORK_1_MASTERNODE_PAYMENTS_ENFORCEMENT)){
        MasternodePayments = false; //
        if(fDebug) LogPrintf("CheckBlock() : Masternode payment enforcement is off\n");
    }

    if(MasternodePayments)
    {
        LOCK2(cs_main, mempool.cs);

        CBlockIndex *pindex = chainActive.Tip();
        if(pindex != NULL){
            if(pindex->GetBlockHash() == block.hashPrevBlock){
                CAmount stakeReward = GetBlockValue(pindex->nHeight + 1);
                CAmount masternodePaymentAmount = GetMasternodePayment(pindex->nHeight+1, stakeReward);//todo++

                bool fIsInitialDownload = IsInitialBlockDownload();

                // If we don't already have its previous block, skip masternode payment step
                if (!fIsInitialDownload && pindex != NULL)
                {
                    bool foundPaymentAmount = false;
                    bool foundPayee = false;
                    bool foundPaymentAndPayee = false;
                    CScript payee;
                    if(!masternodePayments.GetBlockPayee(pindex->nHeight+1, payee) || payee == CScript()){
                        foundPayee = true; //doesn't require a specific payee
                        foundPaymentAmount = true;
                        foundPaymentAndPayee = true;
                        LogPrintf("CheckBlock() : Using non-specific masternode payments %d\n", pindex->nHeight+1);
                    }
                    // todo-- must notice block.vtx[]. to block.vtx[]->
                    // Funtion no Intitial Download
                    for (unsigned int i = 0; i < block.vtx[1].vout.size(); i++) {
                        if(block.vtx[1].vout[i].nValue == masternodePaymentAmount ){
                            foundPaymentAmount = true;
                        }
                        if(block.vtx[1].vout[i].scriptPubKey == payee ){
                            foundPayee = true;
                        }
                        if(block.vtx[1].vout[i].nValue == masternodePaymentAmount && block.vtx[1].vout[i].scriptPubKey == payee){
                            foundPaymentAndPayee = true;
                        }
                    }

                    CTxDestination address1;
                    ExtractDestination(payee, address1);
                    CBitcoinAddress address2(address1);

                    if(!foundPaymentAndPayee) {
                        LogPrintf("CheckBlock() : !!Couldn't find masternode payment(%d|%d) or payee(%d|%s) nHeight %d. \n", foundPaymentAmount, masternodePaymentAmount, foundPayee, address2.ToString().c_str(), chainActive.Tip()->nHeight+1);
                        return false;//state.DoS(100, error("CheckBlock() : Couldn't find masternode payment or payee"));//todo++
                    } else {
                        LogPrintf("CheckBlock() : Found payment(%d|%d) or payee(%d|%s) nHeight %d. \n", foundPaymentAmount, masternodePaymentAmount, foundPayee, address2.ToString().c_str(), chainActive.Tip()->nHeight+1);
                    }
                } else {
                    LogPrintf("CheckBlock() : Is initial download, skipping masternode payment check %d\n", chainActive.Tip()->nHeight+1);
                }
            } else {
                LogPrintf("CheckBlock() : Skipping masternode payment check - nHeight %d Hash %s\n", chainActive.Tip()->nHeight+1, block.GetHash().ToString().c_str());
            }
        } else {
            LogPrintf("CheckBlock() : pindex is null, skipping masternode payment check\n");
        }
    } else {
        LogPrintf("CheckBlock() : skipping masternode payment checks\n");
    }

    const CTransaction& txNew = (nBlockHeight > Params().LAST_POW_BLOCK() ? block.vtx[1] : block.vtx[0]);

    //check if it's a budget block
    if (IsSporkActive(SPORK_13_ENABLE_SUPERBLOCKS)) {
        if (budget.IsBudgetPaymentBlock(nBlockHeight)) {
            if (budget.IsTransactionValid(txNew, nBlockHeight))
                return true;

            LogPrint("fundamentalnode","Invalid budget payment detected %s\n", txNew.ToString().c_str());
            if (IsSporkActive(SPORK_9_FUNDAMENTALNODE_BUDGET_ENFORCEMENT))
                return false;

            LogPrint("fundamentalnode","Budget enforcement is disabled, accepting block\n");
            return true;
        }
    }

    //check for fundamentalnode payee
    if (fundamentalnodePayments.IsTransactionValid(txNew, nBlockHeight))
        return true;
    LogPrint("fundamentalnode","Invalid mn payment detected %s\n", txNew.ToString().c_str());

    if (IsSporkActive(SPORK_8_FUNDAMENTALNODE_PAYMENT_ENFORCEMENT))
        return false;
    LogPrint("fundamentalnode","Fundamentalnode payment enforcement is disabled, accepting block\n");

    return true;
}


void FillBlockPayee(CMutableTransaction& txNew, CAmount nFees, bool fProofOfStake, bool IsMasternode)
{
    CBlockIndex* pindexPrev = chainActive.Tip();
    if (!pindexPrev) return;

    if (IsSporkActive(SPORK_13_ENABLE_SUPERBLOCKS) && budget.IsBudgetPaymentBlock(pindexPrev->nHeight + 1)) {
        budget.FillBlockPayee(txNew, nFees, fProofOfStake);
    } else {
        fundamentalnodePayments.FillBlockPayee(txNew, nFees, fProofOfStake, IsMasternode);
    }
}

std::string GetRequiredPaymentsString(int nBlockHeight)
{
    if (IsSporkActive(SPORK_13_ENABLE_SUPERBLOCKS) && budget.IsBudgetPaymentBlock(nBlockHeight)) {
        return budget.GetRequiredPaymentsString(nBlockHeight);
    } else {
        return fundamentalnodePayments.GetRequiredPaymentsString(nBlockHeight);
    }
}

void CFundamentalnodePayments::FillBlockPayee(CMutableTransaction& txNew, int64_t nFees, bool fProofOfStake, bool IsMasternode)
{
    CBlockIndex* pindexPrev = chainActive.Tip();
    if (!pindexPrev) return;

    bool hasPayment = true;
    bool hasMnPayment = true;
    CScript payee;
    CScript mn_payee;

    //spork
    if (!fundamentalnodePayments.GetBlockPayee(pindexPrev->nHeight + 1, payee)) {
        //no fundamentalnode detected
        CFundamentalnode* winningNode = mnodeman.GetCurrentFundamentalNode(1);
        if (winningNode) {
            payee = GetScriptForDestination(winningNode->pubKeyCollateralAddress.GetID());
        } else {
            LogPrint("fundamentalnode","CreateNewBlock: Failed to detect fundamentalnode to pay\n");
            hasPayment = false;
        }
    }

    //    bool bMasterNodePayment = false;

    //    if ( Params().NetworkID() == CBaseChainParams::TESTNET ){
    //        if (GetTimeMicros() > START_FUNDAMENTALNODE_PAYMENTS_TESTNET ){
    //            bMasterNodePayment = true;
    //        }
    //    }else{
    //        if (GetTimeMicros() > START_FUNDAMENTALNODE_PAYMENTS){
    //            bMasterNodePayment = true;
    //        }
    //    }//

    if(!masternodePayments.GetBlockPayee(pindexPrev->nHeight+1, mn_payee)){
        //no masternode detected
        CMasternode* winningNode = m_nodeman.GetCurrentMasterNode(1);
        if(winningNode){
            mn_payee = GetScriptForDestination(winningNode->pubkey.GetID());
        } else {
            LogPrintf("CreateNewBlock: Failed to detect masternode to pay\n");
            hasMnPayment = false;
        }
    }


    CAmount blockValue = GetBlockValue(pindexPrev->nHeight);
    CAmount fundamentalnodePayment = GetFundamentalnodePayment(pindexPrev->nHeight + 1, blockValue);
    CAmount masternodepayment = GetMasternodePayment(pindexPrev->nHeight +1 , blockValue);

    //txNew.vout[0].nValue = blockValue;

    if (hasPayment) {
        if(IsMasternode && hasMnPayment){
            if (fProofOfStake) {
                /**For Proof Of Stake vout[0] must be null
                 * Stake reward can be split into many different outputs, so we must
                 * use vout.size() to align with several different cases.
                 * An additional output is appended as the fundamentalnode payment
                 */
                unsigned int i = txNew.vout.size();
                txNew.vout.resize(i + 2);
                txNew.vout[i].scriptPubKey = payee;
                txNew.vout[i].nValue = fundamentalnodePayment;
                txNew.vout[i + 1].scriptPubKey = mn_payee;
                txNew.vout[i + 1].nValue = masternodepayment;

                //subtract mn payment from the stake reward
                txNew.vout[i - 1].nValue -= (fundamentalnodePayment + masternodepayment);
            } else {
                txNew.vout.resize(3);
                txNew.vout[2].scriptPubKey = mn_payee;
                txNew.vout[2].nValue = masternodepayment;
                txNew.vout[1].scriptPubKey = payee;
                txNew.vout[1].nValue = fundamentalnodePayment;
                txNew.vout[0].nValue = blockValue - (fundamentalnodePayment + masternodepayment);
            }

            CTxDestination address1;
            ExtractDestination(payee, address1);
            CBitcoinAddress address2(address1);

            LogPrint("fundamentalnode","Fundamentalnode payment of %s to %s\n", FormatMoney(fundamentalnodePayment).c_str(), address2.ToString().c_str());


            CTxDestination address3;
            ExtractDestination(mn_payee, address3);
            CBitcoinAddress address4(address3);

            LogPrint("masternode","Masternode payment of %s to %s\n", FormatMoney(masternodepayment).c_str(), address4.ToString().c_str());

        } else {

            if (fProofOfStake) {
                /**For Proof Of Stake vout[0] must be null
             * Stake reward can be split into many different outputs, so we must
             * use vout.size() to align with several different cases.
             * An additional output is appended as the fundamentalnode payment
             */
                unsigned int i = txNew.vout.size();
                txNew.vout.resize(i + 1);
                txNew.vout[i].scriptPubKey = payee;
                txNew.vout[i].nValue = fundamentalnodePayment;

                //subtract mn payment from the stake reward
                txNew.vout[i - 1].nValue -= fundamentalnodePayment;
            } else {
                txNew.vout.resize(2);
                txNew.vout[1].scriptPubKey = payee;
                txNew.vout[1].nValue = fundamentalnodePayment;
                txNew.vout[0].nValue = blockValue - fundamentalnodePayment;
            }

            CTxDestination address1;
            ExtractDestination(payee, address1);
            CBitcoinAddress address2(address1);

            LogPrint("fundamentalnode","Fundamentalnode payment of %s to %s\n", FormatMoney(fundamentalnodePayment).c_str(), address2.ToString().c_str());
        }
    } else {

        if(IsMasternode && hasMnPayment){
            if (fProofOfStake) {
                /**For Proof Of Stake vout[0] must be null
                 * Stake reward can be split into many different outputs, so we must
                 * use vout.size() to align with several different cases.
                 * An additional output is appended as the fundamentalnode payment
                 */
                unsigned int i = txNew.vout.size();
                txNew.vout.resize(i + 1);
                txNew.vout[i ].scriptPubKey = mn_payee;
                txNew.vout[i ].nValue = masternodepayment;

                //subtract mn payment from the stake reward
                txNew.vout[i - 1].nValue -= ( masternodepayment);
            } else {
                txNew.vout.resize(2);
                txNew.vout[1].scriptPubKey = mn_payee;
                txNew.vout[1].nValue = masternodepayment;

                txNew.vout[0].nValue = blockValue - ( masternodepayment);
            }


            CTxDestination address3;
            ExtractDestination(mn_payee, address3);
            CBitcoinAddress address4(address3);

            LogPrint("masternode","Masternode payment of %s to %s\n", FormatMoney(masternodepayment).c_str(), address4.ToString().c_str());



        } else {


            if (fProofOfStake) {
                /**For Proof Of Stake vout[0] must be null
             * Stake reward can be split into many different outputs, so we must
             * use vout.size() to align with several different cases.
             * An additional output is appended as the fundamentalnode payment
             */

            } else {

                txNew.vout[0].nValue = blockValue ;
            }

        }

    }
}

int CFundamentalnodePayments::GetMinFundamentalnodePaymentsProto()
{
    if (IsSporkActive(SPORK_19_FUNDAMENTALNODE_PAY_UPDATED_NODES))
        return ActiveProtocol();                          // Allow only updated peers
    else
        return MIN_PEER_PROTO_VERSION_BEFORE_ENFORCEMENT; // Also allow old peers as long as they are allowed to run
}

void CFundamentalnodePayments::ProcessMessageFundamentalnodePayments(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if (!fundamentalnodeSync.IsBlockchainSynced()) return;

    if (fLiteMode) return; //disable all Obfuscation/Fundamentalnode related functionality


    if (strCommand == "fnget") { //Fundamentalnode Payments Request Sync
        if (fLiteMode) return;   //disable all Obfuscation/Fundamentalnode related functionality

        int nCountNeeded;
        vRecv >> nCountNeeded;

        if (Params().NetworkID() == CBaseChainParams::MAIN) {
            if (pfrom->HasFulfilledRequest("fnget")) {
                LogPrint("fundamentalnode","fnget - peer already asked me for the list\n");
                Misbehaving(pfrom->GetId(), 20);
                return;
            }
        }

        pfrom->FulfilledRequest("fnget");
        fundamentalnodePayments.Sync(pfrom, nCountNeeded);
        LogPrint("mnpayments", "fnget - Sent Fundamentalnode winners to peer %i\n", pfrom->GetId());
    } else if (strCommand == "fnw") { //Fundamentalnode Payments Declare Winner
        //this is required in litemodef
        CFundamentalnodePaymentWinner winner;
        vRecv >> winner;

        if (pfrom->nVersion < ActiveProtocol()) return;

        int nHeight;
        {
            TRY_LOCK(cs_main, locked);
            if (!locked || chainActive.Tip() == NULL) return;
            nHeight = chainActive.Tip()->nHeight;
        }

        if (fundamentalnodePayments.mapFundamentalnodePayeeVotes.count(winner.GetHash())) {
            LogPrint("mnpayments", "fnw - Already seen - %s bestHeight %d\n", winner.GetHash().ToString().c_str(), nHeight);
            fundamentalnodeSync.AddedFundamentalnodeWinner(winner.GetHash());
            return;
        }

        int nFirstBlock = nHeight - (mnodeman.CountEnabled() * 1.25);
        if (winner.nBlockHeight < nFirstBlock || winner.nBlockHeight > nHeight + 20) {
            LogPrint("mnpayments", "fnw - winner out of range - FirstBlock %d Height %d bestHeight %d\n", nFirstBlock, winner.nBlockHeight, nHeight);
            return;
        }

        std::string strError = "";
        if (!winner.IsValid(pfrom, strError)) {
            // if(strError != "") LogPrint("fundamentalnode","fnw - invalid message - %s\n", strError);
            return;
        }

        if (!fundamentalnodePayments.CanVote(winner.vinFundamentalnode.prevout, winner.nBlockHeight)) {
            //  LogPrint("fundamentalnode","fnw - fundamentalnode already voted - %s\n", winner.vinFundamentalnode.prevout.ToStringShort());
            return;
        }

        if (!winner.SignatureValid()) {
            // LogPrint("fundamentalnode","fnw - invalid signature\n");
            if (fundamentalnodeSync.IsSynced()) Misbehaving(pfrom->GetId(), 20);
            // it could just be a non-synced fundamentalnode
            mnodeman.AskForMN(pfrom, winner.vinFundamentalnode);
            return;
        }

        CTxDestination address1;
        ExtractDestination(winner.payee, address1);
        CBitcoinAddress address2(address1);

        //   LogPrint("mnpayments", "fnw - winning vote - Addr %s Height %d bestHeight %d - %s\n", address2.ToString().c_str(), winner.nBlockHeight, nHeight, winner.vinFundamentalnode.prevout.ToStringShort());

        if (fundamentalnodePayments.AddWinningFundamentalnode(winner)) {
            winner.Relay();
            fundamentalnodeSync.AddedFundamentalnodeWinner(winner.GetHash());
        }
    }
}

bool CFundamentalnodePaymentWinner::Sign(CKey& keyFundamentalnode, CPubKey& pubKeyFundamentalnode)
{
    std::string errorMessage;
    std::string strFundamentalNodeSignMessage;

    std::string strMessage = vinFundamentalnode.prevout.ToStringShort() +
                             boost::lexical_cast<std::string>(nBlockHeight) +
                             payee.ToString();

    if (!obfuScationSigner.SignMessage(strMessage, errorMessage, vchSig, keyFundamentalnode)) {
        LogPrint("fundamentalnode","CFundamentalnodePing::Sign() - Error: %s\n", errorMessage.c_str());
        return false;
    }

    if (!obfuScationSigner.VerifyMessage(pubKeyFundamentalnode, vchSig, strMessage, errorMessage)) {
        LogPrint("fundamentalnode","CFundamentalnodePing::Sign() - Error: %s\n", errorMessage.c_str());
        return false;
    }

    return true;
}

bool CFundamentalnodePayments::GetBlockPayee(int nBlockHeight, CScript& payee)
{
    if (mapFundamentalnodeBlocks.count(nBlockHeight)) {
        return mapFundamentalnodeBlocks[nBlockHeight].GetPayee(payee);
    }

    return false;
}

// Is this fundamentalnode scheduled to get paid soon?
// -- Only look ahead up to 8 blocks to allow for propagation of the latest 2 winners
bool CFundamentalnodePayments::IsScheduled(CFundamentalnode& mn, int nNotBlockHeight)
{
    LOCK(cs_mapFundamentalnodeBlocks);

    int nHeight;
    {
        TRY_LOCK(cs_main, locked);
        if (!locked || chainActive.Tip() == NULL) return false;
        nHeight = chainActive.Tip()->nHeight;
    }

    CScript mnpayee;
    mnpayee = GetScriptForDestination(mn.pubKeyCollateralAddress.GetID());

    CScript payee;
    for (int64_t h = nHeight; h <= nHeight + 8; h++) {
        if (h == nNotBlockHeight) continue;
        if (mapFundamentalnodeBlocks.count(h)) {
            if (mapFundamentalnodeBlocks[h].GetPayee(payee)) {
                if (mnpayee == payee) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool CFundamentalnodePayments::AddWinningFundamentalnode(CFundamentalnodePaymentWinner& winnerIn)
{
    uint256 blockHash = 0;
    if (!GetBlockHash(blockHash, winnerIn.nBlockHeight - 100)) {
        return false;
    }

    {
        LOCK2(cs_mapFundamentalnodePayeeVotes, cs_mapFundamentalnodeBlocks);

        if (mapFundamentalnodePayeeVotes.count(winnerIn.GetHash())) {
            return false;
        }

        mapFundamentalnodePayeeVotes[winnerIn.GetHash()] = winnerIn;

        if (!mapFundamentalnodeBlocks.count(winnerIn.nBlockHeight)) {
            CFundamentalnodeBlockPayees blockPayees(winnerIn.nBlockHeight);
            mapFundamentalnodeBlocks[winnerIn.nBlockHeight] = blockPayees;
        }
    }

    mapFundamentalnodeBlocks[winnerIn.nBlockHeight].AddPayee(winnerIn.payee, 1);

    return true;
}

bool CFundamentalnodeBlockPayees::IsTransactionValid(const CTransaction& txNew)
{
    LOCK(cs_vecPayments);

    int nMaxSignatures = 0;
    int nFundamentalnode_Drift_Count = 0;

    std::string strPayeesPossible = "";

    CAmount nReward = GetBlockValue(nBlockHeight);

    if (IsSporkActive(SPORK_8_FUNDAMENTALNODE_PAYMENT_ENFORCEMENT)) {
        // Get a stable number of fundamentalnodes by ignoring newly activated (< 8000 sec old) fundamentalnodes
        nFundamentalnode_Drift_Count = mnodeman.stable_size() + Params().FundamentalnodeCountDrift();
    }
    else {
        //account for the fact that all peers do not see the same fundamentalnode count. A allowance of being off our fundamentalnode count is given
        //we only need to look at an increased fundamentalnode count because as count increases, the reward decreases. This code only checks
        //for mnPayment >= required, so it only makes sense to check the max node count allowed.
        nFundamentalnode_Drift_Count = mnodeman.size() + Params().FundamentalnodeCountDrift();
    }

    CAmount requiredFundamentalnodePayment = GetFundamentalnodePayment(nBlockHeight, nReward, nFundamentalnode_Drift_Count);

    //require at least 6 signatures
    BOOST_FOREACH (CFundamentalnodePayee& payee, vecPayments)
        if (payee.nVotes >= nMaxSignatures && payee.nVotes >= MNPAYMENTS_SIGNATURES_REQUIRED)
            nMaxSignatures = payee.nVotes;

    // if we don't have at least 6 signatures on a payee, approve whichever is the longest chain
    if (nMaxSignatures < MNPAYMENTS_SIGNATURES_REQUIRED) return true;

    BOOST_FOREACH (CFundamentalnodePayee& payee, vecPayments) {
        bool found = false;
        BOOST_FOREACH (CTxOut out, txNew.vout) {
            if (payee.scriptPubKey == out.scriptPubKey) {
                if(out.nValue >= requiredFundamentalnodePayment)
                    found = true;
                else
                    LogPrint("fundamentalnode","Fundamentalnode payment is out of drift range. Paid=%s Min=%s\n", FormatMoney(out.nValue).c_str(), FormatMoney(requiredFundamentalnodePayment).c_str());
            }
        }

        if (payee.nVotes >= MNPAYMENTS_SIGNATURES_REQUIRED) {
            if (found) return true;

            CTxDestination address1;
            ExtractDestination(payee.scriptPubKey, address1);
            CBitcoinAddress address2(address1);

            if (strPayeesPossible == "") {
                strPayeesPossible += address2.ToString();
            } else {
                strPayeesPossible += "," + address2.ToString();
            }
        }
    }

    LogPrint("fundamentalnode","CFundamentalnodePayments::IsTransactionValid - Missing required payment of %s to %s\n", FormatMoney(requiredFundamentalnodePayment).c_str(), strPayeesPossible.c_str());
    return false;
}

std::string CFundamentalnodeBlockPayees::GetRequiredPaymentsString()
{
    LOCK(cs_vecPayments);

    std::string ret = "Unknown";

    BOOST_FOREACH (CFundamentalnodePayee& payee, vecPayments) {
        CTxDestination address1;
        ExtractDestination(payee.scriptPubKey, address1);
        CBitcoinAddress address2(address1);

        if (ret != "Unknown") {
            ret += ", " + address2.ToString() + ":" + boost::lexical_cast<std::string>(payee.nVotes);
        } else {
            ret = address2.ToString() + ":" + boost::lexical_cast<std::string>(payee.nVotes);
        }
    }

    return ret;
}

std::string CFundamentalnodePayments::GetRequiredPaymentsString(int nBlockHeight)
{
    LOCK(cs_mapFundamentalnodeBlocks);

    if (mapFundamentalnodeBlocks.count(nBlockHeight)) {
        return mapFundamentalnodeBlocks[nBlockHeight].GetRequiredPaymentsString();
    }

    return "Unknown";
}

bool CFundamentalnodePayments::IsTransactionValid(const CTransaction& txNew, int nBlockHeight)
{
    LOCK(cs_mapFundamentalnodeBlocks);

    if (mapFundamentalnodeBlocks.count(nBlockHeight)) {
        return mapFundamentalnodeBlocks[nBlockHeight].IsTransactionValid(txNew);
    }

    return true;
}

void CFundamentalnodePayments::CleanPaymentList()
{
    LOCK2(cs_mapFundamentalnodePayeeVotes, cs_mapFundamentalnodeBlocks);

    int nHeight;
    {
        TRY_LOCK(cs_main, locked);
        if (!locked || chainActive.Tip() == NULL) return;
        nHeight = chainActive.Tip()->nHeight;
    }

    //keep up to five cycles for historical sake
    int nLimit = std::max(int(mnodeman.size() * 1.25), 1000);

    std::map<uint256, CFundamentalnodePaymentWinner>::iterator it = mapFundamentalnodePayeeVotes.begin();
    while (it != mapFundamentalnodePayeeVotes.end()) {
        CFundamentalnodePaymentWinner winner = (*it).second;

        if (nHeight - winner.nBlockHeight > nLimit) {
            LogPrint("mnpayments", "CFundamentalnodePayments::CleanPaymentList - Removing old Fundamentalnode payment - block %d\n", winner.nBlockHeight);
            fundamentalnodeSync.mapSeenSyncMNW.erase((*it).first);
            mapFundamentalnodePayeeVotes.erase(it++);
            mapFundamentalnodeBlocks.erase(winner.nBlockHeight);
        } else {
            ++it;
        }
    }
}

bool CFundamentalnodePaymentWinner::IsValid(CNode* pnode, std::string& strError)
{
    CFundamentalnode* pmn = mnodeman.Find(vinFundamentalnode);

    if (!pmn) {
        strError = strprintf("Unknown Fundamentalnode %s", vinFundamentalnode.prevout.hash.ToString());
        LogPrint("fundamentalnode","CFundamentalnodePaymentWinner::IsValid - %s\n", strError);
        mnodeman.AskForMN(pnode, vinFundamentalnode);
        return false;
    }

    if (pmn->protocolVersion < ActiveProtocol()) {
        strError = strprintf("Fundamentalnode protocol too old %d - req %d", pmn->protocolVersion, ActiveProtocol());
        LogPrint("fundamentalnode","CFundamentalnodePaymentWinner::IsValid - %s\n", strError);
        return false;
    }

    int n = mnodeman.GetFundamentalnodeRank(vinFundamentalnode, nBlockHeight - 100, ActiveProtocol());

    if (n > MNPAYMENTS_SIGNATURES_TOTAL) {
        //It's common to have fundamentalnodes mistakenly think they are in the top 10
        // We don't want to print all of these messages, or punish them unless they're way off
        if (n > MNPAYMENTS_SIGNATURES_TOTAL * 2) {
            strError = strprintf("Fundamentalnode not in the top %d (%d)", MNPAYMENTS_SIGNATURES_TOTAL * 2, n);
            LogPrint("fundamentalnode","CFundamentalnodePaymentWinner::IsValid - %s\n", strError);
            //if (fundamentalnodeSync.IsSynced()) Misbehaving(pnode->GetId(), 20);
        }
        return false;
    }

    return true;
}

bool CFundamentalnodePayments::ProcessBlock(int nBlockHeight)
{
    if (!fFundamentalNode) return false;

    //reference node - hybrid mode

    int n = mnodeman.GetFundamentalnodeRank(activeFundamentalnode.vin, nBlockHeight - 100, ActiveProtocol());

    if (n == -1) {
        LogPrint("mnpayments", "CFundamentalnodePayments::ProcessBlock - Unknown Fundamentalnode\n");
        return false;
    }

    if (n > MNPAYMENTS_SIGNATURES_TOTAL) {
        LogPrint("mnpayments", "CFundamentalnodePayments::ProcessBlock - Fundamentalnode not in the top %d (%d)\n", MNPAYMENTS_SIGNATURES_TOTAL, n);
        return false;
    }

    if (nBlockHeight <= nLastBlockHeight) return false;

    CFundamentalnodePaymentWinner newWinner(activeFundamentalnode.vin);

    if (budget.IsBudgetPaymentBlock(nBlockHeight)) {
        //is budget payment block -- handled by the budgeting software
    } else {
        LogPrint("fundamentalnode","CFundamentalnodePayments::ProcessBlock() Start nHeight %d - vin %s. \n", nBlockHeight, activeFundamentalnode.vin.prevout.hash.ToString());

        // pay to the oldest MN that still had no payment but its input is old enough and it was active long enough
        int nCount = 0;
        CFundamentalnode* pmn = mnodeman.GetNextFundamentalnodeInQueueForPayment(nBlockHeight, true, nCount);

        if (pmn != NULL) {
            LogPrint("fundamentalnode","CFundamentalnodePayments::ProcessBlock() Found by FindOldestNotInVec \n");

            newWinner.nBlockHeight = nBlockHeight;

            CScript payee = GetScriptForDestination(pmn->pubKeyCollateralAddress.GetID());
            newWinner.AddPayee(payee);

            CTxDestination address1;
            ExtractDestination(payee, address1);
            CBitcoinAddress address2(address1);

            LogPrint("fundamentalnode","CFundamentalnodePayments::ProcessBlock() Winner payee %s nHeight %d. \n", address2.ToString().c_str(), newWinner.nBlockHeight);
        } else {
            LogPrint("fundamentalnode","CFundamentalnodePayments::ProcessBlock() Failed to find fundamentalnode to pay\n");
        }
    }

    std::string errorMessage;
    CPubKey pubKeyFundamentalnode;
    CKey keyFundamentalnode;

    if (!obfuScationSigner.SetKey(strFundamentalNodePrivKey, errorMessage, keyFundamentalnode, pubKeyFundamentalnode)) {
        LogPrint("fundamentalnode","CFundamentalnodePayments::ProcessBlock() - Error upon calling SetKey: %s\n", errorMessage.c_str());
        return false;
    }

    LogPrint("fundamentalnode","CFundamentalnodePayments::ProcessBlock() - Signing Winner\n");
    if (newWinner.Sign(keyFundamentalnode, pubKeyFundamentalnode)) {
        LogPrint("fundamentalnode","CFundamentalnodePayments::ProcessBlock() - AddWinningFundamentalnode\n");

        if (AddWinningFundamentalnode(newWinner)) {
            newWinner.Relay();
            nLastBlockHeight = nBlockHeight;
            return true;
        }
    }

    return false;
}

void CFundamentalnodePaymentWinner::Relay()
{
    CInv inv(MSG_FUNDAMENTALNODE_WINNER, GetHash());
    RelayInv(inv);
}

bool CFundamentalnodePaymentWinner::SignatureValid()
{
    CFundamentalnode* pmn = mnodeman.Find(vinFundamentalnode);

    if (pmn != NULL) {
        std::string strMessage = vinFundamentalnode.prevout.ToStringShort() +
                                 boost::lexical_cast<std::string>(nBlockHeight) +
                                 payee.ToString();

        std::string errorMessage = "";
        if (!obfuScationSigner.VerifyMessage(pmn->pubKeyFundamentalnode, vchSig, strMessage, errorMessage)) {
            return error("CFundamentalnodePaymentWinner::SignatureValid() - Got bad Fundamentalnode address signature %s\n", vinFundamentalnode.prevout.hash.ToString());
        }

        return true;
    }

    return false;
}

void CFundamentalnodePayments::Sync(CNode* node, int nCountNeeded)
{
    LOCK(cs_mapFundamentalnodePayeeVotes);

    int nHeight;
    {
        TRY_LOCK(cs_main, locked);
        if (!locked || chainActive.Tip() == NULL) return;
        nHeight = chainActive.Tip()->nHeight;
    }

    int nCount = (mnodeman.CountEnabled() * 1.25);
    if (nCountNeeded > nCount) nCountNeeded = nCount;

    int nInvCount = 0;
    std::map<uint256, CFundamentalnodePaymentWinner>::iterator it = mapFundamentalnodePayeeVotes.begin();
    while (it != mapFundamentalnodePayeeVotes.end()) {
        CFundamentalnodePaymentWinner winner = (*it).second;
        if (winner.nBlockHeight >= nHeight - nCountNeeded && winner.nBlockHeight <= nHeight + 20) {
            node->PushInventory(CInv(MSG_FUNDAMENTALNODE_WINNER, winner.GetHash()));
            nInvCount++;
        }
        ++it;
    }
    node->PushMessage("ssc", FUNDAMENTALNODE_SYNC_MNW, nInvCount);
}

std::string CFundamentalnodePayments::ToString() const
{
    std::ostringstream info;

    info << "Votes: " << (int)mapFundamentalnodePayeeVotes.size() << ", Blocks: " << (int)mapFundamentalnodeBlocks.size();

    return info.str();
}


int CFundamentalnodePayments::GetOldestBlock()
{
    LOCK(cs_mapFundamentalnodeBlocks);

    int nOldestBlock = std::numeric_limits<int>::max();

    std::map<int, CFundamentalnodeBlockPayees>::iterator it = mapFundamentalnodeBlocks.begin();
    while (it != mapFundamentalnodeBlocks.end()) {
        if ((*it).first < nOldestBlock) {
            nOldestBlock = (*it).first;
        }
        it++;
    }

    return nOldestBlock;
}


int CFundamentalnodePayments::GetNewestBlock()
{
    LOCK(cs_mapFundamentalnodeBlocks);

    int nNewestBlock = 0;

    std::map<int, CFundamentalnodeBlockPayees>::iterator it = mapFundamentalnodeBlocks.begin();
    while (it != mapFundamentalnodeBlocks.end()) {
        if ((*it).first > nNewestBlock) {
            nNewestBlock = (*it).first;
        }
        it++;
    }

    return nNewestBlock;
}
