/*
   Network Next. You control the network.
   Copyright © 2017 - 2019 Network Next, Inc. All rights reserved.
*/

package main

import (
	"encoding/base64"
	"fmt"
)

func main() {

	customerId := GenerateCustomerId()

	publicKey, privateKey := KeygenAsymmetric()

	customerPublicKey := make([]byte, 0)
	customerPublicKey = append(customerPublicKey, customerId...)
	customerPublicKey = append(customerPublicKey, publicKey...)

	customerPrivateKey := make([]byte, 0)
	customerPrivateKey = append(customerPrivateKey, customerId...)
	customerPrivateKey = append(customerPrivateKey, privateKey...)

	fmt.Printf("\nWelcome to Network Next!\n\n")
	fmt.Printf("This is your public key:\n\n    %s\n\n", base64.StdEncoding.EncodeToString(customerPublicKey[:]))
	fmt.Printf("This is your private key:\n\n    %s\n\n", base64.StdEncoding.EncodeToString(customerPrivateKey[:]))
	fmt.Printf("IMPORTANT: Save your private key in a secure place and don't share it with anybody, not even us!\n\n")
}
