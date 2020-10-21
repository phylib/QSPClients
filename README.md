# NDN Server-Sync Clients

This repository contains an QSP and SVS client for
inter-server game state synchronization and is published
as software artifacts for the Paper _Philipp Moll, Selina Isak, Hermann Hellwagner, Jeff Burke. A Quadtree-based Synchronization Protocol for Inter-Server Game State Synchronization. Submitted to Computer Networks, Elsevier._

## Installation

In order to compile and run the clients, the NDN libraries must be installed. To do so, please use the `install.sh`
[script from MiniNDN's GitHub repository](https://github.com/named-data/mini-ndn/blob/master/install.sh).

Furthermore, the clients require the Protocol Buffers Library and Spdlog installed. To install these dependencies,
follow the following instructions.

```bash
sudo apt-get -y install cmake

mkdir -p dependencies
pushd dependencies

git clone https://github.com/catchorg/Catch2.git
pushd Catch2
git checkout v2.9.2
cmake -Bbuild -H. -DBUILD_TESTING=OFF
sudo cmake --build build/ --target install
popd

# Install spdlog
git clone https://github.com/gabime/spdlog.git
pushd spdlog
mkdir build
pushd build
cmake ..
make -j
sudo make install
popd
popd

sudo ldconfig

popd
```

With the dependencies installed, the clients can be installed using cmake:

```bash
cmake .
make
```

In order to test if everything was compiled correctly, execute the tests using:

```bash
./SyncTreeTests -d yes
```

If all tests succeed, the following three executables represent the required clients:

- **StateVectorSyncClient**: Game State Synchronization based on StateVectorSync
- **EvaluationSyncClient**: QSP client in server-based mode
- **P2PModeSyncClient**: QSP client in zone-based mode
