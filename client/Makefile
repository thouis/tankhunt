# Makefile for tankhunt client

RM = rm -f
INC = -I/usr/X11R6/include
LDPATH = -L/usr/X11R6/lib
COPT = 
CFLAGS = $(COPT) $(INC)
LDFLAGS = $(COPT) $(LDPATH)
LIBS =	-lX11 -lm

SRC =   data.c \
	input.c \
	main.c \
	rotate.c \
	socket.c \
	update.c \
	x11.c

OBJ = $(SRC:.c=.o)

HDRS =  data.h \
	defs.h \
	packets.h

all:	tankhunt.client

$(OBJ): $(HDRS)

tankhunt.client: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@

clean:
	$(RM) $(OBJ) *~ tankhunt.client
