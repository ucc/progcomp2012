#!/usr/bin/python -u

#NOTE: The -u option is required for unbuffered stdin/stdout.
#	If stdin/stdout are buffered, the manager program will not recieve any messages and assume that the agent has timed out.

'''
 asmodeus.py - A sample Stratego AI for the UCC Programming Competition 2012

 Written in python, the slithery language 

 author Sam Moore (matches) [SZM]
 website http://matches.ucc.asn.au/stratego
 email progcomp@ucc.asn.au or matches@ucc.asn.au
 git git.ucc.asn.au/progcomp2012.git
'''

import sys
import random

ranks = ('B','1','2','3','4','5','6','7','8','9','s','F', '?', '+')

def move(x, y, direction):
	if direction == "UP":
		return (x,y-1)
	elif direction == "DOWN":
		return (x,y+1)
	elif direction == "LEFT":
		return (x-1, y)
	elif direction == "RIGHT":
		return (x+1, y)
	print "Error in move!"
	return (x,y)

def oppositeColour(colour):
	if colour == "RED":
		return "BLUE"
	elif colour == "BLUE":
		return "RED"
	else:
		return "NONE"

class Piece:
	def __init__(self, colour, rank, x, y):
		self.colour = colour
		self.rank = rank
		self.x = x
		self.y = y
	



class Asmodeus:
	def __init__(self):
		#sys.stderr.write("Asmodeus __init__ here...\n");
		self.turn = 0
		self.board = []
		self.units = []
		if self.Setup():
			while self.MoveCycle(): #derp derp derp
				pass
		

	def Setup(self):
		#sys.stderr.write("Asmodeus Setup here...\n");
		setup = sys.stdin.readline().split(' ')
		self.colour = setup[0]
		self.opponentName = setup[1]
		self.width = int(setup[2])
		self.height = int(setup[3])
		for x in range(0, self.width):
			self.board.append([])
			for y in range(0, self.height):		
				self.board[x].append(None)
		if self.colour == "RED":
			print "FB8sB479B8\nBB31555583\n6724898974\n967B669999"
		elif self.colour == "BLUE":
			print "967B669999\n6724898974\nBB31555583\nFB8sB479B8"
		return True

	def MoveCycle(self):
		#sys.stderr.write("Asmodeus MakeMove here...\n");
		if self.InterpretResult() == False or self.ReadBoard() == False or self.MakeMove() == False:
			return False
		self.turn += 1
		return self.InterpretResult()

	def MakeMove(self):
		#TODO: Over-ride this function in base classes with more complex move behaviour

		#sys.stderr.write("Asmodeus MakeMove here...\n")
		#self.debugPrintBoard()
		while True:
			if len(self.units) <= 0:
				return False
			piece = random.choice(self.units)
			if piece == None:
				continue
			if piece.rank == '?' or piece.rank == 'B' or piece.rank == 'F':
				continue

			direction = random.choice(("UP", "DOWN", "LEFT", "RIGHT"))
			p = move(piece.x, piece.y, direction)
			if p[0] >= 0 and p[0] < self.width and p[1] >= 0 and p[1] < self.height:
				if self.board[p[0]][p[1]] == None or self.board[p[0]][p[1]].colour == oppositeColour(self.colour):
					print str(piece.x) + " " + str(piece.y) + " " + direction
					break
		return True
							
			
	def ReadBoard(self):
		#sys.stderr.write("Asmodeus ReadBoard here...\n");
		for y in range(0,self.height):
			row = sys.stdin.readline()
			for x in range(0,len(row)-1):
				if self.turn == 0:
					if row[x] == '.':
						pass
					elif row[x] == '#':
						self.board[x][y] = Piece(oppositeColour(self.colour), '?',x,y)
					elif row[x] == '+':
						self.board[x][y] = Piece("NONE", '+', x, y)
					else:
						self.board[x][y] = Piece(self.colour, row[x],x,y)
						self.units.append(self.board[x][y])
				else:
					pass
		return True
		

	def InterpretResult(self):
		#sys.stderr.write("Asmodeus InterpretResult here...\n")
		result = sys.stdin.readline().split(' ')
		#sys.stderr.write("	Read status line \"" + str(result) + "\"\n")
		if self.turn == 0:
			return True

		x = int(result[0].strip())
		y = int(result[1].strip())


		#sys.stderr.write("	Board position " + str(x) + " " + str(y) + " is OK!\n")		

		direction = result[2].strip()
		outcome = result[3].strip()
		
		p = move(x,y,direction)

		if outcome == "OK":
			self.board[p[0]][p[1]] = self.board[x][y]
			self.board[x][y].x = p[0]
			self.board[x][y].y = p[1]

			self.board[x][y] = None
		elif outcome == "KILLS":
			if self.board[p[0]][p[1]] == None:
				return False

			if self.board[p[0]][p[1]].colour == self.colour:
				self.units.remove(self.board[p[0]][p[1]])
			self.board[x][y].x = p[0]
			self.board[x][y].y = p[1]


			self.board[p[0]][p[1]] = self.board[x][y]
			self.board[x][y].rank = result[4]

			self.board[x][y] = None
			
		elif outcome == "DIES":
			if self.board[p[0]][p[1]] == None:
				return False

			if self.board[x][y].colour == self.colour:
				self.units.remove(self.board[x][y])

			self.board[p[0]][p[1]].rank = result[5]
			self.board[x][y] = None
		elif outcome == "BOTHDIE":
			if self.board[p[0]][p[1]] == None:
				return False


			if self.board[x][y].colour == self.colour:
				self.units.remove(self.board[x][y])
			if self.board[p[0]][p[1]].colour == self.colour:
				self.units.remove(self.board[p[0]][p[1]])
			self.board[p[0]][p[1]] = None
			self.board[x][y] = None
		elif outcome == "FLAG":
			#sys.stderr.write("	Game over!\n")
			return False
		elif outcome == "ILLEGAL":
			#sys.stderr.write("	Illegal move!\n")
			return False
		else:
			#sys.stderr.write("	Don't understand outcome \"" + outcome + "\"!\n");
			return False

		#sys.stderr.write("	Completed interpreting move!\n");		
		return True

	def debugPrintBoard(self):
		for y in range(0, self.height):
			for x in range(0, self.width):
				if self.board[x][y] == None:
					sys.stderr.write(".");
				else:
					sys.stderr.write(str(self.board[x][y].rank));
			sys.stderr.write("\n")
				
			

asmodeus = Asmodeus()

