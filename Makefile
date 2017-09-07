#fireman makefile
NAME=firemen
GAME_C_FILES = data.c firemen.c bitbox_icon.c
include $(BITBOX)/kernel/bitbox.mk

data.c: firemen.png bg.png guy.png digit_[0-9].png angel.png start.png
	python rle_encode2.py $^ > $@
bitbox_icon.c: icon.png
	python $(BITBOX)/2nd_boot/mk_ico.py $^ > $@