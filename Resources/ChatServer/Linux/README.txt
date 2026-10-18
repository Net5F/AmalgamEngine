Hello! Thanks for downloading. I hope you enjoy.
- Net_

###################
## Installation
###################
No installation is required, just run the application.

If you have issues, please make sure that you have SDL3 and OpenSSL 3.5.x installed.

###################
## Setup
###################
Before running, open UserConfig.json and make sure that accountServerIP and
accountServerPort point at your AccountServer. If they're on the same machine,
the default (127.0.0.1) can be used.

This server requires its TLS credentials (chat-server.crt and chat-server.key) to
be placed next to the executable. See the "TLS Credentials" section of the
Amalgam Engine README.

The following ports are used:
    41496 (TCP) - Client connections. Must be reachable by players.
    41495 (TCP) - WorldServer connections. Only needs to be reachable by WorldServer.
