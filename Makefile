all: src/physics.c
	cc src/physics.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o tgt/physics 
	./tgt/physics

clean:
	rm tgt/*
  
