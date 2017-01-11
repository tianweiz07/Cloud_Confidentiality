CC = gcc
MAIN = main
EVENT_CACHE = event_cache
EVENT_CPU = event_training
PATTERN_RECOG = pattern

all: main cache cpu pattern

main: $(MAIN).c
	$(CC) -o $(MAIN) $(MAIN).c -lpthread -lm

cache: $(EVENT_CACHE).c
	$(CC) -o $(EVENT_CACHE) $(EVENT_CACHE).c

cpu: $(EVENT_CPU).c
	$(CC) -o $(EVENT_CPU) $(EVENT_CPU).c -lpthread -lm

pattern: $(PATTERN_RECOG).c
	$(CC) -o $(PATTERN_RECOG) $(PATTERN_RECOG).c -lm

clean:
	rm -rf $(MAIN) $(EVENT_CACHE) $(EVENT_CPU) $(PATTERN_RECOG)

