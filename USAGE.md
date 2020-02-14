<img src="https://static.wixstatic.com/media/799fd4_0512b6edaeea4017a35613b4c0e9fc0b~mv2.jpg/v1/fill/w_1200,h_140,al_c,q_80,usm_0.66_1.00_0.01/networknext_logo_colour_black_RGB_tightc.jpg" alt="Network Next" width="600"/>

<br>

# How to use Network Next

## 1. Direct mode

First, [build](https://github.com/networknext/sdk/blob/master/BUILDING.md) the Network Next SDK and run the client and server examples.

You should see something like:
```
Welcome to Network Next!

0.009219: info: server sdk version is 3.1.0
0.009238: info: server started on 127.0.0.1:32202
0.009250: info: server resolving hostname v3.networknext.com
0.110721: info: server resolved hostname to 146.148.45.204:40000
```

and:
```
Welcome to Network Next!

0.010632: info: client sdk version is 3.1.0
0.021332: info: client opened session to 127.0.0.1:32202
```

At this point the client and server are exchanging packets, but these packets are not accelerated.

In this non-accelerated mode, Network Next simply prefixes your existing packets with a zero byte:
```
[0](regular UDP payload)
```

We call this 'direct mode' and it's what Network Next does by default.

## 2. Create an account on our web portal

To accelerate your application you need access to our web portal:

https://app.v3.networknext.com

Inside the portal, you can see a map of all players using Network Next, drill into sessions for each live player, and configure your usage of Network Next.

Email sales@networknext.com to get access to the portal.

## 3. Generate a private/public keypair

Once you have portal access, the next step is to generate a private/public keypair.

Run the keygen tool under the 'keygen' directory in the SDK:

You should see something like:
```
Welcome to Network Next!

This is your public key:

    PuOtENNoEI7jewa7Kqvl3SHmRKnn6PzAquBhWO1qTRN0H8eJUzUXuA==

This is your private key:

    PuOtENNoEI77YEjPhWbcEGsG5L5dVE7if1KxwRpwvYbBSy5uL1OWJeN7Brsqq+XdIeZEqefo/MCq4GFY7WpNE3Qfx4lTNRe4

IMPORTANT: Save your private key in a secure place and don't share it with anybody, not even us!
```

Paste the base64 public key into the web portal "Game Configuration" page and click save.

Next, copy the private key base64 into server.cpp:
```
static const char * customer_private_key = "PuOtENNoEI77YEjPhWbcEGsG5L5dVE7if1KxwRpwvYbBSy5uL1OWJeN7Brsqq+XdIeZEqefo/MCq4GFY7WpNE3Qfx4lTNRe4";
```

and the public key into client.cpp:
```
static const char * customer_public_key = "PuOtENNoEI7jewa7Kqvl3SHmRKnn6PzAquBhWO1qTRN0H8eJUzUXuA==";
```

## 4. Client and server running in 'upgraded' mode

Next, rebuild and run the client and server.

You should now see something like:
```
Welcome to Network Next!

0.000179: info: server datacenter is 'local'
0.000494: info: server started on 127.0.0.1:32202
0.000521: info: server resolving hostname udp-v3
0.120935: info: server resolved hostname to 10.99.2.5:40001
2.058923: info: server upgraded client 192.168.65.3:44174 to session 7f127fe26281d1c4
```
and:
```
Welcome to Network Next!

0.015976: info: client opened session to 127.0.0.1:32202
0.064203: info: client upgraded to session 7f127fe26281d1c4
```

Your client has been 'upgraded' for possible acceleration by Network Next.
