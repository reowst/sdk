# Network Next Usage

## Client and server connection

First, build the Network Next SDK and run the client and server examples.

At this point, the client and server are exchanging packets between each other, but these packets are not accelerated yet.

In this non-acceleratex 'direct' mode, Network Next simply exchanges your existing packets prefixed with a zero byte:
```
[0](regular UDP payload)
```

## Create an account on our web portal

Our web portal is https://app.v3.networknext.com. Please reach out to sales@networknext.com to begin your free trial.

You'll be given access to our portal, from which you can see a map of all players using Network Next, and sessions for each live player, as well as configure your own usage of Network Next.

## Generate a private/public keypair

The next step is to generate a private/public keypair that is unique to you. 

We need this to ensure that nobody can request routes that bill to your account, except you.

Run the keygen tool which you can find under the 'keygen, directory in the SDK:

Here is a sample output:
```
macbook:keygen glenn$ ./keygen_mac

Welcome to Network Next!

This is your public key:

    PuOtENNoEI7jewa7Kqvl3SHmRKnn6PzAquBhWO1qTRN0H8eJUzUXuA==

This is your private key:

    PuOtENNoEI77YEjPhWbcEGsG5L5dVE7if1KxwRpwvYbBSy5uL1OWJeN7Brsqq+XdIeZEqefo/MCq4GFY7WpNE3Qfx4lTNRe4

IMPORTANT: Save your private key in a secure place and don't share it with anybody, not even us!
```

Please copy the base64 public key and paste it into the "Game Configuration", "Customer Public Key" entry under "Settings" in the web portal.

Next, copy the private key into server.cpp as a string, for example:
```
```

And the public key into client.cpp as a string:
```
```

You can also elect to set the private key on the server via an environment variable, and since anybody who knows this private key can request routes across Network Next that bill to you, we strongly recommend that you migrate to this approach and don't check your private key into.

At this point, please contact us on the Discord support channel, and we can guide the next steps in your integration.

cheers

- Glenn Fiedler, CEO, Network Next.
