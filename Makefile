release: brainfck_interpreter.c
	gcc brainfck_interpreter.c -o bf_interpret -O3

debug: brainfck_interpreter.c
	gcc brainfck_interpreter.c -o bf_interpret_dbg
