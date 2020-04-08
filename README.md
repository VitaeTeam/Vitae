VITAE Core integration/staging repository
=====================================

[![Build Status](https://travis-ci.org/VITAE-Project/VITAE.svg?branch=master)](https://travis-ci.org/VITAE-Project/VITAE) [![GitHub version](https://badge.fury.io/gh/VITAE-Project%2FVITAE.svg)](https://badge.fury.io/gh/VITAE-Project%2FVITAE)

VITAE is a cutting edge cryptocurrency, with many features not available in most other cryptocurrencies.
- Anonymized transactions using the _Zerocoin Protocol_.
- Fast transactions featuring guaranteed zero confirmation transactions, we call it _SwiftX_.
- Decentralized blockchain voting providing for consensus based advancement of the current Masternode
  technology used to secure the network and provide the above features, each Masternode is secured
  with a collateral of 20K VITAE. Each Supernode secured with 10K VITAE burn.

More information at [vitaetoken.io](http://www.vitaetoken.io)

### Coin Specs
<table>
<tr><td>Algo</td><td>Quark</td></tr>
<tr><td>Block Time</td><td>45 Seconds</td></tr>
<tr><td>Difficulty Retargeting</td><td>Every Block</td></tr>
<tr><td>Max Coin Supply (PoW Phase)</td><td>1,105,940 VITAE</td></tr>
<tr><td>Max Coin Supply (PoS Phase)</td><td>100,000,000 VITAE</td></tr>
<tr><td>Premine</td><td>1,105,940 VITAE*</td></tr>
</table>

*800,000 VITAE of the Premine is to be burned for creation of Supernodes.  As of Block [23575](http://vitaetoken.io:8181/block/d1c76ba65be8748cc350f44884b8a084fc9f9de9dfd03dccf147cfd1d4388781) nearly 300,000 VITAE has been burned for the creation of Supernodes.

### Reward Distribution

<table>
<th colspan=4>PoW Phase</th>
<tr><th>Block Height</th><th>Reward Amount</th><th>Notes</th><th>Duration (Days)</th></tr>
<tr><td>1</td><td>30 VITAE</td><td rowspan=2>Initial Premine</td><td rowspan=2> Approx 0 Days</td></tr>
<tr><td>2</td><td>1,100,000 VITAE</td></tr>
<tr><td>3-200</td><td>30 VITAE</td><td rowspan=1>Initial Mining</td><td rowspan=1> Approx 1 Days</td></tr>
<tr><th colspan=4>PoS Phase</th></tr>
<tr><th>Block Height</th><th colspan=3>Reward Amount</th></tr>
<tr><td>201-Infinite</td><td colspan=3>Variable based on <a href="https://vitae.org/knowledge-base/see-saw-rewards-mechanism/">SeeSaw Reward Mechanism</a></td></tr>
</table>

### PoW Rewards Breakdown

<table>
<th>Block Height</th><th>Masternodes</th><th>Miner</th><th>Budget</th>
<tr><td>1-2</td><td>0% (0 VITAE)</td><td>100% (1100030 VITAE)</td><td>N/A</td></tr>
<tr><td>3-200</td><td>0% (0 VITAE)</td><td>100% (30 VITAE)</td><td>N/A</td></tr>
</table>

### PoS Rewards Breakdown

<table>
<th>Phase</th><th>Block Height</th><th>Reward</th><th>Supernodes</th><th>Masternodes & Stakers</th><th>Budget</th>
<tr><td>Phase 1</td><td>201-189750</td><td>30 VITAE</td><td>60% (18 VITAE)</td><td>60% (12 VITAE)</td><td>0% (0 VITAE)</td></tr>
<tr><td>Phase 2</td><td>189750-259200</td><td>30 VITAE</td><td>40% (12 VITAE)</td><td>60% (18 VITAE)</td><td>0% (0 VITAE)</td></tr>
<tr><td>Phase 3</td><td>259200-518400</td><td>20 VITAE</td><td>40% (8 VITAE)</td><td>60% (12 VITAE)</td><td>0% (0 VITAE)</td></tr>
<tr><td>Phase 4</td><td>518400-777600</td><td>15 VITAE</td><td>40% (6 VITAE)</td><td>60% (9 VITAE)</td><td>0% (0 VITAE)</td></tr>
<tr><td>Phase 5</td><td>777600-1036800</td><td>10 VITAE</td><td>40% (4 VITAE)</td><td>60% (6 VITAE)</td><td>0% (0 VITAE)</td></tr>
<tr><td>Phase 6</td><td>1036800-onwards</td><td>5 VITAE</td><td>40% (2 VITAE)</td><td>60% (3 VITAE)</td><td>0% (0 VITAE)</td></tr>
</table>
