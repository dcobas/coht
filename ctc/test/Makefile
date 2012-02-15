
CPU=L865
CFLAGS=-g -Wall

CC=gcc
HEADERS := ../ \
	-I./ 

LOADLIBES := -L../
LDLIBS=-lctc.L865
all: set_clk.$(CPU) set_delay_counter1.$(CPU) set_delay_counter2.$(CPU) set_mode.$(CPU) set_enable.$(CPU)

set_clk.$(CPU): 
	mkdir -p obj
	$(CC) $(CFLAGS) -c -I$(HEADERS) set_clk.c -o ./obj/set_clk.$(CPU).o
	$(CC) $(CFLAGS) -o ./obj/$@ ./obj/set_clk.$(CPU).o $(LOADLIBES) $(LDLIBS)

set_delay_counter1.$(CPU): 
	mkdir -p obj
	$(CC) $(CFLAGS) -c -I$(HEADERS) set_delay_counter1.c -o ./obj/set_delay_counter1.$(CPU).o
	$(CC) $(CFLAGS) -o ./obj/$@ ./obj/set_delay_counter1.$(CPU).o $(LOADLIBES) $(LDLIBS)

set_delay_counter2.$(CPU): 
	mkdir -p obj
	$(CC) $(CFLAGS) -c -I$(HEADERS) set_delay_counter2.c -o ./obj/set_delay_counter2.$(CPU).o
	$(CC) $(CFLAGS) -o ./obj/$@ ./obj/set_delay_counter2.$(CPU).o $(LOADLIBES) $(LDLIBS)

set_mode.$(CPU): 
	mkdir -p obj
	$(CC) $(CFLAGS) -c -I$(HEADERS) set_mode.c -o ./obj/set_mode.$(CPU).o
	$(CC) $(CFLAGS) -o ./obj/$@ ./obj/set_mode.$(CPU).o $(LOADLIBES) $(LDLIBS)

set_enable.$(CPU): 
	mkdir -p obj
	$(CC) $(CFLAGS) -c -I$(HEADERS) set_enable.c -o ./obj/set_enable.$(CPU).o
	$(CC) $(CFLAGS) -o ./obj/$@ ./obj/set_enable.$(CPU).o $(LOADLIBES) $(LDLIBS)




clean:
	rm -rf ./obj/
