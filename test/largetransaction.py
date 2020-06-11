#!/usr/bin/env python3

import subprocess
import os
import tempfile 
import shutil
import codecs
import sys
import time
import json

def makeDirs(path) :
	os.makedirs(path, exist_ok = True)

def writeFile(fileName, content) :
	file = codecs.open(fileName, "w", "utf-8")
	file.write(str(content))
	file.close()

def executeCommand(*args) :
	process = subprocess.Popen(
		args,
		stdin = subprocess.PIPE,
		stdout = subprocess.PIPE,
		stderr = subprocess.PIPE,
		universal_newlines=True
	)
	cliStdout, cliStderr = process.communicate(input = None)
	returncode = process.poll()
	return (cliStdout + "\n" + cliStderr).strip()
	
def decodeJson(content) :
	try :
		return json.loads(content)
	except Exception as e:
		return None
		
def encodeJson(data) :
	try :
		return json.dumps(data)
	except Exception as e:
		return None
		
def die(message) :
	print(message + "\n")
	sys.exit(1)
	
def getNodePort(nodeIndex) :
	return 20000 + nodeIndex

def getNodeRpcPort(nodeIndex) :
	return 30000 + nodeIndex

def getNodeListenAddress(nodeIndex) :
	return '127.0.0.1:%d' % (getNodePort(nodeIndex))
	
def getNodeAlias(nodeIndex) :
	return 'node%d'  % (nodeIndex)

def checkError(result) :
	if result['error'] :
		print('executeCli error: %s, command: %s' % ( result['output'], result['command'] ))
		
# command utility functions

def getUnspent(node, address) :
	if address == None :
		return -1
	json = '["' + address + '"]'
	result = node.executeCli('listunspent', 0, 999999, json)
	if result['error'] :
		print('getUnspent error: ' + result['output'])
		return -1
	amount = 0
	for item in result['json'] :
		amount += item['amount']
	return amount

def getBlockCount(node) :
	result = node.executeCli('getblockcount')
	if result['error'] :
		print('getBlockCount error: ' + result['output'])
		return None
	return result['json']
		
def getBalance(node) :
	result = node.executeCli('getbalance')
	if result['error'] :
		print('getbalance error: ' + result['output'])
		return None
	return result['json']
	
def sendMany(node, destList, fromAccount = '') :
	if len(destList) == 0 :
		return
	data = {}
	for dest in destList :
		data[dest['address']] = dest['amount']
	result = node.executeCli('sendmany', fromAccount, encodeJson(data))
	if result['error'] :
		print('sendmany error: ' + result['output'])
		return None
	return result['json']
		
class Node :
	def __init__(self, app, nodeIndex) :
		self._app = app
		self._nodeIndex = nodeIndex
		self._nodePath = os.path.join(self._app.getRootPath(), 'node%d' % (nodeIndex))
		self._rpcUser = 'rpcuser%d' % (nodeIndex)
		self._rpcPassword = 'rpcpassword%d' % (nodeIndex)
		self._phored = self._app.getPhored()
		self._phoreCli = self._app.getPhoreCli()
		self._daemonProcess = None

	def createDataDir(self, nodeCount) :
		makeDirs(self._nodePath)
		writeFile(
			os.path.join(self._nodePath, 'phore.conf'),
			self._generatePhoreConf(nodeCount)
		)
		
	def startNode(self) :
		self._daemonProcess = subprocess.Popen([ self._phored, '-datadir=' + self._nodePath, '-daemon' ])
		
	def stopNode(self) :
		self.executeCli('stop')
		time.sleep(0.1)
		if self._daemonProcess != None :
			self._daemonProcess.kill()
		time.sleep(2)
	
	def executeCli(self, *args) :
		normalizedArgs = []
		for arg in args :
			normalizedArgs.append(str(arg))
		output = executeCommand(self._phoreCli, '-datadir=' + self._nodePath, *normalizedArgs)
		command = ' '.join(normalizedArgs)
		if output.find('error') >= 0 :
			return {
				'error' : True,
				'output' : output,
				'json' : None,
				'command' : command,
			}
		else :
			json = decodeJson(output)
			if json == None :
				json = output
			return {
				'error' : False,
				'output' : output,
				'json' : json,
				'command' : command,
			}
			
	def waitNodeStarting(self, timeoutSeconds = 15) :
		startTime = time.time()
		while time.time() - startTime < timeoutSeconds :
			if getBlockCount(self) != None :
				return True
			time.sleep(1)
		print('waitNodeStarting failed')
		return False

	def _generatePhoreConf(self, nodeCount) :
		result = ""
		result += "regtest=1\n"
		result += "server=1\n"
		result += "debug=1\n"
		result += "debug=net\n"
		result += "debug=phore\n"
		result += "rpcuser=%s\n" % (self._rpcUser)
		result += "rpcpassword=%s\n" % (self._rpcPassword)
		result += "port=%d\n" % (getNodePort(self._nodeIndex))
		result += "rpcport=%d\n" % (getNodeRpcPort(self._nodeIndex))
		result += "listenonion=0\n"
		result += "txindex=1\n"
		result += "externalip=%s\n" % getNodeListenAddress(self._nodeIndex)
		result += "budgetvotemode=suggest\n"
		for i in range(nodeCount) :
			if i == self._nodeIndex :
				continue
			result += "addnode=%s\n" % getNodeListenAddress(i)
		return result
		
	def dataDirExist(self) :
		return os.path.exists(self._nodePath)

