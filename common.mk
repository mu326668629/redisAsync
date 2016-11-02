TARGET   := demo
DIR_OBJ  := objs
CFLAGS   := -g -Wall
CC       := g++
SUFFIX   := cpp

SRC      := $(wildcard ./*.$(SUFFIX))
OBJS     := $(patsubst %.$(SUFFIX), $(DIR_OBJ)/%.o, $(notdir $(SRC)))
SRC      := $(WILDCARD ./*.$(SUFFIX)

LIBS     := -lpthread
#INC      := -I./

$(TARGET) : $(OBJS)
	$(CC)  -o $(TARGET) $(OBJS) $(LIBS)

$(OBJS) : $(DIR_OBJ)/%.o : %.$(SUFFIX)
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
