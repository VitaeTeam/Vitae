// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activefundamentalnode.h"
#include "db.h"
#include "init.h"
#include "main.h"
#include "fundamentalnode-budget.h"
#include "fundamentalnode-payments.h"
#include "fundamentalnodeconfig.h"
#include "fundamentalnodeman.h"
#include "rpcserver.h"
#include "utilmoneystr.h"

#include "masternode-pos.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "masternode.h"
#include "activemasternode.h"

#include <univalue.h>

#include <boost/tokenizer.hpp>
#include <fstream>


void SendMoney(const CTxDestination& address, CAmount nValue, CWalletTx& wtxNew, AvailableCoinsType coin_type = ALL_COINS)
{
    // Check amount
    if (nValue <= 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount");

    if (nValue > pwalletMain->GetBalance())
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    string strError;
    if (pwalletMain->IsLocked()) {
        strError = "Error: Wallet locked, unable to create transaction!";
        LogPrintf("SendMoney() : %s", strError);
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    // Parse Vitae address
    CScript scriptPubKey = GetScriptForDestination(address);

    // Create and send the transaction
    CReserveKey reservekey(pwalletMain);
    CAmount nFeeRequired;
    if (!pwalletMain->CreateTransaction(scriptPubKey, nValue, wtxNew, reservekey, nFeeRequired, strError, NULL, coin_type)) {
        if (nValue + nFeeRequired > pwalletMain->GetBalance())
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds!", FormatMoney(nFeeRequired));
        LogPrintf("SendMoney() : %s\n", strError);
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    if (!pwalletMain->CommitTransaction(wtxNew, reservekey))
        throw JSONRPCError(RPC_WALLET_ERROR, "Error: The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
}

UniValue obfuscation(const UniValue& params, bool fHelp)
{
    throw runtime_error("Obfuscation is not supported any more. Use Zerocoin\n");

    if (fHelp || params.size() == 0)
        throw runtime_error(
            "obfuscation <vitaeaddress> <amount>\n"
            "vitaeaddress, reset, or auto (AutoDenominate)"
            "<amount> is a real and will be rounded to the next 0.1" +
            HelpRequiringPassphrase());

    if (pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");

    if (params[0].get_str() == "auto") {
        if (fFundamentalNode)
            return "ObfuScation is not supported from fundamentalnodes";

        return "DoAutomaticDenominating " + (obfuScationPool.DoAutomaticDenominating() ? "successful" : ("failed: " + obfuScationPool.GetStatus()));
    }

    if (params[0].get_str() == "reset") {
        obfuScationPool.Reset();
        return "successfully reset obfuscation";
    }

    if (params.size() != 2)
        throw runtime_error(
            "obfuscation <vitaeaddress> <amount>\n"
            "vitaeaddress, denominate, or auto (AutoDenominate)"
            "<amount> is a real and will be rounded to the next 0.1" +
            HelpRequiringPassphrase());

    CBitcoinAddress address(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Vitae address");

    // Amount
    CAmount nAmount = AmountFromValue(params[1]);

    // Wallet comments
    CWalletTx wtx;
    //    string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, wtx, ONLY_DENOMINATED);
    SendMoney(address.Get(), nAmount, wtx, ONLY_DENOMINATED);
    //    if (strError != "")
    //        throw JSONRPCError(RPC_WALLET_ERROR, strError);

    return wtx.GetHash().GetHex();
}


UniValue getpoolinfo(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getpoolinfo\n"
            "\nReturns anonymous pool-related information\n"

            "\nResult:\n"
            "{\n"
            "  \"current\": \"addr\",    (string) VITAE address of current fundamentalnode\n"
            "  \"state\": xxxx,        (string) unknown\n"
            "  \"entries\": xxxx,      (numeric) Number of entries\n"
            "  \"accepted\": xxxx,     (numeric) Number of entries accepted\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getpoolinfo", "") + HelpExampleRpc("getpoolinfo", ""));

    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("current_fundamentalnode", mnodeman.GetCurrentFundamentalNode()->addr.ToString()));
    obj.push_back(Pair("state", obfuScationPool.GetState()));
    obj.push_back(Pair("entries", obfuScationPool.GetEntriesCount()));
    obj.push_back(Pair("entries_accepted", obfuScationPool.GetCountEntriesAccepted()));
    return obj;
}

// This command is retained for backwards compatibility, but is depreciated.
// Future removal of this command is planned to keep things clean.
UniValue fundamentalnode(const UniValue& params, bool fHelp)
{
    string strCommand;
    if (params.size() >= 1)
        strCommand = params[0].get_str();

    if (fHelp ||
        (strCommand != "start" && strCommand != "start-alias" && strCommand != "start-many" && strCommand != "start-all" && strCommand != "start-missing" &&
            strCommand != "start-disabled" && strCommand != "list" && strCommand != "list-conf" && strCommand != "count" && strCommand != "enforce" &&
            strCommand != "debug" && strCommand != "current" && strCommand != "winners" && strCommand != "genkey" && strCommand != "connect" &&
            strCommand != "outputs" && strCommand != "status" && strCommand != "calcscore"))
        throw runtime_error(
            "fundamentalnode \"command\"...\n"
            "\nSet of commands to execute fundamentalnode related actions\n"
            "This command is depreciated, please see individual command documentation for future reference\n\n"

            "\nArguments:\n"
            "1. \"command\"        (string or set of strings, required) The command to execute\n"

            "\nAvailable commands:\n"
            "  count        - Print count information of all known fundamentalnodes\n"
            "  current      - Print info on current fundamentalnode winner\n"
            "  debug        - Print fundamentalnode status\n"
            "  genkey       - Generate new fundamentalnodeprivkey\n"
            "  outputs      - Print fundamentalnode compatible outputs\n"
            "  start        - Start fundamentalnode configured in vitae.conf\n"
            "  start-alias  - Start single fundamentalnode by assigned alias configured in fundamentalnode.conf\n"
            "  start-<mode> - Start fundamentalnodes configured in fundamentalnode.conf (<mode>: 'all', 'missing', 'disabled')\n"
            "  status       - Print fundamentalnode status information\n"
            "  list         - Print list of all known fundamentalnodes (see fundamentalnodelist for more info)\n"
            "  list-conf    - Print fundamentalnode.conf in JSON format\n"
            "  winners      - Print list of fundamentalnode winners\n");

    if (strCommand == "list") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return listfundamentalnodes(newParams, fHelp);
    }

    if (strCommand == "connect") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return fundamentalnodeconnect(newParams, fHelp);
    }

    if (strCommand == "count") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return getfundamentalnodecount(newParams, fHelp);
    }

    if (strCommand == "current") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return fundamentalnodecurrent(newParams, fHelp);
    }

    if (strCommand == "debug") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return fundamentalnodedebug(newParams, fHelp);
    }

    if (strCommand == "start" || strCommand == "start-alias" || strCommand == "start-many" || strCommand == "start-all" || strCommand == "start-missing" || strCommand == "start-disabled") {
        return startfundamentalnode(params, fHelp);
    }

    if (strCommand == "genkey") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return createfundamentalnodekey(newParams, fHelp);
    }

    if (strCommand == "list-conf") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return listfundamentalnodeconf(newParams, fHelp);
    }

    if (strCommand == "outputs") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return getfundamentalnodeoutputs(newParams, fHelp);
    }

    if (strCommand == "status") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return getfundamentalnodestatus(newParams, fHelp);
    }

    if (strCommand == "winners") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return getfundamentalnodewinners(newParams, fHelp);
    }

    if (strCommand == "calcscore") {
        UniValue newParams(UniValue::VARR);
        // forward params but skip command
        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return getfundamentalnodescores(newParams, fHelp);
    }

    return NullUniValue;
}

