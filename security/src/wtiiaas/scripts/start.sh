#!/bin/bash

echo "root:$ROOT_PASSWORD" |chpasswd

sh /root/setupmysql.sh

supervisord -n -c /etc/supervisor/supervisord.conf


