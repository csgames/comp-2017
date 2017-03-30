#!/usr/bin/env python

import errno
import socket
import struct

BEGIN_STARTUP = 0
BUSINESS_PLAN_DATA = 1

ACKNOWLEDGED = 2
ANGEL_INVESTOR = 3
PREDICTION_RESULT = 4
ERROR = 5

CODE_MAP = {
	0: "BEGIN_STARTUP",
	1: "BUSINESS_PLAN_DATA",
	2: "ACKNOWLEDGED",
	3: "ANGEL_INVESTOR",
	4: "PREDICTION_RESULT",
	5: "ERROR",
}

class StartupError(Exception):
	pass

# Error conditions that are detected on the client side
class StartupClientError(StartupError):
	pass

# Error replies from the server
class StartupServerError(StartupError):
	pass

class StartupSimulator(object):
	# High-level interface. You will probably want to use this.
	def __init__(self):
		self.__sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.name = None
		self.capital = None
		self.net_worth = None

	def begin_startup(self, name, capital):
		self.connect()
		code, payload = self.do_command(BEGIN_STARTUP, struct.pack("!I", capital) + name)
		if code == ACKNOWLEDGED:
			self.name = name
			self.capital = capital
			return self.__handle_ack(payload)
		else:
			return self.__response_error(BEGIN_STARTUP, code, ACKNOWLEDGED)

	def business_plan_data(self, chunk):
		if len(chunk) == 0:
			raise StartupClientError("Empty chunk! (did you mean to call `finish`?")
		code, payload = self.do_command(BUSINESS_PLAN_DATA, chunk)
		if code == ACKNOWLEDGED:
			return self.__handle_ack(payload)
		elif code == ANGEL_INVESTOR:
			return self.__handle_angel(payload)
		else:
			return self.__response_error(BUSINESS_PLAN_DATA, code, ACKNOWLEDGED, ANGEL_INVESTOR)

	def finish(self):
		code, payload = self.do_command(BUSINESS_PLAN_DATA, "")
		if code == ANGEL_INVESTOR:
			return self.__handle_angel(payload)
		elif code == PREDICTION_RESULT:
			return self.__handle_prediction_result(payload)
		else:
			return self.__response_error(BUSINESS_PLAN_DATA, code, ANGEL_INVESTOR, PREDICTION_RESULT)

	# Low-level interface. You are responsible for calling connect and close and
	# matching calls to send_command with calls to receive_command.
	def connect(self):
		self.__sock.connect(("localhost", 4321))

	def close(self):
		self.__sock.close()
    
	def do_command(self, command, payload):
		if len(payload) > 0xffff - 4:
			raise StartupClientError("Payload of length %i is too large!" % len(payload))
		expectAngel = False
		try:
			self.send_command(command, payload)
		except socket.socket_error as err:
			if err != errno.EPIPE:
				raise
			expectAngel = True
		
		code, payload = self.receive_result()
		if expectAngel and code != ANGEL_INVESTOR:
			raise StartupClientError("%i is not a valid response code when abruptly closing the connection!" % code)
		return (code, payload)
	
	def send_command(self, command, payload):
		self.__send(struct.pack("!HH", len(payload) + 4, command) + payload)

	def receive_result(self):
		length, code = struct.unpack("!HH", self.__recv(4))
		if length < 4:
			raise StartupClientError("Command response size is expected to be 4 at a minimum, got %i!" % length)
		payload = self.__recv(length - 4) if length != 4 else ""
		if code == ERROR:
			raise StartupServerError(payload)
		return (code, payload)

	# Private interface. You're not meant to use this.
	def __recv(self, size):
		result = self.__sock.recv(size)
		return result

	def __send(self, data):
		return self.__sock.send(data)

	def __handle_ack(self, payload):
		if payload != "":
			raise StartupClientError("Non-empty ACKNOWLEDGED body!")
		return ACKNOWLEDGED

	def __handle_angel(self, payload):
		self.close()
		if payload != "":
			raise StartupClientError("Non-empty ANGEL_INVESTOR body!")
		return ANGEL_INVESTOR

	def __handle_prediction_result(self, payload):
		self.close()
		self.net_worth = list(struct.unpack("!16q", payload))
		return PREDICTION_RESULT

	def __response_error(self, command, actualCode, *codes):
		if len(codes) > 1:
			plural = "s"
			toBe = "are"
		else:
			plural = ""
			toBe = "is"
		commands = ", ".join("%s (%i)" % (CODE_MAP[code], code) for code in codes)
		actualString = CODE_MAP[actualCode] if actualCode in CODE_MAP else "*unknown*"
		raise StartupClientError("The only valid response%s to %s (%i) %s %s, got %s (%i) instead!" % (plural, CODE_MAP[command], command, toBe, commands, actualString, actualCode))

def invoke(name, capital, fd, chunkSize):
	sim = StartupSimulator()
	sim.begin_startup(name, capital)

	chunk = fd.read(chunkSize)
	while len(chunk) > 0:
		code = sim.business_plan_data(chunk)
		if code == ANGEL_INVESTOR:
			return []
		chunk = fd.read(chunkSize)

	code = sim.finish()
	if code == ANGEL_INVESTOR:
		return []
	elif code == PREDICTION_RESULT:
		return sim.net_worth

if __name__ == "__main__":
	import argparse

	parser = argparse.ArgumentParser(description="Startup Simlulator client. Use this to test your server.")
	parser.add_argument("name", help="Startup name", type=str)
	parser.add_argument("--capital", "-c", help="Startup capital", type=int, default=150000)
	parser.add_argument("--pdf", "-p", help="Path to business plan PDF (default: <name>.pdf)", type=str, default=None)
	parser.add_argument("--chunk", "-s", help="Size of business plan chunks that are sent at once", type=int, default=0xffc)
	args = parser.parse_args()

	with open(args.pdf if args.pdf != None else args.name + ".pdf", "rb") as fd:
		result = invoke(args.name, args.capital, fd, args.chunk)

	if len(result) == 0:
		print "INTERRUPTED BY ANGEL INVESTOR"
	else:
		print "Net worth at end of year: %r" % result
