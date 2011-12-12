#!/usr/bin/python -u

'''
 simulate.py - simulation script for the 2012 UCC Programming Competition
	NOTE: This is not the manager program for a stratego game
	It merely calls the manager program as appropriate, and records results
	Plays exactly ONE round, but does not overwrite previously played rounds
	eg: run once to generate round1.results, twice to generate round2.results etc
	Also generates total.scores based on results from every round.
	

 author Sam Moore (matches) [SZM]
 website http://matches.ucc.asn.au/stratego
 email progcomp@ucc.asn.au or matches@ucc.asn.au
 git git.ucc.asn.au/progcomp2012.git
'''

import os
import sys
from time import time

#Global variables/arguments

baseDirectory = "../.." #Base directory for results, logs, agents
nGames = 2 #Number of games played by each agent against each opponent. Half will be played as RED, half as BLUE. If nGames <= 1, then no games will be played (useful for dry run?)
nRounds = 1

if len(sys.argv) >= 2:
	nRounds = int(sys.argv[1])
if len(sys.argv) >= 3:
	nGames = int(sys.argv[2])
	if nGames % 2 != 0:
		print "Warning: nGames should be even. "+str(nGames)+" specified, but only " + str(int(nGames/2) * 2)+" will be played!"
if len(sys.argv) >= 4:
	baseDirectory = sys.argv[3]
if len(sys.argv) >= 6:
	print "Useage: " +sys.argv[0] + " [nRounds=1] [nGames=10] [baseDirectory=\""+baseDirectory+"\"] [managerPath=baseDirectory+\"/judge/manager/stratego\"]"
	sys.exit(1)

resultsDirectory = baseDirectory+"/results/" #Where results will go (results are in the form of text files of agent names and scores)
logDirectory = baseDirectory+"/log/" #Where log files go (direct output of manager program)
agentsDirectory = baseDirectory+"/agents/" #Where agents are found (each agent has its own subdirectory within this directory)
managerPath = baseDirectory+"/judge/manager/stratego" #Path to the executable that plays the games
if len(sys.argv) >= 5:
	managerPath = sys.argv[5] 


#Score dictionary - Tuple is of the form: (end score, other score, other result) where end is the player on whose turn the result occurs, other is the other player, other result indicates what to record the outcome as for the other player.
scores = {"VICTORY":(3,1, "DEFEAT"), "DEFEAT":(1,3, "VICTORY"), "SURRENDER":(1,3, "VICTORY"), "DRAW":(2,2, "DRAW"), "DRAW_DEFAULT":(1,1, "DRAW_DEFAULT"), "ILLEGAL":(-1,2, "DEFAULT"), "DEFAULT":(2,-1, "ILLEGAL"), "BOTH_ILLEGAL":(-1,-1, "BOTH_ILLEGAL"), "INTERNAL_ERROR":(0,0, "INTERNAL_ERROR"), "BAD_SETUP":(0,0,"BAD_SETUP")}


#Verbose - print lots of useless stuff about what you are doing (kind of like matches in irc...)
verbose = True
makePrettyResults = False


#Check the manager program exists TODO: And is executable!
if os.path.exists(managerPath) == False:
	print "Manager program at \""+managerPath+"\" doesn't exist!"
	sys.exit(1)

#Make necessary directories

if os.path.exists(resultsDirectory) == False:
	os.mkdir(resultsDirectory) #Make the results directory if it didn't exist
#Identify the round number by reading the results directory
totalRounds = len(os.listdir(resultsDirectory)) + 1
if totalRounds > 1:
	totalRounds -= 1

if os.path.exists(logDirectory) == False:
	os.mkdir(logDirectory) #Make the log directory if it didn't exist


startTime = time() #Record time at which simulation starts

