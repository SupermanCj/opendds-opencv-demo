#!/bin/bash

rm client.log
./client -ORBDebugLevel 10 -DCPSDebugLevel 10 -DCPSTransportDebugLevel 10 -ORBLogFile client.log -DCPSConfigFile rtps_udp.ini
