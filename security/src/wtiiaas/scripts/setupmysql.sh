/usr/bin/mysqld_safe &
sleep 10
mysql < /root/mysql/permissions.sql
mysql < /root/mysql/data.sql
mysqladmin -u root password "ZVeNAg5beRXKJY8H8uN7BgqQabyZjE"
killall mysqld
sleep 5
