#ifndef FORFAX_H
#define FORFAX_H

#include <vector> //Uses C++ std::vectors to store pieces
#include <string> //Uses C++ std::string
/**
 * Header for the sample Stratego AI "forfax"
 * @author Sam Moore 2011
 */
class Board;		
/**
 * Class to represent a piece on the board
 */
class Piece
{
	public:
		typedef enum {ERROR=14,BOMB=13,MARSHAL=12, GENERAL=11, COLONEL=10, MAJOR=9, CAPTAIN=8, LIEUTENANT=7, SERGEANT=6, MINER=5, SCOUT=4, SPY=3, FLAG=2,BOULDER=1, NOTHING=0} Type; //Type basically defines how strong the piece is
		typedef enum {RED=0, BLUE=1, NONE, BOTH} Colour; //Used for the allegiance of the pieces - terrain counts as NONE.

		Piece(int newX, int newY,const Colour & newColour);
		Piece(int newX, int newY,const Colour & newColour, const Colour & rankKnownBy, const Type & fixedRank);
		virtual ~Piece() {}
		
		void SetCoords(int newX, int newY) {x = newX; y = newY;}
		void GetCoords(int & storeX, int & storeY) const {storeX = x; storeY = y;}
		const Colour & GetColour() const {return colour;}

		static  char tokens[]; //The tokens used to identify various pieces
		static int maxUnits[]; //The maximum allowed number of units of each piece

		static Type GetType(char fromToken); //Retrieves the type of a piece given its character token
		static Colour Opposite(const Colour & colour) {return colour == RED ? BLUE : RED;}


		int x; int y;
		const Colour colour; //The colour of the piece
		Type minRank[2]; //The minimum possible rank of the piece, according to each colour
		Type maxRank[2]; //The maximum possible rank of the piece, according to each colour
		int lastMove;
		
		

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
		Piece *  Set(int x, int y, Piece * newPiece); //Add piece to board

		int Width() const {return width;}
		int Height() const {return height;}

		typedef enum {UP=0, DOWN=1, LEFT=2, RIGHT=3, NONE=4} Direction;
		static Direction StrToDir(const std::string & str);
		static void DirToStr(const Direction & dir, std::string & buffer);

		static void MoveInDirection(int & x, int & y, const Direction & dir, int multiplier = 1);
		static Direction DirectionBetween(int x1, int y1, int x2, int y2);

		

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
 * Small class to manage the Forfax AI
 */
class Forfax
{
	public:
		Forfax();
		virtual ~Forfax();
		bool Setup(); //Waits for input to determine colour and board size, and then responds with setup
		bool MakeMove(); //Should be called each turn - determines Forfax's move

		double CombatSuccessChance(Piece * attacker, Piece * defender, const Piece::Colour & accordingTo) const;
		double MovementBaseScore(Piece * move, const Board::Direction & dir, const Piece::Colour & accordingTo) const;
		double MovementTotalScore(Piece * move, const Board::Direction & dir, const Piece::Colour & accordingTo) const;

	protected:
		bool MakeFirstMove(); //Should only be called on the first turn
		bool InterpretMove();
	private:
		Board * board; //Forfax stores the state on a board
		Piece::Colour colour; //Forfax needs to know his colour
		std::string strColour; //String of colour
		int turnNumber; //Forfax needs to know what turn number it is
		
		int remainingUnits[14][2][2]; //Known remaining units, accessed by (type, colour, accordingTo)

};

class MovementChoice
{
	public:
		MovementChoice(Piece * newPiece, const Board::Direction & newDir, const Forfax & forfax, const Piece::Colour & accordingTo) : piece(newPiece), dir(newDir) 
		{
			score = forfax.MovementBaseScore(piece, dir, accordingTo);
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

class MovementTotalChoice : public MovementChoice
{
	public:
		MovementTotalChoice(Piece * newPiece, const Board::Direction & newDir, const Forfax & forfax, const Piece::Colour & accordingTo) : MovementChoice(newPiece, newDir, forfax, accordingTo)
		{
			score = score/(forfax.MovementTotalScore(piece, dir, Piece::Opposite(accordingTo)));
		}
};

#endif //FORFAX_H

//EOF


