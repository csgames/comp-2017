CC=gcc
CFLAGS=-m32 -lcrypto -Wall -I.
BUILD_DIR="../../build/File_Transfer"
BUILD_SERVER=$(BUILD_DIR)/server_folder
BUILD_CLIENT=$(BUILD_DIR)/client_folder

all: file_server

file_server:
	$(CC) -o server_folder/server file_server.c Base64Encode.c Base64Decode.c $(CFLAGS)

final:
	if [ ! -d $(BUILD_DIR) ];then mkdir -p $(BUILD_SERVER);mkdir -p $(BUILD_CLIENT);else rm -rf $(BUILD_DIR)/*;fi;
	$(CC) -o $(BUILD_SERVER)/server file_server.c Base64Encode.c Base64Decode.c $(CFLAGS)
	touch $(BUILD_CLIENT)/client
	chmod +x $(BUILD_CLIENT)/client
	cp client_folder/run.sh $(BUILD_CLIENT)/run.sh
	mkdir $(BUILD_CLIENT)/local
	mkdir $(BUILD_SERVER)/srv


