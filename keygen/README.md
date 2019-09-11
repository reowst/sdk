<img src="https://static.wixstatic.com/media/799fd4_0512b6edaeea4017a35613b4c0e9fc0b~mv2.jpg/v1/fill/w_1200,h_140,al_c,q_80,usm_0.66_1.00_0.01/networknext_logo_colour_black_RGB_tightc.jpg" alt="Network Next" width="600"/>

<br>

# Keygen Tool

## Using prebuilt binaries

All prebuilt binaries are for 64-bit operating systems. If you have a different architecture, refer to "Building from source" below.

### Windows

Usage: `keygen.exe`

### macOS

You must have libsodium 1.0.18 installed. If you don't, use `brew install libsodium` to install it. If you can't install this version, see "Building from source" below.

Usage: `./keygen_mac`

### Linux (Ubuntu 18.04.2 LTS)

You must have libsodium 1.0.16-2 installed. If you don't, use `sudo apt install libsodium23` to install it. If you can't install this version, see "Building from source" below.

Usage: `./keygen_linux`

## Building from source

### Windows

1. Install MinGW-w64 and make sure GCC is on your path.
2. `cd src && go build keygen.go keygen_windows.go`

### macOS

1. `brew install libsodium pkg-config`
2. `cd src && go build keygen.go keygen_unix.go`

### Linux

1. `sudo apt install pkg-config libsodium-dev libsodium23`
2. `cd src && go build keygen.go keygen_unix.go`
