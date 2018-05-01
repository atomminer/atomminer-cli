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

This app was created and tested on Ubuntu 17.10 but should compile with no problem on other distros. Windows build will need WinUSB and openSSL libraries installed...maybe more.

Dependencies 
-----------------------------
+ libcurl
+ openssl
+ libusb
+ ncusrses
+ Qt4
+ libboost [tested versions: 1.53 - 1.63]

sudo apt-get install curl libcurl4-openssl-dev libssl-dev libusb-dev libncurses-dev qt4-qmake libqt4-dev libboost-all-dev
