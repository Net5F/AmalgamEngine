This directory contains a systemd service (amalgam-server.service), for use with the Amalgam Engine Server application.

Once started, this service will make sure that the Server is always running. If the Server crashes or exits for any reason, it will be re-launched.

###################
## Installation
###################
Make sure that your Server application can be found at /root/Server/Server (if different, change the ExecStart field in amalgam-server.service).

Then, copy the service into the systemd directory:
    cp amalgam-server.service /etc/systemd/system/

###################
## Usage
###################
To start the service:
    systemctl start amalgam-server
    
To check the service's status:
    systemctl status amalgam-server
    
To see the application's output:
    journalctl -u amalgam-server
    
To stop the service:
    systemctl stop amalgam-server
