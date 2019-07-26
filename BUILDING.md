# How to build the Network Next SDK

## All platforms

* [Install git](https://git-scm.com/) _(Not necessary on Mac)_

* Create an account on GitHub if you don't have one already: https://github.com

* Clone the SDK git repository: 
```
git clone git@github.com:networknext/sdk.git
```

## If you are developing on Mac

* Install brew:
```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

* Install dependencies via brew (of these only libsodium is a runtime dependency):
```
brew install libsodium pkg-config golang curl
```

* Install premake5
```
blahblahblah
```

* Generate the makefiles with premake:
```
premake5 gmake
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

* Go to USAGE.md for next steps!

## If you are developing on Windows

* [Install VS Community 2019](https://visualstudio.microsoft.com/vs/community/) if you don't already have a visual studio environment.

* ...

* ...

## If you are developing on Linux

* ...
