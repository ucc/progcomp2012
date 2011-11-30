#include "forfax.h"

#include <cstdlib>

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <list>

#include <cmath>

using namespace std;

/**
 * Static variables
 */

//nothing, boulder, flag, spy, scout, miner, sergeant, lietenant, captain, major, colonel, general, marshal, bomb, error
char  Piece::tokens[] = {'.','+','F','y','s','n','S','L','c','m','C','G','M','B','?'};
int Piece::maxUnits[] = {0,0,1,1,8,5,4,4,4,3,2,1,1,6,0};


int Board::redUnits[] = {0,0,1,1,8,5,4,4,4,3,2,1,1,6,0};
int Board::blueUnits[] = {0,0,1,1,8,5,4,4,4,3,2,1,1,6,0};


/**
 * Constructor for a piece
 * @param newX - x coord
 * @param newY - y coord
 * @param newColour - colour
 */
Piece::Piece(int newX, int newY,const Colour & newColour)
	: x(newX), y(newY), colour(newColour), lastMove(0)
{
	minRank[RED] = Piece::FLAG;
	minRank[BLUE] = Piece::FLAG;
	maxRank[RED] = Piece::BOMB;
	maxRank[BLUE] = Piece::BOMB;
}

/**
 * Constructor for a piece
 * @param newX - x coord
 * @param newY - y coord
 * @param newColour - colour
 * @param rankKnownBy - Colour that knows the piece's rank
 * @param fixedRank - Rank the piece has
 */
Piece::Piece(int newX, int newY,const Colour & newColour, const Colour & rankKnownBy, const Type & fixedRank)
	: x(newX), y(newY), colour(newColour), lastMove(0)
{
	if (rankKnownBy == BOTH)
	{
		minRank[RED] = fixedRank;
		minRank[BLUE] = fixedRank;
		maxRank[RED] = fixedRank;
		maxRank[BLUE] = fixedRank;
	}
	else
	{
		minRank[rankKnownBy] = fixedRank;
		maxRank[rankKnownBy] = fixedRank;

		Colour opposite = Opposite(rankKnownBy);
		minRank[opposite] = Piece::FLAG;
		maxRank[opposite] = Piece::BOMB;

	}
	
}






/**
 * Returns the Piece::Type matching a given character
 * @param token - The character to match
 * @returns A Piece::Type corresponding to the character, or Piece::ERROR if none was found
 */
Piece::Type Piece::GetType(char token)
{
	for (int ii=0; ii < Piece::ERROR; ++ii)
	{
		if (Piece::tokens[ii] == token)
			return (Type)(ii);
	}
	return Piece::ERROR;
}

/**
 * Constructor for the board
 * @param newWidth - width of the board
 * @param newHeight - height of the board
 *
 */
Board::Board(int newWidth, int newHeight) : width(newWidth), height(newHeight), board(NULL), red(), blue()
{
	board = new Piece**[width];
	for (int x=0; x < width; ++x)
	{
		board[x] = new Piece*[height];
		for (int y=0; y < height; ++y)
			board[x][y] = NULL;
	}
}

/**
 * Destroy the board
 */
Board::~Board()
{
	for (int x=0; x < width; ++x)
	{
		for (int y=0; y < height; ++y)
			delete board[x][y];
		delete [] board[x];
	}
}

/**
 * Retrieve a piece from the board
 * @param x - x coord of the piece
 * @param y - y coord of the piece
 * @returns Piece* to the piece found at (x,y), or NULL if there was no piece, or the coords were invalid
 */
Piece * Board::Get(int x, int y) const
{
	if (board == NULL || x < 0 || y < 0 || x > width || y > height)
		return NULL;
	return board[x][y];
}

/**
 * Add a piece to the board
 *	Also updates the red or blue arrays if necessary
 * @param x - x coord of the piece
 * @param y - y coord of the piece
 * @param newPiece - pointer to the piece to add
 * @returns newPiece if the piece was successfully added, NULL if it was not
 *
 */
Piece * Board::Set(int x, int y, Piece * newPiece)
{
	if (board == NULL || x < 0 || y < 0 || x > width || y > height)
		return NULL;
	board[x][y] = newPiece;

	//if (newPiece->GetColour() == Piece::RED)
	//	red.push_back(newPiece);
	//else if (newPiece->GetColour() == Piece::BLUE)
	//	blue.push_back(newPiece);

	return newPiece;
}


/**
 * Convert a string to a direction
 * @param str - The string to convert to a direction
 * @returns The equivelent Direction
 */
Board::Direction Board::StrToDir(const string & str)
{
	if (str == "UP")
		return UP;
	else if (str == "DOWN")
		return DOWN;
	else if (str == "LEFT")
		return LEFT;
	else if (str == "RIGHT")
		return RIGHT;

	return NONE;
}

