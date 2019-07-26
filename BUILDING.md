# How to build the Network Next SDK

## All platforms

* [Download the latest SDK source code](https://github.com/networknext/sdk/archive/master.zip)

_OR..._

* [Install git](https://git-scm.com/)

* Create an account on GitHub if you don't have one already: https://github.com

* Clone the SDK git repository: 
```
git clone git@github.com:networknext/sdk.git
```

## If you are developing on Mac

* Install the XCode command line development tools if you haven't already:
```
xcode-select --install
```

* Install [brew](https://brew.sh):
```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

* Install dependencies via brew:
```
brew install libsodium pkg-config golang wget
```

* Install [premake5](https://premake.github.io/download.html):
```
wget https://github.com/premake/premake-core/releases/download/v5.0.0-alpha14/premake-5.0.0-alpha14-macosx.tar.gz && tar -zxvf premake-*.tar.gz && rm premake-*.tar.gz
```

* Generate the makefiles with premake:
```
./premake5 gmake
```

* Make everything
```
make
```

* Run the server:
```
./bin/server
```
You should see something like:
```
macbook:sdk glenn$ ./bin/server

Welcome to Network Next!

0.009219: info: server sdk version is 3.1.0
0.009238: info: server started on 127.0.0.1:32202
0.009250: info: server resolving hostname v3.networknext.com
0.110721: info: server resolved hostname to 146.148.45.204:40000
```

* Switch to another terminal and run the client:
```
./bin/client
```
You should see something like:
```
macbook:sdk glenn$ ./bin/client

Welcome to Network Next!

0.010632: info: client sdk version is 3.1.0
0.021332: info: client opened session to 127.0.0.1:32202
```

* [Go to USAGE.md for next steps](https://github.com/networknext/sdk/blob/master/USAGE.md)

## If you are developing on Windows

* [Install VS Community 2019](https://visualstudio.microsoft.com/vs/community/) if you don't already have a visual studio environment.

* ...

* ...

## If you are developing on Linux

These instructions are designed for Ubuntu 14.04-18.04.

* Install GCC:

   ```shell
   sudo apt install build-essential
   ```

* Install premake:

   ```shell
   curl -L -O https://github.com/premake/premake-core/releases/download/v5.0.0-alpha14/premake-5.0.0-alpha14-src.zip && \
   unzip premake-5.0.0-alpha14-src.zip && \
   cd premake-5.0.0-alpha14 && \
   cd build/gmake.unix && \
   make && \
   sudo mv ../../bin/release/premake5 /usr/local/bin && \
   cd ../../../ && \
   rm -rf premake-*
   ```

* If you want to build 32-bit:
   ```shell
   sudo dpkg --add-architecture i386
   sudo apt update
   sudo apt install gcc-multilib g++-multilib
   ```

* Choose a build configuration:
   - `debug_static32`
   - `debug_shared32`
   - `debug_static64`
   - `debug_shared64`
   - `release_static32`
   - `release_shared32`
   - `release_static64`
   - `release_shared64`

   Then:

    ```shell
    premake5 gmake
    make config=<config>
    ./bin/server &
    ./bin/client
    ```

* [Go to USAGE.md for next steps](https://github.com/networknext/sdk/blob/master/USAGE.md)