UniValue masternode(const UniValue& params, bool fHelp)
{
    string strCommand;
    if (params.size() >= 1)
        strCommand = params[0].get_str();

    if (fHelp  ||
            (strCommand != "start" && strCommand != "start-alias" && strCommand != "start-many" && strCommand != "stop" && strCommand != "stop-alias" && strCommand != "stop-many" && strCommand != "list-conf" && strCommand != "count"  && strCommand != "enforce"
             && strCommand != "debug" && strCommand != "current" && strCommand != "winners" && strCommand != "genkey" && strCommand != "connect" && strCommand != "outputs" /* && strCommand != "vote-many" && strCommand != "vote" */))
        throw runtime_error(
                "masternode \"command\"... ( \"passphrase\" )\n"
                "Set of commands to execute masternode related actions\n"
                "\nArguments:\n"
                "1. \"command\"        (string or set of strings, required) The command to execute\n"
                "2. \"passphrase\"     (string, optional) The wallet passphrase\n"
                "\nAvailable commands:\n"
                "  count        - Print number of all known masternodes (optional: 'enabled', 'both')\n"
                "  current      - Print info on current masternode winner\n"
                "  debug        - Print masternode status\n"
                "  genkey       - Generate new masternodeprivkey\n"
                "  enforce      - Enforce masternode payments\n"
                "  outputs      - Print masternode compatible outputs\n"
                "  start        - Start masternode configured in bitsend.conf\n"
                "  start-alias  - Start single masternode by assigned alias configured in masternode.conf\n"
                "  start-many   - Start all masternodes configured in masternode.conf\n"
                "  stop         - Stop masternode configured in bitsend.conf\n"
                "  stop-alias   - Stop single masternode by assigned alias configured in masternode.conf\n"
                "  stop-many    - Stop all masternodes configured in masternode.conf\n"
                "  list         - see masternodelist, This command has been removed.\n"
                "  list-conf    - Print masternode.conf in JSON format\n"
                "  winners      - Print list of masternode winners\n"
                "  vote-many    - Not implemented\n"
                "  vote         - Not implemented\n"
                );

    if (strCommand == "stop")
    {
        if(!fMasterNode) return "you must set masternode=1 in the configuration";

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                            "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        std::string errorMessage;
        if(!activeMasternode.StopMasterNode(errorMessage)) {
            return "stop failed: " + errorMessage;
        }
        pwalletMain->Lock();
        /*CService service;
        CService service2(LookupNumeric(strMasterNodeAddr.c_str(), 0));
        service = service2;
        g_connman->OpenNetworkConnection((CAddress)service, false, NULL, service.ToString().c_str());*/

        if(activeMasternode.status == MASTERNODE_STOPPED) return "successfully stopped masternode";
        if(activeMasternode.status == MASTERNODE_NOT_CAPABLE) return "not capable masternode";

        return "unknown";
    }

    if (strCommand == "stop-alias")
    {
        if (params.size() < 2){
            throw runtime_error(
                        "command needs at least 2 parameters\n");
        }

        std::string alias = params[1].get_str().c_str();

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 3){
                strWalletPass = params[2].get_str().c_str();
            } else {
                throw runtime_error(
                            "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        bool found = false;

        //Object statusObj;
        UniValue statusObj(UniValue::VOBJ);
        statusObj.push_back(Pair("alias", alias));

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            if(mne.getAlias() == alias) {
                found = true;
                std::string errorMessage;
                bool result = activeMasternode.StopMasterNode(mne.getIp(), mne.getPrivKey(), errorMessage);

                statusObj.push_back(Pair("result", result ? "successful" : "failed"));
                if(!result) {
                    statusObj.push_back(Pair("errorMessage", errorMessage));
                }
                break;
            }
        }

        if(!found) {
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("errorMessage", "could not find alias in config. Verify with list-conf."));
        }

        pwalletMain->Lock();
        return statusObj;
    }

    if (strCommand == "stop-many")
    {
        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                            "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        int total = 0;
        int successful = 0;
        int fail = 0;


        //Object resultsObj;
        UniValue resultsObj(UniValue::VOBJ);

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            total++;

            std::string errorMessage;
            bool result = activeMasternode.StopMasterNode(mne.getIp(), mne.getPrivKey(), errorMessage);

            //Object statusObj;
            UniValue statusObj(UniValue::VOBJ);
            statusObj.push_back(Pair("alias", mne.getAlias()));
            statusObj.push_back(Pair("result", result ? "successful" : "failed"));

            if(result) {
                successful++;
            } else {
                fail++;
                statusObj.push_back(Pair("errorMessage", errorMessage));
            }

            resultsObj.push_back(Pair("status", statusObj));
        }
        pwalletMain->Lock();

        //Object returnObj;
        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", "Successfully stopped " + std::to_string(successful) + " masternodes, failed to stop " +
                                 std::to_string(fail) + ", total " + std::to_string(total)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;

    }

    if (strCommand == "count")
    {
        if (params.size() > 2){
            throw runtime_error(
                        "too many parameters\n");
        }
        UniValue rtnStr(UniValue::VSTR);
        if (params.size() == 2)
        {
            /*if(params[1] == "enabled"){
                                return mnodeman.CountEnabled();
                                //return rtnStr;
                        }*/
            /* if(params[1] == "both"){
                                rtnStr = std::to_string(mnodeman.CountEnabled()) + " / " + std::to_string(mnodeman.size());
                                return rtnStr;
                        } */
        }
        UniValue obj(UniValue::VOBJ);
//        int nCount = 0;
        int ipv4 = 0, ipv6 = 0, onion = 0;

//        if (chainActive.Tip())
//            m_nodeman.GetNextMasternodeInQueueForPayment(chainActive.Tip()->nHeight, true, nCount);

        m_nodeman.CountNetworks(ActiveProtocol(), ipv4, ipv6, onion);

        obj.push_back(Pair("total", m_nodeman.size()));
        obj.push_back(Pair("stable", m_nodeman.stable_size()));
        obj.push_back(Pair("obfcompat", m_nodeman.CountMasternodesAboveProtocol(ActiveProtocol())));
        obj.push_back(Pair("enabled", m_nodeman.CountEnabled()));
//        obj.push_back(Pair("inqueue", nCount));
        obj.push_back(Pair("ipv4", ipv4));
        obj.push_back(Pair("ipv6", ipv6));
        obj.push_back(Pair("onion", onion));

        return obj;
    }

    if (strCommand == "start")
    {
        if(!fMasterNode) return "you must set masternode=1 in the configuration";

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                            "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        if(activeMasternode.status != MASTERNODE_REMOTELY_ENABLED && activeMasternode.status != MASTERNODE_IS_CAPABLE){
            activeMasternode.status = MASTERNODE_NOT_PROCESSED; // TODO: consider better way
            std::string errorMessage;
            activeMasternode.ManageStatus();
            pwalletMain->Lock();
        }

        if(activeMasternode.status == MASTERNODE_REMOTELY_ENABLED) return "masternode started remotely";
        if(activeMasternode.status == MASTERNODE_INPUT_TOO_NEW) return "masternode input must have at least 15 confirmations";
        if(activeMasternode.status == MASTERNODE_STOPPED) return "masternode is stopped";
        if(activeMasternode.status == MASTERNODE_IS_CAPABLE) return "successfully started masternode";
        if(activeMasternode.status == MASTERNODE_NOT_CAPABLE) return "not capable masternode: " + activeMasternode.notCapableReason;
        if(activeMasternode.status == MASTERNODE_SYNC_IN_PROCESS) return "sync in process. Must wait until client is synced to start.";

        return "unknown";
    }

    if (strCommand == "start-alias")
    {
        if (params.size() < 2){
            throw runtime_error(
                        "command needs at least 2 parameters\n");
        }

        std::string alias = params[1].get_str().c_str();

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 3){
                strWalletPass = params[2].get_str().c_str();
            } else {
                throw runtime_error(
                            "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        bool found = false;

        //Object statusObj;
        UniValue statusObj(UniValue::VOBJ);
        statusObj.push_back(Pair("alias", alias));

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            if(mne.getAlias() == alias) {
                found = true;
                std::string errorMessage;

                std::string strDonateAddress = mne.getDonationAddress();
                std::string strDonationPercentage = mne.getDonationPercentage();

                bool result = activeMasternode.Register(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strDonateAddress, strDonationPercentage, errorMessage);

                statusObj.push_back(Pair("result", result ? "successful" : "failed"));
                if(!result) {
                    statusObj.push_back(Pair("errorMessage", errorMessage));
                }
                break;
            }
        }

        if(!found) {
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("errorMessage", "could not find alias in config. Verify with list-conf."));
        }

        pwalletMain->Lock();
        return statusObj;

    }

    if (strCommand == "start-many")
    {
        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                            "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
        mnEntries = masternodeConfig.getEntries();

        int total = 0;
        int successful = 0;
        int fail = 0;

        //Object resultsObj;
        UniValue resultsObj(UniValue::VOBJ);

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            total++;

            std::string errorMessage;

            std::string strDonateAddress = mne.getDonationAddress();
            std::string strDonationPercentage = mne.getDonationPercentage();

            bool result = activeMasternode.Register(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strDonateAddress, strDonationPercentage, errorMessage);

            //Object statusObj;
            UniValue statusObj(UniValue::VOBJ);
            statusObj.push_back(Pair("alias", mne.getAlias()));
            statusObj.push_back(Pair("result", result ? "successful" : "failed"));

            if(result) {
                successful++;
            } else {
                fail++;
                statusObj.push_back(Pair("errorMessage", errorMessage));
            }

            resultsObj.push_back(Pair("status", statusObj));
        }
        pwalletMain->Lock();

        //Object returnObj;
        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", "Successfully started " + std::to_string(successful) + " masternodes, failed to start " +
                                 std::to_string(fail) + ", total " + std::to_string(total)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }

    if (strCommand == "debug")
    {
        if(activeMasternode.status == MASTERNODE_REMOTELY_ENABLED) return "masternode started remotely";
        if(activeMasternode.status == MASTERNODE_INPUT_TOO_NEW) return "masternode input must have at least 15 confirmations";
        if(activeMasternode.status == MASTERNODE_IS_CAPABLE) return "successfully started masternode";
        if(activeMasternode.status == MASTERNODE_STOPPED) return "masternode is stopped";
        if(activeMasternode.status == MASTERNODE_NOT_CAPABLE) return "not capable masternode: " + activeMasternode.notCapableReason;
        if(activeMasternode.status == MASTERNODE_SYNC_IN_PROCESS) return "sync in process. Must wait until client is synced to start.";

        CTxIn vin = CTxIn();
        //CPubKey pubkey = CScript();
        CPubKey pubkey;
        CKey key;
        bool found = activeMasternode.GetMasterNodeVin(vin, pubkey, key);
        if(!found){
            return "Missing masternode input, please look at the documentation for instructions on masternode creation";
        } else {
            return "No problems were found";
        }
    }

    if (strCommand == "create")
    {

        return "Not implemented yet, please look at the documentation for instructions on masternode creation";
    }

    if (strCommand == "current")
    {
        CMasternode* winner = m_nodeman.GetCurrentMasterNode(1);
        if(winner) {
            //Object obj;
            UniValue obj(UniValue::VOBJ);
            CScript pubkey;
            pubkey = GetScriptForDestination(winner->pubkey.GetID());
            CTxDestination address1;
            ExtractDestination(pubkey, address1);
            CBitcoinAddress address2(address1);

            obj.push_back(Pair("IP:port",       winner->addr.ToString().c_str()));
            obj.push_back(Pair("protocol",      (int64_t)winner->protocolVersion));
            obj.push_back(Pair("vin",           winner->vin.prevout.hash.ToString().c_str()));
            obj.push_back(Pair("pubkey",        address2.ToString().c_str()));
            obj.push_back(Pair("lastseen",      (int64_t)winner->lastTimeSeen));
            obj.push_back(Pair("activeseconds", (int64_t)(winner->lastTimeSeen - winner->sigTime)));
            return obj;
        }

        return "unknown";
    }

    if (strCommand == "genkey")
    {
        CKey secret;
        secret.MakeNewKey(false);

        return CBitcoinSecret(secret).ToString();
    }

    if (strCommand == "winners")
    {
        //Object obj;
        UniValue obj(UniValue::VOBJ);

        for(int nHeight = chainActive.Tip()->nHeight-10; nHeight < chainActive.Tip()->nHeight+20; nHeight++)
        {
            CScript payee;
            if(masternodePayments.GetBlockPayee(nHeight, payee)){
                CTxDestination address1;
                ExtractDestination(payee, address1);
                CBitcoinAddress address2(address1);
                obj.push_back(Pair(std::to_string(nHeight),       address2.ToString().c_str()));
            } else {
                obj.push_back(Pair(std::to_string(nHeight),       ""));
            }
        }

        return obj;
    }

    if(strCommand == "enforce")
    {
        return (uint64_t)enforceMasternodePaymentsTime;
    }

    if(strCommand == "connect")
    {
        std::string strAddress = "";
        if (params.size() == 2){
            strAddress = params[1].get_str().c_str();
        } else {
            throw runtime_error(
                        "Masternode address required\n");
        }

        CService addr = CService(strAddress);

        if(ConnectNode((CAddress)addr, NULL , true )){
            return "successfully connected";
        } else {
            return "error connecting";
        }


    }

    if(strCommand == "list-conf")
    {
        std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
        mnEntries = masternodeConfig.getEntries();

        //Object resultObj;
        UniValue resultObj(UniValue::VOBJ);

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            //Object mnObj;
            UniValue mnObj(UniValue::VOBJ);
            mnObj.push_back(Pair("alias", mne.getAlias()));
            mnObj.push_back(Pair("address", mne.getIp()));
            mnObj.push_back(Pair("privateKey", mne.getPrivKey()));
            mnObj.push_back(Pair("txHash", mne.getTxHash()));
            mnObj.push_back(Pair("outputIndex", mne.getOutputIndex()));
            mnObj.push_back(Pair("donationAddress", mne.getDonationAddress()));
            mnObj.push_back(Pair("donationPercent", mne.getDonationPercentage()));
            resultObj.push_back(Pair("masternode", mnObj));
        }

        return resultObj;
    }

    if (strCommand == "outputs"){
        // Find possible candidates
        vector<COutput> possibleCoins = activeMasternode.SelectCoinsMasternode();

        //Object obj;
        UniValue obj(UniValue::VOBJ);
        BOOST_FOREACH(COutput& out, possibleCoins) {
            obj.push_back(Pair(out.tx->GetHash().ToString().c_str(), std::to_string(out.i)));
        }

        return obj;

    }

    /*if(strCommand == "vote-many")
    {
        std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
        mnEntries = masternodeConfig.getEntries();

        if (params.size() != 2)
        throw runtime_error("You can only vote 'yea' or 'nay'");

        std::string vote = params[1].get_str().c_str();
        if(vote != "yea" && vote != "nay") return "You can only vote 'yea' or 'nay'";
        int nVote = 0;
        if(vote == "yea") nVote = 1;
        if(vote == "nay") nVote = -1;


                int success = 0;
                int failed = 0;

        Object resultObj;

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            std::string errorMessage;
            std::vector<unsigned char> vchMasterNodeSignature;
            std::string strMasterNodeSignMessage;

            CPubKey pubKeyCollateralAddress;
            CKey keyCollateralAddress;
            CPubKey pubKeyMasternode;
            CKey keyMasternode;

                        if(!darkSendSigner.SetKey(mne.getPrivKey(), errorMessage, keyMasternode, pubKeyMasternode)){
                                printf(" Error upon calling SetKey for %s\n", mne.getAlias().c_str());
                                failed++;
                                continue;
                        }

                        CMasternode* pmn = mnodeman.Find(pubKeyMasternode);
                        if(pmn == NULL)
                        {
                                printf("Can't find masternode by pubkey for %s\n", mne.getAlias().c_str());
                                failed++;
                                continue;
            }

            std::string strMessage = pmn->vin.ToString() + std::to_string(nVote);

                        if(!darkSendSigner.SignMessage(strMessage, errorMessage, vchMasterNodeSignature, keyMasternode)){
                                printf(" Error upon calling SignMessage for %s\n", mne.getAlias().c_str());
                                failed++;
                                continue;
                        }

                        if(!darkSendSigner.SignMessage(strMessage, errorMessage, vchMasterNodeSignature, keyMasternode)){
                                printf(" Error upon calling SignMessage for %s\n", mne.getAlias().c_str());
                                failed++;
                                continue;
                        }

                        success++;

            //send to all peers
            //LOCK(cs_vNodes);
            //BOOST_FOREACH(CNode* pnode, vNodes)
               // pnode->PushMessage("mvote", pmn->vin, vchMasterNodeSignature, nVote);

        }
                return("Voted successfully " + std::to_string(success) + " time(s) and failed " + std::to_string(failed) + " time(s).");
    }*/

    /*if (strCommand == "list")
    {
        UniValue newParams(UniValue::VARR);

        for (unsigned int i = 1; i < params.size(); i++) {
            newParams.push_back(params[i]);
        }
        return masternodelist(newParams);
    }*/
}

