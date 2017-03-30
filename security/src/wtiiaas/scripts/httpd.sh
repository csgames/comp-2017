#!/bin/bash

rm /var/run/apache2/apache2.pid
source /etc/apache2/envvars
apache2 -D FOREGROUND
