#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "$ ./run.sh <username> <password>"
    exit
fi

# Constants
DEFAULT="\x1B[0m"
GREEN="\x1B[32m"
RED="\x1B[31m"
YELLOW="\x1B[33m"

DOWNLOAD_FILE="to_download"
DOWNLOAD_CONTENT="DOWNLOAD-ME-PLEASE"
UPLOAD_FILE="to_upload"
UPLOAD_CONTENT="UPLOAD-ME-PLEASE"

SERVER_FOLDER="../server_folder"
SERVER_MD5=$(md5sum $SERVER_FOLDER/server | cut -d ' ' -f 1)
REAL_MD5="d7eed5b88f41673a63cc8bc221798eda"

# Points
PTS=0
PTS_TOTAL=0

PTS_CLOSE=1
PTS_LOGIN=2
PTS_LISTING=2
PTS_DOWNLOAD=3
PTS_UPLOAD=3

PTS_1=$(($PTS_LOGIN+$PTS_LISTING+$PTS_CLOSE));
PTS_2=$(($PTS_DOWNLOAD+$PTS_CLOSE));
PTS_3=$(($PTS_UPLOAD+$PTS_CLOSE));

PTS_FINAL=$(($PTS_1+$PTS_2+$PTS_3));


# Default settings
rm -f $SERVER_FOLDER/srv/*
rm -f ./local/$DOWNLOAD_FILE 2>/dev/null
echo $DOWNLOAD_CONTENT > $SERVER_FOLDER/srv/$DOWNLOAD_FILE
echo $UPLOAD_CONTENT > ./local/$UPLOAD_FILE



# Server Integrity #
#echo -e "$YELLOW\0MD5 du serveur\n$DEFAULT"
#if [ "$SERVER_MD5" != $REAL_MD5 ]; then
#    echo -e "$RED$SERVER_MD5$DEFAULT";exit
#else
#    echo -e "$GREEN$SERVER_MD5$DEFAULT";
#fi



# Server Launch
echo -e "$RED\0Stopping Server$DEFAULT"
killall -9 server 

cd $SERVER_FOLDER
./server& 
cd ../client_folder




# File Listing test
echo
echo -e "$YELLOW\0File listing test"
echo -e "-----------------$DEFAULT"
#./client 127.0.0.1 $1 $2 list_files
LIST_TEST=$(./client 127.0.0.1 $1 $2 list_files);
# Le login est bon
if [[ $LIST_TEST == *"The key is correct"* ]];
then
    echo -e "$GREEN\0Login is correct +$PTS_LOGIN$DEFAULT";PTS=$(($PTS+$PTS_LOGIN));
fi

if [[ $LIST_TEST == *"$DOWNLOAD_FILE"* ]];then
    echo -e "$GREEN\0The file to download is listed +$PTS_LISTING$DEFAULT";PTS=$(($PTS+$PTS_LISTING))
fi

if [[ $LIST_TEST == *"Session closing..."* ]];
then
    echo -e "$GREEN\0Session is closed cleanly +$PTS_CLOSE$DEFAULT";PTS=$(($PTS+$PTS_CLOSE));
fi

echo -e "$GREEN$PTS/$PTS_1 pts$DEFAULT"
PTS_TOTAL=$(($PTS_TOTAL+$PTS))
PTS=0




# File Download Test
echo  
echo -e "$YELLOW\0File download test"
echo -e "-----------------$DEFAULT"
DOWNLOAD_TEST=$(./client 127.0.0.1 $1 $2 download $DOWNLOAD_FILE)
#echo "$DOWNLOAD_TEST" >> $CORRECT_FILE

if cmp -s "./local/$DOWNLOAD_FILE" "$SERVER_FOLDER/srv/$DOWNLOAD_FILE"
then
        echo -e "$GREEN\0The file was downloaded +$PTS_DOWNLOAD$DEFAULT";PTS=$(($PTS+$PTS_DOWNLOAD));
    else
        echo -e "$RED\0The downloaded file doesn't match the original$DEFAULT"
fi
if [[ $LIST_TEST == *"Session closing..."* ]];
then
    echo -e "$GREEN\0Session is closed cleanly +$PTS_CLOSE$DEFAULT";PTS=$(($PTS+$PTS_CLOSE));
fi
echo -e "$GREEN$PTS/$PTS_2 pts$DEFAULT"
PTS_TOTAL=$(($PTS_TOTAL+$PTS))
PTS=0




# File Upload Test
echo 
echo -e "$YELLOW\0File upload test"
echo -e "-----------------$DEFAULT"
UPLOAD_TEST=$(./client 127.0.0.1 $1 $2 upload $UPLOAD_FILE)
#echo "$UPLOAD_TEST" >> $CORRECT_FILE

if cmp -s "./local/$UPLOAD_FILE" "$SERVER_FOLDER/srv/$UPLOAD_FILE"
then
        echo -e "$GREEN\0The file was uploaded +$PTS_UPLOAD$DEFAULT";PTS=$(($PTS+$PTS_UPLOAD));
    else
        echo -e "$RED\0The downloaded file doesn't match the original$DEFAULT"
fi
if [[ $LIST_TEST == *"Session closing..."* ]];
then
    echo -e "$GREEN\0Session is closed cleanly +$PTS_CLOSE$DEFAULT";PTS=$(($PTS+$PTS_CLOSE));
fi
echo -e "$GREEN$PTS/$PTS_3 pts$DEFAULT"
PTS_TOTAL=$(($PTS_TOTAL+$PTS))
PTS=0



# Fin 
echo -e "\n\n$GREEN$PTS_TOTAL/$PTS_FINAL pts$DEFAULT"

echo -e "$RED\0Stopping Server$DEFAULT"
killall -9 server 
