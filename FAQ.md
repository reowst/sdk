# Network Next FAQ

## What is Network Next?

Network Next is a marketplace for premium network transit. 

We sell 10 second slices of access to private networks, we're basically _Google Adwords for Packets_.

This results in improved network transit from players around the world to your game servers. Players experience lower latency, less hitching, and less packet loss while they play over Network Next.

Players that we can't improve (yet), or are _good enough_ according to your requirements take the public internet and are _free_.

## How does Network Next work?

Network Next SDK steers UDP game traffic across the best path from each player to the game server using proprietary technology. 

We do this via our centrally planned routing system (control plane), driven by a marketplace bidding system run once every 10 seconds per-player, across a series of software defined routers (data plane) across multiple redundant supplier networks.

Routing via bidding process ensures that we remain _neutral_ and always steer traffic across the route that has the best performance at the best price.

## What sort of improvements can Network Next provide?

It depends on the time of day, ISP and location of each player, and on the data center where the traffic is being accelerated to... but overall across all players, we are typicalyl able to provide significant performance improvements for at least 10% of your playerbase (>10ms latency reduction, or 1% packet loss reduction or greater).

## What's the biggest improvement Network Next has provided?

We've seen players with 400ms round trip time reduced to 200ms or less, and players with almost 100% packet loss reduced down to zero. This is not the common case, but these players sure appreciate it :)

## How much does Network Next cost?

Network Next by targets only the bottom 10% of your playerbase having a really bad experience, so it is very cost effective. 

You don't pay anything for the 90% of players who already have a good connection to your server.

Bandwidth for players where we provide significant improvement costs 10c per-gigabyte on average right now, but we expect this to reduce over time.

## How does billing work?

We sell 10 second slices of access across multiple networks according to the amount of bandwidth reserved. We need to bill on the envelope of traffic your games uses, rather than the actual usage, since the private networks we work with operate by reserving this much capacity for each player.

Each month we sum up the total of 10 second bandwidth slices served by all suppliers (often multiple suppliers are involved in each route), and that is the total amount we bill to you.

You don't pay for any routes that don't meet your quality expectations, or players for whom we aren't able to provide significant benefit.

## How can I get access to the Network Next portal

Please contact us at sales@networknext.com and we'll onboard you for a free trial.

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

If you'd like to evaluate Network Next but you host your game servers somewhere else, please let us know at sales@networknext.com

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

Not yet. In the rare cases where TCP makes sense for a game, we may be able to provide some benefit via a TCP to UDP shim, but this has not been implemented yet. Please talk to us so we can understand your use case sales@networknext.com

## Can I use Network Next to accelerate my WebRTC game?

Not yet. We are actively investigating this as an option. Please talk to us so we can understand your use case sales@networknext.com
