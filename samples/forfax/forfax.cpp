/**
 * "forfax", a sample Stratego AI for the UCC Programming Competition 2012
 * Implementations of classes Piece, Board and Forfax
 * @author Sam Moore (matches) [SZM]
 * @website http://matches.ucc.asn.au/stratego
 * @email progcomp@ucc.asn.au or matches@ucc.asn.au
 * @git git.ucc.asn.au/progcomp2012.git
 */

#include "forfax.h"

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <cmath>

//#define DEBUG

using namespace std;




/**
 * The characters used to represent various pieces
 * NOTHING, BOULDER, FLAG, SPY, SCOUT, MINER, SERGEANT, LIETENANT, CAPTAIN, MAJOR, COLONEL, GENERAL, MARSHAL, BOMB, ERROR
 */

char  Piece::tokens[] = {'.','*','F','s','9','8','7','6','5','4','3','2','1','B','?'};


/**
 * The number of units remaining for each colour
 * Accessed by [COLOUR][TYPE]
 * COLOUR: RED, BLUE
 * TYPE: NOTHING, BOULDER, FLAG, SPY, SCOUT, MINER, SERGEANT, LIETENANT, CAPTAIN, MAJOR, COLONEL, GENERAL, MARSHAL, BOMB, ERROR
 */
int Forfax::remainingUnits[][15] = {{0,0,1,1,8,5,4,4,4,3,2,1,1,6,0},{0,0,1,1,8,5,4,4,4,3,2,1,1,6,0}};




/**
 * Constructor for a piece of unknown rank
 * @param newX - x coord
 * @param newY - y coord
 * @param newColour - colour
 */
Piece::Piece(int newX, int newY,const Colour & newColour)
	: x(newX), y(newY), colour(newColour), minRank(Piece::FLAG), maxRank(Piece::BOMB), lastMove(0), lastx(newX), lasty(newY)
{

}

/**
 * Constructor for a piece of known rank
 * @param newX - x coord
 * @param newY - y coord
 * @param newColour - colour
 * @param fixedRank - rank of the new piece
 */
Piece::Piece(int newX, int newY,const Colour & newColour, const Type & fixedRank)
	: x(newX), y(newY), colour(newColour), minRank(fixedRank), maxRank(fixedRank), lastMove(0),  lastx(newX), lasty(newY)
{
	
	
}






/**
 * HELPER - Returns the Piece::Type matching a given character
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
	//Construct 2D array of Piece*'s
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
	//Destroy the 2D array of Piece*'s
	for (int x=0; x < width; ++x)
	{
		for (int y=0; y < height; ++y)
			delete board[x][y];
		delete [] board[x];
	}
}

/**
 * Retrieve a piece from the board at specified coordinates
 * @param x - x coord of the piece
 * @param y - y coord of the piece
 * @returns Piece* to the piece found at (x,y), or NULL if there was no piece, or the coords were invalid
 */
Piece * Board::Get(int x, int y) const
{
	if (board == NULL || x < 0 || y < 0 || x >= width || y >= height)
		return NULL;
	return board[x][y];
}

/**
 * Add a piece to the board
 *	Also updates the red or blue arrays if necessary
 * @param x - x coord of the piece
 * @param y - y coord of the piece
 * @param newPiece - pointer to the piece to add
 * @returns newPiece if the piece was successfully added, NULL if it was not (ie invalid coordinates specified)
 *
 */
Piece * Board::Set(int x, int y, Piece * newPiece)
{
	if (board == NULL || x < 0 || y < 0 || x >= width || y >= height)
		return NULL;
	board[x][y] = newPiece;

	//if (newPiece->GetColour() == Piece::RED)
	//	red.push_back(newPiece);
	//else if (newPiece->GetColour() == Piece::BLUE)
	//	blue.push_back(newPiece);

	return newPiece;
}


