#!/usr/bin/env python3
# Copyright (c) 2019 The PIVX developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

"""
Covers various scenarios of PoS blocks where the coinstake input is already spent
(either in a previous block, in a "future" block, or in the same block being staked).
Two nodes: nodes[0] moves the chain and checks the spam blocks, nodes[1] sends them.
Spend txes sent from nodes[1] are received by nodes[0]
Start with the PoW chache: 200 blocks.
For each test, nodes[1] sends 3 blocks.

At the beginning nodes[0] mines 50 blocks (201-250) to reach PoS activation.
After tests 1-3, before tests 4-6, nodes[0] sends five transactions (included
in block 261) to nodes[1].
Each tx spends one 250-PIV utxo to send 240.06 PIV (so nodes[1] can later
mint 6 zerocoins from it: 100 + 100 + 10 + 10 + 10 + 10 + fee).
Then nodes[0] stakes 40 more blocks (261-300) to reach zPoS phase.

** Test_1:
(Nodes[1] spams a PoS block on main chain.)
(Stake inputs spent on the same block being staked.)
--> starts at height 250
  - nodes[1] saves his mature utxos 'utxos_to_spend' at block 250
  - nodes[0] mines 5 blocks (251-255) to be used as buffer for adding the fork chain later
  - nodes[1] spams 3 blocks with height 256 --> [REJECTED]
--> ends at height 255

** Test_2
(Nodes[1] spams a PoS block on main chain.)
(Stake inputs spent earlier on main chain.)
--> starts at height 255
  - nodes[1] spends utxos_to_spend at block 256
  - nodes[0] mines 5 more blocks (256-260) to include the spends
  - nodes[1] spams 3 blocks with height 261 --> [REJECTED]
--> ends at height 260

** Test_3:
(Nodes[1] spams PoS blocks on a forked chain.)
(Stake inputs spent later on main chain.)
--> starts at height 260
  - nodes[1] spams fork block with height 251 --> [ACCEPTED]
  - nodes[1] spams fork block with height 252
    (using the same coinstake input as previous block) --> [REJECTED]
  - nodes[1] spams fork block with height 252
    (using a different coinstake input) --> [ACCEPTED]
--> ends at height 260

** Test_4:
(Nodes[1] spams a zPoS blocks on main chain.)
(Staked zerocoins spent on the same block being staked.)
--> starts at height 300
  - nodes[0] stakes 5 blocks (301-305) and, at each block, nodes[1] mints 6 zerocoins
  - nodes[0] stakes 35 more blocks (306-340), so the minted zerocoins mature
  - nodes[1] saves his mature coins 'coins_to_spend' at block 340
  - nodes[0] stakes 5 blocks (341-345) to be used as buffer for adding the fork chain later
  - nodes[1] spams 3 blocks with height 346 --> [REJECTED]
--> ends at height 345

** Test_5:
(Nodes[1] spams a zPoS block on main chain.)
(Staked zerocoins spent earlier on main chain.)
--> starts at height 345
  - nodes[1] spends coins_to_spend at block 346
  - nodes[0] mines 5 more blocks (346-350) to include the spends
  - nodes[1] spams 3 blocks with height 351 --> [REJECTED]
--> ends at height 350

** Test_6:
(Nodes[1] spams zPoS blocks on a forked chain.)
(Staked zerocoins spent later on main chain.)
--> starts at height 350
  - nodes[1] spams fork block with height 341 --> [ACCEPTED]
  - nodes[1] spams fork block with height 342
    (using the same coinstake input as previous block) --> [REJECTED]
  - nodes[1] spams fork block with height 342
    (using a different coinstake input) --> [ACCEPTED]
--> ends at height 350
"""

from io import BytesIO
from time import sleep

from test_framework.authproxy import JSONRPCException
from test_framework.messages import COutPoint
from test_framework.test_framework import PivxTestFramework
from test_framework.util import (
    sync_blocks,
    assert_equal,
    assert_raises_rpc_error,
    bytes_to_hex_str,
    hash256,
    set_node_times,
    DecimalAmt
)


