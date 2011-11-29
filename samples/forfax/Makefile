#Makefile for Forfax

CPP = g++ -Wall -pedantic -lSDL -lGL  -g
OBJ = main.o forfax.o

BIN = forfax



$(BIN) : $(OBJ) 
	$(CPP) -o $(BIN) $(OBJ)




%.o : %.cpp %.h
	$(CPP) -c $<

clean :
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~