UniValue masternodelist(const UniValue& params, bool fHelp)
{
        std::string strMode = "status";
    std::string strFilter = "";

    if (params.size() >= 1) strMode = params[0].get_str();
    if (params.size() == 2) strFilter = params[1].get_str();

    if (fHelp ||
            (strMode != "status" && strMode != "vin" && strMode != "pubkey" && strMode != "lastseen" && strMode != "activeseconds" && strMode != "rank"
                && strMode != "protocol" && strMode != "full" && strMode != "votes" && strMode != "donation" && strMode != "pose"))
    {
        throw runtime_error(
                "masternodelist ( \"mode\" \"filter\" )\n"
                "Get a list of masternodes in different modes\n"
                "\nArguments:\n"
                "1. \"mode\"      (string, optional/required to use filter, defaults = status) The mode to run list in\n"
                "2. \"filter\"    (string, optional) Filter results. Partial match by IP by default in all modes, additional matches in some modes\n"
                "\nAvailable modes:\n"
                "  activeseconds  - Print number of seconds masternode recognized by the network as enabled\n"
                "  donation       - Show donation settings\n"
                "  full           - Print info in format 'status protocol pubkey vin lastseen activeseconds' (can be additionally filtered, partial match)\n"
                "  lastseen       - Print timestamp of when a masternode was last seen on the network\n"
                "  pose           - Print Proof-of-Service score\n"
                "  protocol       - Print protocol of a masternode (can be additionally filtered, exact match))\n"
                "  pubkey         - Print public key associated with a masternode (can be additionally filtered, partial match)\n"
                "  rank           - Print rank of a masternode based on current block\n"
                "  status         - Print masternode status: ENABLED / EXPIRED / VIN_SPENT / REMOVE / POS_ERROR (can be additionally filtered, partial match)\n"
                "  vin            - Print vin associated with a masternode (can be additionally filtered, partial match)\n"
                "  votes          - Print all masternode votes for a Bitsend initiative (can be additionally filtered, partial match)\n"
                );
    }

    //Object obj;
        UniValue obj(UniValue::VOBJ);
    if (strMode == "rank") {
        std::vector<pair<int, CMasternode> > vMasternodeRanks = m_nodeman.GetMasternodeRanks(chainActive.Tip()->nHeight);
        BOOST_FOREACH(PAIRTYPE(int, CMasternode)& s, vMasternodeRanks) {
            std::string strAddr = s.second.addr.ToString();
            if(strFilter !="" && strAddr.find(strFilter) == string::npos) continue;
            obj.push_back(Pair(strAddr,       s.first));
        }
    } else {
        std::vector<CMasternode> vMasternodes = m_nodeman.GetFullMasternodeVector();
        BOOST_FOREACH(CMasternode& mn, vMasternodes) {
            std::string strAddr = mn.addr.ToString();
            if (strMode == "activeseconds") {
                if(strFilter !="" && strAddr.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strAddr,       (int64_t)(mn.lastTimeSeen - mn.sigTime)));
            } else if (strMode == "donation") {
                CTxDestination address1;
                ExtractDestination(mn.donationAddress, address1);
                CBitcoinAddress address2(address1);

                if(strFilter !="" && address2.ToString().find(strFilter) == string::npos &&
                    strAddr.find(strFilter) == string::npos) continue;

                std::string strOut = "";

                if(mn.donationPercentage != 0){
                    strOut = address2.ToString().c_str();
                    strOut += ":";
                    strOut += std::to_string(mn.donationPercentage);
                }
                obj.push_back(Pair(strAddr,       strOut.c_str()));
            } else if (strMode == "full") {
                CScript pubkey;
                pubkey = GetScriptForDestination(mn.pubkey.GetID());
                CTxDestination address1;
                ExtractDestination(pubkey, address1);
                CBitcoinAddress address2(address1);

                std::ostringstream addrStream;
                addrStream << setw(21) << strAddr;

                std::ostringstream stringStream;
                stringStream << setw(10) <<
                               mn.Status() << " " <<
                               mn.protocolVersion << " " <<
                               address2.ToString() << " " <<
                               addrStream.str() << " " <<
                               mn.lastTimeSeen << " " << setw(8) <<
                               (mn.lastTimeSeen - mn.sigTime);
                std::string output = stringStream.str();
                stringStream << " " << strAddr;
                if(strFilter !="" && stringStream.str().find(strFilter) == string::npos &&
                        strAddr.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(mn.vin.prevout.hash.ToString(), output));
            } else if (strMode == "lastseen") {
                if(strFilter !="" && strAddr.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strAddr,       (int64_t)mn.lastTimeSeen));
            } else if (strMode == "protocol") {
                if(strFilter !="" && strFilter != std::to_string(mn.protocolVersion) &&
                    strAddr.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strAddr,       (int64_t)mn.protocolVersion));
            } else if (strMode == "pubkey") {
                CScript pubkey;
                pubkey = GetScriptForDestination(mn.pubkey.GetID());
                CTxDestination address1;
                ExtractDestination(pubkey, address1);
                CBitcoinAddress address2(address1);

                if(strFilter !="" && address2.ToString().find(strFilter) == string::npos &&
                    strAddr.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strAddr,       address2.ToString().c_str()));
            } else if (strMode == "pose") {
                if(strFilter !="" && strAddr.find(strFilter) == string::npos) continue;
                std::string strOut = std::to_string(mn.nScanningErrorCount);
                obj.push_back(Pair(strAddr,       strOut.c_str()));
            } else if(strMode == "status") {
                std::string strStatus = mn.Status();
                if(strFilter !="" && strAddr.find(strFilter) == string::npos && strStatus.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strAddr,       strStatus.c_str()));
            } else if (strMode == "vin") {
                if(strFilter !="" && mn.vin.prevout.hash.ToString().find(strFilter) == string::npos &&
                    strAddr.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strAddr,       mn.vin.prevout.hash.ToString().c_str()));
            } else if(strMode == "votes"){
                std::string strStatus = "ABSTAIN";

                //voting lasts 7 days, ignore the last vote if it was older than that
                if((GetAdjustedTime() - mn.lastVote) < (60*60*8))
                {
                    if(mn.nVote == -1) strStatus = "NAY";
                    if(mn.nVote == 1) strStatus = "YEA";
                }

                if(strFilter !="" && (strAddr.find(strFilter) == string::npos && strStatus.find(strFilter) == string::npos)) continue;
                obj.push_back(Pair(strAddr,       strStatus.c_str()));
            }
        }
    }
    return obj;
}

