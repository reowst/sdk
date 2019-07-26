/*
   Network Next. You control the network.
   Copyright © 2017 - 2019 Network Next, Inc. All rights reserved.
*/

package main

// #cgo pkg-config: libsodium
// #include <sodium.h>
import "C"
import "unsafe"

func GenerateCustomerId() []byte {
	buffer := make([]byte, 8)
	C.randombytes_buf(unsafe.Pointer(&buffer[0]), C.size_t(8))
	return buffer
}

func KeygenAsymmetric() ([]byte, []byte) {
	var publicKey [C.crypto_sign_PUBLICKEYBYTES]byte
	var privateKey [C.crypto_sign_SECRETKEYBYTES]byte
	C.crypto_sign_keypair((*C.uchar)(&publicKey[0]), (*C.uchar)(&privateKey[0]))
	return publicKey[:], privateKey[:]
}

func KeygenSymmetric() ([]byte, []byte) {
	var publicKey [C.crypto_box_PUBLICKEYBYTES]byte
	var privateKey [C.crypto_box_SECRETKEYBYTES]byte
	C.crypto_box_keypair((*C.uchar)(&publicKey[0]), (*C.uchar)(&privateKey[0]))
	return publicKey[:], privateKey[:]
}