/**
 * Convert a Direction to a string
 * @param dir - the Direction to convert
 * @param str - A buffer string 
 */
void Board::DirToStr(const Direction & dir, string & str)
{
	str.clear();
	switch (dir)
	{
		case UP:
			str = "UP";
			break;
		case DOWN:
			str = "DOWN";
			break;
		case LEFT:
			str = "LEFT";
			break;
		case RIGHT:
			str = "RIGHT";
			break;
		default:
			str = "NONE";
			break;
	}
}

/**
 * Moves the co-ords in the specified direction
 * @param x - x coord
 * @param y - y coord
 * @param dir - Direction to move in
 * @param multiplier - Number of times to move
 *
 */
void Board::MoveInDirection(int & x, int & y, const Direction & dir, int multiplier)
{
	switch (dir)
	{
		case UP:
			y -= multiplier;
			break;
		case DOWN:
			y += multiplier;
			break;
		case LEFT:
			x -= multiplier;
			break;
		case RIGHT:
			x += multiplier;
			break;
		default:
			break;
	}
}

/**
 * Returns the best direction to move in to get from one point to another
 * @param x1 - x coord of point 1
 * @param y1 - y coord of point 1
 * @param x2 - x coord of point 2
 * @param y2 - y coord of point 2
 * @returns The best direction to move in
 */
Board::Direction Board::DirectionBetween(int x1, int y1, int x2, int y2)
{


	double xDist = (x2 - x1);
	double yDist = (y2 - y1);
	if (abs(xDist) >= abs(yDist))
	{
		if (xDist < 0)
			return LEFT;
		else 
			return RIGHT;
	}
	else
	{
		if (yDist < 0)
			return UP;
		else
			return DOWN;
	}
	return NONE;

	

	
}

/**
 * Searches the board's red and blue arrays for the piece, and removes it
 * DOES NOT delete the piece. Calling function should delete piece after calling this function.
 * @param forget - The Piece to forget about
 * @returns true if the piece was actually found
 */
bool Board::ForgetPiece(Piece * forget)
{	
	if (forget == NULL)
		return false;
	
	vector<Piece*> & in = GetPieces(forget->colour); bool result = false;
	for (vector<Piece*>::iterator i=in.begin(); i != in.end(); ++i)
	{
		
		if ((*i) == forget)
		{
			i = in.erase(i);
			result = true;
			
			continue;
		}
		
		
	}

	
	return result;
}

/**
 * Construct Forfax
 */
Forfax::Forfax() : board(NULL), colour(Piece::NONE), strColour("NONE"), turnNumber(0)
{
	for (int ii=0; ii <= Piece::BOMB; ++ii)
	{
		remainingUnits[ii][Piece::RED][Piece::RED] = Piece::maxUnits[ii];
		remainingUnits[ii][Piece::RED][Piece::BLUE] = Piece::maxUnits[ii];
		remainingUnits[ii][Piece::BLUE][Piece::RED] = Piece::maxUnits[ii];
		remainingUnits[ii][Piece::BLUE][Piece::BLUE] = Piece::maxUnits[ii];


	}
}

/**
 * Destroy Forfax
 */
Forfax::~Forfax()
{
	//fprintf(stderr,"Curse you mortal for casting me into the fires of hell!\n");
	//Errr...
	if (board != NULL)
		delete board;
}

/**
 * Calculate the probability that attacker beats defender in combat, from the point of view of a certain player
 */

double Forfax::CombatSuccessChance(Piece * attacker, Piece * defender, const Piece::Colour & accordingTo) const
{
	double probability=1;
	for (Piece::Type aRank = attacker->minRank[accordingTo]; aRank <= attacker->maxRank[accordingTo]; aRank = (Piece::Type)((int)(aRank) + 1))
	{
		double lesserRanks; double greaterRanks;
		for (Piece::Type dRank = defender->minRank[accordingTo]; dRank <= defender->maxRank[accordingTo]; dRank = (Piece::Type)((int)(dRank) + 1))
		{
			if (dRank < aRank)
				lesserRanks++;
			else if (dRank > aRank)
				greaterRanks++;
			else
			{
				lesserRanks++; greaterRanks++;
			}
		}
		probability *= lesserRanks/(lesserRanks + greaterRanks);
	}
	return probability;
}

/**
 * Calculate the base score of a move
 * @param piece - The Piece to move
 * @param dir - The direction in which to move
 * @param accordingTo - the colour to use for assumptions
 */
double Forfax::MovementBaseScore(Piece * piece, const Board::Direction & dir, const Piece::Colour & accordingTo) const
{
	int x2 = piece->x; int y2 = piece->y;
	Board::MoveInDirection(x2, y2, dir);

	if (board->Get(x2, y2) == NULL)
		return 1;
	else if (board->Get(x2, y2)->colour == piece->colour)
		return 0;
	else 
		return CombatSuccessChance(piece, board->Get(x2, y2), accordingTo);
}

