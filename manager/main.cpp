#include <stdlib.h>
#include <stdio.h>






#include "game.h"

using namespace std;

int main(int argc, char ** argv)
{

	char * red = NULL; char * blue = NULL; double timeout = 0.00001; bool graphics = false; bool allowIllegal = false; FILE * log = NULL;
	Piece::Colour reveal = Piece::BOTH;
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
					graphics = true;
					break;
				case 'i':
					allowIllegal = true;
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
	if (argc == 1)
	{
		fprintf(stderr, "Usage: stratego [options] red blue\n");
		fprintf(stderr, "       stratego --help\n");
		exit(EXIT_SUCCESS);
		
	}
	
	Game game(red, blue, graphics, timeout, allowIllegal, log, reveal);
	
	
	if (!game.Setup(red, blue))
	{
		fprintf(stdout, "NONE %d\n",game.TurnCount());
		exit(EXIT_SUCCESS);
	}

	MovementResult result = game.Play();
	game.PrintEndMessage(result);

	Piece::Colour winner = game.Turn();
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
			fprintf(stdout, "%s RED %d\n", red,game.TurnCount());	
			break;
		case Piece::BLUE:
			fprintf(stdout, "%s BLUE %d\n", blue,game.TurnCount());	
			break;
		case Piece::BOTH:
			fprintf(stdout, "DRAW %d\n",game.TurnCount());	
			break;
		case Piece::NONE:
			fprintf(stdout, "NONE %d\n",game.TurnCount());	
			break;

	}

	
	



	exit(EXIT_SUCCESS);
	
	return 0;
}


