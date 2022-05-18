#!/bin/bash
shmid=`ipcs -m | sed "/^.*0x00002326.*$/p" -n | awk '{print $2}' `
ipcrm -m $shmid
