# Network Next FAQ

## What is Network Next?

Network Next is a marketplace for premium network transit. 

## How does the marketplace work?

We sell 10 second slices of access to private networks via a two-sided marketplace, in other words, we're basically _Google Adwords for Packets_.

## Why use Network Next?

Network Next improves the quality of network transit from players around the world to your game servers. Players have lower latency, fewer hitches, and less packet loss when they play over Network Next.

## How does Network Next work?

Network Next SDK steers UDP game traffic across the best path from each player to the game server via centrally planned routing system driven by a marketplace bidding system run once every 10 seconds per-player.

## Does Network Next violate Network Neutrality?

No. Routing via bidding process ensures which ensures that we remain _supplier neutral_ and always steer traffic across the route that has the best performance at the best price, without favoring any one supplier or buyer over another.

## What sort of improvements can Network Next provide?

It depends on the time of day, ISP and location of each player, and on the data center where game server is located... but overall across all players, we are typically able to provide significant performance improvements for at least 10% of your playerbase (>10ms latency reduction, or 1% packet loss reduction or greater).

## What's the biggest improvement Network Next has provided?

We've seen players with over 1000ms round trip time reduced to 50ms and players with almost 100% packet loss reduced down to zero. This is not the common case, but these players certainly appreciate it :)

## How much does Network Next cost?

Bandwidth for players where we provide significant improvement costs around 10c per-gigabyte on average right now, but Network Next targets only the bottom 10% or so of your player-base, giving an amortized cost of approximately 1c per-GB.

## Do players have to pay to use Network Next?

No. Players do not have to pay. Our customer is the game developer, not the player.

## How does billing work?

Network Next sells 10 second slices of access across multiple networks according to the amount of bandwidth reserved for each player.

We bill on the envelope of traffic games uses, rather than the actual usage, since the private networks we work with reserve this much capacity for each player.

Each month we sum up the total of 10 second bandwidth slices served by all suppliers for a game (often multiple suppliers are involved in each route), and that is the amount billed to the game developer.

## How can I get access to the Network Next portal

Please contact us at sales@networknext.com and we'll onboard you for a free trial.

## What games use Network Next?

Network Next is in production with [Rocket League](https://rocketleague.com) for several months now, and we accelerate millions of players every day. We're also live with several other games that aren't announced yet!

## What suppliers work with Network Next?

We work with top edge computing and infrastructure suppliers such as:

* [Limelight](https://limelight.com)
* [Stackpath](https://stackpath.com)
* [Packet](https://packet.com)
* [Zenlayer](https://zenlayer.com)
* [Multiplay](https://multiplay.com)
* [I3D](https://i3d.com)
* [Vultr.com](https://vultr.com)
* [GameServers.com](https://gameservers.com)
* [AWS](https://aws.amazon.com)
* [Google](https://cloud.google.com/)
* [Azure](https://azure.microsoft.com)
* [Oracle Cloud](https://cloud.oracle.com/)
* [IBM Cloud](https://ibm.com/cloud)

_And many more we can't talk about yet!_

## Where can I host game servers with Network Next?

We support game servers hosted by:

* [Multiplay](https://multiplay.com)
* [I3D](https://i3d.com)
* [Vultr.com](https://vultr.com)
* [GameServers.com](https://gameservers.com)
* [AWS](https://aws.amazon.com)
* [Google](https://cloud.google.com/)
* [Azure](https://azure.microsoft.com)

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

## What languages are supported?

The Network Next SDK is written in low-level C style C++ so it's easy to call from other additional languages. Get involved and contribute a binding for your preferred language to our open source SDK! https://github.com/networknext/sdk

## What locations are supported by Network Next?

Network Next is deployed worldwide. Please contact sales@networknext.com to discuss a trial so we can ensure that we optimally target the location of your game's userbase.

## Do you have an Unreal Engine 4 Plugin?

Yes. Please contact sales@networknext.com 

## Do you have a Unity plugin?

Not yet, but we're actively working on it. Please contact sales@networknext.com and let us know you're interested.

## Can I use Network Next for player hosted servers?

No. We only support acceleration to servers hosted in datacenters with a known public IP.

## Can I use Network Next for my peer-to-peer game?

No. We only support acceleration of client/server traffic between clients and dedicated servers hosted in datacenters.

## Can I use Network Next to accelerate my TCP game?

Not yet. In the rare cases where TCP makes sense, we may be able to provide some benefit via a TCP shim that reduces the impact of packet loss induced head of line blocking across your client/server connection, but this has not been implemented yet. 

Please talk to us so we can understand your use case sales@networknext.com

## Can I use Network Next to accelerate my WebRTC game?

Not yet. We are actively investigating this as an option. Please talk to us so we can understand your use case sales@networknext.com

## Will players be disconnected from my game if Network Next goes down?

No. If our SDK cannot talk to our backend for any reason, players automatically fall back to the public internet and are _free_.

## Are you hiring?

Always. https://www.networknext.com/careers