UniValue getmasternodestatus (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "getmasternodestatus\n"
            "\nPrint masternode status\n"

            "\nResult:\n"
            "{\n"
            "  \"txhash\": \"xxxx\",      (string) Collateral transaction hash\n"
            "  \"outputidx\": n,        (numeric) Collateral transaction output index number\n"
            "  \"netaddr\": \"xxxx\",     (string) Masternode network address\n"
            "  \"addr\": \"xxxx\",        (string) VITAE address for masternode payments\n"
            "  \"status\": \"xxxx\",      (string) Masternode status\n"
            "  \"message\": \"xxxx\"      (string) Masternode status message\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getmasternodestatus", "") + HelpExampleRpc("getmasternodestatus", ""));

    if (!fMasterNode) throw runtime_error("This is not a masternode");

    CMasternode* pmn = m_nodeman.Find(activeMasternode.vin);

    if (pmn) {
        UniValue mnObj(UniValue::VOBJ);
        mnObj.push_back(Pair("txhash", activeMasternode.vin.prevout.hash.ToString()));
        mnObj.push_back(Pair("outputidx", (uint64_t)activeMasternode.vin.prevout.n));
        mnObj.push_back(Pair("netaddr", activeMasternode.service.ToString()));
        //pubkeyCollateralAddress missed, need to check about it
        //mnObj.push_back(Pair("addr", CBitcoinAddress(pmn->pubKeyCollateralAddress.GetID()).ToString()));
        mnObj.push_back(Pair("status", activeMasternode.status));
        mnObj.push_back(Pair("message", activeMasternode.GetStatus()));
        return mnObj;
    }
    throw runtime_error("Masternode not found in the list of available masternodes. Current status: "
                        + activeMasternode.GetStatus());
}

