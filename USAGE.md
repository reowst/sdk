# Network Next Usage

## 1. Client and server in direct mode

First, build the Network Next SDK and run the client and server examples.

You should see something like:
```
Welcome to Network Next!

0.009219: info: server sdk version is 3.1.0
0.009238: info: server started on 127.0.0.1:32202
0.009250: info: server resolving hostname v3.networknext.com
0.110721: info: server resolved hostname to 146.148.45.204:40000
```

and on the client:
```
Welcome to Network Next!

0.010632: info: client sdk version is 3.1.0
0.021332: info: client opened session to 127.0.0.1:32202
```

At this point the client and server are exchanging packets between each other, but these packets are not accelerated yet.

In this non-accelerated 'direct' mode, Network Next simply prefixes your existing packets with a zero byte:
```
[0](regular UDP payload)
```

## 2. Create an account on our web portal

To move past direct mode and get acceleration for your application, you need access to our web portal: https://app.v3.networknext.com

Please reach out to sales@networknext.com to get access and begin your free trial.

Inside the portal, you'll see a map of all players using Network Next, be able to drill into sessions for each live player, and configure your own usage of Network Next.

## 3. Generate a private/public keypair

The next step is to generate a private/public keypair.

We need this to ensure that nobody can request routes that bill to your account, except you.

Run the keygen tool under the 'keygen', directory in the SDK:

You should see something like:
```
macbook:keygen glenn$ ./keygen_mac

Welcome to Network Next!

This is your public key:

    PuOtENNoEI7jewa7Kqvl3SHmRKnn6PzAquBhWO1qTRN0H8eJUzUXuA==

This is your private key:

    PuOtENNoEI77YEjPhWbcEGsG5L5dVE7if1KxwRpwvYbBSy5uL1OWJeN7Brsqq+XdIeZEqefo/MCq4GFY7WpNE3Qfx4lTNRe4

IMPORTANT: Save your private key in a secure place and don't share it with anybody, not even us!
```

Now copy the base64 *public key* and paste it into the "Customer Public Key" entry under "Settings" -> "Game Configuration" in the portal and click save.

Next, copy the same private key into server.cpp as a string, for example:
```
static const char * customer_private_key = "PuOtENNoEI77YEjPhWbcEGsG5L5dVE7if1KxwRpwvYbBSy5uL1OWJeN7Brsqq+XdIeZEqefo/MCq4GFY7WpNE3Qfx4lTNRe4";
```

And the public key into client.cpp:
```
static const char * customer_public_key = "PuOtENNoEI7jewa7Kqvl3SHmRKnn6PzAquBhWO1qTRN0H8eJUzUXuA==";
```

You can also set the server private via an environment variable: NEXT_CUSTOMER_PRIVATE_KEY, which we strongly recommend past this initial step.

## 4. Client and server running in 'upgraded' mode

Rebuild and run the client and server as per-the steps described for your platform.

You should now see:
```
```
on the server, and:

on the client.

Seeing this output means that the customer private/public keypair working, and the client player are being correctly "upgraded" by the server.

Upgraded means that the client has performed key exchange with the server, and Network Next is ready to accelerate that player if we can provide significant benefit over the public internet.

## Next steps

Congratulations! You have successfully setup your account and private/public keypair. 

You are ready to begin integration with your game.

At this point, please contact us on the Discord support channel which you can access via "Support" in the top menu bar, and we will guide you through the next steps in your integration.

cheers

- Glenn Fiedler, CEO, Network Next