/**
 * HELPER - Convert a string to a direction
 * @param str - The string to convert to a direction
 * @returns The equivalent Direction
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
 * HELPER - Convert a Direction to a string
 * @param dir - the Direction to convert
 * @param str - A buffer string, which will contain the string representation of the Direction once this function returns.
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
 * HELPER - Translates the given coordinates in a specified direction
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
 * HELPER - Returns the best direction to move in to get from one point to another
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
 * HELPER - Returns the number of moves between two points
 * @param x1 x coordinate of the first point
 * @param y1 y coordinate of the first point
 * @param x2 x coordinate of the second point
 * @param y2 y coordinate of the second point
 * @returns The number of moves taken to progress from (x1, y1) to (x2, y2), assuming no obstacles
 */
int Board::NumberOfMoves(int x1, int y1, int x2, int y2)
{
	return (abs(x2 - x1) + abs(y2 - y1)); //Pieces can't move diagonally, so this is pretty straight forward
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
	vector<Piece*>::iterator i=in.begin(); 
	while (i != in.end())
	{
		
		if ((*i) == forget)
		{
			i = in.erase(i);
			result = true;
			
			continue;
		}
		++i;
		
	}

	
	return result;
}

/**
 * Gets the closest Piece of a specified colour to a point
 * @param x The x coordinate of the point
 * @param y The y coordinate of the point
 * @param colour The colour that the piece must match (may be Piece::BOTH to match either colour)
 * @returns Piece* pointing to the closest piece of a matching colour, NULL if no piece found
 */
Piece * Board::GetClosest(int x, int y, const Piece::Colour & colour) const
{
	if (x < 0 || y < 0 || x >= width || y >= height)
		return NULL;

	for (int dist = 0; dist < max<int>(width, height); ++dist)
	{
		
		for (int yy = y-dist; yy <= y+dist; ++yy)
		{
			Piece * get = Get(x+dist, y);
			if ((get != NULL) && (get->colour == colour || colour == Piece::BOTH))
				return get;
		}
		for (int yy = y-dist; yy <= y+dist; ++yy)
		{
			Piece * get = Get(x-dist, y);
			if ((get != NULL) && (get->colour == colour || colour == Piece::BOTH))
				return get;
		}
		for (int xx = x-dist; xx <= x+dist; ++xx)
		{
			Piece * get = Get(xx, y+dist);
			if ((get != NULL) && (get->colour == colour || colour == Piece::BOTH))
				return get;
		}
		for (int xx = x-dist; xx <= y+dist; ++xx)
		{
			Piece * get = Get(xx, y-dist);
			if ((get != NULL) && (get->colour == colour || colour == Piece::BOTH))
				return get;
		}

	}
	
	return NULL;
	
	
	
	
}

/**
 * Construct the Forfax AI
 */
