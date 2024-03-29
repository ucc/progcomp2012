#!/usr/bin/python -u

'''
 simulate.py - simulation script for the 2012 UCC Programming Competition
	NOTE: This is not the manager program for a stratego game
	It merely calls the manager program as appropriate, and records results
	Plays exactly ONE round, but does not overwrite previously played rounds
	eg: run once to generate round1.results, twice to generate round2.results etc
	Also generates total.scores based on results from every round.

	Now (sortof) generates .html files to display results in a prettiful manner.
	
	THIS FILE IS TERRIBLE

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
scores = {"VICTORY":(0.3,0.1, "DEFEAT"), "DEFEAT":(0.1,0.3, "VICTORY"), "SURRENDER":(0,0.3, "VICTORY"), "DRAW":(0.2,0.2, "DRAW"), "DRAW_DEFAULT":(0.1,0.1, "DRAW_DEFAULT"), "ILLEGAL":(-0.1,0.2, "DEFAULT"), "DEFAULT":(0.2,-0.1, "ILLEGAL"), "BOTH_ILLEGAL":(-0.1,-0.1, "BOTH_ILLEGAL"), "INTERNAL_ERROR":(0,0, "INTERNAL_ERROR"), "BAD_SETUP":(0,0,"BAD_SETUP")}


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
	
	if os.path.exists(agentExecutable.split(" ")[0]) == False:
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

#BACKUP THE RESULTS DIRECTORY GOD DAMMIT
os.system("mkdir .before_round"+str(totalRounds)+"_BACKUP/; cp " + resultsDirectory+"*" + " .before_round"+str(totalRounds)+"_BACKUP/")


if os.path.exists(resultsDirectory + "index.html") == True:
	os.remove(resultsDirectory + "index.html") #Delete the file
totalFile = open(resultsDirectory + "index.html", "w")
totalFile.write("<html>\n<head>\n <title> Round in progress... </title>\n</head>\n<body>\n")
if nRounds > 1:
	totalFile.write("<h1> Rounds " + str(totalRounds) + " to " + str(totalRounds + nRounds-1) + " in progress...</h1>\n")
else:
	totalFile.write("<h1> Round " + str(totalRounds) + " in progress...</h1>\n")
totalFile.write("<p> Please wait for the rounds to finish. You can view the current progress by watching the <a href = \"../log\"/>Log Files</a> </p>")
if totalRounds > 1:
	totalFile.write("<h2> Round Summaries </h2>\n")
	totalFile.write("<table border=\"0\" cellpadding=\"10\">\n")
	for i in range(1, totalRounds):
		totalFile.write("<tr> <td> <a href=round"+str(i)+".html>Round " + str(i) + "</a> </td> </tr>\n")
	totalFile.write("</table>\n")

totalFile.write("</body>\n<!-- Total Results file autogenerated by \"" + sys.argv[0] + "\" at time " + str(time()) + " -->\n</html>\n\n")
totalFile.close()


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
		if line.strip() == "</body>" or line.strip() == "<!--end-->":
			break
		elif line == "<h3> Round Overview </h3>\n":
			agentFile.write(line)
			line = oldFile.readline()
			agentFile.write(line)
			line = oldFile.readline()
			if line == "<tr> <th> Score </th> <th> Wins </th> <th> Losses </th> <th> Draws </th> <th> Illegal </th> <th> Errors </th></tr>\n":
				#sys.stdout.write("Adding scores... " + line + "\n")
				agentFile.write(line)
				line = oldFile.readline()
				values = line.split(' ')
				agent["totalScore"] += float(values[2].strip())
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
	print "Total: " + str(totalGames) + " games to be played. This could take a while..."



	managerErrors = 0
	#This double for loop simulates a round robin, with each agent getting the chance to play as both red and blue against every other agent.
	gameNumber = 0
	for red in agents:  #for each agent playing as red,
		for blue in agents: #against each other agent, playing as blue
			if red == blue:
				continue #Exclude battles against self
			
			
			for i in range(1, nGames/2 + 1):
				gameNumber += 1
				gameID = str(roundNumber) + "." + str(gameNumber)
				#Play a game and read the result. Note the game is logged to a file based on the agent's names
				if verbose:
					sys.stdout.write("Agents: \""+red["name"]+"\" and \""+blue["name"]+"\" playing game (ID: " + gameID + ") ... ")
				logFile = logDirectory + "round"+str(roundNumber) + "/"+red["name"]+".vs."+blue["name"]+"."+str(gameID)
				errorLog = [logDirectory + "error/" + red["name"] + "."+str(gameID), logDirectory + "error/" + blue["name"] + "."+str(gameID)]
				#Run the game, outputting to logFile; stderr of (both) AI programs is directed to logFile.stderr
				outline = os.popen(managerPath + " -m 1000 -o " + logFile + " -T " + str(timeoutValue) + " \"" + red["path"] + "\" \"" + blue["path"] + "\" 2>> " + logFile+".stderr", "r").read()
				
				#os.system("mv tmp.mp4 " + logFile + ".mp4")
				
				#If there were no errors, get rid of the stderr file
				if os.stat(logFile+".stderr").st_size <= 0:
					os.remove(logFile+".stderr")
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
						#Write scores to raw text files
						for agent in [endColour, otherColour]:
							scoreFile = open(resultsDirectory + agent["name"] + ".scores", "a")
							scoreFile.write(str(agent["totalScore"] + agent["score"][0]) + "\n")
							scoreFile.close()
						
						

					
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
				logFile = "../log/round"+str(roundNumber) + "/"+agent["name"]+".vs."+agent["ALL"][index][0]+"."+str(agent["ALL"][index][1])
			else:
				logFile = "../log/round"+str(roundNumber) + "/"+agent["ALL"][index][0]+".vs."+agent["name"]+"."+str(agent["ALL"][index][1])
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

	command = "cp scores.plt " + resultsDirectory + "scores.plt;"
	os.system(command)

	scorePlot = open(resultsDirectory + "scores.plt", "a")
	scorePlot.write("plot ")
	for i in range(0, len(agents)):
		if i > 0:
			scorePlot.write(", ")
		scorePlot.write("\""+agents[i]["name"]+".scores\" using ($0+1):1 with linespoints title \""+agents[i]["name"]+"\"")

	scorePlot.write("\nexit\n")
	scorePlot.close()

	command = "d=$(pwd); cd " + resultsDirectory + ";"
	command += "gnuplot scores.plt;"
	command += "rm -f scores.plt;"
	command += "mv scores.png round"+str(roundNumber)+".png;"
	command += "cd $d;"
	os.system(command)

	roundFile.write("<h2> Accumulated Scores - up to Round " + str(roundNumber)+" </h2>\n")
	roundFile.write("<img src=\"round"+str(roundNumber)+".png\" alt = \"round"+str(roundNumber)+".png\" title = \"round"+str(roundNumber)+".png\" width = \"640\" height = \"480\"/>\n")

	roundFile.write("<p> <a href=index.html>Current Scoreboard</a></p>\n")
	roundFile.write("</body>\n<!-- Results file for Round " + str(roundNumber) + " autogenerated by \"" + sys.argv[0] + "\" at time " + str(time()) + " -->\n</html>\n\n")
	roundFile.close()


	
	

if verbose:
	print "Finalising .html files... "
for agent in agents:
	agentFile = open(resultsDirectory + agent["name"]+".html", "a")
	agentFile.write("<!--end-->\n")
	#Write a graph
	#Comment out if you don't have gnuplot

	command  = "rm -f " + agent["name"] + ".png;"
	command += "cp template.plt " + resultsDirectory + agent["name"] + ".plt;"
	command += "d=$(pwd); cd " + resultsDirectory + ";"
	command += "sed -i \"s:\[NAME\]:"+agent["name"]+":g\" " +resultsDirectory + agent["name"]+".plt;"
	command += "gnuplot " + resultsDirectory + agent["name"]+".plt;"
	command += "rm -f " + resultsDirectory + agent["name"] + ".plt;"
	command += "cd $d;"
	os.system(command)
	agentFile.write("<!--end-->\n")
	agentFile.write("<h3> Score Graph </h3>\n")
	agentFile.write("<img src=\""+agent["name"]+".png\" alt=\""+agent["name"]+".png\" title=\""+agent["name"]+".png\" width=\"640\" height=\"480\"/>\n")

	#Link to main file
	agentFile.write("<p> <a href=\"index.html\"/>Total Statistics</a> </p>\n")

	agentFile.write("</body>\n<!-- Results file for \"" + agent["name"] + "\" autogenerated by \"" + sys.argv[0] + "\" at time " + str(time()) + " -->\n</html>\n\n")
	agentFile.close()

if os.path.exists(resultsDirectory + "index.html") == True:
	os.remove(resultsDirectory + "index.html") #Delete the file

totalFile = open(resultsDirectory + "index.html", "w")
totalFile.write("<html>\n<head>\n <title> Total Overview </title>\n</head>\n<body>\n")
totalFile.write("<h1> Total Overview </h1>\n")
totalFile.write("<table border=\"0\" cellpadding=\"10\">\n")
totalFile.write("<tr> <th> Name </th> <th> Total Score </th> </tr>\n")
agents.sort(key = lambda e : e["totalScore"], reverse=True)
for agent in agents:
	totalFile.write("<tr> <td> <a href="+agent["name"]+".html>"+agent["name"] + " </a> </td> <td> " + str(agent["totalScore"]) + " </td> </tr>\n")
totalFile.write("</table>\n")

totalFile.write("<h2> Score Graph </h2>\n")


command = "d=$(pwd);"
command += "cd " + resultsDirectory + ";"
command += "rm -f scores.png;"
command += "cp round"+str(roundNumber)+".png scores.png;"
command += "cd $d;"
os.system(command)

totalFile.write("<img src=\"scores.png\" alt=\"scores.png\" title=\"scores.png\" width=\"640\" height=\"480\"/>\n")


totalFile.write("<h2> Round Summaries </h2>\n")
totalFile.write("<table border=\"0\" cellpadding=\"10\">\n")
for i in range(1, totalRounds+1):
	totalFile.write("<tr> <td> <a href=round"+str(i)+".html>Round " + str(i) + "</a> </td> </tr>\n")
totalFile.write("</table>\n")

totalFile.write("</body>\n<!-- Total Results file autogenerated by \"" + sys.argv[0] + "\" at time " + str(time()) + " -->\n</html>\n\n")
totalFile.close()

#Write results to a raw text file as well
textResults = open(resultsDirectory + "total.txt", "w")
for agent in agents:
	textResults.write(agent["name"] + " " + str(agent["totalScore"]) + "\n")
textResults.close()
	
if verbose:
	print "Done!"

endTime = time()
print "Completed simulating " + str(nRounds) + " rounds in " + str(endTime - startTime) + " seconds."
sys.exit(0)