#Do each round...
for roundNumber in range(totalRounds, totalRounds + nRounds):

	if os.path.exists(logDirectory + "round"+str(roundNumber)) == False:
		os.mkdir(logDirectory + "round"+str(roundNumber)) #Check there is a directory for this round's logs

	if verbose:
		print "Simulating ROUND " +str(roundNumber)
		print "Identifying possible agents in \""+agentsDirectory+"\""

	#Get all agent names from agentsDirectory
	#TODO: Move this part outside the loop? It only has to happen once
	agentNames = os.listdir(agentsDirectory) 
	agents = []
	for name in agentNames:
		#sys.stdout.write("\nLooking at Agent: \""+ str(name)+"\"... ")
		if verbose:
			sys.stdout.write("Scan \""+name+"\"... ")
		if os.path.isdir(agentsDirectory+name) == False: #Remove non-directories
			if verbose:
				sys.stdout.write(" Invalid! (Not a directory)\n")
			continue

		if os.path.exists(agentsDirectory+name+"/info") == False: #Try and find the special "info" file in each directory; ignore if it doesn't exist	
			if verbose:
				sys.stdout.write(" Invalid! (No \"info\" file found)\n")
			continue

	
	
		agentExecutable = agentsDirectory+name+"/"+(open(agentsDirectory+name+"/info").readline().strip())
	
		if os.path.exists(agentExecutable) == False:
			if verbose:
				sys.stdout.write(" Invalid! (File \""+agentExecutable+"\" does not exist!)\n")
			continue


		if verbose:
			sys.stdout.write(" Valid! (To run: \""+agentExecutable+"\")\n")

		#Convert array of valid names into array of dictionaries containing information about each agent
		#I'm starting to like python...
		agents.append({"name":name, "path":agentExecutable,"score":[0], "totalScore":0, "VICTORY":[], "DEFEAT":[], "DRAW":[], "ILLEGAL":[], "INTERNAL_ERROR":[]})	
	if len(agents) == 0:
		print "Couldn't find any agents! Check paths (Edit this script) or generate \"info\" files for agents."
		sys.exit(0)
	if verbose:
		print "Total: " + str(len(agents)) + " valid agents found (From "+str(len(agentNames))+" possibilities)"
		print ""
		print "Commencing ROUND " + str(roundNumber) + " combat! This could take a while... "


	normalGames = 0
	draws = 0
	aiErrors = 0
	managerErrors = 0
	#This double for loop simulates a round robin, with each agent getting the chance to play as both red and blue against every other agent.
	gameID = 0
	for red in agents:  #for each agent playing as red,
		for blue in agents: #against each other agent, playing as blue
			if red == blue:
				continue #Exclude battles against self
			gameID += 1
			for i in range(1, nGames/2 + 1):
				#Play a game and read the result. Note the game is logged to a file based on the agent's names
				if verbose:
					sys.stdout.write("Agents: \""+red["name"]+"\" and \""+blue["name"]+"\" playing game " + str(i) + "/"+str(nGames/2) + "... ")
				logFile = logDirectory + "round"+str(roundNumber) + "/"+red["name"]+".vs."+blue["name"]+"."+str(i)
				outline = os.popen(managerPath + " -o " + logFile + " " + red["path"] + " " + blue["path"], "r").read()
				results = outline.split(' ')
			
				if len(results) != 6:
					if verbose:
						sys.stdout.write("Garbage output! \"" + outline + "\"\n")
					red["manager_errors"].append((gameID, blue["name"]))
					managerErrors += 1
				else:

					if results[1] == "RED":
						endColour = red
						otherColour = blue
					elif results[1] == "BLUE":
						endColour = blue
						otherColour = red
					if results[1] == "BOTH":
						pass
					else:
						endColour["score"].insert(0,endColour["score"][0] + scores[results[2]][0])
						endColour[results[2]].append((otherColour["name"], gameID, scores[results[2]][0]))
						otherColour["score"].insert(0, otherColour["score"][0] + scores[results[2]][1])
						otherColour[scores[results[2]][2]].append((endColour["name"], gameID, scores[results[2]][1]))

					if verbose:
						sys.stdout.write(" Result \"")
						for ii in range(1, len(results)):
							sys.stdout.write(results[ii].strip())
							if ii < (len(results) - 1):
								sys.stdout.write(" ")
						sys.stdout.write("\"\n")
		
	if verbose:
		print "Completed combat. Total of " + str(normalGames + draws + aiErrors + managerErrors) + " games played. "
	if managerErrors != 0:
		print "WARNING: Recieved "+str(managerErrors)+" garbage outputs. Check the manager program."

	if verbose:
		print "" 
	#We should now have complete score values.
		
	if verbose:
		sys.stdout.write("Creating results files for ROUND " + str(roundNumber) + "... ")

	agents.sort(key = lambda e : e["score"], reverse=True) #Sort the agents based on score
	
	resultsFile = open(resultsDirectory+"round"+str(roundNumber)+".results", "w") #Create a file to store all the scores for this round
	for agent in agents:
		resultsFile.write(agent["name"] + " " + str(agent["score"]) +"\n") #Write the agent names and scores into the file, in descending order

	if verbose:
		sys.stdout.write(" Complete!\n")
		sys.stdout.write("Updating total scores... ");
	
	#Now update the total scores
	if os.path.exists(resultsDirectory+"total.scores"):
		if verbose:
			sys.stdout.write(" Reading from \""+resultsDirectory+"total.scores\" to update scores... ")
		totalFile = open(resultsDirectory+"total.scores", "r") #Try to open the total.scores file
		for line in totalFile: #For all entries, 
			data = line.split(' ')
			for agent in agents:
				if agent["name"] == data[0]:
					agent["totalScore"] = int(data[1]) + agent["score"][0] #Simply increment the current score by the recorded total score of the matching file entry
					break
		totalFile.close() #Close the file, so we can delete it
		os.remove(resultsDirectory+"total.scores") #Delete the file
		#Sort the agents again
		agents.sort(key = lambda e : e["totalScore"], reverse=True)

	else:
		if verbose:
			sys.stdout.write(" First round - creating \""+resultsDirectory+"total.scores\"... ")
	if verbose:
		sys.stdout.write(" Complete!\n")
		print "Finished writing results for ROUND " + str(roundNumber)
		print ""
	
	
	print "RESULTS FOR ROUND " + str(roundNumber)

	totalFile = open(resultsDirectory+"total.scores", "w") #Recreate the file
	for agent in agents:
		totalFile.write(agent["name"] + " " + str(agent["totalScore"]) +"\n") #Write the total scores in descending order
		print "Agent: " + str(agent)


	#I just want to say the even though I still think python is evil, it is much better than bash. Using bash makes me cry.

endTime = time()
print "Completed simulating " + str(nRounds) + " rounds in " + str(endTime - startTime) + " seconds."

if makePrettyResults:
	if verbose:
		print "Now creating prettiful .html files..."
	
if verbose:
	print "Done!"
sys.exit(0)