class Application :
	def __init__(self) :
		self._nodeCount = 2
		
		self._nodeList = []
		self._removeFolderAfterExit = not True
		self._addressCount = 1000;

	def run(self) :
		self._testFailedLargeTransaction()
		self._testSuccessLargeTransaction()
		
	def _testFailedLargeTransaction(self) :
		self._setup()
		try :
			self._doTestFailedLargeTransaction()
		finally :
			self._cleanup()
	
	def _testSuccessLargeTransaction(self) :
		self._setup()
		try :
			self._doTestSuccessLargeTransaction()
		finally :
			self._cleanup()
	
	def _setup(self) :
		self._nodeList = []
		self._rootPath = self._makeRootPath()
		makeDirs(self._rootPath)
		print('Root path: %s' % (self._rootPath))
		
		self._phored = os.getenv('PHORED', None)
		if not self._phored :
			die('Undefined PHORED')
		self._phoreCli = os.getenv('PHORECLI', None)
		if not self._phoreCli :
			die('Undefined PHORECLI')
		print('phored: %s' % (self._phored))
		
		shutil.rmtree(self._rootPath)

	def _cleanup(self) :
		self._stopAllNodes()
		if self._removeFolderAfterExit :
			shutil.rmtree(self._rootPath)

	def _mineInitialCoins(self) :
		print('Mining initial coins...')
		blocksPerMine = 100
		for i in range(0, self._addressCount + blocksPerMine, blocksPerMine) :
			self._mineBlocks(self._nodeList[0], blocksPerMine)
			
	def _generateAddresses(self) :
		print('Generating addresses...')
		addressList = []
		for i in range(self._addressCount) :
			address = self._nodeList[1].executeCli('getnewaddress')['json']
			addressList.append(address)
		return addressList

	def _doTestFailedLargeTransaction(self) :
		print('Testing failed large transaction')
		self._createNodes()
		
		self._mineInitialCoins()
		addressList = self._generateAddresses()
		
		node0 = self._nodeList[0]
		node1 = self._nodeList[1]
		
		print('Split coins...')
		splitAmount = 100
		totalAmount = splitAmount * self._addressCount
		destList = []
		for i in range(self._addressCount) :
			address = addressList[i]
			destList.append({ 'address' : address, 'amount' : splitAmount })
			if len(destList) >= 100 :
				sendMany(node0, destList)
				destList = []
		sendMany(node0, destList)
		
		self._mineBlocks(node0, 100)
		print('Node0 balance:', getBalance(node0))
		balance = getBalance(node1)
		print('Node1 balance:', balance)
		
		print('Send via large transaction, the result should be error and the message should be "Transaction too large" ...')

		address = node0.executeCli('getnewaddress')['json']
		amountToSend = totalAmount * 9 // 10
		result = node1.executeCli('sendtoaddress', address, amountToSend)
		print(result)
		self._mineBlocks(node0, 100)
		print('Node0 balance:', getBalance(node0))
		print('Node1 balance:', getBalance(node1))
		
		
	def _doTestSuccessLargeTransaction(self) :
		print('Testing success large transaction')
		self._createNodes()
		
		self._mineInitialCoins()
		addressList = self._generateAddresses()
		
		node0 = self._nodeList[0]
		node1 = self._nodeList[1]
		
		print('Split coins...')
		splitAmount = 100
		totalAmount = splitAmount * self._addressCount
		addressCountForLargeAmount = 5
		addressCountForSmallAmount = self._addressCount - addressCountForLargeAmount
		smallAmount = 2
		totalLargeAmount = totalAmount * 9 // 10
		largeAmount = totalLargeAmount / addressCountForLargeAmount
		print('largeAmount', largeAmount)
		amountToSend = totalLargeAmount
		print('amountToSend', amountToSend)

		destList = []
		for i in range(self._addressCount) :
			address = addressList[i]
			amount = smallAmount
			if i % (self._addressCount // addressCountForLargeAmount) == 0 :
				amount = largeAmount
			destList.append({ 'address' : address, 'amount' : amount })
			if len(destList) >= 100 :
				sendMany(node0, destList)
				destList = []
		sendMany(node0, destList)
		
		self._mineBlocks(node0, 100)
		print('Node0 balance:', getBalance(node0))
		balance = getBalance(node1)
		print('Node1 balance:', balance)
		
		print('Send via large transaction, the result should be success ...')

		address = node0.executeCli('getnewaddress')['json']
		result = node1.executeCli('sendtoaddress', address, amountToSend)
		print(result)
		self._mineBlocks(node0, 100)
		print('Node0 balance:', getBalance(node0))
		print('Node1 balance:', getBalance(node1))
		
		
	def _mineBlocks(self, node, count) :
		node.executeCli('setgenerate', 'true', count)
		self._syncAllNodes()
		
	def _createNodes(self) :
		nodesExist = True
		for i in range(0, self._nodeCount) :
			if not Node(self, i).dataDirExist() :
				nodesExist = False
				break

		if nodesExist :
			print("All nodes data dirs exist, resuming")
			for i in range(0, self._nodeCount) :
				node = Node(self, i)
				self._nodeList.append(node)
		else :
			for i in range(0, self._nodeCount) :
				node = Node(self, i)
				self._nodeList.append(node)
				node.createDataDir(self._nodeCount)

		for node in reversed(self._nodeList) :
			node.startNode()
			node.waitNodeStarting()
		
	def _stopAllNodes(self) :
		for node in self._nodeList :
			node.stopNode()
			
	def _syncAllNodes(self, timeoutSeconds = 60) :
		if not self._syncBlocks(timeoutSeconds) :
			return False
		#if not self._syncMemPools(timeoutSeconds) :
		#	return False
		return True

	def _syncBlocks(self, timeoutSeconds) :
		startTime = time.time()
		printError = True
		while time.time() - startTime < timeoutSeconds :
			tips = []
			for node in self._nodeList :
				result = node.executeCli('getbestblockhash')
				tips.append(result['json'])
				if tips[-1] == None :
					if printError :
						print('getbestblockhash error: %s' % (result['output']))
						printError = False
			if tips == [ tips[0] ]*len(tips):
				return True
			time.sleep(1)
		print(tips)
		print('_syncBlocks failed')
		return False

	def _syncMemPools(self, timeoutSeconds) :
		startTime = time.time()
		while time.time() - startTime < timeoutSeconds :
			pool = set(self._nodeList[0].executeCli('getrawmempool')['json'])
			matchedCount = 1
			for i in range(1, len(self._nodeList)):
				if set(self._nodeList[i].executeCli('getrawmempool')['json']) == pool :
					matchedCount = matchedCount + 1
			if matchedCount == len(self._nodeList):
				return True
			time.sleep(1)
		print('_syncMemPools failed')
		return False

	def _makeRootPath(self) :
		return '/tmp/testphore/'
		return tempfile.mkdtemp(
			suffix = None,
			prefix = 'testphore',
			dir = None
		)
		
	def getRootPath(self) :
		return self._rootPath
		
	def getPhored(self) :
		return self._phored

	def getPhoreCli(self) :
		return self._phoreCli

if __name__ == '__main__':
    Application().run()
