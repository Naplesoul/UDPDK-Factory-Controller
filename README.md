# UDPDK-Factory-Controller

### build dependencies

```bash
sudo apt-get install git libtool automake build-essential pkg-config

sudo apt-get install libssl-dev doxygen asciidoc valgrind libcunit1 libcunit1-doc libcunit1-dev libconfig-dev
```

#### build OpenDataPlane

```bash
git clone https://git.linaro.org/lng/odp.git
cd odp
git checkout v1.19.0.2
./bootstrap
./configure
make
sudo make install
```

#### build OpenFastPath

```bash
git clone https://github.com/OpenFastPath/ofp
cd ofp
./bootstrap
./configure --with-odp
make
make install
```

