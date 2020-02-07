/**
 * @file       denominations_functions.h
 *
 * @brief      Denomination functions for the Zerocoin library.
 *
 * @copyright  Copyright 2018 PIVX Developers
 * @license    This project is released under the MIT license.
 **/
// Copyright (c) 2015-2018 The PIVX developers

#include "reverse_iterate.h"
#include "util.h"
#include "libzerocoin/Denominations.h"
#include "zvit/zerocoin.h"
#include <list>
#include <map>
std::vector<CMintMeta> SelectMintsFromList(const CAmount nValueTarget, CAmount& nSelectedValue,
                                               int nMaxNumberOfSpends,
                                               int& nCoinsReturned,
                                               const std::list<CMintMeta>& listMints,
                                               const std::map<libzerocoin::CoinDenomination, CAmount> mapDenomsHeld,
                                               int& nNeededSpends
                                               );

int calculateChange(
    int nMaxNumberOfSpends,
    const CAmount nValueTarget,
    const std::map<libzerocoin::CoinDenomination, CAmount>& mapOfDenomsHeld,
    std::map<libzerocoin::CoinDenomination, CAmount>& mapOfDenomsUsed);

void listSpends(const std::vector<CMintMeta>& vSelectedMints);
