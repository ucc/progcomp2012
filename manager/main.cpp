#include <stdlib.h>
#include <stdio.h>






#include "game.h"

using namespace std;

void CreateGame(int argc, char ** argv);
void DestroyGame();
void PrintResults(const MovementResult & result);

int main(int argc, char ** argv)
{
	


	if (argc == 1)
	{
		fprintf(stderr, "Usage: stratego [options] red blue\n");
		fprintf(stderr, "       stratego --help\n");
		exit(EXIT_SUCCESS);
		
	}
	CreateGame(argc, argv);
	if (Game::theGame == NULL)
	{
		fprintf(stderr, "ERROR: Couldn't create a game!\n");
		exit(EXIT_FAILURE);
	}

	MovementResult result = Game::theGame->Play();
	Game::theGame->PrintEndMessage(result);
	PrintResults(result);

	exit(EXIT_SUCCESS);
	return 0;
}

void CreateGame(int argc, char ** argv)
{
	char * red = NULL; char * blue = NULL; double timeout = 0.00001; bool graphics = false; bool allowIllegal = false; FILE * log = NULL;
	Piece::Colour reveal = Piece::BOTH; char * inputFile = NULL; int maxTurns = 5000; bool printBoard = false;
	for (int ii=1; ii < argc; ++ii)
	{
		if (argv[ii][0] == '-')
		{
			switch (argv[ii][1])
			{
				case 't':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "Expected timeout value after -t switch!\n");
						exit(EXIT_FAILURE);
					}
					timeout = atof(argv[ii+1]);
					++ii;
					break;
				case 'g':
					graphics = !graphics;
					break;
				case 'p':
					printBoard = !printBoard;
					break;
				case 'i':
					allowIllegal = !allowIllegal;
					break;

				case 'o':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "Expected filename or \"stdout\" after -o switch!\n");
						exit(EXIT_FAILURE);
					}
					if (log != NULL)
					{
						fprintf(stderr, "Expected at most ONE -o switch!\n");
						exit(EXIT_FAILURE);
					}
					if (strcmp(argv[ii+1], "stdout") == 0)
						log = stdout;
					else
						log = fopen(argv[ii+1], "w");
					setbuf(log, NULL);
				
					++ii;
					break;	

				case 'r':
					if (reveal == Piece::BOTH)
						reveal = Piece::BLUE;
					else
						reveal = Piece::NONE;
					break;			
				case 'b':
					if (reveal == Piece::BOTH)
						reveal = Piece::RED;
					else
						reveal = Piece::NONE;
					break;
				case 'm':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "Expected max_turns value after -m switch!\n");
						exit(EXIT_FAILURE);
					}
					if (strcmp(argv[ii+1], "inf"))
						maxTurns = -1;
					else
						maxTurns = atoi(argv[ii+1]);
					++ii;
					break;
				case 'f':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "Expected filename after -f switch!\n");
						exit(EXIT_FAILURE);
					}
					if (log != NULL)
					{
						fprintf(stderr, "Expected at most ONE -f switch!\n");
						exit(EXIT_FAILURE);
					}
					red = (char*)("file");
					blue = (char*)("file");
					inputFile = argv[ii+1];
					++ii;
					break;
				case 'h':
					system("clear");	
					system("less manual.txt");
					exit(EXIT_SUCCESS);
					break;
				case '-':
					if (strcmp(argv[ii]+2, "help") == 0)
					{
						system("clear");	
						system("less manual.txt");
						exit(EXIT_SUCCESS);
					}
					else
					{
						fprintf(stderr, "Unrecognised switch \"%s\"...\n", argv[ii]);
						exit(EXIT_FAILURE);
					}
			}
			
		}
		else
		{
			if (red == NULL)
				red = argv[ii];
			else if (blue == NULL)
				blue = argv[ii];
			else
			{
				fprintf(stderr, "Unexpected argument \"%s\"...\n", argv[ii]);
				exit(EXIT_FAILURE);
			}
		}
	}

	if (inputFile == NULL)
	{
		Game::theGame = new Game(red,blue, graphics, timeout, allowIllegal,log, reveal,maxTurns, printBoard);
	}
	else
	{
		Game::theGame = new Game(inputFile, graphics, timeout, allowIllegal,log, reveal,maxTurns, printBoard);
	}
	if (!Game::theGame->Setup(red, blue))
	{
		fprintf(stdout, "NONE %d\n",Game::theGame->TurnCount());
		exit(EXIT_SUCCESS);
	}
	
	atexit(DestroyGame);

}

void PrintResults(const MovementResult & result)
{
	Piece::Colour winner = Game::theGame->Turn();
	if (Board::LegalResult(result))
	{
		if (winner == Piece::BOTH)
			winner = Piece::NONE;
		else
		{
			if (winner == Piece::RED)
				winner = Piece::BLUE;
			else
				winner = Piece::RED;
		}
	}
	

	switch (winner)
	{
		case Piece::RED:
			fprintf(stdout, "%s RED %d\n", Game::theGame->red->name.c_str(),Game::theGame->TurnCount());	
			Game::theGame->logMessage("%s RED %d\n", Game::theGame->red->name.c_str(),Game::theGame->TurnCount());
			break;
		case Piece::BLUE:
			fprintf(stdout, "%s BLUE %d\n", Game::theGame->blue->name.c_str(),Game::theGame->TurnCount());	
			Game::theGame->logMessage("%s BLUE %d\n", Game::theGame->blue->name.c_str(),Game::theGame->TurnCount());
			break;
		case Piece::BOTH:
			fprintf(stdout, "DRAW %d\n",Game::theGame->TurnCount());
			Game::theGame->logMessage("DRAW %d\n",Game::theGame->TurnCount());	
			break;
		case Piece::NONE:
			fprintf(stdout, "NONE %d\n",Game::theGame->TurnCount());	
			Game::theGame->logMessage("NONE %d\n",Game::theGame->TurnCount());
			break;

	}
}

void DestroyGame()
{
	delete Game::theGame;
	Game::theGame = NULL;
}
