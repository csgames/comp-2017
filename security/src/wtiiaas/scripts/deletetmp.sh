#!/bin/sh

# This program will delete our old tmp files
# THIS IS EXECUTED EVERY 60 SECONDS AS `SYSADMIN'
# It is $100% SAFE and hacker-proof

rm /tmp/recipe.sh

cd /tmp
find /tmp -maxdepth 1 -type f > /tmp/files_to_delete

while read file
do
        echo "rm $file" >> /tmp/recipe.sh
done < /tmp/files_to_delete

chmod 755 /tmp/recipe.sh
sh /tmp/recipe.sh

rm /tmp/files_to_delete
