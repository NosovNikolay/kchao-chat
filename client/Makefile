rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CC=clang
CFLAGS=-std=c11 -Wall -Werror -Wextra -g -lcurl
CLIBS=-pthread

NAME=../uchat

.OBJECTS=$(call rwildcard,src,*.c)

FW_PATHS=$(wildcard ../frameworks/*)
foreachfw=$(foreach fw, $(FW_PATHS), $(wildcard $(fw)$1))

FW_INC=$(addprefix "-I" ,$(call foreachfw,/inc*/))
FW_BIN=$(call foreachfw,/*.a)

GTK_LINK_FLAGS=`pkg-config --cflags --libs libmongoc-1.0` \
`pkg-config --cflags --libs gtk+-3.0` \
`pkg-config --cflags --libs vte-2.91`

all: $(FW_PATHS) $(NAME)

$(NAME): $(FW_BIN) $(.OBJECTS)
	$(CC) $(CFLAGS) $(GTK_LINK_FLAGS) $(CLIBS) $(FW_INC) -Iinc -o $(NAME) $(.OBJECTS) $(FW_BIN)

$(FW_PATHS):
	$(MAKE) -C $@

clean:
	rm -rf obj

uninstall: clean
	rm -f $(NAME)

reinstall:
	rm -f $(NAME)
	$(MAKE) $(NAME)

run: all
	./$(NAME)

.PHONY: $(FW_PATHS)
