// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2016-2019 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPORKID_H
#define SPORKID_H

/*
    Don't ever reuse these IDs for other sporks
    - This would result in old clients getting confused about which spork is for what
*/

enum SporkId : int32_t {
    SPORK_2_SWIFTTX                             = 10001,
    SPORK_3_SWIFTTX_BLOCK_FILTERING             = 10002,
    SPORK_5_MAX_VALUE                           = 10004,
    SPORK_7_FUNDAMENTALNODE_SCANNING            = 10006,
    SPORK_8_FUNDAMENTALNODE_PAYMENT_ENFORCEMENT = 10007,
    SPORK_9_FUNDAMENTALNODE_BUDGET_ENFORCEMENT  = 10008,
    //SPORK_10_FUNDAMENTALNODE_PAY_UPDATED_NODES= 10009,
    SPORK_13_ENABLE_SUPERBLOCKS                 = 10012,
    SPORK_14_NEW_PROTOCOL_ENFORCEMENT           = 10013,
    SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2         = 10014,
    SPORK_16_NEW_PROTOCOL_ENFORCEMENT_3         = 10015,
    SPORK_17_NEW_PROTOCOL_ENFORCEMENT_4         = 10016,
    SPORK_18_NEW_PROTOCOL_ENFORCEMENT_5         = 10017,
    SPORK_19_FUNDAMENTALNODE_PAY_UPDATED_NODES  = 10018,
    SPORK_20_ZEROCOIN_MAINTENANCE_MODE          = 10019,
    SPORK_21_MASTERNODE_PAY_UPDATED_NODES       = 10020,

    SPORK_START                                 = SPORK_2_SWIFTTX,
    SPORK_END                                   = SPORK_21_MASTERNODE_PAY_UPDATED_NODES,
    SPORK_INVALID                               = -1
};

#endif