UniValue listfundamentalnodes(const UniValue& params, bool fHelp)
{
    std::string strFilter = "";

    if (params.size() == 1) strFilter = params[0].get_str();

    if (fHelp || (params.size() > 1))
        throw runtime_error(
            "listfundamentalnodes ( \"filter\" )\n"
            "\nGet a ranked list of fundamentalnodes\n"

            "\nArguments:\n"
            "1. \"filter\"    (string, optional) Filter search text. Partial match by txhash, status, or addr.\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"rank\": n,           (numeric) Fundamentalnode Rank (or 0 if not enabled)\n"
            "    \"txhash\": \"hash\",    (string) Collateral transaction hash\n"
            "    \"outidx\": n,         (numeric) Collateral transaction output index\n"
            "    \"status\": s,         (string) Status (ENABLED/EXPIRED/REMOVE/etc)\n"
            "    \"addr\": \"addr\",      (string) Fundamentalnode VITAE address\n"
            "    \"version\": v,        (numeric) Fundamentalnode protocol version\n"
            "    \"lastseen\": ttt,     (numeric) The time in seconds since epoch (Jan 1 1970 GMT) of the last seen\n"
            "    \"activetime\": ttt,   (numeric) The time in seconds since epoch (Jan 1 1970 GMT) fundamentalnode has been active\n"
            "    \"lastpaid\": ttt,     (numeric) The time in seconds since epoch (Jan 1 1970 GMT) fundamentalnode was last paid\n"
            "  }\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n" +
            HelpExampleCli("fundamentalnodelist", "") + HelpExampleRpc("fundamentalnodelist", ""));

    UniValue ret(UniValue::VARR);
    int nHeight;
    {
        LOCK(cs_main);
        CBlockIndex* pindex = chainActive.Tip();
        if(!pindex) return 0;
        nHeight = pindex->nHeight;
    }
    std::vector<pair<int, CFundamentalnode> > vFundamentalnodeRanks = mnodeman.GetFundamentalnodeRanks(nHeight);
    BOOST_FOREACH (PAIRTYPE(int, CFundamentalnode) & s, vFundamentalnodeRanks) {
        UniValue obj(UniValue::VOBJ);
        std::string strVin = s.second.vin.prevout.ToStringShort();
        std::string strTxHash = s.second.vin.prevout.hash.ToString();
        uint32_t oIdx = s.second.vin.prevout.n;

        CFundamentalnode* mn = mnodeman.Find(s.second.vin);

        if (mn != NULL) {
            if (strFilter != "" && strTxHash.find(strFilter) == string::npos &&
                mn->Status().find(strFilter) == string::npos &&
                CBitcoinAddress(mn->pubKeyCollateralAddress.GetID()).ToString().find(strFilter) == string::npos) continue;

            std::string strStatus = mn->Status();
            std::string strHost;
            int port;
            SplitHostPort(mn->addr.ToString(), port, strHost);
            CNetAddr node = CNetAddr(strHost, false);
            std::string strNetwork = GetNetworkName(node.GetNetwork());

            obj.push_back(Pair("rank", (strStatus == "ENABLED" ? s.first : 0)));
            obj.push_back(Pair("network", strNetwork));
            obj.push_back(Pair("txhash", strTxHash));
            obj.push_back(Pair("outidx", (uint64_t)oIdx));
            obj.push_back(Pair("status", strStatus));
            obj.push_back(Pair("addr", CBitcoinAddress(mn->pubKeyCollateralAddress.GetID()).ToString()));
            obj.push_back(Pair("version", mn->protocolVersion));
            obj.push_back(Pair("lastseen", (int64_t)mn->lastPing.sigTime));
            obj.push_back(Pair("activetime", (int64_t)(mn->lastPing.sigTime - mn->sigTime)));
            obj.push_back(Pair("lastpaid", (int64_t)mn->GetLastPaid()));

            ret.push_back(obj);
        }
    }

    return ret;
}

UniValue fundamentalnodeconnect(const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 1))
        throw runtime_error(
            "fundamentalnodeconnect \"address\"\n"
            "\nAttempts to connect to specified fundamentalnode address\n"

            "\nArguments:\n"
            "1. \"address\"     (string, required) IP or net address to connect to\n"

            "\nExamples:\n" +
            HelpExampleCli("fundamentalnodeconnect", "\"192.168.0.6:8765\"") + HelpExampleRpc("fundamentalnodeconnect", "\"192.168.0.6:8765\""));

    std::string strAddress = params[0].get_str();

    CService addr = CService(strAddress);

    CNode* pnode = ConnectNode((CAddress)addr, NULL, false);
    if (pnode) {
        pnode->Release();
        return NullUniValue;
    } else {
        throw runtime_error("error connecting\n");
    }
}

UniValue getfundamentalnodecount (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() > 0))
        throw runtime_error(
            "getfundamentalnodecount\n"
            "\nGet fundamentalnode count values\n"

            "\nResult:\n"
            "{\n"
            "  \"total\": n,        (numeric) Total fundamentalnodes\n"
            "  \"stable\": n,       (numeric) Stable count\n"
            "  \"obfcompat\": n,    (numeric) Obfuscation Compatible\n"
            "  \"enabled\": n,      (numeric) Enabled fundamentalnodes\n"
            "  \"inqueue\": n       (numeric) Fundamentalnodes in queue\n"
            "}\n"
            "\nExamples:\n" +
            HelpExampleCli("getfundamentalnodecount", "") + HelpExampleRpc("getfundamentalnodecount", ""));

    UniValue obj(UniValue::VOBJ);
    int nCount = 0;
    int ipv4 = 0, ipv6 = 0, onion = 0;

    if (chainActive.Tip())
        mnodeman.GetNextFundamentalnodeInQueueForPayment(chainActive.Tip()->nHeight, true, nCount);

    mnodeman.CountNetworks(ActiveProtocol(), ipv4, ipv6, onion);

    obj.push_back(Pair("total", mnodeman.size()));
    obj.push_back(Pair("stable", mnodeman.stable_size()));
    obj.push_back(Pair("obfcompat", mnodeman.CountEnabled(ActiveProtocol())));
    obj.push_back(Pair("enabled", mnodeman.CountEnabled()));
    obj.push_back(Pair("inqueue", nCount));
    obj.push_back(Pair("ipv4", ipv4));
    obj.push_back(Pair("ipv6", ipv6));
    obj.push_back(Pair("onion", onion));

    return obj;
}

UniValue fundamentalnodecurrent (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "fundamentalnodecurrent\n"
            "\nGet current fundamentalnode winner\n"

            "\nResult:\n"
            "{\n"
            "  \"protocol\": xxxx,        (numeric) Protocol version\n"
            "  \"txhash\": \"xxxx\",      (string) Collateral transaction hash\n"
            "  \"pubkey\": \"xxxx\",      (string) FN Public key\n"
            "  \"lastseen\": xxx,       (numeric) Time since epoch of last seen\n"
            "  \"activeseconds\": xxx,  (numeric) Seconds FN has been active\n"
            "}\n"
            "\nExamples:\n" +
            HelpExampleCli("fundamentalnodecurrent", "") + HelpExampleRpc("fundamentalnodecurrent", ""));

    CFundamentalnode* winner = mnodeman.GetCurrentFundamentalNode(1);
    if (winner) {
        UniValue obj(UniValue::VOBJ);

        obj.push_back(Pair("protocol", (int64_t)winner->protocolVersion));
        obj.push_back(Pair("txhash", winner->vin.prevout.hash.ToString()));
        obj.push_back(Pair("pubkey", CBitcoinAddress(winner->pubKeyCollateralAddress.GetID()).ToString()));
        obj.push_back(Pair("lastseen", (winner->lastPing == CFundamentalnodePing()) ? winner->sigTime : (int64_t)winner->lastPing.sigTime));
        obj.push_back(Pair("activeseconds", (winner->lastPing == CFundamentalnodePing()) ? 0 : (int64_t)(winner->lastPing.sigTime - winner->sigTime)));
        return obj;
    }

    throw runtime_error("unknown");
}

UniValue fundamentalnodedebug (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "fundamentalnodedebug\n"
            "\nPrint fundamentalnode status\n"

            "\nResult:\n"
            "\"status\"     (string) Fundamentalnode status message\n"
            "\nExamples:\n" +
            HelpExampleCli("fundamentalnodedebug", "") + HelpExampleRpc("fundamentalnodedebug", ""));

    if (activeFundamentalnode.status != ACTIVE_FUNDAMENTALNODE_INITIAL || !fundamentalnodeSync.IsSynced())
        return activeFundamentalnode.GetStatus();

    CTxIn vin = CTxIn();
    CPubKey pubkey;
    CKey key;
    if (!activeFundamentalnode.GetFundamentalNodeVin(vin, pubkey, key))
        throw runtime_error("Missing fundamentalnode input, please look at the documentation for instructions on fundamentalnode creation\n");
    else
        return activeFundamentalnode.GetStatus();
}

