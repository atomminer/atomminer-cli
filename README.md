AtomMiner CLI miner
==============

Current state: work-in-progress

Technical Information:
-----------------------------

Supported hardware: AtomMiner AM01 miner
Supported pools: most of the existing pools. 
Preferred pools: pools with atom rpc extension enabled [to be released].

Build
-----------------------------

This app was created and tested on Ubuntu 17.10 but should compile with no problem on other distros. Windows build will need WinUSB and openSSL libraries installed...maybe more...to be tested.

Dependencies 
-----------------------------
+ libcurl
+ openssl
+ libusb
+ ncusrses
+ Qt5.10

```sh
$ sudo apt-get install build-essential curl libcurl4-openssl-dev libssl-dev libusb-dev libncurses-dev 
```

Install Qt 5.10
-----------------------------
```sh
$ wget http://download.qt.io/official_releases/qt/5.10/5.10.0/qt-opensource-linux-x64-5.10.0.run
$ chmod +x qt-opensource-linux-x64-5.10.0.run
$ ./qt-opensource-linux-x64-5.10.0.run
```

as an alternative, there's unofficial ppa for Qt5.10:
https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.10.0-xenial


Documentation
==============
[API](docs/api.md)

the rest is to be written yet :)
