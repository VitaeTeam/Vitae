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

Minimum Supported MacOS Version
------

The minimum supported version of MacOS (OSX) has been moved from 10.8 Mountain Lion to 10.10 Yosemite. Users still running a MacOS version prior to Yosemite will need to upgrade their OS if they wish to continue using the latest version(s) of the VITAE Core wallet.

Bug Fixes
------

### Improper rejection of valid blocks

A bug was discovered in the block acceptance portion of the code that resulted in rejection of otherwise valid blocks. This caused a race condition where some clients ended up on a low-difficulty chain.

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
