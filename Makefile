
TARGET = async 

CC = gcc

SUFFIX = c

CFLAGS = -g -Wall

INC = -Ihiredis/
LIB = -Lhiredis -lhiredis
SRC  := $(wildcard ./*.$(SUFFIX))

OBJS := $(patsubst %.$(SUFFIX), ./%.o, $(notdir $(SRC)))


$(TARGET) : $(OBJS)
	$(CC) $(OBJS) $(LIB) -o $(TARGET)

$(OBJS) : $(SRC)
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
