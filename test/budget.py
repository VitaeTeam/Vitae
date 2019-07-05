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

	def createDataDir(self, nodeCount, masterNodePrivateKey = None) :
		makeDirs(self._nodePath)
		writeFile(
			os.path.join(self._nodePath, 'phore.conf'),
			self._generatePhoreConf(nodeCount, masterNodePrivateKey)
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

	def _generatePhoreConf(self, nodeCount, masterNodePrivateKey) :
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
		if masterNodePrivateKey != None :
			result += "masternode=1\n"
			result += "masternodeprivkey=%s\n" % (masterNodePrivateKey)
			result += "masternodeaddr=%s\n" % getNodeListenAddress(i)
		return result
		
	def writeMasterNodeConfig(self, config) :
		writeFile(
			os.path.join(self._nodePath, 'regtest', 'masternode.conf'),
			config
		)
		
	def isMasterNodeSynced(self) :
		json = self.executeCli('mnsync', 'status')['json']
		if json == None :
			return False
		if json['RequestedMasternodeAssets'] > 100 :
			return True
		return False
		
	def dataDirExist(self) :
		return os.path.exists(self._nodePath)

class Application :
	def __init__(self) :
		self._nodeCount = 4
		
		self._nodeList = []
		self._budgetCycle = 864
		self._removeFolderAfterExit = not True

	def run(self) :
		self._setup()
		try :
			self._doRun()
		finally :
			self._cleanup()
	
	def _setup(self) :
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
	
	def _cleanup(self) :
		self._stopAllNodes()
		if self._removeFolderAfterExit :
			shutil.rmtree(self._rootPath)
	
	def _doRun(self) :
		self._createNodes()
		
		node = self._nodeList[0]
		self._mineBlocks(node, 200)

		address = node.executeCli('getnewaddress')['json']
		address = node.executeCli('getnewaddress')['json']
		print('Before budget: ' + str(getUnspent(node, address)))
		
		blockCount = getBlockCount(node)
		superBlock = blockCount - blockCount % self._budgetCycle + self._budgetCycle
		wtx = node.executeCli('preparebudget', 'ppp1', 'http://test1.com', 5, superBlock, address, 100)['json']
		print('preparebudget: ' + wtx)

		self._mineBlocks(node, 100)
		hash = node.executeCli('submitbudget', 'ppp1', 'http://test1.com', 5, superBlock, address, 100, wtx)['json']
		print('submitbudget: ' + hash)

		self._mineBlocks(node, 100)

		result = node.executeCli('getbudgetinfo')
		for i in range(1, self._nodeCount) :
			result = self._nodeList[i].executeCli('mnbudgetvote', 'local', hash, 'yes')
			print(result['output'])

		for i in range(self._nodeCount) :
			masterNode = self._nodeList[i]
			blockCount = getBlockCount(node)
			blocksToMine = self._budgetCycle - blockCount % self._budgetCycle - 1
			if blocksToMine == 0 :
				blocksToMine = self._budgetCycle
			blocksToMine = blocksToMine - 1
			self._mineBlocks(masterNode, blocksToMine)
			previousBlockCount = getBlockCount(masterNode)
			if previousBlockCount == None :
				continue
			self._mineBlocks(masterNode, 1)
			self._mineBlocks(masterNode, 1)
			self._mineBlocks(masterNode, 1)
			newBlockCount = getBlockCount(masterNode)
			print(
				'During super block: previousBlockCount=%d newBlockCount=%d expect=%d'
				% (previousBlockCount, newBlockCount, previousBlockCount + 3)
			)
			self._mineBlocks(masterNode, 100)
			#print(node.executeCli('getbudgetinfo')['output'])
		
		print('After budget: ' + str(getUnspent(node, address)))
		
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
			controllingNode = self._createControllingNode()
			for i in range(1, self._nodeCount) :
				node = Node(self, i)
				self._nodeList.append(node)
				key = controllingNode['masterNodePrivateKeyList'][i - 1]
				node.createDataDir(self._nodeCount, key)

		for node in reversed(self._nodeList) :
			node.startNode()
			node.waitNodeStarting()

		for node in self._nodeList :
			self._mineBlocks(node, 200)

		time.sleep(3)

		self._syncMasterNodes()
		for i in range(1, self._nodeCount) :
			result = self._nodeList[0].executeCli('startmasternode', 'alias', 'false', getNodeAlias(i))
			print(result['output'])
			
	def _createControllingNode(self) :
		node = Node(self, 0)
		self._nodeList.append(node)
		node.createDataDir(self._nodeCount)
		node.startNode()
		node.waitNodeStarting()
		self._mineBlocks(node, 200)

		masterNodePrivateKeyList = []
		masterNodeConfig = ''
		for i in range(1, self._nodeCount) :
			key = node.executeCli('masternode', 'genkey')['json']
			masterNodePrivateKeyList.append(key)
			nodeName = getNodeAlias(i)
			# Intended to generate address twice
			address = node.executeCli('getnewaddress')['json']
			address = node.executeCli('getnewaddress')['json']
			result = node.executeCli('sendtoaddress', address, 10000)
			#print(getUnspent(node, address))
			checkError(result)
			tx = result['json']
			outputs = node.executeCli('masternode', 'outputs')
			outputsList = outputs['json']
			txIndex = 0
			for o in outputsList :
				if o['txhash'] == tx :
					txIndex = o['outputidx']
					break
			masterNodeConfig += "%s %s %s %s %s\n" % (nodeName, getNodeListenAddress(i), key, tx, str(txIndex))
		self._mineBlocks(node, 100)
		node.writeMasterNodeConfig(masterNodeConfig)
		node.stopNode()
		print('Created controlling node')
		
		return {
			'node' : node,
			'masterNodePrivateKeyList' : masterNodePrivateKeyList,
		}
		
	def _stopAllNodes(self) :
		for node in self._nodeList :
			node.stopNode()
			
	def _syncAllNodes(self, timeoutSeconds = 60) :
		if not self._syncBlocks(timeoutSeconds) :
			return False
		if not self._syncMemPools(timeoutSeconds) :
			return False
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

	def _syncMasterNodes(self, timeoutSeconds = 60) :
		return # it never works, don't waste time to try

		startTime = time.time()
		while timeoutSeconds < 0 or time.time() - startTime < timeoutSeconds :
			allSynced = True
			for i in range(1, len(self._nodeList)):
				if not self._nodeList[i].isMasterNodeSynced() :
					#print('MN %d status %s' % (i, self._nodeList[i].executeCli('mnsync', 'status')['output']))
					allSynced = False
					break
			if allSynced :
				return True
			time.sleep(1)
		print('_syncMasterNodes failed')
		return False

	def _makeRootPath(self) :
		return '/tmp/testbudget/'
		return tempfile.mkdtemp(
			suffix = None,
			prefix = 'testbudget_',
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
