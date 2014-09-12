DB2PATH= /home/db2admn/sqllib
CURPATH= /usr
CFLAGS= -m64 -g
HEADER= -I$(DB2PATH)/include -I$(CURPATH)/include
LIBS= -Wl,-rpath,$(DB2PATH)/lib64 -L$(DB2PATH)/lib64 -ldb2 -lm -L$(CURPATH)/lib64 -lncurses 
OBJ=db2topas.o db2snap.o db2curses.o db2list.o db2topasutil.o stack.o list.o
CC=gcc

clean:  
	rm -rf *.o core.*

db2topas: $(OBJ)
	$(CC) $(CFLAGS) $? -o $@ $(LIBS)

db2topasutil.o: db2topasutil.c db2topasutil.h stack.h
	$(CC) $(CFLAGS) ${HEADER} -c $< -D_REENTRANT

db2list.o: db2list.c db2list.h db2topasutil.h stack.h
	$(CC) $(CFLAGS) ${HEADER} -c $<

db2snap.o: db2snap.c db2snap.h db2list.h db2topasutil.h stack.h
	$(CC) $(CFLAGS) ${HEADER} -c $< -D_REENTRANT

db2curses.o: db2curses.c db2curses.h db2snap.h db2list.h db2topasutil.h stack.h
	$(CC) $(CFLAGS) ${HEADER} -c $< -D_REENTRANT

db2topas.o: db2topas.c db2snap.h db2curses.h db2list.h db2topasutil.h stack.h
	$(CC) $(CFLAGS) ${HEADER} -c $< -D_REENTRANT

stack.o: stack.c stack.h list.h
	$(CC) $(CFLAGS) ${HEADER} -c $<

list.o: list.c list.h
	$(CC) $(CFLAGS) ${HEADER} -c $<

