# Makefile for tankhunt server

RM = rm -f

SRC =   data.c \
	fast_atan.c \
	input.c \
	main.c \
	sintab.c \
	update.c \
	walls.c
OBJ = $(SRC:.c=.o)
HDRS =  data.h \
	defs.h \
	fast_dis.h \
	packets.h \
	sintab.h \
	struct.h

LIBS = -lm

all: tankhunt.server

$(OBJ): $(HDRS)

tankhunt.server: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIBS) -o $@

clean:
	$(RM) $(OBJ) *~ tankhunt.server
