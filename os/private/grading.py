# -*- coding: UTF-8 -*-
import json
import hashlib
import timeit
import subprocess
import traceback
import time
from startup_simulator import *

class Dmesg(object):
	def __init__(self):
		self.__lastTime = 0
		self.__dmesg = subprocess.Popen(["dmesg", "-w"], stdout=subprocess.PIPE)
	
	def read_for(self, startups):
		startups = set(startups)
		result = {}
		while len(startups) > 0:
			line = self.__dmesg.stdout.readline()
			if line.find("STARTUP SIMULATOR: ") == -1:
				continue
			if line.find("***") == -1:
				continue
			
			if line.find("angel investment in startup") != -1:
				quote = line.find('"') + 1
				name = line[quote:line.find('"', quote)]
				result[name] = None
			else:
				worthObj = json.loads(line[line.find('***') + 4:])
				name = str(worthObj["name"])
				result[name] = worthObj["worth"]
			
			if name in startups:
				startups.remove(name)
			if len(startups) == 0:
				return result

class TestContext(object):
	def __init__(self, plan):
		self.dmesg = Dmesg()
		self.plan = open(plan, "rb").read()
		self.__count = 0
		self.__old_names = []
		self.__current_names = []
	
	def begin(self):
		self.__old_names += self.__current_names
		self.__current_names = []
	
	def wait_all(self):
		result = self.dmesg.read_for(self.__current_names)
		self.__current_names = []
		return result
	
	def startup_name(self, name):
		self.__count += 1
		name = hashlib.md5("%i-%s" % (self.__count, name)).hexdigest()
		self.__current_names.append(name)
		return name

def test_connect_twice(ctx):
	for i in range(2):
		ss = StartupSimulator()
		ss.connect()
		ss.close()

def test_create_startup(ctx):
	ss = StartupSimulator()
	try:
		ss.begin_startup(ctx.startup_name("startup 1"), 222222)
	finally:
		ss.close()

def test_send_plan(ctx):
	name = ctx.startup_name("startup 0")
	ss = StartupSimulator()
	try:
		ss.begin_startup(name, 123456)
		midpoint = len(ctx.plan) / 2
	
		angel = True
		if ss.business_plan_data(ctx.plan[:midpoint]) != ANGEL_INVESTOR:
			if ss.business_plan_data(ctx.plan[midpoint:]) != ANGEL_INVESTOR:
				if ss.finish() != ANGEL_INVESTOR:
					angel = False
	
		results = ctx.wait_all()
		assert name in results
		if angel:
			assert results[name] == None
		else:
			for a, b in zip(results[name], ss.net_worth):
				assert a == b
	finally:
		ss.close()

def test_cleanup_startups(ctx):
	for i in range(10):
		ss = StartupSimulator()
		try:
			ss.begin_startup(ctx.startup_name("startup 2"), 222222)
		finally:
			ss.close()

def test_angel(ctx):
	name = ctx.startup_name("startup 3")
	ss = StartupSimulator()
	try:
		ss.begin_startup(name, 123456)
		midpoint = len(ctx.plan) / 2
	
		if ss.business_plan_data(ctx.plan[:midpoint]) != ANGEL_INVESTOR:
			subprocess.check_call(["./ngelctl", "-s", name])
			r = ss.business_plan_data(ctx.plan[midpoint:])
			assert r == ANGEL_INVESTOR
	finally:
		ss.close()

def test_concurrency_low(ctx):
	pass

def test_concurrency_high(ctx):
	pass

if __name__ == "__main__":
	# Score, additional performance-based points, test
	tests = [
		(1, 0, test_connect_twice),
		(1, 0, test_create_startup),
		(2, 0, test_cleanup_startups),
		(8, 2, test_send_plan),
		(10, 0, test_angel),
		#(10, 10, test_concurrency_low),
		#(10, 10, test_concurrency_high),
		# test around edge cases
	]
	
	subprocess.check_call(["dmesg", "-C"])
	subprocess.check_call(["chmod", "0700", "/bin/dmesg"])
	try:
		subprocess.check_call(["chmod", "0700", "ngelctl"])
		ctx = TestContext("testplan.pdf")
		score = 0
		max = 0
		successes = 0
		total_time = 0
		for points, performance_score, test in tests:
			max += points
			try:
				ctx.begin()
				now = timeit.default_timer()
				test(ctx)
				elapsed = timeit.default_timer() - now
			
				success = True
				score += points
				successes += 1
			except:
				traceback.print_exc()
				success = False
				elapsed = float("inf")
			print "%20s, %i, %i,\t\t%i, %8g" % (test.__name__, points, performance_score, points if success else 0, elapsed)
			time.sleep(1)
		print "Total %i/%i, %i successes, %i failures" % (score, max, successes, len(tests) - successes)
	finally:
		subprocess.check_call(["chmod", "0755", "/bin/dmesg"])
