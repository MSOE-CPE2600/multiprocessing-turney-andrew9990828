CC=gcc
CFLAGS=-c -Wall -g

# Add pthread support here
LDFLAGS=-ljpeg -lpthread
MOVIE_LDFLAGS=-lm -lpthread

SOURCES= mandel.c jpegrw.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mandel

MOVIE_SRC=mandelmovie.c
MOVIE_OBJ=mandelmovie.o
MOVIE_EXEC=mandelmovie

all: $(EXECUTABLE) $(MOVIE_EXEC)

-include $(OBJECTS:.o=.d)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(MOVIE_EXEC): $(MOVIE_OBJ)
	$(CC) $(MOVIE_OBJ) $(MOVIE_LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	$(CC) -MM $< > $*.d

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) *.d $(MOVIE_OBJ) $(MOVIE_EXEC)