Forfax::Forfax() : board(NULL), colour(Piece::NONE), strColour("NONE"), turnNumber(0)
{
	//By default, Forfax knows nothing; the main function in main.cpp calls Forfax's initialisation functions
	srand(time(NULL));
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
 * Calculate the probability that attacker beats defender in combat
 * @param attacker The attacking piece
 * @param defender The defending piece
 * @returns A double between 0 and 1 indicating the probability of success
 */

double Forfax::CombatSuccessChance(Piece * attacker, Piece * defender) const
{
	double probability=1;
	for (Piece::Type aRank = attacker->minRank; aRank <= attacker->maxRank; aRank = (Piece::Type)((int)(aRank) + 1))
	{
		double lesserRanks=0; double greaterRanks=0;
		for (Piece::Type dRank = defender->minRank; dRank <= defender->maxRank; dRank = (Piece::Type)((int)(dRank) + 1))
		{
			if (dRank < aRank)
				lesserRanks += remainingUnits[defender->colour][(int)(dRank)];
			else if (dRank > aRank)
				greaterRanks += remainingUnits[defender->colour][(int)(dRank)];
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
 * Calculate the score of a move
 * TODO: Alter this to make it better
 * @param piece - The Piece to move
 * @param dir - The direction in which to move
 * @returns a number between 0 and 1, indicating how worthwhile the move is, or a negative number (-1) if the move is illegal
 */
double Forfax::MovementScore(Piece * piece, const Board::Direction & dir) const
{
	assert(piece != NULL);

	
	int x2 = piece->x; int y2 = piece->y;
	Board::MoveInDirection(x2, y2, dir);

	

	double basevalue;
	if (!board->ValidPosition(x2, y2) || !piece->Mobile())
	{
		
		return -1; //No point attempting to move immobile units, or moving off the edges of the board
	}
	else if (board->Get(x2, y2) == NULL)
	{
		//If the position is empty...
		Piece * closestEnemy = board->GetClosest(x2, y2, Piece::Opposite(piece->colour));
		if (closestEnemy == NULL)
		{
			basevalue = 0.05*IntrinsicWorth(x2, y2); //Should never get this unless there are no enemies left
		}
		else
		{
			//Allocate score based on score of Combat with nearest enemy to the target square, decrease with distance between target square and the enemy
			double multiplier = (double)(max<int>(board->Width(), board->Height()) - Board::NumberOfMoves(closestEnemy->x, closestEnemy->y, x2, y2)) / (double)(max<int>(board->Width(), board->Height()));

			basevalue = CombatScore(closestEnemy->x, closestEnemy->y, piece)*multiplier*multiplier;

		}
		
		
	}
	else if (board->Get(x2, y2)->colour != Piece::Opposite(piece->colour))
	{
		return -1; //The position is occupied by an ally, and so its pointless to try and move there
	}
	else 
	{
		basevalue = CombatScore(x2, y2, piece); //The position is occupied by an enemy; compute combat score
		
	}

	if (basevalue > 0)
	{
		//Hack which decreases score for units that moved recently
		//Does not decrease below a threshold (so that at the start of the game units will actually move!)
		double oldValue = basevalue;
		basevalue -= (double)(1.0/((double)(1.0 + (turnNumber - piece->lastMove))));
		if (basevalue < oldValue/1000.0)
			basevalue = oldValue/1000.0;
	}
	

	if (x2 == piece->lastx && y2 == piece->lasty) //Hack which decreases score for backtracking moves
		basevalue = basevalue/100;

	if (rand() % 10 == 0) //Hack which introduces some randomness by boosting one in every 10 scores
		basevalue *= 4;
	if (basevalue > 1)
		basevalue = 1;
	return basevalue;
}


/**
 * Initialisation for Forfax
 * Reads information from stdin about the board, and Forfax's colour. Initialises board, and prints appropriate setup to stdout.
 * @returns true if Forfax was successfully initialised, false otherwise.
 */
Forfax::Status Forfax::Setup()
{
	//The first line is used to identify Forfax's colour, and the size of the board
	//Currently the name of the opponent is ignored.

	//Forfax then responds with a setup.
	//Forfax only uses one of two setups, depending on what colour he was assigned.
	
	
	//Variables to store information read from stdin
	strColour.clear();
	string strOpponent; int boardWidth; int boardHeight;

	cin >> strColour; cin >> strOpponent; cin >> boardWidth; cin >> boardHeight;
	if (cin.get() != '\n')
		return NO_NEWLINE;
	
	//Determine Forfax's colour and respond with an appropriate setup
	if (strColour == "RED")
	{
		colour = Piece::RED;
		cout << "FB8sB479B8\n";
		cout << "BB31555583\n";
		cout << "6724898974\n";
		cout << "967B669999\n";
	}
	else if (strColour == "BLUE")
	{
		colour = Piece::BLUE;
		cout << "967B669999\n";	
		cout << "6724898974\n";
		cout << "BB31555583\n";
		cout << "FB8sB479B8\n";
	}
	else
		return INVALID_QUERY;


	//Create the board
	//NOTE: At this stage, the board is still empty. The board is filled on Forfax's first turn
	//	The reason for this is because the opponent AI has not placed pieces yet, so there is no point adding only half the pieces to the board
	
	board = new Board(boardWidth, boardHeight);
	if (board == NULL)
		return BOARD_ERROR;
	return OK;
}

/**
 * Make a single move
 * 1. Read result of previous move from stdin (or "START" if Forfax is RED and it is the very first move)
 * 2. Read in board state from stdin (NOTE: Unused - all information needed to maintain board state is in 1. and 4.)
 *	TODO: Considering removing this step from the protocol? (It makes debugging annoying because I have to type a lot more!)
 * 3. Print desired move to stdout
 * 4. Read in result of chosen move from stdin
 * @returns true if everything worked, false if there was an error or unexpected query
 */
Forfax::Status Forfax::MakeMove()
{
	++turnNumber;
	
	if (turnNumber == 1)
	{
		Status firstMove = MakeFirstMove();
		if (firstMove != OK)
			return firstMove;
	}
	else
	{
		//Read and interpret the result of the previous move
		Status interpret = InterpretMove();
		if (interpret != OK) {return interpret;}

		//Forfax ignores the board state; he only uses the move result lines
		#ifndef DEBUG
		for (int y=0; y < board->Height(); ++y)
		{
			for (int x = 0; x < board->Width(); ++x)
				cin.get();
			if (cin.get() != '\n')
				return NO_NEWLINE;
		}
		#endif //DEBUG
		
	}
	
	//Now compute the best move
	// 1. Construct list of all possible moves
 	//	As each move is added to the list, a score is calculated for that move. 
	//	WARNING: This is the "tricky" part!
 	// 2. Sort the moves based on their score
	// 3. Simply use the highest scoring move!
	
	list<MovementChoice> choices;
	vector<Piece*> & allies = board->GetPieces(colour);
	for (vector<Piece*>::iterator i = allies.begin(); i != allies.end(); ++i)
	{
		choices.push_back(MovementChoice((*i), Board::UP, *this));
		choices.push_back(MovementChoice((*i), Board::DOWN, *this));
		choices.push_back(MovementChoice((*i), Board::LEFT, *this));
		choices.push_back(MovementChoice((*i), Board::RIGHT, *this));

	}
	
	choices.sort(); //Actually sort the choices!!!
	MovementChoice & choice = choices.back(); //The best one is at the back, because sort() sorts the list in ascending order
	
	

	//Convert information about the move into a printable form
	string direction;  Board::DirToStr(choice.dir, direction);

	//Print chosen move to stdout
	cout << choice.piece->x << " " << choice.piece->y << " " << direction << "\n";

	//cerr << "\nForfax move " << choice.piece->x << " " << choice.piece->y << " " << direction << " [score = " << choice.score << "]\n";



	
	//Interpret the result of the chosen move
	return InterpretMove();

	

}

/**
 * Reads and interprets the result of a move
 * Reads information from stdin
 * @returns true if the result was successfully interpreted, false if there was a contradiction or error
 */
Forfax::Status Forfax::InterpretMove()
{
	//Variables to store move information
	int x; int y; string direction; string result = ""; int multiplier = 1; 
	Piece::Type attackerRank = Piece::NOTHING;
	Piece::Type defenderRank = Piece::NOTHING;

	//Read in information from stdin
	cin >> x; cin >> y; cin >> direction; cin >> result;

	//If necessary, read in the ranks of involved pieces (this happens if the outcome was DIES or KILLS or BOTHDIE)
	if (cin.peek() != '\n')
	{
		string buf = "";		
		stringstream s(buf);
		cin >> buf;
		s.clear(); s.str(buf);
		char temp;
		s >> temp;
		attackerRank = Piece::GetType(temp);

		buf.clear();
		cin >> buf;	
		s.clear(); s.str(buf);
		s >> temp;
		defenderRank = Piece::GetType(temp);

		
	}
	
	//TODO: Deal with move multipliers somehow (when a scout moves more than one space)

	//Check that the line ends where expected...
	if (cin.get() != '\n')
	{
		return NO_NEWLINE;
	}



	



	//Work out the square moved into
	int x2 = x; int y2 = y;
	Board::Direction dir = Board::StrToDir(direction);

	Board::MoveInDirection(x2, y2, dir, multiplier);


	//Determine the attacker and defender (if it exists)
	Piece * attacker = board->Get(x, y);
	Piece * defender = board->Get(x2, y2);


	//If ranks were supplied, update the known ranks of the involved pieces
	if (attackerRank != Piece::NOTHING && attacker != NULL)
	{
		//assert(attacker->minRank <= attackerRank && attacker->maxRank >= attackerRank);
		attacker->minRank = attackerRank;
		attacker->maxRank = attackerRank;
	}
	if (defenderRank != Piece::NOTHING && defender != NULL)
	{
		//assert(defender->minRank <= defenderRank && defender->maxRank >= defenderRank);
		defender->minRank = defenderRank;
		defender->maxRank = defenderRank;

	}

	//There should always be an attacking piece (but not necessarily a defender)
	if (attacker == NULL)
		return EXPECTED_ATTACKER;


	attacker->lastMove = turnNumber; //Update stats of attacking piece (last move)

	//Eliminate certain ranks from the possibilities for the piece based on its movement
	//(This is useful if the piece was an enemy piece)
	if (attacker->minRank == Piece::FLAG)
		attacker->minRank = Piece::SPY;
	if (attacker->maxRank == Piece::BOMB)
		attacker->maxRank = Piece::MARSHAL;
	if (multiplier > 1)
	{
		attacker->maxRank = Piece::SCOUT;
		attacker->minRank = Piece::SCOUT;
	}




	//Now look at the result of the move (I wish you could switch strings in C++)


	//The move was uneventful (attacker moved into empty square)
	if (result == "OK")
	{
		if (defender != NULL)
			return UNEXPECTED_DEFENDER;

		//Update board and piece
		board->Set(x2, y2, attacker);
		board->Set(x, y, NULL);
		attacker->lastx = attacker->x;
		attacker->lasty = attacker->y;
		attacker->x = x2;
		attacker->y = y2;
	}
	else if (result == "KILLS") //The attacking piece killed the defending piece
	{
		if (defender == NULL || defender->colour == attacker->colour)
			return COLOUR_MISMATCH;


		

		board->Set(x2, y2, attacker);
		board->Set(x, y, NULL);
		attacker->lastx = attacker->x;
		attacker->lasty = attacker->y;
		attacker->x = x2;
		attacker->y = y2;

		remainingUnits[(int)(defender->colour)][(int)(defenderRank)]--;
		

		if (!board->ForgetPiece(defender))
			return NO_DEFENDER;
		delete defender;

	}
	else if (result == "DIES") //The attacking piece was killed by the defending piece
	{
		
		if (defender == NULL || defender->colour == attacker->colour)
			return COLOUR_MISMATCH;

		remainingUnits[(int)(attacker->colour)][(int)(attackerRank)]--;

		if (!board->ForgetPiece(attacker))
			return NO_ATTACKER;
		delete attacker;

		board->Set(x, y, NULL);
	}
	else if (result == "BOTHDIE") //Both attacking and defending pieces died
	{
		if (defender == NULL || defender->colour == attacker->colour)
			return COLOUR_MISMATCH;

		remainingUnits[(int)(defender->colour)][(int)(defenderRank)]--;
		remainingUnits[(int)(attacker->colour)][(int)(attackerRank)]--;

		if (board->ForgetPiece(attacker) == false)
			return NO_ATTACKER;
		if (board->ForgetPiece(defender) == false)
			return NO_DEFENDER;
		delete attacker;
		delete defender;
		board->Set(x2, y2, NULL);
		board->Set(x, y, NULL);
	}
	else if (result == "VICTORY") //The attacking piece captured a flag
	{
		return VICTORY; 
		
	}
	return OK;
}

/**
 * Forfax's first move
 * Sets the state of the board
 * @returns true if the board was successfully read, false if an error occurred.
 *
 */
Forfax::Status Forfax::MakeFirstMove()
{
	if (colour == Piece::RED)
	{
		string derp;
		cin >> derp;
		if (derp != "START")
			return INVALID_QUERY;
		if (cin.get() != '\n')
			return NO_NEWLINE;
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
				case '.': //Empty square
					break;
				case '+': //Boulder/Obstacle
					board->Set(x, y, new Piece(x, y, Piece::NONE, Piece::BOULDER));
					break;
				case '#': //Enemy piece occupies square
				case '*':
				{
					Piece * toAdd = new Piece(x, y, Piece::Opposite(colour));
					board->Set(x, y, toAdd);
					board->GetPieces(toAdd->colour).push_back(toAdd);
					break;
				}
				default: //Allied piece occupies square
				{
					Piece::Type type = Piece::GetType(c);
					Piece * toAdd = new Piece(x, y, colour, type);
					board->Set(x, y, toAdd);
					board->GetPieces(toAdd->colour).push_back(toAdd);
					break;
				}
			}
		}
		if (cin.get() != '\n')
			return NO_NEWLINE;
	}
	
	return OK;
}

/**
 * Calculates the intrinsic strategic worth of a point on the board
 * @param x the x coordinate of the point
 * @param y the y coordinate of the point
 * @returns a value between 0 and 1, with 0 indicating worthless and 1 indicating highly desirable
 * (NOTE: No points will actually be worth 0)
 */
double Forfax::IntrinsicWorth(int x, int y) const
{
	static double intrinsicWorth[][10][10] =
	{
		//Red
		{
		{0.1,0.5,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1},
		{0.5,0.5,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1},
		{0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2},
		{0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3},
		{0.6,0.6,0.1,0.1,0.65,0.65,0.1,0.1,0.6,0.6},
		{0.6,0.6,0.1,0.1,0.65,0.65,0.1,0.1,0.6,0.6},
		{0.6,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.6},
		{0.6,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.6},
		{0.6,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.6},
		{0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7}


		},
		//Blue
		{
		{0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7},
		{0.6,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.6},
		{0.6,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.6},
		{0.6,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.6},
		{0.6,0.6,0.1,0.1,0.65,0.65,0.1,0.1,0.6,0.6},
		{0.6,0.6,0.1,0.1,0.65,0.65,0.1,0.1,0.6,0.6},
		{0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3},
		{0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2},
		{0.5,0.5,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1},
		{0.1,0.5,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}
		}
	};

	return intrinsicWorth[(int)(colour)][x][y];
}

/**
 * Calculates a score assuming that attacker will beat defender, indicating how much killing that piece is worth
 * @param attacker the Attacking piece
 * @param defender the Defending piece
 * @returns a value between 0 and 1, with 0 indicating worthless and 1 indicating highly desirable
 */
double Forfax::VictoryScore(Piece * attacker, Piece * defender) const
{

	//If defender's rank is known, flags or bombs are worth more than usual
	if (defender->minRank == defender->maxRank)
	{
		if (defender->minRank == Piece::FLAG)
			return 1;
		else if (defender->minRank == Piece::BOMB)
			return 0.9;
	}
	//Return average of normalised defender ranks
	return max<double>(((defender->maxRank / Piece::BOMB) + (defender->minRank / Piece::BOMB))/2, 1);
}

/**
 * Calculates a score assuming that attacker will lose to defender, indicating how much learning the rank of that piece is worth
 * @param attacker the Attacking piece
 * @param defender the Defending piece
 * @returns a value between 0 and 1, with 0 indicating worthless and 1 indicating highly desirable
 */
double Forfax::DefeatScore(Piece * attacker, Piece * defender) const
{
	
	double result = 0;
	if (defender->minRank == defender->maxRank) //If the defender's rank is known for certain...
	{
		if (defender->minRank == Piece::BOMB) //Committing suicide to destroy bombs has a value that decreases with the attacker's rank
			result = 1 - 0.5*(double)((double)(attacker->minRank) / (double)(Piece::BOMB));
		else if (defender->minRank == Piece::FLAG)
			result = 1; //Its impossible to lose to the flag anyway...
		else
		{
			//This is committing suicide on a higher ranked non-bomb enemy piece.
			//Basically pointless, but the greater the attacker's rank the more pointless!
			double temp = (double)((double)(attacker->minRank) / (double)(Piece::BOMB));
			result = 0.01*(1 - temp)*(1 - temp);

			
		}
	}	
	else //The defender's rank is not known
	{

		//Score is allocated based on how much knowledge is gained by attacking defender
		//The more possible ranks for the defender, the greater the score
		//The score decreases as the rank of the attacker is increased.

		double possibleRanks = 0; double totalRanks = 0; double bonus = 0; 
		for (Piece::Type rank = Piece::NOTHING; rank <= Piece::BOMB; rank = Piece::Type((int)(rank) + 1))
		{
			totalRanks += remainingUnits[(int)(defender->colour)][(int)(rank)];

			if (rank >= defender->minRank && rank <= defender->maxRank)
			{
				possibleRanks += remainingUnits[(int)(defender->colour)][(int)(rank)];
				if (rank == Piece::BOMB)
					bonus += remainingUnits[(int)(defender->colour)][(int)(rank)];
				if (rank == Piece::FLAG)
					bonus += 2*remainingUnits[(int)(defender->colour)][(int)(rank)];
			}
			
		}


		if (totalRanks > 0)
		{
			double multiplier = ((double)(Piece::BOMB) - (double)(attacker->minRank)) / (double)(Piece::BOMB);
			result = (possibleRanks/totalRanks) * multiplier * multiplier;
		}

		result += bonus / totalRanks;
		
		if (result > 1)
			result = 1;
	}

	if (attacker->minRank == Piece::SPY) //Spies are slightly more valuable than usual since they kill the Marshal
		result = result / 1.5;
	return result;

}

/**
 * Calculates a score indicating the worth of invoking combat in a square
 * @param x The x coordinate
 * @param y The y coordinate
 * @param attacker The piece invoking the combat
 * @returns A value between 0 in 1, with 0 indicating worthless (or no combat) and 1 indicating highest value
 */
double Forfax::CombatScore(int x, int y, Piece * attacker) const
{
	Piece * defender = board->Get(x, y);
	if (defender == NULL)
		return 0;
	double combatSuccess = CombatSuccessChance(attacker, defender);
	return IntrinsicWorth(x, y)*combatSuccess*VictoryScore(attacker, defender) + (1.0 - combatSuccess)*DefeatScore(attacker, defender);		
}

/**
 * DEBUG - Print the board seen by Forfax to a stream
 * @param out The stream to print to
 */
void Forfax::PrintBoard(ostream & out)
{
	for (int y = 0; y < board->Height(); ++y)
	{
		for (int x = 0; x < board->Width(); ++x)
		{
			Piece * at = board->Get(x, y);
			if (at == NULL)
				out << ".";
			else
			{
				if (at->colour == colour)
				{
					out << Piece::tokens[(int)(at->minRank)];
				}
				else if (at->colour == Piece::Opposite(colour))
				{
					out << "#";
				}
				else
				{
					out << "+";
				}
			}
		}
		out << "\n";
	}
}

//EOF

