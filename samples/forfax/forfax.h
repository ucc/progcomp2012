/**
 * "forfax", a sample Stratego AI for the UCC Programming Competition 2012
 * Declarations for classes Piece, Board and Forfax, Declaration/Implementation of helper class MovementChoice
 * @author Sam Moore (matches) [SZM]
 * @website http://matches.ucc.asn.au/stratego
 * @email progcomp@ucc.asn.au or matches@ucc.asn.au
 * @git git.ucc.asn.au/progcomp2012.git
 */

#ifndef FORFAX_H
#define FORFAX_H

#include <vector> //Uses C++ std::vectors to store pieces
#include <string> //Uses C++ std::string
#include <iostream> //For debug
#include <cassert> //For debug



class Board;	//Forward declaration used by class Piece
	
/**
 * Class to represent a piece on the board
 */
class Piece
{
	public:
		typedef enum {ERROR=14,BOMB=13,MARSHAL=12, GENERAL=11, COLONEL=10, MAJOR=9, CAPTAIN=8, LIEUTENANT=7, SERGEANT=6, MINER=5, SCOUT=4, SPY=3, FLAG=2,BOULDER=1, NOTHING=0} Type; //Type basically defines how strong the piece is
		typedef enum {RED=0, BLUE=1, NONE, BOTH} Colour; //Used for the allegiance of the pieces - terrain counts as NONE.

		Piece(int newX, int newY,const Colour & newColour);
		Piece(int newX, int newY,const Colour & newColour, const Type & fixedRank);
		virtual ~Piece() {}
		
		void SetCoords(int newX, int newY) {x = newX; y = newY;}
		void GetCoords(int & storeX, int & storeY) const {storeX = x; storeY = y;}
		const Colour & GetColour() const {return colour;}

		static  char tokens[]; //The tokens used to identify various pieces
		static int maxUnits[]; //The maximum allowed number of units of each piece

		static Type GetType(char fromToken); //Retrieves the type of a piece given its character token
		static Colour Opposite(const Colour & colour) {return colour == RED ? BLUE : RED;}
		bool Mobile() const
		{
			if (minRank == maxRank)
				return (minRank != Piece::FLAG && minRank != Piece::BOMB);
			else
				return true;
		}

		int x; int y;
		const Colour colour; //The colour of the piece
		Type minRank; //The minimum possible rank of the piece
		Type maxRank; //The maximum possible rank of the piece
		int lastMove;
		int lastx; int lasty;
		
		

};

/**
 * Class to represent a board
 */
class Board
{
	public:
		Board(int width, int height);
		virtual ~Board();


		std::vector<Piece*> & GetPieces(const Piece::Colour & colour) {return colour == Piece::RED ? red : blue;} //retrieve array of pieces
		
		Piece * Get(int x, int y) const; //Retrieve single piece
		Piece * GetClosest(int x, int y, const Piece::Colour & search = Piece::BOTH) const; //Retrieve closest piece of specified colour to the point
		Piece *  Set(int x, int y, Piece * newPiece); //Add piece to board

		bool ValidPosition(int x, int y) const {return (x > 0 && x < width && y > 0 && y < height);}

		int Width() const {return width;}
		int Height() const {return height;}

		typedef enum {UP=0, DOWN=1, LEFT=2, RIGHT=3, NONE=4} Direction;
		static Direction StrToDir(const std::string & str);
		static void DirToStr(const Direction & dir, std::string & buffer);

		static void MoveInDirection(int & x, int & y, const Direction & dir, int multiplier = 1);
		static Direction DirectionBetween(int x1, int y1, int x2, int y2);
		static int NumberOfMoves(int x1, int y1, int x2, int y2);
		

		static int redUnits[];
		static int blueUnits[];
	

		bool ForgetPiece(Piece * forget); //removes piece from the red and blue vectors

	private:
		friend class Forfax;

		int width;
		int height;
		Piece ** * board;

		std::vector<Piece*> red; //Store all red pieces
		std::vector<Piece*> blue; //Store all blue pieces
	

};

/** 
 * Class to manage the Forfax AI
 */
class Forfax
{
	public:
		Forfax();
		virtual ~Forfax();

		typedef enum {OK, NO_NEWLINE, EXPECTED_ATTACKER, UNEXPECTED_DEFENDER, NO_ATTACKER, NO_DEFENDER, COLOUR_MISMATCH, INVALID_QUERY, BOARD_ERROR, VICTORY} Status;

		Status Setup(); //Waits for input to determine colour and board size, and then responds with setup
		Status MakeMove(); //Should be called each turn - determines Forfax's move


		//Move score functions
		double MovementScore(Piece * move, const Board::Direction & dir) const; //Calculate total score
		double CombatSuccessChance(Piece * attacker, Piece * defender) const; //Calculate chance of success in combat
		double CombatScore(int x, int y, Piece * attacker) const; //Calculate total worth of combat at a point
		double IntrinsicWorth(int x, int y) const; //How much a given point on the board is worth
		double VictoryScore(Piece * attacker, Piece * defender) const; //How much killing the defender is worth
		double DefeatScore(Piece * attacker, Piece * defender) const; //How much losing is worth


		void PrintBoard(std::ostream & out);

	protected:
		Status MakeFirstMove(); //Should only be called on the first turn
		Status InterpretMove();
	private:
		Board * board; //Forfax stores the state on a board
		Piece::Colour colour; //Forfax needs to know his colour
		std::string strColour; //String of colour
		int turnNumber; //Forfax needs to know what turn number it is
		
		static int remainingUnits[2][15]; //Known remaining units, accessed by [colour][type]

};

/**
 * Helper class used to store various moves in the board, and their associated scores
 */
class MovementChoice
{
	public:
		MovementChoice(Piece * newPiece, const Board::Direction & newDir, const Forfax & forfax) : piece(newPiece), dir(newDir) 
		{
			score = forfax.MovementScore(piece, dir);
		}

		MovementChoice(const MovementChoice & cpy) : piece(cpy.piece), dir(cpy.dir), score(cpy.score)
		{

		}
		
		bool operator<(const MovementChoice & a) const {return score < a.score;}
		bool operator>(const MovementChoice & a) const {return score > a.score;}
		bool operator<=(const MovementChoice & a) const {return score <= a.score;}
		bool operator>=(const MovementChoice & a) const {return score >= a.score;}
		bool operator==(const MovementChoice & a) const {return score == a.score;}
		bool operator!=(const MovementChoice & a) const {return score != a.score;}

		Piece * piece;
		Board::Direction dir;
		double score;
				
	
};



#endif //FORFAX_H

//EOF