/**
 * Calculate the total score of a move according to certain colour
 * @param piece - the piece to move
 * @param dir - the direction to move in
 * @param accordingTo - the colour to use
 */
double Forfax::MovementTotalScore(Piece * piece, const Board::Direction & dir, const Piece::Colour & accordingTo) const
{
	double base = MovementBaseScore(piece, dir, accordingTo);

	if (base == 0)
		return base;


	int x = piece->x; int y = piece->y;
	Board::MoveInDirection(x, y, dir);
	Piece * old = board->Get(x, y);
	board->Set(x, y, piece);
	board->Set(piece->x, piece->y, NULL);

	list<MovementChoice> opponentMoves;
	vector<Piece*> & enemies = board->GetPieces(Piece::Opposite(accordingTo));
	for (vector<Piece*>::iterator i = enemies.begin(); i != enemies.end(); ++i)
	{
		opponentMoves.push_back(MovementChoice((*i), Board::UP, *this,Piece::Opposite(accordingTo)));
		opponentMoves.push_back(MovementChoice((*i), Board::DOWN, *this,Piece::Opposite(accordingTo)));
		opponentMoves.push_back(MovementChoice((*i), Board::LEFT, *this,Piece::Opposite(accordingTo)));
		opponentMoves.push_back(MovementChoice((*i), Board::RIGHT, *this,Piece::Opposite(accordingTo)));
	}

	opponentMoves.sort();


	
	MovementChoice & best = opponentMoves.back();

	board->Set(x, y, old);
	board->Set(piece->x, piece->y, piece);

	return base / best.score;
	


}



/**
 * Forfax sets himself up
 * Really should just make input and output stdin and stdout respectively, but whatever
 */
bool Forfax::Setup()
{
	//The first line is used to identify Forfax's colour, and the size of the board
	//Currently the name of the opponent is ignored
		
	strColour.clear();
	string strOpponent; int boardWidth; int boardHeight;

	cin >> strColour; cin >> strOpponent; cin >> boardWidth; cin >> boardHeight;
	if (cin.get() != '\n')
		return false;
	
	if (strColour == "RED")
	{
		colour = Piece::RED;
		cout <<  "FB..........B.\n";
		cout << "BBCM....cccc.C\n";
		cout << "LSGmnsBmSsnsSm\n";
		cout << "sLSBLnLssssnyn\n";
	}
	else if (strColour == "BLUE")
	{
		colour = Piece::BLUE;
		cout << "sLSBLnLssssnyn\n";
		cout << "LSGmnsBmSsnsSm\n";
		cout << "BBCM....cccc.C\n";
		cout <<  "FB..........B.\n";
		
		
		

	}
	else
		return false;



	board = new Board(boardWidth, boardHeight);
	return (board != NULL);
}

/**
 * Forfax makes a move
 *
 */
bool Forfax::MakeMove()
{
	++turnNumber;
	cerr << "Forfax " << strColour << " making move number " << turnNumber << "...\n";
	if (turnNumber == 1)
	{
		if (!MakeFirstMove())
		{
			return false;
		}
		
	}
	else
	{
		if (!InterpretMove())
		{
			
			return false;
		}


		//Forfax ignores the board state; he only uses the move result lines
		for (int y=0; y < board->Height(); ++y)
		{
			for (int x = 0; x < board->Width(); ++x)
				cin.get();
			if (cin.get() != '\n')
				return false;
		}
	}
	
	//Make move here

	list<MovementTotalChoice> choices;
	vector<Piece*> & allies = board->GetPieces(colour);
	for (vector<Piece*>::iterator i = allies.begin(); i != allies.end(); ++i)
	{
		choices.push_back(MovementTotalChoice((*i), Board::UP, *this, colour));
		choices.push_back(MovementTotalChoice((*i), Board::DOWN, *this, colour));
		choices.push_back(MovementTotalChoice((*i), Board::LEFT, *this, colour));
		choices.push_back(MovementTotalChoice((*i), Board::RIGHT, *this, colour));

	}

	MovementTotalChoice & choice = choices.back();
	
	string direction; Board::DirToStr(choice.dir, direction);
	cerr << "Forfax %s computed optimum move of " << choice.piece->x << " " << choice.piece->y << " " << direction << " [score=" << choice.score << "]\n";
	cout << choice.piece->x << " " << choice.piece->y << " " << direction << "\n";


	return InterpretMove();
	

}