UniValue startfundamentalnode (const UniValue& params, bool fHelp)
{
    std::string strCommand;
    if (params.size() >= 1) {
        strCommand = params[0].get_str();

        // Backwards compatibility with legacy 'fundamentalnode' super-command forwarder
        if (strCommand == "start") strCommand = "local";
        if (strCommand == "start-alias") strCommand = "alias";
        if (strCommand == "start-all") strCommand = "all";
        if (strCommand == "start-many") strCommand = "many";
        if (strCommand == "start-missing") strCommand = "missing";
        if (strCommand == "start-disabled") strCommand = "disabled";
    }

    if (fHelp || params.size() < 2 || params.size() > 3 ||
        (params.size() == 2 && (strCommand != "local" && strCommand != "all" && strCommand != "many" && strCommand != "missing" && strCommand != "disabled")) ||
        (params.size() == 3 && strCommand != "alias"))
        throw runtime_error(
            "startfundamentalnode \"local|all|many|missing|disabled|alias\" lockwallet ( \"alias\" )\n"
            "\nAttempts to start one or more fundamentalnode(s)\n"

            "\nArguments:\n"
            "1. set         (string, required) Specify which set of fundamentalnode(s) to start.\n"
            "2. lockwallet  (boolean, required) Lock wallet after completion.\n"
            "3. alias       (string) Fundamentalnode alias. Required if using 'alias' as the set.\n"

            "\nResult: (for 'local' set):\n"
            "\"status\"     (string) Fundamentalnode status message\n"

            "\nResult: (for other sets):\n"
            "{\n"
            "  \"overall\": \"xxxx\",     (string) Overall status message\n"
            "  \"detail\": [\n"
            "    {\n"
            "      \"node\": \"xxxx\",    (string) Node name or alias\n"
            "      \"result\": \"xxxx\",  (string) 'success' or 'failed'\n"
            "      \"error\": \"xxxx\"    (string) Error message, if failed\n"
            "    }\n"
            "    ,...\n"
            "  ]\n"
            "}\n"
            "\nExamples:\n" +
            HelpExampleCli("startfundamentalnode", "\"alias\" \"0\" \"my_mn\"") + HelpExampleRpc("startfundamentalnode", "\"alias\" \"0\" \"my_mn\""));

    bool fLock = (params[1].get_str() == "true" ? true : false);

    if (strCommand == "local") {
        if (!fFundamentalNode) throw runtime_error("you must set fundamentalnode=1 in the configuration\n");

        if (pwalletMain->IsLocked())
            throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");

        if (activeFundamentalnode.status != ACTIVE_FUNDAMENTALNODE_STARTED) {
            activeFundamentalnode.status = ACTIVE_FUNDAMENTALNODE_INITIAL; // TODO: consider better way
            activeFundamentalnode.ManageStatus();
            if (fLock)
                pwalletMain->Lock();
        }

        return activeFundamentalnode.GetStatus();
    }

    if (strCommand == "all" || strCommand == "many" || strCommand == "missing" || strCommand == "disabled") {
        if (pwalletMain->IsLocked())
            throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");

        if ((strCommand == "missing" || strCommand == "disabled") &&
            (fundamentalnodeSync.RequestedFundamentalnodeAssets <= FUNDAMENTALNODE_SYNC_LIST ||
                fundamentalnodeSync.RequestedFundamentalnodeAssets == FUNDAMENTALNODE_SYNC_FAILED)) {
            throw runtime_error("You can't use this command until fundamentalnode list is synced\n");
        }

        std::vector<CFundamentalnodeConfig::CFundamentalnodeEntry> mnEntries;
        mnEntries = fundamentalnodeConfig.getEntries();

        int successful = 0;
        int failed = 0;

        UniValue resultsObj(UniValue::VARR);

        BOOST_FOREACH (CFundamentalnodeConfig::CFundamentalnodeEntry mne, fundamentalnodeConfig.getEntries()) {
            std::string errorMessage;
            int nIndex;
            if(!mne.castOutputIndex(nIndex))
                continue;
            CTxIn vin = CTxIn(uint256(mne.getTxHash()), uint32_t(nIndex));
            CFundamentalnode* pmn = mnodeman.Find(vin);
            CFundamentalnodeBroadcast mnb;

            if (pmn != NULL) {
                if (strCommand == "missing") continue;
                if (strCommand == "disabled" && pmn->IsEnabled()) continue;
            }

            bool result = activeFundamentalnode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb);

            UniValue statusObj(UniValue::VOBJ);
            statusObj.push_back(Pair("alias", mne.getAlias()));
            statusObj.push_back(Pair("result", result ? "success" : "failed"));

            if (result) {
                successful++;
                statusObj.push_back(Pair("error", ""));
            } else {
                failed++;
                statusObj.push_back(Pair("error", errorMessage));
            }

            resultsObj.push_back(statusObj);
        }
        if (fLock)
            pwalletMain->Lock();

        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", strprintf("Successfully started %d fundamentalnodes, failed to start %d, total %d", successful, failed, successful + failed)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }

    if (strCommand == "alias") {
        std::string alias = params[2].get_str();

        if (pwalletMain->IsLocked())
            throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");

        bool found = false;
        int successful = 0;
        int failed = 0;

        UniValue resultsObj(UniValue::VARR);
        UniValue statusObj(UniValue::VOBJ);
        statusObj.push_back(Pair("alias", alias));

        BOOST_FOREACH (CFundamentalnodeConfig::CFundamentalnodeEntry mne, fundamentalnodeConfig.getEntries()) {
            if (mne.getAlias() == alias) {
                found = true;
                std::string errorMessage;
                CFundamentalnodeBroadcast mnb;

                bool result = activeFundamentalnode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb);

                statusObj.push_back(Pair("result", result ? "successful" : "failed"));

                if (result) {
                    successful++;
                    mnodeman.UpdateFundamentalnodeList(mnb);
                    mnb.Relay();
                } else {
                    failed++;
                    statusObj.push_back(Pair("errorMessage", errorMessage));
                }
                break;
            }
        }

        if (!found) {
            failed++;
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("error", "could not find alias in config. Verify with list-conf."));
        }

        resultsObj.push_back(statusObj);

        if (fLock)
            pwalletMain->Lock();

        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", strprintf("Successfully started %d fundamentalnodes, failed to start %d, total %d", successful, failed, successful + failed)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }
    return NullUniValue;
}

UniValue createfundamentalnodekey (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "createfundamentalnodekey\n"
            "\nCreate a new fundamentalnode private key\n"

            "\nResult:\n"
            "\"key\"    (string) Fundamentalnode private key\n"
            "\nExamples:\n" +
            HelpExampleCli("createfundamentalnodekey", "") + HelpExampleRpc("createfundamentalnodekey", ""));

    CKey secret;
    secret.MakeNewKey(false);

    return CBitcoinSecret(secret).ToString();
}

UniValue getfundamentalnodeoutputs (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "getfundamentalnodeoutputs\n"
            "\nPrint all fundamentalnode transaction outputs\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"txhash\": \"xxxx\",    (string) output transaction hash\n"
            "    \"outputidx\": n       (numeric) output index number\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n" +
            HelpExampleCli("getfundamentalnodeoutputs", "") + HelpExampleRpc("getfundamentalnodeoutputs", ""));

    // Find possible candidates
    vector<COutput> possibleCoins = activeFundamentalnode.SelectCoinsFundamentalnode();

    UniValue ret(UniValue::VARR);
    BOOST_FOREACH (COutput& out, possibleCoins) {
        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("txhash", out.tx->GetHash().ToString()));
        obj.push_back(Pair("outputidx", out.i));
        ret.push_back(obj);
    }

    return ret;
}

UniValue listfundamentalnodeconf (const UniValue& params, bool fHelp)
{
    std::string strFilter = "";

    if (params.size() == 1) strFilter = params[0].get_str();

    if (fHelp || (params.size() > 1))
        throw runtime_error(
            "listfundamentalnodeconf ( \"filter\" )\n"
            "\nPrint fundamentalnode.conf in JSON format\n"

            "\nArguments:\n"
            "1. \"filter\"    (string, optional) Filter search text. Partial match on alias, address, txHash, or status.\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"alias\": \"xxxx\",        (string) fundamentalnode alias\n"
            "    \"address\": \"xxxx\",      (string) fundamentalnode IP address\n"
            "    \"privateKey\": \"xxxx\",   (string) fundamentalnode private key\n"
            "    \"txHash\": \"xxxx\",       (string) transaction hash\n"
            "    \"outputIndex\": n,       (numeric) transaction output index\n"
            "    \"status\": \"xxxx\"        (string) fundamentalnode status\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n" +
            HelpExampleCli("listfundamentalnodeconf", "") + HelpExampleRpc("listfundamentalnodeconf", ""));

    std::vector<CFundamentalnodeConfig::CFundamentalnodeEntry> mnEntries;
    mnEntries = fundamentalnodeConfig.getEntries();

    UniValue ret(UniValue::VARR);

    BOOST_FOREACH (CFundamentalnodeConfig::CFundamentalnodeEntry mne, fundamentalnodeConfig.getEntries()) {
        int nIndex;
        if(!mne.castOutputIndex(nIndex))
            continue;
        CTxIn vin = CTxIn(uint256(mne.getTxHash()), uint32_t(nIndex));
        CFundamentalnode* pmn = mnodeman.Find(vin);

        std::string strStatus = pmn ? pmn->Status() : "MISSING";

        if (strFilter != "" && mne.getAlias().find(strFilter) == string::npos &&
            mne.getIp().find(strFilter) == string::npos &&
            mne.getTxHash().find(strFilter) == string::npos &&
            strStatus.find(strFilter) == string::npos) continue;

        UniValue mnObj(UniValue::VOBJ);
        mnObj.push_back(Pair("alias", mne.getAlias()));
        mnObj.push_back(Pair("address", mne.getIp()));
        mnObj.push_back(Pair("privateKey", mne.getPrivKey()));
        mnObj.push_back(Pair("txHash", mne.getTxHash()));
        mnObj.push_back(Pair("outputIndex", mne.getOutputIndex()));
        mnObj.push_back(Pair("status", strStatus));
        ret.push_back(mnObj);
    }

    return ret;
}

