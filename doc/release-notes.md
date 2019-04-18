VITAE Core version *3.2.1* is now available from:  <https://github.com/vitae-project/vitae/releases>

This is a new minor version release, including various bug fixes and performance improvements.

Please report bugs using the issue tracker at github: <https://github.com/vitae-project/vitae/issues>

Supplemental Update
==============

VITAE Core v3.2.1 is a **supplemental update** to v3.2.0 containing minor bug fixes. Users are still advised to read the [v3.2.0 Release Notes](https://github.com/vitae-project/vitae/blob/master/doc/release-notes/release-notes-3.2.0.md) to familiarize themselves with the major feature changes.

While updating from v3.2.0 is not required, it is recommended, especially for anyone encountering the issues detailed in the Notable Changes section below.

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

### GUI crash when recalculating zPIV data

A GUI only crash when recalculating zPIV data (mints/spends/supply) has been fixed. Clients syncing via the network from a point prior to any recalculations can now do so without error again.

### macOS installer mounting

The macOS installer image (`.dmg` file) had an issue with it's stylesheet that caused an error after mounting the image. Affected macOS users will now see the expected behavior of a finder window appearing after mounting, allowing drag-n-drop installation of the PIVX-Qt.app

### macOS "Pink Pinstripes"

A GUI wallet stylesheet issue was causing "pink pinstripes" to display in many of the wallet's views, this has now been resolved.

### Incorrect seed warning in zPIV control dialog

Because of the way the zPIV master seed is handled, locked wallets were showing a status message in the zPIV control dialog window which mentioned that the master seed was not the same used to mint the denom. This message was not entirely correct, and a more appropriate message is now displayed for locked wallets.

### Invalid chain state on shutdown

An issue in how the wallet shutdown procedure is carried out was sometimes leading to marking an incoming block as invalid when it in fact was valid. This would cause the client to seem "stuck" when starting it again. This issue is now resolved.

Performance Improvements
------

### New checkpoints

More recent checkpoints have been added for both mainnet and testnet. These help alleviate some of the load when (re-)syncing from the network.

*3.2.0* Change log
==============

Detailed release notes follow. This overview includes changes that affect behavior, not code moves, refactors and string updates. For convenience in locating the code changes and accompanying discussion, both the pull request and git merge commit are mentioned.

### Build System
 - #858 `a2c801205e` [Build] [macOS] Fix macOS dmg issue (10.12+) (Jonas Schnelli)
 - #866 `9cd6016f3a` [Build] Update debian contrib files (Fuzzbawls)

### P2P Protocol and Network Code
 - #861 `909ed11702` [Net] Add new checkpoints for mainnet/testnet (Fuzzbawls)

### GUI
 - #860 `2cefebd1f7` [Qt] Prevent double deletion of progress dialog (Fuzzbawls)
 - #852 `37e88b892f` [QT] Fix a display bug about zPIV mints (warrows)
 - #863 `89b84a4f5a` [Qt] Stop using a solid white image as a border image (Fuzzbawls)

### Miscellaneous
 - #865 `ede1af4e10` [Main] Don't return an invalid state when shutting down the wallet (Fuzzbawls)
 - #868 `a1080d8658` [Performances] Decrease the number of wasted CPU cycles (warrows)
 
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
