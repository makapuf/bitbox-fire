#fireman makefile
NO_SDCARD=1

NAME=firemen
GAME_C_FILES = data.c firemen.c
include $(BITBOX)/lib/bitbox.mk

data.c: firemen.png bg.png guy.png digit_[0-9].png angel.png start.png
	python rle_encode2.py $^ > $@
