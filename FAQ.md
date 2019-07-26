# Network Next SDK

## What is Network Next?

Network Next is a marketplace for premium network transit. 

We sell 10 seconds of slices of access to private networks. Typically, these private networks have lower latency, less congestion and less frequent packet loss than the public internet.

The result is improved network transit from players around the world to your game servers. Players experiencing lower latency, less hitching, and less packet loss while they play.

Players that we can't improve (yet), or are good enough according to your requirements take the public internet and are _free_.

## How does Network Next work?

Network Next SDK steers UDP game traffic across the best path from each player to the game server.

We do this by steering traffic across overlay routers run by our suppliers via centrally planned routing system, driven by a marketplace bidding system run once every 10 seconds per-player.

This way we remain neutral and always steer traffic across the route that has the best performance at the best price.

## What sort of improvements can Network Next provide?

It depends on the time of day, ISP and location of each player, but overall across all players, we provide significant performance improvements for 10-20% of your playerbase (>10ms latency reduction, or 1% packet loss reduction or greater).

## What's the biggest improvement Network Next has provided?

We've seen players have 400ms latency reduced to 200ms or less, and players with almost 100% packet loss have that reduced down to zero. This is not the common case, but these players sure appreciate the boost they get :)

## How much does Network Next cost?

On average, bandwidth costs 10c per-gigabyte right now, but we expect this to reduce over time.

Importantly, Network Next by default targets at the bottom 10% of your playerbase having a bad experience, so it is very cost effective. So you don't pay anything for the 90% of players who already have a good connection to your server.

## How does billing work?

We sell 10 second slices of access across multiple networks according to the amount of bandwidth reserved. We need to bill on the envelope of traffic your games uses, rather than the actual usage, since the private networks we work with operate by reserving this much capacity for each player.

Each month we sum up the total of 10 second bandwidth slices served by all suppliers (often multiple suppliers are involved in each optimal route), and that is the total amount we charge to customers.

## How can I get access to the Network Next portal

Please contact us at sales@networknext.com and we can onboard you for a free trial.

## What suppliers does Network Next work with?

We work with top infrastructure suppliers such as:

* Limelight
* Stackpath
* Packet
* Zenlayer
* Multiplay
* I3D
* Vultr.com
* AWS
* Google
* Azure
* Oracle Cloud
* IBM Cloud

And many more we can't talk about yet!

## Where can I host game servers with Network Next?

We support game servers hosted by:

* Multiplay
* I3D
* Vultr.com
* AWS
* Google
* Azure

If you'd like to tryout Network Next but you host your game servers somewhere else, please let us know. 

## What platforms do you support?

* PC
* Mac
* Linux
* iOS
* PS4
* Xbox One
* Nintendo Switch

Access to console platform support requires proof that you are a registered developer for that platform. Please contact sales@networknext.com for more information.

## Do you have an Unreal Engine 4 Plugin?

Yes. Please contact sales@networknext.com 

## Do you have a Unity plugin?

Not yet, but we're actively working on it right now! Coming very soon.

## Can I use Network Next for player hosted servers?

No. We only support acceleration to servers hosted in datacenters with a known public IP.

## Can I use Network Next for my peer-to-peer game?

No. We only support acceleration of client/server traffic between clients and dedicated servers (hosted in datacenters) right now.

## Can I use Network Next to accelerate my TCP game?

In the rare cases where TCP does make sense for a game, we may be able to provide some benefit via a TCP to UDP shim, but this has not been implemented yet.

If this is important to you, please talk to us via sales@networknext.com so we can understand your use case.

## Can I use Network Next to accelerate my WebRTC game?

Not yet. Please talk to us. We are actively investigating this as an option. sales@networknext.com