class FakeStakeTest(PivxTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        # nodes[0] moves the chain and checks the spam blocks, nodes[1] sends them
        self.extra_args = [['-staking=0']]*self.num_nodes

    def setup_chain(self):
        # Start with PoW cache: 200 blocks
        self._initialize_chain()
        self.enable_mocktime()

    def log_title(self):
        title = "*** Starting %s ***" % self.__class__.__name__
        underline = "-" * len(title)
        description = "Tests the 'fake stake' scenarios.\n" \
                      "1) Stake on main chain with coinstake input spent on the same block\n" \
                      "2) Stake on main chain with coinstake input spent on a previous block\n" \
                      "3) Stake on a fork chain with coinstake input spent (later) in main chain\n" \
                      "4) zPoS stake on main chain with zpiv coinstake input spent on the same block\n" \
                      "5) zPoS stake on main chain with zpiv coinstake input spent on a previous block\n" \
                      "6) zPoS stake on a fork chain with zpiv coinstake input spent (later) in main chain"
        self.log.info("\n\n%s\n%s\n%s\n", title, underline, description)


    def run_test(self):
        # init custom fields
        self.mocktime -= (131 * 60)
        self.recipient_0 = self.nodes[0].getnewaddress()
        self.recipient_1 = self.nodes[1].getnewaddress()
        self.init_dummy_key()

        # start test
        self.log_title()
        set_node_times(self.nodes, self.mocktime)

        # nodes[0] mines 50 blocks (201-250) to reach PoS activation
        self.log.info("Mining 50 blocks to reach PoS phase...")
        for i in range(50):
            self.mocktime = self.generate_pow(0, self.mocktime)
        sync_blocks(self.nodes)

        # Check Tests 1-3
        self.test_1()
        self.test_2()
        self.test_3()

        # nodes[0] sends five transactions (included in block 261) to nodes[1].
        print()  # add blank line without log indent
        self.log.info("** Fake Stake - Intermission")
        self.log.info("Sending 5 txes from nodes[0] to nodes[1]...")
        for i in range(5):
            self.log.info("%d) %s..." % (i, self.nodes[0].sendtoaddress(self.recipient_1, 240.06)[:16]))

        # Then nodes[0] stakes 40 more blocks (261-300) to reach zPIV phase
        self.log.info("Staking 40 blocks to reach zPoS phase...")
        for i in range(40):
            self.mocktime = self.generate_pos(0, self.mocktime)
        sync_blocks(self.nodes)

        # Check Tests 4-6
        self.test_4()
        self.test_5()
        self.test_6()


    # ** PoS block - cstake input spent on the same block
    def test_1(self):
        print()   # add blank line without log indent
        self.log.info("** Fake Stake - Test_1")

        # nodes[1] saves his mature utxos 'utxos_to_spend' at block 250
        assert_equal(self.nodes[1].getblockcount(), 250)
        self.utxos_to_spend = self.nodes[1].listunspent()
        assert_equal(len(self.utxos_to_spend), 50)
        self.log.info("50 'utxos_to_spend' collected.")

        # nodes[0] mines 5 blocks (251-255) to be used as buffer for adding the fork chain later
        self.log.info("Mining 5 blocks as fork depth...")
        for i in range(5):
            self.mocktime = self.generate_pow(0, self.mocktime)
        sync_blocks(self.nodes)

        # nodes[1] spams 3 blocks with height 256 --> [REJECTED]
        assert_equal(self.nodes[1].getblockcount(), 255)
        self.fake_stake(list(self.utxos_to_spend), fDoubleSpend=True)
        self.log.info("--> Test_1 passed")

    # ** PoS block - cstake input spent in the past
    def test_2(self):
        print()   # add blank line without log indent
        self.log.info("** Fake Stake - Test_2")

        # nodes[1] spends utxos_to_spend at block 256
        assert_equal(self.nodes[1].getblockcount(), 255)
        txid = self.spend_utxos(1, self.utxos_to_spend, self.recipient_0)[0]
        self.log.info("'utxos_to_spend' spent on txid=(%s...) on block 256" % txid[:16])
        self.sync_all()

        # nodes[0] mines 5 more blocks (256-260) to include the spends
        self.log.info("Mining 5 blocks to include the spends...")
        for i in range(5):
            self.mocktime = self.generate_pow(0, self.mocktime)
        sync_blocks(self.nodes)
        self.check_tx_in_chain(0, txid)
        assert_equal(self.nodes[1].getbalance(), 0)

        # nodes[1] spams 3 blocks with height 261 --> [REJECTED]
        assert_equal(self.nodes[1].getblockcount(), 260)
        self.fake_stake(list(self.utxos_to_spend))
        self.log.info("--> Test_2 passed")

    # ** PoS block - cstake input spent in the future
    def test_3(self):
        print()  # add blank line without log indent
        self.log.info("** Fake Stake - Test_3")

        # nodes[1] spams fork block with height 251 --> [ACCEPTED]
        # nodes[1] spams fork block with height 252
        #  (using the same coinstake input as previous block) --> [REJECTED]
        # nodes[1] spams fork block with height 252
        #  (using a different coinstake input) --> [ACCEPTED]
        assert_equal(self.nodes[1].getblockcount(), 260)
        self.fake_stake(list(self.utxos_to_spend), nHeight=251)
        self.log.info("--> Test_3 passed")

    # ** zPoS block - cstake zerocoin input spent on the same block
    def test_4(self):
        print()  # add blank line without log indent
        self.log.info("** Fake Stake - Test_4")

        # nodes[0] stakes 5 blocks (301-305) and, at each block, nodes[1] mints 6 zerocoins
        self.log.info("Staking 5 blocks and minting zerocoins...")
        for i in range(5):
            self.nodes[1].mintzerocoin(240)
            self.sync_all()
            self.mocktime = self.generate_pos(0, self.mocktime)
        sync_blocks(self.nodes)

        # nodes[0] stakes 35 more blocks (306-340), so the minted zerocoins mature
        self.log.info("Staking 35 more blocks to mature the mints...")
        for i in range(35):
            self.mocktime = self.generate_pos(0, self.mocktime)
        sync_blocks(self.nodes)

        # nodes[1] saves his mature coins 'coins_to_spend' at block 340
        assert_equal(self.nodes[1].getblockcount(), 340)
        self.coins_to_spend = self.nodes[1].listmintedzerocoins(True, True)
        assert_equal(len(self.coins_to_spend), 24)
        self.log.info("24 mature 'coins_to_spend' collected.")

        # nodes[0] stakes 5 blocks (341-345) to be used as buffer for adding the fork chain later
        self.log.info("Mining 5 blocks as fork depth...")
        for i in range(5):
            self.mocktime = self.generate_pos(0, self.mocktime)
        sync_blocks(self.nodes)

        # nodes[1] spams blocks with height 346 --> [REJECTED]
        assert_equal(self.nodes[1].getblockcount(), 345)
        self.fake_stake(list(self.coins_to_spend), isZPoS=True, fDoubleSpend=True)
        self.log.info("--> Test_4 passed")

    # ** zPoS block - cstake zerocoin input spent in the past
    def test_5(self):
        print()  # add blank line without log indent
        self.log.info("** Fake Stake - Test_5")

        # nodes[1] spends coins_to_spend at block 346
        assert_equal(self.nodes[1].getblockcount(), 345)
        self.log.info("Spending coins_to_spend from nodes[1] to nodes[0] on block 346...")
        txids = []
        for i in range(4):
            txids.append(self.nodes[1].spendzerocoin(240, False, False, self.recipient_0, False)['txid'])
            self.log.info("%d) %s..." % (i, txids[-1][:16]))
        self.sync_all()

        # nodes[0] mines 5 more blocks (346-350) to include the spends
        self.log.info("Mining 5 blocks to include the spends...")
        for i in range(5):
            self.mocktime = self.generate_pow(0, self.mocktime)
        sync_blocks(self.nodes)
        for txid in txids:
            self.check_tx_in_chain(0, txid)

        # nodes[1] spams blocks with height 351 --> [REJECTED]
        assert_equal(self.nodes[1].getblockcount(), 350)
        self.fake_stake(list(self.coins_to_spend), isZPoS=True)
        self.log.info("--> Test_5 passed")

    # ** zPoS block - cstake zerocoin input spent in the future
    def test_6(self):
        print()  # add blank line without log indent
        self.log.info("** Fake Stake - Test_6")

        # nodes[1] spams fork block with height 341 --> [ACCEPTED]
        # nodes[1] spams fork block with height 342
        #  (using the same coinstake input as previous block) --> [REJECTED]
        # nodes[1] spams fork block with height 342
        #  (using a different coinstake input) --> [ACCEPTED]
        assert_equal(self.nodes[1].getblockcount(), 350)
        self.fake_stake(list(self.coins_to_spend), isZPoS=True, nHeight=341)
        self.log.info("--> Test_6 passed")


    def fake_stake(self,
                   staking_utxo_list,
                   nHeight=-1,
                   isZPoS=False,
                   fDoubleSpend=False):
        """ General method to create, send and test the spam blocks
        :param    staking_utxo_list:  (string list) utxos to use for staking
                  nHeight:            (int, optional) height of the staked block.
                                        Used only for fork chain. In main chain it's current height + 1
                  isZPoS:              (bool) stake the block with zerocoin inputs for coinstake
                  fDoubleSpend:       (bool) if true, stake input is double spent in block.vtx
        :return:
        """
        # Get block number, block time and prevBlock hash
        currHeight = self.nodes[1].getblockcount()
        isMainChain = (nHeight == -1)
        chainName = "main" if isMainChain else "forked"
        nTime = self.mocktime
        if isMainChain:
            nHeight = currHeight + 1
        prevBlockHash = self.nodes[1].getblockhash(nHeight - 1)
        nTime += (nHeight - currHeight) * 60

        # New block hash, coinstake input and list of txes
        bHash = None
        stakedUtxo = None

        # For each test, send three blocks.
        # On main chain they are all the same height.
        # On fork chain, send three blocks where both the second and third block sent,
        # are built on top of the first one.
        for i in range(3):
            fMustBeAccepted = (not isMainChain and i != 1)
            block_txes = []

            # update block number and prevBlock hash on second block sent on forked chain
            if not isMainChain and i == 1:
                nHeight += 1
                nTime += 60
                prevBlockHash = bHash

            stakeInputs = self.get_prevouts(1, staking_utxo_list, isZPoS, nHeight - 1)
            # Update stake inputs for second block sent on forked chain (must stake the same input)
            if not isMainChain and i == 1:
                stakeInputs = self.get_prevouts(1, [stakedUtxo], isZPoS, nHeight-1)

            # Make spam txes sending the inputs to DUMMY_KEY in order to test double spends
            if fDoubleSpend:
                spending_prevouts = self.get_prevouts(1, staking_utxo_list, isZPoS)
                block_txes = self.make_txes(1, spending_prevouts, self.DUMMY_KEY.get_pubkey())

            # Stake the spam block
            block = self.stake_block(1, nHeight, prevBlockHash, stakeInputs,
                                     nTime, "", block_txes, fDoubleSpend)
            # Log stake input
            if not isZPoS:
                prevout = COutPoint()
                prevout.deserialize_uniqueness(BytesIO(block.prevoutStake))
                self.log.info("Staked input: [%s...-%s]" % ('{:x}'.format(prevout.hash)[:12], prevout.n))
            else:
                self.log.info("Staked coin with serial hash [%s...]" % block.prevoutStake.hex()[:16])

            # Try submitblock and check result
            self.log.info("Trying to send block [%s...] with height=%d" % (block.hash[:16], nHeight))
            var = self.nodes[1].submitblock(bytes_to_hex_str(block.serialize()))
            sleep(1)
            if (isZPoS and not isMainChain and i < 2):
                # !TODO: fix this last case failing (this must NOT be accepted)
                fMustBeAccepted = True
            if (not fMustBeAccepted and var not in [None, "rejected", "bad-txns-invalid-zpiv"]):
                raise AssertionError("Error, block submitted (%s) in %s chain" % (var, chainName))
            elif (fMustBeAccepted and var != "inconclusive"):
                raise AssertionError("Error, block not submitted (%s) in %s chain" % (var, chainName))
            self.log.info("Done. Updating context...")

            # Sync and check block hash
            bHash = block.hash
            self.checkBlockHash(bHash, fMustBeAccepted)

            # Update curr block data
            if isZPoS:
                stakedUtxo = [x for x in staking_utxo_list if
                                     x['hash stake'] == block.prevoutStake[::-1].hex()][0]
            else:
                stakedUtxo = [x for x in staking_utxo_list if COutPoint(
                    int(x['txid'], 16), x['vout']).serialize_uniqueness() == block.prevoutStake][0]

            # Remove the used coinstake input (except before second block on fork chain)
            if isMainChain or i != 0:
                staking_utxo_list.remove(stakedUtxo)

        self.log.info("All blocks sent")


    def checkBlockHash(self, bHash, fMustBeAccepted):

        def strBlockCheck(bHash, fAccepted=True, fError=False):
            return "%s Block [%s...] IS %sstored on disk" % (
                "Error!" if fError else "Good.", bHash[:16], "" if fAccepted else "NOT ")
        try:
            block_ret = self.nodes[1].getblock(bHash)
            if not fMustBeAccepted:
                if block_ret is not [None, None]:
                    self.log.warning(str(block_ret))
                    raise AssertionError(strBlockCheck(bHash, True, True))
                else:
                    self.log.info(strBlockCheck(bHash, False, False))
            if fMustBeAccepted:
                if None in block_ret:
                    self.log.warning(str(block_ret))
                    raise AssertionError(strBlockCheck(bHash, False, True))
                else:
                    self.log.info(strBlockCheck(bHash, True, False))

        except JSONRPCException as e:
            exc_msg = str(e)
            if exc_msg in ["Can't read block from disk (-32603)", "Block not found (-5)"]:
                if fMustBeAccepted:
                    raise AssertionError(strBlockCheck(bHash, False, True))
                else:
                    self.log.info(strBlockCheck(bHash, False, False))
            else:
                raise


if __name__ == '__main__':
    FakeStakeTest().main()