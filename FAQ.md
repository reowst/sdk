# Network Next FAQ

## What is Network Next?

Network Next is a marketplace for premium network transit. 

We sell 10 second slices of access to private networks via a two-sided marketplace, we're basically _Google Adwords for Packets_.

The results is improved network transit from players around the world to your game servers. Players experience lower latency, fewer hitches, and less packet loss when they play over Network Next.

Players that we can't improve (yet), or are _good enough_ according to your requirements, take the public internet and are _free_.

## How does Network Next work?

Network Next SDK steers UDP game traffic across the best path from each player to the game server using proprietary technology. 

We do this via our centrally planned routing system (control plane), driven by a marketplace bidding system run once every 10 seconds per-player.

This is our key innovation. Routing via bidding process ensures which ensures that we remain _supplier neutral_ and always steer traffic across the route that has the best performance at the best price.

## What sort of improvements can Network Next provide?

It depends on the time of day, ISP and location of each player, and on the data center where the traffic is accelerated to... but overall across all players, we are typically able to provide significant performance improvements for at least 10% of your playerbase (>10ms latency reduction, or 1% packet loss reduction or greater).

## What's the biggest improvement Network Next has provided?

We've seen players with 400ms round trip time reduced to 200ms or less, and players with almost 100% packet loss reduced down to zero. This is not the common case, but these players certainly appreciate it :)

## How much does Network Next cost?

Network Next targets only the bottom 10% of your playerbase having a really bad experience, so it is very cost effective. 

You don't pay _anything_ for the 90% of players who already have a good connection to your server.

Bandwidth for players where we provide significant improvement costs 10c per-gigabyte on average right now, but we expect this to reduce over time.

## Do players have to pay to use Network Next?

No. Players do not have to pay. Our customer is the game developer, not the player.

The game developer is happy to sponsor players so they have a better experience playing their game.

## How does billing work?

Network Next sells 10 second slices of access across multiple networks according to the amount of bandwidth reserved up and down for the game. 

Therefore, we bill on the envelope of traffic your games uses, rather than the usage, since the private networks we work with operate by reserving this much capacity for each player.

Each month we sum up the total of 10 second bandwidth slices served by all suppliers for a game (often multiple suppliers are involved in each route), and that is the total amount we bill to to the game developer.

Game developers don't pay for any routes that don't meet your quality expectations, or for players for whom we aren't able to provide significant benefit (according to their own definition of what significant benefit may be).

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

_And many more we can't talk about yet!_

## Where can I host game servers with Network Next?

We support game servers hosted by:

* Multiplay
* I3D
* Vultr.com
* AWS
* Google
* Azure

If you'd like to evaluate Network Next but you host game servers somewhere else, please let us know at sales@networknext.com

## What platforms do you support?

* PC
* Mac
* Linux
* iOS
* PS4
* Xbox One
* Nintendo Switch

Access to console platform support requires proof that you are a registered developer for that platform. Please contact sales@networknext.com for more information.

## What if my platform isn't listed above

Please contact sales@networknext.com so we can understand your use case. Thanks!

## What locations are supported by Network Next?

Network Next is now deployed worldwide. Please contact sales@networknext.com to discuss a trial so we can ensure that we optimally target the location of your game's userbase.

## Do you have an Unreal Engine 4 Plugin?

Yes. Please contact sales@networknext.com 

## Do you have a Unity plugin?

Not yet, but we're actively working on it. Please contact sales@networknext.com and let us know you're interested.

## Can I use Network Next for player hosted servers?

No. We only support acceleration to servers hosted in datacenters with a known public IP.

## Can I use Network Next for my peer-to-peer game?

No. We only support acceleration of client/server traffic between clients and dedicated servers (hosted in datacenters) right now.

## Can I use Network Next to accelerate my TCP game?

Not yet. In the rare cases where TCP makes sense, we may be able to provide some benefit via a TCP shim that reduces the impact of packet loss induced head of line blocking across your client to server connection, but this has not been implemented yet. 

Please talk to us so we can understand your use case sales@networknext.com

## Can I use Network Next to accelerate my WebRTC game?

Not yet. We are actively investigating this as an option. Please talk to us so we can understand your use case sales@networknext.com

## What about Network Neutrality?

We care deeply about Network Neutrality and have designed our marketplace to be as neutral as possible.

In addition, suppliers cannot identify buyers and therefore act in an anti-competitive manner against buyers.

We think Network Next is a way to boost network p

## Do players have to pay to use Netowrk Next?
