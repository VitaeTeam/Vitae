VITAE Core version *3.2.2* is now available from:  <https://github.com/vitae-project/vitae/releases>

This is a new minor version release, including various bug fixes and performance improvements.

Please report bugs using the issue tracker at github: <https://github.com/vitae-project/vitae/issues>

Supplemental Update
==============

VITAE Core v3.2.2 is a **supplemental update** to v3.2.0/1 containing minor bug fixes. Users are still advised to read the [v3.2.0 Release Notes](https://github.com/vitae-project/vitae/blob/master/doc/release-notes/release-notes-3.2.0.md) to familiarize themselves with the major feature changes.

While updating from v3.2.0 is not required, it is highly recommended, especially for anyone encountering the issues detailed in the Notable Changes section below.

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely shut down (which might take a few minutes for older versions), then run the installer (on Windows) or just copy over /Applications/VITAE-Qt (on Mac) or vitaed/vitae-qt (on Linux).


Compatibility
==============

VITAE Core is extensively tested on multiple operating systems using the Linux kernel, macOS 10.10+, and Windows 7 and later.

Microsoft ended support for Windows XP on [April 8th, 2014](https://www.microsoft.com/en-us/WindowsForBusiness/end-of-xp-support),
No attempt is made to prevent installing or running the software on Windows XP, you
can still do so at your own risk but be aware that there are known instabilities and issues.
Please do not report issues about Windows XP to the issue tracker.

VITAE Core is extensively tested on multiple operating systems using the Linux kernel, macOS 10.10+, and Windows 7 and later.

Microsoft ended support for Windows XP on [April 8th, 2014](https://www.microsoft.com/en-us/WindowsForBusiness/end-of-xp-support), No attempt is made to prevent installing or running the software on Windows XP, you can still do so at your own risk but be aware that there are known instabilities and issues. Please do not report issues about Windows XP to the issue tracker.

Apple released it's last Mountain Lion update August 13, 2015, and officially ended support on [December 14, 2015](http://news.fnal.gov/2015/10/mac-os-x-mountain-lion-10-8-end-of-life-december-14/). VITAE Core software starting with v3.2.0 will no longer run on MacOS versions prior to Yosemite (10.10). Please do not report issues about MacOS versions prior to Yosemite to the issue tracker.

VITAE Core should also work on most other Unix-like systems but is not frequently tested on them.


Notable Changes
==============

## zVIT Public Spends

Recent exploits of the Zerocoin protocol (Wrapped serials and broken P1 proof) required us to enable the zerocoin spork and deactivate zVIT functionality in order to secure the supply until the pertinent review process was completed.

Moving forward from this undesired situation, we are enabling a secure and chain storage friendly solution for the zerocoin public spend (aka zVIT to VIT conversion).

The explanation of how this works can be found in #891

After block `1,880,000` has past, `SPORK_16` will be deactivated to allow zVIT spends to occur using this new public spend method for version 2 zVIT (version 1 zVIT won't be spendable, see note below). zVIT public spends, as the name suggests, are **NOT** private, they reveal the input mint that is being spent. The minting of **NEW** zVIT, as well as zVIT staking will remain disabled for the time being.

It is advised that users spend/convert their existing zVIT to VIT, which can be done via the GUI or RPC as it was prior to the disabling of zVIT. Note that with the public spend method, the restriction on the number of denominations per transaction (previously 7) has been lifted, and now allows for several hundred denominations per transaction.

*Note on version 1 zVIT*: Version 1 zVIT was only available to me minted between versions v3.0.0 (Oct 6, 2017) and v3.1.0 (May 8, 2018). The announcement that version 1 zVIT was deprecated went out on May 1, 2018 with a recommendation for users to spend/convert their version 1 zVIT.

Version 1 zVIT will be made spendable at a later date due to the extra work required in order to make these version 1 mints spendable.

## GUI Changes

### Options Dialog Cleanup

The options/settings UI dialog has been cleaned up to no longer show settings that are wallet related when running in "disable wallet" (`-disablewallet`) mode.

### Privacy Tab

Notice text has been added to the privacy tab indicating that zVIT minting is disabled, as well as the removal of UI elements that supported such functionality. Notice text has also been added indicating that zVIT spends are currently **NOT** private.

## RPC Changes

### Removal of Deprecated Commands

The `masternode` and `mnbudget` RPC commands, which were marked as deprecated in VITAE Core v2.3.1 (September 19, 2017), have now been completely removed from VITAE Core.

Several new commands were added in v2.3.1 to replace the two aforementioned commands, reference the [v2.3.1 Release Notes](https://github.com/VitaeTeam/Vitae/blob/master/doc/release-notes/release-notes-2.3.1.md#rpc-changes) for further details.

### New `getblockindexstats` Command

A new RPC command (`getblockindexstats`) has been introduced which serves the purpose of obtaining statistical information on a range of blocks. The information returned is as follows:
  * transaction count (not including coinbase/coinstake txes)
  * transaction count (including coinbase/coinstake txes)
  * zVIT per-denom mint count
  * zVIT per-denom spend count
  * total transaction bytes
  * total fees in block range
  * average fee per kB

Command Reference:
```$xslt
getblockindexstats height range ( fFeeOnly )
nReturns aggregated BlockIndex data for blocks
height, height+1, height+2, ..., height+range-1]

nArguments:
1. height             (numeric, required) block height where the search starts.
2. range              (numeric, required) number of blocks to include.
3. fFeeOnly           (boolean, optional, default=False) return only fee info.
```
Result:
```
{
  first_block: x,              (integer) First counted block
  last_block: x,               (integer) Last counted block
  txcount: xxxxx,              (numeric) tx count (excluding coinbase/coinstake)
  txcount_all: xxxxx,          (numeric) tx count (including coinbase/coinstake)
  mintcount: {              [if fFeeOnly=False]
        denom_1: xxxx,         (numeric) number of mints of denom_1 occurred over the block range
        denom_5: xxxx,         (numeric) number of mints of denom_5 occurred over the block range
         ...                    ... number of mints of other denominations: ..., 10, 50, 100, 500, 1000, 5000
  },
  spendcount: {             [if fFeeOnly=False]
        denom_1: xxxx,         (numeric) number of spends of denom_1 occurred over the block range
        denom_5: xxxx,         (numeric) number of spends of denom_5 occurred over the block range
         ...                    ... number of spends of other denominations: ..., 10, 50, 100, 500, 1000, 5000
  },
  pubspendcount: {          [if fFeeOnly=False]
        denom_1: xxxx,         (numeric) number of PUBLIC spends of denom_1 occurred over the block range
        denom_5: xxxx,         (numeric) number of PUBLIC spends of denom_5 occurred over the block range
         ...                   ... number of PUBLIC spends of other denominations: ..., 10, 50, 100, 500, 1000, 5000
  },
  txbytes: xxxxx,              (numeric) Sum of the size of all txes (zVIT excluded) over block range
  ttlfee: xxxxx,               (numeric) Sum of the fee amount of all txes (zVIT mints excluded) over block range
  ttlfee_all: xxxxx,           (numeric) Sum of the fee amount of all txes (zVIT mints included) over block range
  feeperkb: xxxxx,             (numeric) Average fee per kb (excluding zc txes)
}
```

## Build System Changes

### New Architectures for Depends

This issue has been fixed, and no user funds were at risk.


Performance Improvements
------

### New checkpoints

More recent checkpoints have been added for mainnet. These help alleviate some of the load when (re-)syncing from the network.

*3.2.2* Change log
==============

Detailed release notes follow. This overview includes changes that affect behavior, not code moves, refactors and string updates. For convenience in locating the code changes and accompanying discussion, both the pull request and git merge commit are mentioned.

### P2P Protocol and Network Code
 - #880 `a890dc97cd` [NET] Valid forked blocks rejected fix. (furszy)
 - #884 `013676df00` [Net] Add additional checkpoints (Fuzzbawls)
 - #887 `ec7993eac8` [Net] Fix incorrect last checkpoint timestamp (Fuzzbawls)



## Credits
Thanks to everyone who directly contributed to this release:

 - Fuzzbawls
 - Mrs-X
 - SHTDJ
 - Sieres
 - Warrows
 - fanquake
 - gpdionisio
 - presstab


As well as everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/vitae-project-translations/), the QA team during Testing and the Node hosts supporting our Testnet.
