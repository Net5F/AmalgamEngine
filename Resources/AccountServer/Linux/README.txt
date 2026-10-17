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
Account data is stored in Accounts.db, which is created next to the
executable the first time the server runs. Back it up, and keep it when updating.

This server requires its TLS credentials (account-server.crt and account-server.key) to
be placed next to the executable. See the "TLS Credentials" section of the
Amalgam Engine README.

The following ports are used:
    41498 (TCP) - Client connections. Must be reachable by players.
    41497 (TCP) - Service connections. Only needs to be reachable by the
                  ChatServer and WorldServer.
