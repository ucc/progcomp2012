#ifndef STRATEGO_H
#define STRATEGO_H

#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

#ifdef GRAPHICS
	#include "graphics.h"
	#include "array.h"
#endif //GRAPHICS

/**
 * Contains classes for a game of Stratego
 */


/**
 * Class for a game piece
 */
class Piece
{
	public:
		typedef enum {ERROR=14,BOMB=13,MARSHAL=12, GENERAL=11, COLONEL=10, MAJOR=9, CAPTAIN=8, LIEUTENANT=7, SERGEANT=6, MINER=5, SCOUT=4, SPY=3, FLAG=2,BOULDER=1, NOTHING=0} Type; //Type basically defines how strong the piece is
		

		typedef enum {RED=0, BLUE=1, NONE=2, BOTH=3} Colour; //Used for the allegiance of the pieces - terrain counts as NONE.

		Piece(const Type & newType, const Colour & newColour) : type(newType), colour(newColour) {}
		virtual ~Piece() {}


		//Operators compare the strength of two pieces
		bool operator==(const Piece & equ) const {return type == equ.type;}
		bool operator<(const Piece & equ) const {return type < equ.type;}
		bool operator>(const Piece & equ) const {return type > equ.type;}
		
		bool operator!=(const Piece & equ) const {return !operator==(equ);}
		bool operator<=(const Piece & equ) const {return (operator<(equ) || operator==(equ));}
		bool operator>=(const Piece & equ) const {return (operator>(equ) || operator==(equ));}

		//Contains the characters used to identify piece types when the board is printed to a stream
		static  char tokens[];
		static int maxUnits[];

		static Type GetType(char fromToken);


		//Attributes of the piece
		const Type type; 
		const Colour colour;

	#ifdef GRAPHICS
		public:

			class TextureManager : public Graphics::TextureManager<LUint>, private Array<Texture*>
			{
				public:
					TextureManager() : Graphics::TextureManager<LUint>(), Array<Texture*>() {}
					virtual ~TextureManager();

					virtual Texture & operator[](const LUint & at);
			};
			static TextureManager textures;

			static Graphics::Colour GetGraphicsColour(const Piece::Colour & colour)
			{
				switch (colour)
				{
					case RED:
						return Graphics::Colour(1,0,0);
						break;
					case BLUE:
						return Graphics::Colour(0,0,1);
						break;
					case NONE:
						return Graphics::Colour(0.5,0.5,0.5);
						break;
					case BOTH:
						return Graphics::Colour(1,0,1);
						break;
				}
			}

			
			
	#endif //GRAPHICS
		
};

#include "movementresult.h"

/**
 * A Stratego board
 */
class Board
{
	public:
		Board(int newWidth, int newHeight); //Constructor

		virtual ~Board(); //Destructor

		void Print(FILE * stream, const Piece::Colour & reveal=Piece::BOTH); //Print board
		#ifdef GRAPHICS
			void Draw(const Piece::Colour & reveal=Piece::BOTH); //Draw board
		#endif //GRAPHICS

		bool AddPiece(int x, int y, const Piece::Type & newType, const Piece::Colour & newColour); //Add piece to board


		Piece * GetPiece(int x, int y); //Retrieve piece from a location on the board


		typedef enum {UP, DOWN, LEFT, RIGHT} Direction;

		
		
		static bool LegalResult(const MovementResult & result)
		{
			return (result == MovementResult::OK || result == MovementResult::DIES || result == MovementResult::KILLS || result == MovementResult::BOTH_DIE);
		}	

		MovementResult MovePiece(int x, int y, const Direction & direction, int multiplier=1,const Piece::Colour & colour=Piece::NONE); //Move piece from position in direction
		static Board theBoard;

		Piece::Colour winner;

		int Width() const {return width;}
		int Height() const {return height;}
	private:
		int width;
		int height;
		Piece ** * board;
};

#endif //STRATEGO_H

//EOF