UniValue getfundamentalnodestatus (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "getfundamentalnodestatus\n"
            "\nPrint fundamentalnode status\n"

            "\nResult:\n"
            "{\n"
            "  \"txhash\": \"xxxx\",      (string) Collateral transaction hash\n"
            "  \"outputidx\": n,        (numeric) Collateral transaction output index number\n"
            "  \"netaddr\": \"xxxx\",     (string) Fundamentalnode network address\n"
            "  \"addr\": \"xxxx\",        (string) VITAE address for fundamentalnode payments\n"
            "  \"status\": \"xxxx\",      (string) Fundamentalnode status\n"
            "  \"message\": \"xxxx\"      (string) Fundamentalnode status message\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getfundamentalnodestatus", "") + HelpExampleRpc("getfundamentalnodestatus", ""));

    if (!fFundamentalNode) throw runtime_error("This is not a fundamentalnode");

    CFundamentalnode* pmn = mnodeman.Find(activeFundamentalnode.vin);

    if (pmn) {
        UniValue mnObj(UniValue::VOBJ);
        mnObj.push_back(Pair("txhash", activeFundamentalnode.vin.prevout.hash.ToString()));
        mnObj.push_back(Pair("outputidx", (uint64_t)activeFundamentalnode.vin.prevout.n));
        mnObj.push_back(Pair("netaddr", activeFundamentalnode.service.ToString()));
        mnObj.push_back(Pair("addr", CBitcoinAddress(pmn->pubKeyCollateralAddress.GetID()).ToString()));
        mnObj.push_back(Pair("status", activeFundamentalnode.status));
        mnObj.push_back(Pair("message", activeFundamentalnode.GetStatus()));
        return mnObj;
    }
    throw runtime_error("Fundamentalnode not found in the list of available fundamentalnodes. Current status: "
                        + activeFundamentalnode.GetStatus());
}

UniValue getfundamentalnodewinners (const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw runtime_error(
            "getfundamentalnodewinners ( blocks \"filter\" )\n"
            "\nPrint the fundamentalnode winners for the last n blocks\n"

            "\nArguments:\n"
            "1. blocks      (numeric, optional) Number of previous blocks to show (default: 10)\n"
            "2. filter      (string, optional) Search filter matching MN address\n"

            "\nResult (single winner):\n"
            "[\n"
            "  {\n"
            "    \"nHeight\": n,           (numeric) block height\n"
            "    \"winner\": {\n"
            "      \"address\": \"xxxx\",    (string) VITAE MN Address\n"
            "      \"nVotes\": n,          (numeric) Number of votes for winner\n"
            "    }\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nResult (multiple winners):\n"
            "[\n"
            "  {\n"
            "    \"nHeight\": n,           (numeric) block height\n"
            "    \"winner\": [\n"
            "      {\n"
            "        \"address\": \"xxxx\",  (string) VITAE MN Address\n"
            "        \"nVotes\": n,        (numeric) Number of votes for winner\n"
            "      }\n"
            "      ,...\n"
            "    ]\n"
            "  }\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n" +
            HelpExampleCli("getfundamentalnodewinners", "") + HelpExampleRpc("getfundamentalnodewinners", ""));

    int nHeight;
    {
        LOCK(cs_main);
        CBlockIndex* pindex = chainActive.Tip();
        if(!pindex) return 0;
        nHeight = pindex->nHeight;
    }

    int nLast = 10;
    std::string strFilter = "";

    if (params.size() >= 1)
        nLast = atoi(params[0].get_str());

    if (params.size() == 2)
        strFilter = params[1].get_str();

    UniValue ret(UniValue::VARR);

    for (int i = nHeight - nLast; i < nHeight + 20; i++) {
        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("nHeight", i));

        std::string strPayment = GetRequiredPaymentsString(i);
        if (strFilter != "" && strPayment.find(strFilter) == std::string::npos) continue;

        if (strPayment.find(',') != std::string::npos) {
            UniValue winner(UniValue::VARR);
            boost::char_separator<char> sep(",");
            boost::tokenizer< boost::char_separator<char> > tokens(strPayment, sep);
            BOOST_FOREACH (const string& t, tokens) {
                UniValue addr(UniValue::VOBJ);
                std::size_t pos = t.find(":");
                std::string strAddress = t.substr(0,pos);
                uint64_t nVotes = atoi(t.substr(pos+1));
                addr.push_back(Pair("address", strAddress));
                addr.push_back(Pair("nVotes", nVotes));
                winner.push_back(addr);
            }
            obj.push_back(Pair("winner", winner));
        } else if (strPayment.find("Unknown") == std::string::npos) {
            UniValue winner(UniValue::VOBJ);
            std::size_t pos = strPayment.find(":");
            std::string strAddress = strPayment.substr(0,pos);
            uint64_t nVotes = atoi(strPayment.substr(pos+1));
            winner.push_back(Pair("address", strAddress));
            winner.push_back(Pair("nVotes", nVotes));
            obj.push_back(Pair("winner", winner));
        } else {
            UniValue winner(UniValue::VOBJ);
            winner.push_back(Pair("address", strPayment));
            winner.push_back(Pair("nVotes", 0));
            obj.push_back(Pair("winner", winner));
        }

            ret.push_back(obj);
    }

    return ret;
}

UniValue getfundamentalnodescores (const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getfundamentalnodescores ( blocks )\n"
            "\nPrint list of winning fundamentalnode by score\n"

            "\nArguments:\n"
            "1. blocks      (numeric, optional) Show the last n blocks (default 10)\n"

            "\nResult:\n"
            "{\n"
            "  xxxx: \"xxxx\"   (numeric : string) Block height : Fundamentalnode hash\n"
            "  ,...\n"
            "}\n"
            "\nExamples:\n" +
            HelpExampleCli("getfundamentalnodescores", "") + HelpExampleRpc("getfundamentalnodescores", ""));

    int nLast = 10;

    if (params.size() == 1) {
        try {
            nLast = std::stoi(params[0].get_str());
        } catch (const boost::bad_lexical_cast &) {
            throw runtime_error("Exception on param 2");
        }
    }
    UniValue obj(UniValue::VOBJ);

    std::vector<CFundamentalnode> vFundamentalnodes = mnodeman.GetFullFundamentalnodeVector();
    for (int nHeight = chainActive.Tip()->nHeight - nLast; nHeight < chainActive.Tip()->nHeight + 20; nHeight++) {
        uint256 nHigh = 0;
        CFundamentalnode* pBestFundamentalnode = NULL;
        BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
            uint256 n = mn.CalculateScore(1, nHeight - 100);
            if (n > nHigh) {
                nHigh = n;
                pBestFundamentalnode = &mn;
            }
        }
        if (pBestFundamentalnode)
            obj.push_back(Pair(strprintf("%d", nHeight), pBestFundamentalnode->vin.prevout.hash.ToString().c_str()));
    }

    return obj;
}

bool DecodeHexMnb(CFundamentalnodeBroadcast& mnb, std::string strHexMnb) {

    if (!IsHex(strHexMnb))
        return false;

    vector<unsigned char> mnbData(ParseHex(strHexMnb));
    CDataStream ssData(mnbData, SER_NETWORK, PROTOCOL_VERSION);
    try {
        ssData >> mnb;
    }
    catch (const std::exception&) {
        return false;
    }

    return true;
}

