#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
Covers the scenario of a PoS block where the coinstake input prevout is already spent.
'''

from time import sleep

from base_test import PIVX_FakeStakeTest

class Test_01(PIVX_FakeStakeTest):

    def run_test(self):
        self.description = "Covers the scenario of a PoS block where the coinstake input prevout is already spent."
        self.init_test()

        INITAL_MINED_BLOCKS = 150   # First mined blocks (rewards collected to spend)
        MORE_MINED_BLOCKS = 100     # Blocks mined after spending
        STAKE_AMPL_ROUNDS = 2       # Rounds of stake amplification
        self.NUM_BLOCKS = 3         # Number of spammed blocks

        # 1) Starting mining blocks
        self.log.info("Mining %d blocks.." % INITAL_MINED_BLOCKS)
        self.node.generate(INITAL_MINED_BLOCKS)

        # 2) Collect the possible prevouts
        self.log.info("Collecting all unspent coins which we generated from mining...")

        # 3) Create 10 addresses - Do the stake amplification
        self.log.info("Performing the stake amplification (%d rounds)..." % STAKE_AMPL_ROUNDS)
        utxo_list = self.node.listunspent()
        address_list = []
        for i in range(10):
            address_list.append(self.node.getnewaddress())
        utxo_list = self.stake_amplification(utxo_list, STAKE_AMPL_ROUNDS, address_list)

        self.log.info("Done. Utxo list has %d elements." % len(utxo_list))
        sleep(2)

        # 4) Start mining again so that spent prevouts get confirmted in a block.
        self.log.info("Mining %d more blocks..." % MORE_MINED_BLOCKS)
        self.node.generate(MORE_MINED_BLOCKS)
        sleep(2)

        # 5) Create "Fake Stake" blocks and send them
        self.log.info("Creating Fake stake blocks")
        err_msgs = self.test_spam("Main", utxo_list)
        if not len(err_msgs) == 0:
            self.log.error("result: " + " | ".join(err_msgs))
            raise AssertionError("TEST FAILED")

        self.log.info("%s PASSED" % self.__class__.__name__)
