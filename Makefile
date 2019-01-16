CC = gcc
OBJECTS = main.o p2au_au2p.o
INC = -I include
objects := $(wildcard *.o)
vpath %.c src


transcoding: $(OBJECTS) 
	$(CC) -o transcoding $(OBJECTS)


$(OBJECTS) : %.o:%.c 
	$(CC) -c $< $(INC) -o $@


.PHONY: cleanall cleanexe cleanobj
cleanall:
	rm -f transcoding $(objects)
cleanexe:
	rm -f transcoding
cleanobj:
	rm -f *.o
