#!/bin/bash
rm server.log
./server  -ORBDebugLevel 10 -DCPSDebugLevel 10 -DCPSTransportDebugLevel 10 -ORBLogFile server.log -DCPSConfigFile rtps_udp.ini
