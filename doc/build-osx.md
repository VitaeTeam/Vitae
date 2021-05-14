macOS Build Instructions and Notes
====================================
The commands in this guide should be executed in a Terminal application.
The built-in one is located in `/Applications/Utilities/Terminal.app`.

Preparation
-----------
Install the macOS command line tools:

`xcode-select --install`

When the popup appears, click `Install`.

Then install [Homebrew](https://brew.sh).

Dependencies
----------------------

    brew install autoconf automake berkeley-db4 libtool boost miniupnpc openssl pkg-config protobuf python3 qt5 zmq libevent qrencode gmp

See [dependencies.md](dependencies.md) for a complete overview.

If you want to build the disk image with `make deploy` (.dmg / optional), you need RSVG:

    brew install librsvg

Berkeley DB
-----------
It is recommended to use Berkeley DB 4.8. If you have to build it yourself,
you can use [the installation script included in contrib/](/contrib/install_db4.sh)
like so:

```shell
./contrib/install_db4.sh .
```

from the root of the repository.

**Note**: You only need Berkeley DB if the wallet is enabled (see [*Disable-wallet mode*](/doc/build-osx.md#disable-wallet-mode)).

Build Vitae Core
------------------------

#### Install dependencies using Homebrew

        brew install autoconf automake berkeley-db4 libtool boost miniupnpc openssl pkg-config protobuf qt5 zmq libevent

### Building `vitaed`

1. Clone the github tree to get the source code and go into the directory.

        git clone https://github.com/vitaeteam/VITAE.git
        cd VITAE

2.  Make the Homebrew OpenSSL headers visible to the configure script  (do ```brew info openssl``` to find out why this is necessary, or if you use Homebrew with installation folders different from the default).

        export LDFLAGS+=-L/usr/local/opt/openssl/lib
        export CPPFLAGS+=-I/usr/local/opt/openssl/include
        
3.  Build vitaed:

        ./autogen.sh
        ./configure
        make

4.  It is also a good idea to build and run the unit tests:

        make check

5.  (Optional) You can also install vitaed to your path:

        make install

Use Qt Creator as IDE
------------------------
You can use Qt Creator as IDE, for debugging and for manipulating forms, etc.
Download Qt Creator from http://www.qt.io/download/. Download the "community edition" and only install Qt Creator (uncheck the rest during the installation process).

1. Make sure you installed everything through homebrew mentioned above
2. Do a proper ./configure --with-gui=qt5 --enable-debug
3. In Qt Creator do "New Project" -> Import Project -> Import Existing Project
4. Enter "vitae-qt" as project name, enter src/qt as location
5. Leave the file selection as it is
6. Confirm the "summary page"
7. In the "Projects" tab select "Manage Kits..."
8. Select the default "Desktop" kit and select "Clang (x86 64bit in /usr/bin)" as compiler
9. Select LLDB as debugger (you might need to set the path to your installtion)
10. Start debugging with Qt Creator

Creating a release build
------------------------
You can ignore this section if you are building `vitaed` for your own use.


        make deploy

Disable-wallet mode
--------------------
**Note:** This functionality is not yet completely implemented, and compilation using the below option will currently fail.

When the intention is to run only a P2P node without a wallet, VITAE Core may be compiled in
disable-wallet mode with:

    ./configure --disable-wallet

In this case there is no dependency on Berkeley DB 4.8.

Running
-------

Vitae Core is now available at `./src/vitaed`

Before running, you may create an empty configuration file:

    mkdir -p "/Users/${USER}/Library/Application Support/VITAE"

    touch "/Users/${USER}/Library/Application Support/VITAE/vitae.conf"

    chmod 600 "/Users/${USER}/Library/Application Support/VITAE/vitae.conf"

The first time you run vitaed, it will start downloading the blockchain. This process could take many hours, or even days on slower than average systems.

You can monitor the download process by looking at the debug.log file:

    tail -f $HOME/Library/Application\ Support/VITAE/debug.log

Other commands:
-------

    ./vitaed -daemon # to start the vitae daemon.
    ./vitae-cli --help  # for a list of command-line options.
    ./vitae-cli help # Outputs a list of RPC commands when the daemon is running.
