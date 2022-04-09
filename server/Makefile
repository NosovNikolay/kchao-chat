rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
OS := $(shell uname)
CC=clang
CFLAGS=-std=c11 -Wall -Wextra -Werror -g

ifeq ($(OS),Darwin)
CFLAGS+= -Wpedantic
else
CFLAGS+= -Wno-error=newline-eof -Wno-error=visibility 
endif

CLIBS=-pthread -lcurl \
	`pkg-config --libs --cflags libmongoc-1.0`
ifneq ($(OS),Darwin)
CLIBS+=`pkg-config --libs --cflags openssl` 
endif

CDEFINES=

NAME=../uchat_server

.OBJECTS=$(call rwildcard,src,*.c)

FW_PATHS=$(wildcard ../frameworks/*)
foreachfw=$(foreach fw, $(FW_PATHS), $(wildcard $(fw)$1))

FW_INC=$(addprefix "-I" ,$(call foreachfw,/inc*/))
FW_BIN=$(call foreachfw,/*.a)

all: $(FW_PATHS) $(NAME)

$(NAME): $(FW_BIN) $(.OBJECTS)
	$(CC) $(CFLAGS) $(CLIBS) $(FW_INC) -Iinc -o $(NAME) $(.OBJECTS) $(FW_BIN) $(CDEFINES)

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
	./$(NAME) - --no-demon

deploy: $(FW_PATHS)
	$(MAKE) -C .. deploy

.PHONY: $(FW_PATHS)
