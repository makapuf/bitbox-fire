#fireman makefile

NAME=firemen
GAME_C_FILES = data.c firemen.c
include lib/bitbox.mk

data.c: firemen.png bg.png guy.png digit_[0-9].png angel.png start.png
	python rle_encode2.py $^ > $@