bool Forfax::InterpretMove()
{
	int x; int y; string direction; string result; int multiplier = 1; int attackerVal = (int)(Piece::BOMB); int defenderVal = (int)(Piece::BOMB);

	cerr << "Forfax " << strColour << " waiting for movement information...\n";
	cin >> x; cin >> y; cin >> direction; cin >> result;
	if (cin.peek() != '\n')
	{
		cerr << "Forfax " << strColour << " reading multiplier\n";
		stringstream s(result);
		s >> multiplier;
		result.clear();
		cin >> result;

		if (cin.peek() != '\n')
		{
			cerr << "Forfax " << strColour << " reading ranks of pieces\n";
			s.clear(); s.str(result);
			s >> attackerVal;
			result.clear();
			cin >> result;	
			s.clear(); s.str(result);
			s >> defenderVal;
			result.clear();

			cin >> result;
		}
	}
	if (cin.get() != '\n')
	{
		cerr << "Forfax " << strColour << " didn't recieve new line. Very angry.\n";
		cerr << "Read result so far: " << x << " " << y <<  " " << direction << " " << result << " ...\n";
		return false;
	}

	Piece::Type attackerRank = Piece::Type(Piece::BOMB - attackerVal);
	Piece::Type defenderRank = Piece::Type(Piece::BOMB - defenderVal);

	cerr << "Forfax " << strColour << " interpreting movement result of " << x << " " << y <<  " " << direction << " " << result << " ...\n";


	int x2 = x; int y2 = y;
	Board::Direction dir = Board::StrToDir(direction);

	Board::MoveInDirection(x2, y2, dir, multiplier);

	Piece * attacker = board->Get(x, y);
	Piece * defender = board->Get(x2, y2);

	if (attacker == NULL)
		return false;


	Piece::Colour oppositeColour = Piece::Opposite(attacker->colour);
	if (attacker->minRank[oppositeColour] == Piece::FLAG)
		attacker->minRank[oppositeColour] = Piece::SPY;
	if (attacker->maxRank[oppositeColour] == Piece::BOMB)
		attacker->maxRank[oppositeColour] = Piece::MARSHAL;
	if (multiplier > 1)
	{
		attacker->maxRank[oppositeColour] = Piece::SCOUT;
		attacker->minRank[oppositeColour] = Piece::SCOUT;
	}





	if (result == "OK")
	{
		if (defender != NULL)
			return false;
		board->Set(x2, y2, attacker);
		board->Set(x, y, NULL);
		attacker->x = x2;
		attacker->y = y2;
	}
	else if (result == "KILLS")
	{
		if (defender == NULL || defender->colour == attacker->colour)
			return false;


		

		board->Set(x2, y2, attacker);
		board->Set(x, y, NULL);
		attacker->x = x2;
		attacker->y = y2;

		if (attacker->minRank[oppositeColour] < defender->maxRank[oppositeColour])
			attacker->minRank[oppositeColour] = defender->maxRank[oppositeColour];
		

		if (!board->ForgetPiece(defender))
			return false;
		delete defender;

	}
	else if (result == "DIES")
	{
		
		if (defender == NULL || defender->colour == attacker->colour)
			return false;
cerr << "Forfax - Unit " << attacker << " dies \n";
		if (!board->ForgetPiece(attacker))
			return false;
		delete attacker;

		board->Set(x, y, NULL);
	}
	else if (result == "BOTHDIE")
	{
		if (defender == NULL || defender->colour == attacker->colour)
			return false;
		if (board->ForgetPiece(attacker) == false)
			return false;
		if (board->ForgetPiece(defender) == false)
			return false;
		delete attacker;
		delete defender;
		board->Set(x2, y2, NULL);
		board->Set(x, y, NULL);
	}
	else if (result == "VICTORY")
	{
		return false;
	}
	return true;
}

/**
 * First move only
 *
 */
bool Forfax::MakeFirstMove()
{
	if (colour == Piece::RED)
	{
		string derp;
		cin >> derp;
		if (derp != "START")
			return false;
		if (cin.get() != '\n')
			return false;
	}
	else
	{
		//TODO: Fix hack where BLUE ignores RED's first move
		while (cin.get() != '\n');
	}
	
	for (int y=0; y < board->Height(); ++y)
	{
		for (int x = 0; x < board->Width(); ++x)	
		{
			char c = cin.get();
			switch (c)
			{
				case '.':
					break;
				case '+':
					board->Set(x, y, new Piece(x, y, Piece::NONE, Piece::BOTH, Piece::BOULDER));
					break;
				case '#':
				case '*':
				{
					Piece * toAdd = new Piece(x, y, Piece::Opposite(colour));
					board->Set(x, y, toAdd);
					board->GetPieces(toAdd->colour).push_back(toAdd);
					break;
				}
				default:
				{
					Piece::Type type = Piece::GetType(c);
					Piece * toAdd = new Piece(x, y, colour, colour, type);
					board->Set(x, y, toAdd);
					board->GetPieces(toAdd->colour).push_back(toAdd);
					break;
				}
			}
		}
		if (cin.get() != '\n')
			return false;
	}
	
	return true;
}

//EOF

