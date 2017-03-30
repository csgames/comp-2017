#!/bin/sh
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
  alias dosbox='/Applications/DOSBox.app/Contents/MacOS/DOSBox'
fi

if [ -z "$1" ]; then
  echo 'Usage: host.sh GAME'
  exit 1
fi

ip=$(ifconfig | perl -nle 's/inet (\S+)/print $1/e' | grep -v 127.0.0.1 | head -n 1)
matchmaker="gaming.csgames:8080"
curl "$matchmaker/host?g=$1&ip=$ip"

tmpfile=$(mktemp /tmp/dosbox-wc2.XXXXXX)
cp ./dosbox-wc2-host.conf $tmpfile
sed -ie "s/%PORT%/1337/g" $tmpfile
dosbox -conf $tmpfile
