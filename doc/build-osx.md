Mac OS X Build Instructions and Notes
====================================
The commands in this guide should be executed in a Terminal application.
The built-in one is located in `/Applications/Utilities/Terminal.app`.

Preparation
-----------
Install the OS X command line tools:

`xcode-select --install`

When the popup appears, click `Install`.

Then install [Homebrew](https://brew.sh).

Dependencies
----------------------

    brew install automake libtool boost miniupnpc openssl pkg-config protobuf python3 qt libevent

See [dependencies.md](dependencies.md) for a complete overview.

If you want to build the disk image with `make deploy` (.dmg / optional), you need RSVG

    brew install librsvg

Berkeley DB
It is recommended to use Berkeley DB 5.3

```shell
brew tap zeroc-ice/tap
brew install zeroc-ice/tap/berkeley-db@5.3
```

**Note**: You only need Berkeley DB if the wallet is enabled (see the section *Disable-Wallet mode* below).

Build Soferox Core
------------------------

1. Clone the soferox source code and cd into `soferox`

        git clone https://github.com/Soferox/soferox.git
        cd soferox

2.  Build soferox-core:

    Configure and build the headless soferox binaries as well as the GUI (if Qt is found).

    You can disable the GUI build by passing `--without-gui` to configure.

        ./autogen.sh
        ./configure LDFLAGS=-L/usr/local/opt/berkeley-db@5.3/lib CPPFLAGS=-I/usr/local/opt/berkeley-db@5.3/include
        make

3.  You can also create a .dmg that contains the .app bundle (optional):

        make deploy

Running
-------

Soferox Core is now available at `./src/soferoxd`

Before running, it's recommended you create an RPC configuration file.

    echo -e "rpcuser=soferoxrpc\nrpcpassword=$(xxd -l 16 -p /dev/urandom)" > "/Users/${USER}/Library/Application Support/Soferox/soferox.conf"

    chmod 600 "/Users/${USER}/Library/Application Support/Soferox/soferox.conf"

The first time you run soferoxd, it will start downloading the blockchain. This process could take several hours.

You can monitor the download process by looking at the debug.log file:
    tail -f $HOME/Library/Application\ Support/Soferox/debug.log

Once dependencies are compiled, see [doc/release-process.md](release-process.md) for how the Soferox-Qt.app
bundle is packaged and signed to create the .dmg disk image that is distributed.

Other commands:
-------

    ./src/soferoxd -daemon # Starts the soferox daemon.
    ./src/soferox-cli --help # Outputs a list of command-line options.
    ./src/soferox-cli help # Outputs a list of RPC commands when the daemon is running.

Using Qt Creator as IDE
------------------------
You can use Qt Creator as an IDE, for soferox development.
Download and install the community edition of [Qt Creator](https://www.qt.io/download/).
Uncheck everything except Qt Creator during the installation process.

1. Make sure you installed everything through Homebrew mentioned above
2. Do a proper ./configure --enable-debug
3. In Qt Creator do "New Project" -> Import Project -> Import Existing Project
4. Enter "soferox-qt" as project name, enter src/qt as location
5. Leave the file selection as it is
6. Confirm the "summary page"
7. In the "Projects" tab select "Manage Kits..."
8. Select the default "Desktop" kit and select "Clang (x86 64bit in /usr/bin)" as compiler
9. Select LLDB as debugger (you might need to set the path to your installation)
10. Start debugging with Qt Creator


* Tested on OS X 10.8 through 10.13 on 64-bit Intel processors only.

* Building with downloaded Qt binaries is not officially supported. See the notes in [#7714](https://github.com/bitcoin/bitcoin/issues/7714)
