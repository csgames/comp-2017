CC=gcc
CFLAGS=-m32 -Wall -lcrypto -lssl -I.
BUILD_DIR="../../build/Password_Manager"

all: pwManager

pwManager:
	$(CC) -o pwManager pwManager.c $(CFLAGS)

final:
	if [ ! -d $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR);else rm -rf $(BUILD_DIR)/*;fi;
	$(CC) -m32 -o $(BUILD_DIR)/pwManager pwManager.c $(CFLAGS)
