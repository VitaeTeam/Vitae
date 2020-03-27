// Copyright (c) 2014-2015 The Bitsend developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "netbase.h"
#include "masternodeconfig.h"
#include "util.h"
#include "ui_interface.h"
#include <base58.h>

CMasternodeConfig masternodeConfig;

void CMasternodeConfig::add(std::string alias, std::string ip, std::string privKey, std::string txHash, std::string outputIndex, std::string donationAddress, std::string donationPercent) {
    CMasternodeEntry cme(alias, ip, privKey, txHash, outputIndex, donationAddress, donationPercent);
    entries.push_back(cme);
}

bool CMasternodeConfig::read(std::string& strErr)
{
    int linenumber = 1;
    boost::filesystem::path pathMasternodeConfigFile = GetMasternodeConfigFile();
    boost::filesystem::ifstream streamConfig(pathMasternodeConfigFile);

    if (!streamConfig.good()) {
        FILE* configFile = fopen(pathMasternodeConfigFile.string().c_str(), "a");
        if (configFile != NULL) {
            fclose(configFile);
        }
        return true; // Nothing to read, so just return
	}

    for (std::string line; std::getline(streamConfig, line); linenumber++) {
        if(line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string alias, ip, privKey, txHash, outputIndex, donation, donationAddress, donationPercent;
        if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex >> donation)) {
            donationAddress = "";
            donationPercent = "";
            iss.str(line);
            iss.clear();
            if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
                strErr = "Could not parse masternode.conf line: " + line;
                streamConfig.close();
                return false;
            }
        } else {
            size_t pos = donation.find_first_of(":");
            if(pos == string::npos) { // no ":" found
                donationPercent = "100";
                donationAddress = donation;
            } else {
                donationPercent = donation.substr(pos + 1);
                donationAddress = donation.substr(0, pos);
            }
            CBitcoinAddress address(donationAddress);
            if (!address.IsValid()) {
                strErr = "Invalid Bitsend address in masternode.conf line: " + line;
                streamConfig.close();
                return false;
            }
        }

        if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
            iss.str(line);
            iss.clear();
            if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
                strErr = _("Could not parse masternode.conf") + "\n" +
                         strprintf(_("Line: %d"), linenumber) + "\n\"" + line + "\"";
                streamConfig.close();
                return false;
            }
        }

        int port = 0;
        std::string hostname = "";
        SplitHostPort(ip, port, hostname);
        if(port == 0 || hostname == "") {
            strErr = _("Failed to parse host:port string") + "\n"+
                     strprintf(_("Line: %d"), linenumber) + "\n\"" + line + "\"";
            streamConfig.close();
            return false;
        }

        if (Params().NetworkID() == CBaseChainParams::MAIN) {
            if (port != 8765) {
                strErr = _("Invalid port detected in masternode.conf") + "\n" +
                         strprintf(_("Line: %d"), linenumber) + "\n\"" + line + "\"" + "\n" +
                         _("(must be 51472 for mainnet)");
                streamConfig.close();
                return false;
            }
        } else if (port == 8765) {
            strErr = _("Invalid port detected in masternode.conf") + "\n" +
                     strprintf(_("Line: %d"), linenumber) + "\n\"" + line + "\"" + "\n" +
                     _("(51472 could be used only on mainnet)");
            streamConfig.close();
            return false;
        }


        add(alias, ip, privKey, txHash, outputIndex, donationAddress, donationPercent);
    }

    streamConfig.close();
    return true;
}
bool CMasternodeConfig::CMasternodeEntry::castOutputIndex(int &n)
{
    try {
        n = std::stoi(outputIndex);
    } catch (const std::exception e) {
        LogPrintf("%s: %s on getOutputIndex\n", __func__, e.what());
        return false;
    }

    return true;
}
