#!/bin/sh
plugctl -p 0 -n 0 iPCR[0].channel=0 
plugctl -p 0 -n 0 oMPR.bcast_channel=0 
plugctl -p 0 -n 0 oPCR[0].channel=0 
plugctl -p 0 -n 0 iPCR[0].n_p2p_connections=0 
plugctl -p 0 -n 0 oPCR[0].n_p2p_connections=0
plugctl -p 0 -n 0 oPCR[0].bcast_connection=1
plugctl -p 0 -n 0 iPCR[0].bcast_connection=1

plugctl -p 1 -n 0 iPCR[0].channel=1 
plugctl -p 1 -n 0 oMPR.bcast_channel=1 
plugctl -p 1 -n 0 oPCR[0].channel=1 
plugctl -p 1 -n 0 iPCR[0].n_p2p_connections=0
plugctl -p 1 -n 0 oPCR[0].n_p2p_connections=0 
plugctl -p 1 -n 0 oPCR[0].bcast_connection=1
plugctl -p 1 -n 0 iPCR[0].bcast_connection=1
