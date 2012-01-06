#!/usr/bin/python -u

'''
 simulate.py - simulation script for the 2012 UCC Programming Competition
	NOTE: This is not the manager program for a stratego game
	It merely calls the manager program as appropriate, and records results
	Plays exactly ONE round, but does not overwrite previously played rounds
	eg: run once to generate round1.results, twice to generate round2.results etc
	Also generates total.scores based on results from every round.

	Now (sortof) generates .html files to display results in a prettiful manner.
	

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

timeoutValue = 2

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

resultsDirectory = baseDirectory+"/web/results/" #Where results will go (results are in the form of text files of agent names and scores)
logDirectory = baseDirectory+"/web/log/" #Where log files go (direct output of manager program)
agentsDirectory = baseDirectory+"/agents/" #Where agents are found (each agent has its own subdirectory within this directory)
managerPath = baseDirectory+"/judge/manager/stratego" #Path to the executable that plays the games
if len(sys.argv) >= 5:
	managerPath = sys.argv[5] 


#Score dictionary - Tuple is of the form: (end score, other score, other result) where end is the player on whose turn the result occurs, other is the other player, other result indicates what to record the outcome as for the other player.
scores = {"VICTORY":(3,1, "DEFEAT"), "DEFEAT":(1,3, "VICTORY"), "SURRENDER":(1,3, "VICTORY"), "DRAW":(2,2, "DRAW"), "DRAW_DEFAULT":(1,1, "DRAW_DEFAULT"), "ILLEGAL":(-1,2, "DEFAULT"), "DEFAULT":(2,-1, "ILLEGAL"), "BOTH_ILLEGAL":(-1,-1, "BOTH_ILLEGAL"), "INTERNAL_ERROR":(0,0, "INTERNAL_ERROR"), "BAD_SETUP":(0,0,"BAD_SETUP")}


#Verbose - print lots of useless stuff about what you are doing (kind of like matches talking on irc...)
verbose = True



#Check the manager program exists TODO: And is executable!
if os.path.exists(managerPath) == False:
	print "Manager program at \""+managerPath+"\" doesn't exist!"
	sys.exit(1)

#Make necessary directories

if os.path.exists(resultsDirectory) == False:
	os.mkdir(resultsDirectory) #Make the results directory if it didn't exist

	
#Identify the round number by reading from the "info" file in the results directory, if it doesn't exist then start at round 1.
if os.path.exists(resultsDirectory+"info") == False:
	totalRounds = 1
else:
	info = open(resultsDirectory+"info", "r")
	totalRounds = int(info.readline().strip())
	info.close()
	os.remove(resultsDirectory+"info")

info = open(resultsDirectory+"info", "w")
info.write(str(totalRounds + nRounds) + "\n")
info.close()
	


if os.path.exists(logDirectory) == False:
	os.mkdir(logDirectory) #Make the log directory if it didn't exist


startTime = time() #Record time at which simulation starts

if verbose:
	if nRounds > 1:
		print "Simulating " + str(nRounds) + " rounds (" + str(totalRounds) + " to " + str(totalRounds + nRounds-1) + ")"
	else:
		print "Simulating one round."
	print ""
	print "Identifying possible agents in \""+agentsDirectory+"\""

#Get all agent names from agentsDirectory
agentNames = os.listdir(agentsDirectory) 
agents = []
for name in agentNames:
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

	infoFile = open(agentsDirectory+name+"/info", "r")
	agentExecutable = agentsDirectory+name+"/"+(infoFile.readline().strip())
	author = infoFile.readline().strip()
	language = infoFile.readline().strip()
	description = ""
	while True:
		line = infoFile.readline()
		if len(line) > 0:
			description += line
		else:
			break
	infoFile.close()
	
	if os.path.exists(agentExecutable) == False:
		if verbose:
			sys.stdout.write(" Invalid! (Path: \""+agentExecutable+"\" does not exist!)\n")
		continue


	if verbose:
		sys.stdout.write(" Valid! (Path: \""+agentExecutable+"\")\n")

	#Convert array of valid names into array of dictionaries containing information about each agent
	#I'm starting to like python...
	agents.append({"name":name, "path":agentExecutable, "author":author, "language":language, "description":description, "score":[0], "VICTORY":[], "DEFEAT":[], "DRAW":[], "ILLEGAL":[], "DEFAULT":[], "INTERNAL_ERROR":[], "SURRENDER":[], "DRAW_DEFAULT":[], "BOTH_ILLEGAL":[], "BAD_SETUP":[], "ALL":[], "totalScore":0, "Wins":0, "Losses":0, "Draws":0, "Illegal":0, "Errors":0})

if len(agents) == 0:
	print "Couldn't find any agents! Check paths (Edit this script) or generate \"info\" files for agents."
	sys.exit(0)
if verbose:
	print "Total: " + str(len(agents)) + " valid agents found (From "+str(len(agentNames))+" possibilities)"
	print ""

#Prepare the pretty .html files if they don't exist
if verbose:
	print "Preparing .html results files..."


for agent in agents:
	if os.path.exists(resultsDirectory+agent["name"] + ".html") == False:
		agentFile = open(resultsDirectory+agent["name"] + ".html", "w")
		agentFile.write("<html>\n<head>\n <title> " + agent["name"] + " overview</title>\n</head>\n<body>\n<h1> Overview for " + agent["name"]+" </h1>\n")
		agentFile.write("<table border=\"0\" cellpadding=\"10\">\n")
		agentFile.write("<tr> <th> Name </th> <th> Author </th> <th> Language </th> </tr>\n")
		agentFile.write("<tr> <td> "+agent["name"]+" </td> <td> "+agent["author"]+" </td> <td> "+agent["language"]+" </td> </tr>\n")
		agentFile.write("</table>\n");

		agentFile.write("<p> <b>Description</b> </p>\n")
		agentFile.write("<p> " + agent["description"] + " </p>\n")
		agentFile.close()

	os.rename(resultsDirectory+agent["name"] + ".html", "tmpfile")
	
	oldFile = open("tmpfile", "r")
	agentFile = open(resultsDirectory+agent["name"] + ".html", "w")
	line = oldFile.readline()
	while line != "":
		#if verbose:
		#	print "Interpreting line \"" + line.strip() + "\""
		if line.strip() == "</body>":
			break
		elif line == "<tr> <th> Score </th> <th> Wins </th> <th> Losses </th> <th> Draws </th> <th> Illegal </th> <th> Errors </th></tr>\n":
			agentFile.write(line)
			line = oldFile.readline()
			
			values = line.split(' ')
			agent["totalScore"] += int(values[2].strip())
			agent["Wins"] += int(values[5].strip())
			agent["Losses"] += int(values[8].strip())
			agent["Draws"] += int(values[11].strip())
			agent["Illegal"] += int(values[14].strip())
			agent["Errors"] += int(values[17].strip())
		agentFile.write(line)
		line = oldFile.readline()

	if verbose:
		print "Prepared results file \"" + resultsDirectory+agent["name"] + ".html\"."
	oldFile.close()
	agentFile.close()
	os.remove("tmpfile")

if verbose:
	print ""

#Do each round...
totalGames = nGames/2 * len(agents) * (len(agents)-1)
for roundNumber in range(totalRounds, totalRounds + nRounds):

	if os.path.exists(logDirectory + "round"+str(roundNumber)) == False:
		os.mkdir(logDirectory + "round"+str(roundNumber)) #Check there is a directory for this round's logs

	for agent in agents:
		agent.update({"name":agent["name"], "path":agent["path"],  "score":[0], "VICTORY":[], "DEFEAT":[], "DRAW":[], "ILLEGAL":[], "DEFAULT":[], "INTERNAL_ERROR":[], "SURRENDER":[], "DRAW_DEFAULT":[], "BOTH_ILLEGAL":[], "BAD_SETUP":[], "ALL":[]})

	
	print "Commencing ROUND " + str(roundNumber) + " combat!"
	print "Total: " + str(totalGames) + " games to be played. This could take a while... (Estimate 60s/game)"



	managerErrors = 0
	#This double for loop simulates a round robin, with each agent getting the chance to play as both red and blue against every other agent.
	gameNumber = 0
	for red in agents:  #for each agent playing as red,
		for blue in agents: #against each other agent, playing as blue
			if red == blue:
				continue #Exclude battles against self
			gameNumber += 1
			gameID = str(roundNumber) + "." + str(gameNumber)
			for i in range(1, nGames/2 + 1):
				#Play a game and read the result. Note the game is logged to a file based on the agent's names
				if verbose:
					sys.stdout.write("Agents: \""+red["name"]+"\" and \""+blue["name"]+"\" playing game (ID: " + gameID + ") ... ")
				logFile = logDirectory + "round"+str(roundNumber) + "/"+red["name"]+".vs."+blue["name"]+"."+str(gameID)
				outline = os.popen(managerPath + " -o " + logFile + " -T " + str(timeoutValue) + " " + red["path"] + " " + blue["path"], "r").read()
				results = outline.split(' ')
			
				if len(results) != 6:
					if verbose:
						sys.stdout.write("Garbage output! \"" + outline + "\"\n")
					red["INTERNAL_ERROR"].append((blue["name"], gameID, scores["INTERNAL_ERROR"][0]))
					blue["INTERNAL_ERROR"].append((red["name"], gameID, scores["INTERNAL_ERROR"][0]))
					red["ALL"].append((blue["name"], gameID, scores["INTERNAL_ERROR"][0], "INTERNAL_ERROR", "RED"))
					blue["ALL"].append((red["name"], gameID, scores["INTERNAL_ERROR"][0], "INTERNAL_ERROR", "BLUE"))
					managerErrors += 1
				else:

					if results[1] == "RED":
						endColour = red
						otherColour = blue
						endStr = "RED"
						otherStr = "BLUE"
					elif results[1] == "BLUE":
						endColour = blue
						otherColour = red
						endStr = "BLUE"
						otherStr = "RED"


					if results[1] == "BOTH":
						red["INTERNAL_ERROR"].append((blue["name"], gameID, scores["INTERNAL_ERROR"][0]))
						blue["INTERNAL_ERROR"].append((red["name"], gameID, scores["INTERNAL_ERROR"][0]))
						red["ALL"].append((blue["name"], gameID, scores["INTERNAL_ERROR"][0], "INTERNAL_ERROR", "RED"))
						blue["ALL"].append((red["name"], gameID, scores["INTERNAL_ERROR"][0], "INTERNAL_ERROR", "BLUE"))
						managerErrors += 1
					else:
						endColour["score"].insert(0,endColour["score"][0] + scores[results[2]][0])
						endColour[results[2]].append((otherColour["name"], gameID, scores[results[2]][0]))
						endColour["ALL"].append((otherColour["name"], gameID, scores[results[2]][0], results[2], endStr))
						otherColour["score"].insert(0, otherColour["score"][0] + scores[results[2]][1])
						otherColour[scores[results[2]][2]].append((endColour["name"], gameID, scores[results[2]][1]))
						otherColour["ALL"].append((endColour["name"], gameID, scores[results[2]][1], scores[results[2]][2], otherStr))

					
					if verbose:
						sys.stdout.write(" Result \"")
						for ii in range(1, len(results)):
							sys.stdout.write(results[ii].strip())
							if ii < (len(results) - 1):
								sys.stdout.write(" ")
						sys.stdout.write("\"\n")
		
	if verbose:
		print "Completed combat. Total of " + str(gameNumber) + " games played. "
	if managerErrors != 0:
		print "WARNING: Registered "+str(managerErrors)+" errors. Check the manager program."

	if verbose:
		print "" 
	#We should now have complete score values.
		
	'''
		Obselete, non prettified results
	if verbose:
		sys.stdout.write("Creating raw results files for ROUND " + str(roundNumber) + "... ")

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
	'''
	if verbose:	
		print "RESULTS FOR ROUND " + str(roundNumber)

	#totalFile = open(resultsDirectory+"total.scores", "w") #Recreate the file
		#for agent in agents:	
		#totalFile.write(agent["name"] + " " + str(agent["totalScore"]) +"\n") #Write the total scores in descending order
		#if verbose:
		#		print "Agent: " + str(agent)
	

	if verbose:
		print "Updating pretty .html files... "

	for agent in agents:
		agentFile = open(resultsDirectory + agent["name"]+".html", "a")
		agentFile.write("<h2> Round " + str(roundNumber) + "</h2>\n")
		agentFile.write("<h3> Round Overview </h3>\n")
		agentFile.write("<table border=\"0\" cellpadding=\"10\">\n")
		agentFile.write("<tr> <th> Score </th> <th> Wins </th> <th> Losses </th> <th> Draws </th> <th> Illegal </th> <th> Errors </th></tr>\n")
		agentFile.write("<tr> <td> "+str(agent["score"][0])+" </td> <td> "+str(len(agent["VICTORY"]) + len(agent["DEFAULT"]))+" </td> <td> "+str(len(agent["DEFEAT"]) + len(agent["SURRENDER"]))+" </td> <td> "+str(len(agent["DRAW"]) + len(agent["DRAW_DEFAULT"]))+" </td> <td> "+str(len(agent["ILLEGAL"]) + len(agent["BOTH_ILLEGAL"]) + len(agent["BAD_SETUP"]))+" </td> <td> " +str(len(agent["INTERNAL_ERROR"]))+" </td> </tr>\n")

		agentFile.write("</table>\n")
		agentFile.write("<p> <a href=round"+str(roundNumber)+".html>Round "+str(roundNumber) + " Scoreboard</a></p>\n")

		agentFile.write("<h3> Detailed </h3>\n")
		agentFile.write("<table border=\"0\" cellpadding=\"10\">\n")
		agentFile.write("<tr> <th> Game ID </th> <th> Opponent </th> <th> Played as </th> <th> Outcome </th> <th> Score </th> <th> Accumulated Score </th> </tr> </th>\n")
		
		

		for index in range(0, len(agent["ALL"])):
			if agent["ALL"][index][4] == "RED":
				logFile = "log/round"+str(roundNumber) + "/"+agent["name"]+".vs."+agent["ALL"][index][0]+"."+str(agent["ALL"][index][1])
			else:
				logFile = "log/round"+str(roundNumber) + "/"+agent["ALL"][index][0]+".vs."+agent["name"]+"."+str(agent["ALL"][index][1])
			agentFile.write("<tr> <td> <a href="+logFile+">" + str(agent["ALL"][index][1]) + " </a> </td> <td> <a href="+agent["ALL"][index][0]+".html>"+agent["ALL"][index][0] + " </a> </td> <td> " + agent["ALL"][index][4] + " </td> <td> " + agent["ALL"][index][3] + " </td> <td> " + str(agent["ALL"][index][2]) + "</td> <td> " + str(agent["score"][len(agent["score"])-index -2]) + " </td> </tr> </th>\n")
		agentFile.write("</table>\n")
		
		agent["totalScore"] += agent["score"][0]
		agent["Wins"] += len(agent["VICTORY"]) + len(agent["DEFAULT"])
		agent["Losses"] += len(agent["DEFEAT"]) + len(agent["SURRENDER"])
		agent["Draws"] += len(agent["DRAW"]) + len(agent["DRAW_DEFAULT"])
		agent["Illegal"] += len(agent["ILLEGAL"]) + len(agent["BOTH_ILLEGAL"]) + len(agent["BAD_SETUP"])
		agent["Errors"] += len(agent["INTERNAL_ERROR"])

		agentFile.write("<h3> Accumulated Results </h3>\n")
		agentFile.write("<table border=\"0\" cellpadding=\"10\">\n")
		agentFile.write("<tr> <th> Score </th> <th> Wins </th> <th> Losses </th> <th> Draws </th> <th> Illegal </th> <th> Errors </th></tr>\n")
		agentFile.write("<tr> <td> "+str(agent["totalScore"])+" </td> <td> "+str(agent["Wins"])+" </td> <td> "+str(agent["Losses"])+" </td> <td> "+str(agent["Draws"])+" </td> <td> "+str(agent["Illegal"])+" </td> <td> " +str(agent["Errors"])+" </td> </tr>\n")

		agentFile.write("</table>\n")


		agentFile.close()	

	#Update round file
	roundFile = open(resultsDirectory + "round"+str(roundNumber)+".html", "w")
	roundFile.write("<html>\n<head>\n <title> Round " +str(roundNumber)+ " Overview </title>\n</head>\n<body>\n")
	roundFile.write("<h1> Round " +str(roundNumber)+ " Overview </h1>\n")
	roundFile.write("<table border=\"0\" cellpadding=\"10\">\n")
	roundFile.write("<tr> <th> Name </th> <th> Score </th> <th> Total Score </th> </tr>\n")
	agents.sort(key = lambda e : e["score"][0], reverse=True)
	for agent in agents:
		roundFile.write("<tr> <td> <a href="+agent["name"]+".html>"+agent["name"] + " </a> </td> <td> " + str(agent["score"][0]) + " </td> <td> " + str(agent["totalScore"]) + " </td> </tr>\n")
	roundFile.write("</table>\n")
	roundFile.write("<p> <a href=total.html>Current Scoreboard</a></p>\n")
	roundFile.write("</body>\n<!-- Results file for Round " + str(roundNumber) + " autogenerated by \"" + sys.argv[0] + "\" at time " + str(time()) + " -->\n</html>\n\n")
	roundFile.close()


	
	

if verbose:
	print "Finalising .html files... "
for agent in agents:
	agentFile = open(resultsDirectory + agent["name"]+".html", "a")

	#Write the "total" statistics

	agentFile.write("</body>\n<!-- Results file for \"" + agent["name"] + "\" autogenerated by \"" + sys.argv[0] + "\" at time " + str(time()) + " -->\n</html>\n\n")
	agentFile.close()

	if os.path.exists(resultsDirectory + "total.html") == True:
		os.remove(resultsDirectory + "total.html") #Delete the file

totalFile = open(resultsDirectory + "total.html", "w")
totalFile.write("<html>\n<head>\n <title> Total Overview </title>\n</head>\n<body>\n")
totalFile.write("<h1> Total Overview </h1>\n")
totalFile.write("<table border=\"0\" cellpadding=\"10\">\n")
totalFile.write("<tr> <th> Name </th> <th> Total Score </th> </tr>\n")
agents.sort(key = lambda e : e["totalScore"], reverse=True)
for agent in agents:
	totalFile.write("<tr> <td> <a href="+agent["name"]+".html>"+agent["name"] + " </a> </td> <td> " + str(agent["totalScore"]) + " </td> </tr>\n")
totalFile.write("</table>\n")

totalFile.write("<h2> Round Summaries </h2>\n")
totalFile.write("<table border=\"0\" cellpadding=\"10\">\n")
for i in range(1, totalRounds+1):
	totalFile.write("<tr> <td> <a href=round"+str(i)+".html>Round " + str(i) + "</a> </td> </tr>\n")
totalFile.write("</table>\n")

totalFile.write("</body>\n<!-- Total Results file autogenerated by \"" + sys.argv[0] + "\" at time " + str(time()) + " -->\n</html>\n\n")
totalFile.close()

	
if verbose:
	print "Done!"

endTime = time()
print "Completed simulating " + str(nRounds) + " rounds in " + str(endTime - startTime) + " seconds."
sys.exit(0)
