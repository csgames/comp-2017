#!/bin/sh
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
  alias dosbox='/Applications/DOSBox.app/Contents/MacOS/DOSBox'
fi

if [ -z "$1" ]; then
  echo 'Usage: join.sh GAME'
  exit 1
fi

matchmaker="gaming.csgames:8080"
ip=$(curl "$matchmaker/join?g=$1")

ping -c 1 "$ip" &> /dev/null
if [[ $? -ne 0 ]]; then
  echo "$1 is not reachable."
  exit 1
fi

tmpfile=$(mktemp /tmp/dosbox-wc2.XXXXXX)
cp ./dosbox-wc2-join.conf $tmpfile
sed -ie "s/%PORT%/1337/g" $tmpfile
sed -ie "s/%ADDRESS%/$ip/g" $tmpfile
dosbox -conf $tmpfile
