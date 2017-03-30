CC=gcc
CFLAGS=-Wall -I.
BUILD_DIR=../build
OS:=$(shell uname)

all: final

final: 
	if [ ! -d $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR);else rm -rf $(BUILD_DIR)/*;fi;
	echo "\033[1m\033[32m Building File Transfer\033[0m"
	$(MAKE) -C File_Transfer final
	echo "\033[1m\033[32m Building Password Manager\033[0m"
	$(MAKE) -C Password_Manager final;
	echo "\033[1m\033[32m Copying Client List\033[0m"
	$(MAKE) -C Liste_Clients
	

