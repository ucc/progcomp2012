#!/usr/bin/python -u

#NOTE: The -u option is required for unbuffered stdin/stdout.
#	If stdin/stdout are buffered, the manager program will not recieve any messages and assume that the agent has timed out.

'''
 vixen.py - A sample Stratego AI for the UCC Programming Competition 2012

 Written in python, the slithery language 

 author Sam Moore (matches) [SZM]
 website http://matches.ucc.asn.au/stratego
 email progcomp@ucc.asn.au or matches@ucc.asn.au
 git git.ucc.asn.au/progcomp2012.git
'''

from basic_python import *
from path import *



class Vixen(BasicAI):
	" Python based AI, improves upon Asmodeus by taking into account probabilities, and common paths "
	def __init__(self):
		#sys.stderr.write("Vixen initialised...\n")
		BasicAI.__init__(self)
		
		
		#self.bombScores = {'1' : -0.9 , '2' : -0.8 , '3' : -0.5 , '4' : 0.1, '5' : 0.1, '6' : 0.3, '7' : 0.7, '8' : 1 , '9' : 0.6, 's' : 0}
		#self.bombScores = {'1' : -0.9 , '2' : -0.8 , '3' : -0.5 , '4' : -0.5, '5' : -0.4, '6' : -0.5, '7' : -0.2, '8' : 1.0 , '9' : -0.1, 's' : -0.2}
		self.suicideScores = {'1' : -0.8 , '2' : -0.6 , '3' : -0.5, '4' : -0.25, '5' : -0.2, '6' : 0.0, '7' : 0.1, '8' : -1.0 , '9' : 0.0, 's' : -0.4}
		self.killScores = {'1' : 1.0 , '2' : 0.9 , '3' : 0.9 , '4' : 0.8, '5' : 0.8, '6' : 0.8, '7' : 0.8, '8' : 0.9 , '9' : 0.7, 's' : 1.0}	
		self.riskScores = {'1' : -0.3, '2' : -0.3, '3' : 0.0, '4': 0.4, '5': 0.6, '6': 0.7, '7':0.8, '8': 0.0, '9' : 1.0, 's' : 0.1}



			

	def MakeMove(self):
		#sys.stderr.write("Vixen MakingMove...\n")
		" Over-rides the default BasicAI.MakeMove function "

		moveList = []
		for unit in self.units:
			if unit.mobile() == False:
				continue

			scores = {"LEFT":0, "RIGHT":0, "UP":0, "DOWN":0}
			

			for target in self.enemyUnits:
				if target == unit:
					continue
				path = PathFinder().pathFind((unit.x, unit.y), (target.x, target.y), self.board)
				if path == False or len(path) == 0:
					continue
				#moveList.append({"unit":unit, "direction":path[0], "score":self.CalculateScore(unit, target, path)})
				scores[path[0]] += self.CalculateScore(unit, target, path)

			bestScore = sorted(scores.items(), key = lambda e : e[1], reverse=True)[0]
			moveList.append({"unit":unit, "direction":bestScore[0], "score":bestScore[1]})
			

		if len(moveList) <= 0:
			print "NO_MOVE"
			return True

		moveList.sort(key = lambda e : e["score"], reverse=True)
		#sys.stderr.write("vixen - best move: " + str(moveList[0]["unit"].x) + " " + str(moveList[0]["unit"].y) + " " + moveList[0]["direction"] + " [ score = " + str(moveList[0]["score"]) + " ]\n")
		#if moveList[0]["score"] == 0:
		#	print "NO_MOVE"
		#	return True

		
		print str(moveList[0]["unit"].x) + " " + str(moveList[0]["unit"].y) + " " + moveList[0]["direction"]
		return True
				
			
	def tailFactor(self, pathLength):
		#if pathLength >= len(self.tailFactors) or pathLength <= 0:
		#	return 0.0
		#return self.tailFactors[pathLength]
		#return 0.5 * (1.0 + pow(pathLength, 0.75))
		return 1.0 / pathLength


	def CalculateScore(self, attacker, defender, path):
		p = move(attacker.x, attacker.y, path[0], 1)
		if p[0] < 0 or p[0] >= len(self.board) or p[1] < 0 or p[1] >= len(self.board[p[0]]):
			return -100.0

		total = 0.0
		count = 0.0
		for rank in ranks:
			prob = self.rankProbability(defender, rank)			
			if prob > 0.0:
				#sys.stderr.write("	" + str(attacker.rank) + " vs. " + str(rank) + " [" + str(prob) + "] score " + str(self.combatScore(attacker.rank, rank, len(path))) + "\n")
				total += prob * self.combatScore(attacker.rank, rank, len(path))
				count += 1
				
		
		#if count > 1:
		#	total = total / count + self.riskScore(attacker.rank)


		total = total * self.tailFactor(len(path))
		#HACK - Prevent "oscillating" by decreasing the value of backtracks
		if len(path) > 1 and len(attacker.positions) > 1 and attacker.positions[1][0] == p[0] and attacker.positions[1][1] == p[1]:
			total = total / 100
		#sys.stderr.write("Total score for " + str(attacker) + " vs. " + str(defender) + " is " + str(total) + "\n")
		return total

	def combatScore(self, attackerRank, defenderRank, pathLength):
		if defenderRank == 'F':
			return 1.0
		elif defenderRank == 'B':
			return self.bombScore(attackerRank)
		elif defenderRank == 's' and attackerRank == '1' and pathLength == 2:
			return self.suicideScore(attackerRank)
		elif defenderRank == '1' and attackerRank == 's' and pathLength != 2:
			return self.killScore(attackerRank)

		if valuedRank(attackerRank) > valuedRank(defenderRank):
			return self.killScore(defenderRank)
		elif valuedRank(attackerRank) < valuedRank(defenderRank):
			return self.suicideScore(attackerRank)
		return self.killScore(defenderRank) + self.suicideScore(attackerRank)

	def killScore(self, defenderRank):
		return self.killScores[defenderRank]

	def bombScore(self, attackerRank):
		if attackerRank == '8':
			return 1.0
		else:
			return self.suicideScore(attackerRank)

	def suicideScore(self, attackerRank):
		return self.suicideScores[attackerRank]

	def riskScore(self, attackerRank):
		return self.riskScores[attackerRank]

	def rankProbability(self, target, targetRank):
		if targetRank == '+' or targetRank == '?':
			return 0.0
		if target.rank == targetRank:
			return 1.0
		elif target.rank != '?':
			return 0.0

		total = 0.0
		for rank in ranks:
			if rank == '+' or rank == '?':
				continue
			elif rank == 'F' or rank == 'B':
				if target.lastMoved < 0:
					total += self.hiddenEnemies[rank]
			else:
				total += self.hiddenEnemies[rank]

		if total == 0.0:
			return 0.0
		return float(float(self.hiddenEnemies[targetRank]) / float(total))
	

	

				
				
		
if __name__ == "__main__":
	vixen = Vixen()
	if vixen.Setup():
		while vixen.MoveCycle():
			pass

