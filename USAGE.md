# Network Next Usage

## First steps

First, build the Network Next SDK and run the client and server examples.

You should see:
```
```

and:
```
```

The client and server are exchanging packets between each other, but these packets are not accelerated yet.

In this non-accelerated 'direct' mode, Network Next simply prefixes your existing packets with a zero byte:
```
[0](regular UDP payload)
```

## Create an account on our web portal

To move past direct mode and get acceleration for your application, the next step is to sign up for our web portal.

Please reach out to sales@networknext.com to get access to https://app.v3.networknext.com and begin your free trial.

Inside the portal, you can see a map of all players using Network Next, and drill into sessions for each live player, and you can configure own usage of Network Next.

## Generate a private/public keypair

Now that you are setup with the portal the next step is to generate a private/public keypair that is unique to you. 

We need this to ensure that nobody can request routes that bill to your account, except you.

Run the keygen tool which you can find under the 'keygen, directory in the SDK:
```
macbook:keygen glenn$ ./keygen_mac

Welcome to Network Next!

This is your public key:

    PuOtENNoEI7jewa7Kqvl3SHmRKnn6PzAquBhWO1qTRN0H8eJUzUXuA==

This is your private key:

    PuOtENNoEI77YEjPhWbcEGsG5L5dVE7if1KxwRpwvYbBSy5uL1OWJeN7Brsqq+XdIeZEqefo/MCq4GFY7WpNE3Qfx4lTNRe4

IMPORTANT: Save your private key in a secure place and don't share it with anybody, not even us!
```

Then copy the base64 *public key* _(not the private key!)_ and paste it into the "Game Configuration", "Customer Public Key" entry under "Settings" in the web portal.

Next, copy the same private key into server.cpp as a string, for example:
```
```

And the public key into client.cpp:
```
```

You can also elect to set the private key on the server via an environment variable, which we strongly recommend past this initial integration step.

You should now see 

## Next steps

At this point, please contact us on the Discord support channel, which you can access via "Support" in the top menu bar, and we will guide the next steps in your integration.

cheers

- Glenn Fiedler, CEO, Network Next.
