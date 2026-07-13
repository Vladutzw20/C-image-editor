CC=gcc
CFLAGS=-Wall -Wextra -std=c99

SRCS=image_editor.c cmd.c file_io.c

TARGETS=image_editor

build: $(TARGETS)

$(TARGETS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGETS) -lm

pack:
	zip -FSr 313CA_GILCA_FLORIAN_VLADUT_Tema3.zip README Makefile *.c *.h

clean:
	rm -f $(TARGETS)
.PHONY: pack clean