UniValue createfundamentalnodebroadcast(const UniValue& params, bool fHelp)
{
	string strCommand;
	if (params.size() >= 1)
		strCommand = params[0].get_str();
	if (fHelp || (strCommand != "alias" && strCommand != "all") || (strCommand == "alias" && params.size() < 2))
		throw runtime_error(
				"createfundamentalnodebroadcast \"command\" ( \"alias\")\n"
				"Creates a fundamentalnode broadcast message for one or all fundamentalnodes configured in fundamentalnode.conf\n"
				"\nArguments:\n"
				"1. \"command\"      (string, required) \"alias\" for single fundamentalnode, \"all\" for all fundamentalnodes\n"
				"2. \"alias\"        (string, required if command is \"alias\") Alias of the fundamentalnode\n"
				+ HelpRequiringPassphrase());

    if (strCommand == "alias")
    {
        // wait for reindex and/or import to finish
        if (fImporting || fReindex)
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Wait for reindex and/or import to finish");

        std::string alias = params[1].get_str();

        if(pwalletMain->IsLocked()) {
            throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Your wallet is locked, it needs to be unlocked first.");
        }

        bool found = false;

        UniValue statusObj(UniValue::VOBJ);
        statusObj.push_back(Pair("alias", alias));

        BOOST_FOREACH(CFundamentalnodeConfig::CFundamentalnodeEntry mne, fundamentalnodeConfig.getEntries()) {
            if(mne.getAlias() == alias) {
                found = true;
                std::string errorMessage;
                CFundamentalnodeBroadcast mnb;

                bool success = activeFundamentalnode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb, true);

                statusObj.push_back(Pair("success", success));
                if(success) {
                    CDataStream ssMnb(SER_NETWORK, PROTOCOL_VERSION);
                    ssMnb << mnb;
                    statusObj.push_back(Pair("hex", HexStr(ssMnb.begin(), ssMnb.end())));
                } else {
                    statusObj.push_back(Pair("error_message", errorMessage));
                }
                break;
            }
        }

        if(!found) {
            statusObj.push_back(Pair("success", false));
            statusObj.push_back(Pair("error_message", "Could not find alias in config. Verify with list-conf."));
        }

        pwalletMain->Lock();
        return statusObj;

    }

    if (strCommand == "all")
    {
        // wait for reindex and/or import to finish
        if (fImporting || fReindex)
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Wait for reindex and/or import to finish");

        if(pwalletMain->IsLocked()) {
            throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Your wallet is locked, it needs to be unlocked first.");
        }

        std::vector<CFundamentalnodeConfig::CFundamentalnodeEntry> mnEntries;
        mnEntries = fundamentalnodeConfig.getEntries();

        int successful = 0;
        int failed = 0;

        UniValue resultsObj(UniValue::VARR);

        BOOST_FOREACH(CFundamentalnodeConfig::CFundamentalnodeEntry mne, fundamentalnodeConfig.getEntries()) {
            std::string errorMessage;

            CTxIn vin = CTxIn(uint256S(mne.getTxHash()), uint32_t(atoi(mne.getOutputIndex().c_str())));
            CFundamentalnodeBroadcast mnb;

            bool success = activeFundamentalnode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb, true);

            UniValue statusObj(UniValue::VOBJ);
            statusObj.push_back(Pair("alias", mne.getAlias()));
            statusObj.push_back(Pair("success", success));

            if(success) {
                successful++;
                CDataStream ssMnb(SER_NETWORK, PROTOCOL_VERSION);
                ssMnb << mnb;
                statusObj.push_back(Pair("hex", HexStr(ssMnb.begin(), ssMnb.end())));
            } else {
                failed++;
                statusObj.push_back(Pair("error_message", errorMessage));
            }

            resultsObj.push_back(statusObj);
        }
        pwalletMain->Lock();

        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", strprintf("Successfully created broadcast messages for %d fundamentalnodes, failed to create %d, total %d", successful, failed, successful + failed)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }
    
    return NullUniValue;
}

UniValue decodefundamentalnodebroadcast(const UniValue& params, bool fHelp)
{
	if (fHelp || params.size() != 1)
		throw runtime_error(
				"decodefundamentalnodebroadcast \"hexstring\"\n"
				"Command to decode fundamentalnode broadcast messages\n"
				"\nArgument:\n"
				"1. \"hexstring\"        (hex string) The fundamentalnode broadcast message\n"
		        "\nResult:\n"
		        "{\n"
		        "  \"vin\": \"xxxx\"                (COutPoint) The unspent output which is holding the fundamentalnode collateral\n"
		        "  \"addr\": \"xxxx\"               (string) IP address of the fundamentalnode\n"
		        "  \"pubkeycollateral\": \"xxxx\"   (string) Collateral address's public key\n"
		        "  \"pubkeymasternode\": \"xxxx\"   (string) Fundamentalnode's public key\n"
		        "  \"vchsig\": \"xxxx\"             (string) Base64-encoded signature of this message (verifiable via pubkeycollateral)\n"
		        "  \"sigtime\": \"nnn\"             (numeric) Signature timestamp\n"
		        "  \"protocolversion\": \"nnn\"     (numeric) Fundamentalnode's protocol version\n"
		        "  \"nlastdsq\": \"nnn\"            (numeric) The last time the fundamentalnode sent a DSQ message (for mixing) (DEPRECATED)\n"
		        "  \"lastping\" : {                 (json object) information about the fundamentalnode's last ping\n"
		        "      \"vin\": \"xxxx\"            (COutPoint) The unspent output of the fundamentalnode which is signing the message\n"
		        "      \"blockhash\": \"xxxx\"      (string) Current chaintip blockhash minus 12\n"
		        "      \"sigtime\": \"nnn\"         (numeric) Signature time for this ping\n"
		        "      \"vchsig\": \"xxxx\"         (string) Base64-encoded signature of this ping (verifiable via pubkeyfundamentalnode)\n"
		        "}\n"
				"\nExamples:\n" +
				        HelpExampleCli("decodefundamentalnodebroadcast", "hexstring") +
						HelpExampleRpc("decodefundamentalnodebroadcast", "hexstring"));

    CFundamentalnodeBroadcast mnb;

    if (!DecodeHexMnb(mnb, params[0].get_str()))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Fundamentalnode broadcast message decode failed");

    if(!mnb.VerifySignature())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Fundamentalnode broadcast signature verification failed");

    UniValue resultObj(UniValue::VOBJ);

    resultObj.push_back(Pair("vin", mnb.vin.prevout.ToString()));
    resultObj.push_back(Pair("addr", mnb.addr.ToString()));
    resultObj.push_back(Pair("pubkeycollateral", CBitcoinAddress(mnb.pubKeyCollateralAddress.GetID()).ToString()));
    resultObj.push_back(Pair("pubkeyfundamentalnode", CBitcoinAddress(mnb.pubKeyFundamentalnode.GetID()).ToString()));
    resultObj.push_back(Pair("vchsig", EncodeBase64(&mnb.sig[0], mnb.sig.size())));
    resultObj.push_back(Pair("sigtime", mnb.sigTime));
    resultObj.push_back(Pair("protocolversion", mnb.protocolVersion));
    resultObj.push_back(Pair("nlastdsq", mnb.nLastDsq));

    UniValue lastPingObj(UniValue::VOBJ);
    lastPingObj.push_back(Pair("vin", mnb.lastPing.vin.prevout.ToString()));
    lastPingObj.push_back(Pair("blockhash", mnb.lastPing.blockHash.ToString()));
    lastPingObj.push_back(Pair("sigtime", mnb.lastPing.sigTime));
    lastPingObj.push_back(Pair("vchsig", EncodeBase64(&mnb.lastPing.vchSig[0], mnb.lastPing.vchSig.size())));

    resultObj.push_back(Pair("lastping", lastPingObj));

    return resultObj;
}


UniValue relayfundamentalnodebroadcast(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
        		"relayfundamentalnodebroadcast \"hexstring\"\n"
        		"Command to relay fundamentalnode broadcast messages\n"
        		"1. \"hexstring\"        (hex string) The fundamentalnode broadcast message\n"
        		"\nExamples:\n" +
				HelpExampleCli("relayfundamentalnodebroadcast", "hexstring") +
				HelpExampleRpc("relayfundamentalnodebroadcast", "hexstring"));

    CFundamentalnodeBroadcast mnb;

    if (!DecodeHexMnb(mnb, params[0].get_str()))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Fundamentalnode broadcast message decode failed");

    if(!mnb.VerifySignature())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Fundamentalnode broadcast signature verification failed");

    mnodeman.UpdateFundamentalnodeList(mnb);
    mnb.Relay();

    return strprintf("Fundamentalnode broadcast sent (service %s, vin %s)", mnb.addr.ToString(), mnb.vin.ToString());
}
