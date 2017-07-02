#!/bin/bash

cp plane-zero-server.service /etc/systemd/system/
cp plane-zero-server.sh /usr/local/bin/
chmod a+x /usr/local/bin/plane-zero-server.sh

sudo systemctl enable plane-zero-server.service