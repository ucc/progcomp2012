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

baseDirectory = "/home/sam/Documents/progcomp2012/"
resultsDirectory = baseDirectory+"results/" #Where results will go (results are in the form of text files of agent names and scores)
agentsDirectory = baseDirectory+"samples/" #Where agents are found (each agent has its own directory)
logDirectory = baseDirectory+"log/" #Where log files go
nGames = 10 #Number of games played by each agent against each opponent. Half will be played as RED, half as BLUE
managerPath = baseDirectory+"manager/stratego" #Path to the manager program


scores = {"WIN":3, "DRAW":2, "LOSS":1, "NONE":0} #Score dictionary
#NONE occurs if both programs crash. If only one program crashes the result is reported as a win-loss



#Make necessary directories
if os.path.exists(resultsDirectory) == False:
	os.mkdir(resultsDirectory) #Make the results directory if it didn't exist
#Identify the round number by reading the results directory
roundNumber = len(os.listdir(resultsDirectory)) + 1
if roundNumber > 1:
	roundNumber -= 1

if os.path.exists(logDirectory) == False:
	os.mkdir(logDirectory) #Make the log directory if it didn't exist



if os.path.exists(logDirectory + "round"+str(roundNumber)) == False:
	os.mkdir(logDirectory + "round"+str(roundNumber)) #Check there is a directory for this round's logs

print "Simulating ROUND " +str(roundNumber)
print "Identifying possible agents in \""+agentsDirectory+"\""

#Get all agent names from agentsDirectory
agentNames = os.listdir(agentsDirectory) 
agents = []
for name in agentNames:
	#sys.stdout.write("\nLooking at Agent: \""+ str(name)+"\"... ")
	sys.stdout.write("Scan \""+name+"\"... ")
	if os.path.isdir(agentsDirectory+name) == False: #Remove non-directories
		sys.stdout.write(" Invalid! (Not a directory)\n")
		continue

	if os.path.exists(agentsDirectory+name+"/info") == False: #Try and find the special "info" file in each directory; ignore if it doesn't exist
		sys.stdout.write(" Invalid! (No \"info\" file found)\n")
		continue
	sys.stdout.write(" Valid!")
	#sys.stdout.write("OK")
	#Convert the array of names to an array of triples
	#agents[0] - The name of the agent (its directory)
	#agents[1] - The path to the program for the agent (typically agentsDirectory/agent/agent). Read from agentsDirectory/agent/info file
	#agents[2] - The score the agent achieved in _this_ round. Begins at zero
	agentExecutable = agentsDirectory+name+"/"+(open(agentsDirectory+name+"/info").readline().strip())
	agents.append([name, agentExecutable, 0])
	sys.stdout.write(" (Run program \""+agentExecutable+"\")\n")

if len(agents) == 0:
	print "Couldn't find any agents! Check paths (Edit this script) or generate \"info\" files for agents."
	sys.exit(0)

print "Total: " + str(len(agents)) + " valid agents found (From "+str(len(agentNames))+" possibilities)"

print ""

print "Commencing ROUND " + str(roundNumber) + " combat! ("+str(nGames)+" games per pairing)"
print "Points values are: "+str(scores)
print ""

normalGames = 0
draws = 0
aiErrors = 0
managerErrors = 0
#This double for loop simulates a round robin, with each agent getting the chance to play as both red and blue against every other agent.
for red in agents:  #for each agent playing as red,
	for blue in agents: #against each other agent, playing as blue
		if red == blue:
			continue #Exclude battles against self
		for i in range(1, nGames/2 + 1):
			#Play a game and read the result. Note the game is logged to a file based on the agent's names
			sys.stdout.write("Agents: \""+red[0]+"\" and \""+blue[0]+"\" playing game " + str(i) + "/"+str(nGames/2) + "... ")
			logFile = logDirectory + "round"+str(roundNumber) + "/"+red[0]+"_vs_"+blue[0]+"_"+str(i)
			outline = os.popen(managerPath + " -o " + logFile + " " + red[1] + " " + blue[1], "r").read()
			results = outline.split(' ')
			#Look at who won, and adjust scores based on that
			if results[0] == "NONE":
				red[2] += scores["NONE"]
				blue[2] += scores["NONE"]
				sys.stdout.write(" No contest. (Check AI for errors).\n")
				aiErrors += 1
			elif results[0] == "DRAW":
				red[2] += scores["DRAW"]
				blue[2] += scores["DRAW"]
				sys.stdout.write(" Draw.\n")
				draws += 1
			elif results[0] == red[1]:
				red[2] += scores["WIN"]
				blue[2] += scores["LOSS"]
				sys.stdout.write(" \""+red[0]+"\"\n")
				normalGames += 1
			elif results[0] == blue[1]:
				red[2] += scores["LOSS"]
				blue[2] += scores["WIN"]
				sys.stdout.write(" \""+blue[0]+"\"\n")
				normalGames += 1
			else:
				sys.stdout.write(" Garbage output! \""+outline+"\" (log file \""+logFile+"\")\n")
				managerErrors += 1
		

print "Completed combat. Total of " + str(normalGames + draws + aiErrors + managerErrors) + " games played. "
if managerErrors != 0:
	print "	WARNING: Recieved "+str(managerErrors)+" garbage outputs. Check the manager program."

print "" 
#We should now have complete score values.
		

sys.stdout.write("Creating results files for ROUND " + str(roundNumber) + "... ")

agents.sort(key = lambda e : e[2], reverse=True) #Sort the agents based on score

resultsFile = open(resultsDirectory+"round"+str(roundNumber)+".results", "w") #Create a file to store all the scores for this round
for agent in agents:
	resultsFile.write(agent[0] + " " + str(agent[2]) +"\n") #Write the agent names and scores into the file, in descending order

sys.stdout.write(" Complete!\n")

sys.stdout.write("Updating total scores... ");

#Now update the total scores
if os.path.exists(resultsDirectory+"total.scores"):
	sys.stdout.write(" Reading from \""+resultsDirectory+"total.scores\" to update scores... ")
	totalFile = open(resultsDirectory+"total.scores", "r") #Try to open the total.scores file
	for line in totalFile: #For all entries, 
		data = line.split(' ')
		for agent in agents:
			if agent[0] == data[0]:
				agent.append(agent[2]) #Store the score achieved this round at the end of the list
				agent[2] += int(data[1]) #Simply increment the current score by the recorded total score of the matching file entry
				break
	totalFile.close() #Close the file, so we can delete it
	os.remove(resultsDirectory+"total.scores") #Delete the file
	#Sort the agents again
	agents.sort(key = lambda e : e[2], reverse=True)

else:
	sys.stdout.write(" First round - creating \""+resultsDirectory+"total.scores\"... ")
sys.stdout.write(" Complete!\n")

print "Finished writing results for ROUND " + str(roundNumber)
print ""

print "RESULTS FOR ROUND " + str(roundNumber)
print "Agent: [name, path, total_score, recent_score]"

totalFile = open(resultsDirectory+"total.scores", "w") #Recreate the file
for agent in agents:
	totalFile.write(agent[0] + " " + str(agent[2]) +"\n") #Write the total scores in descending order
	print "Agent: " + str(agent)


#I just want to say the even though I still think python is evil, it is much better than bash. Using bash makes me cry.

