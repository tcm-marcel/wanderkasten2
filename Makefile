# (c) 2017 Marcel Kost

# --- Configuration -----------------------------------------------

SKETCH = ./main/main.ino
BUILD_DIR = ./bin
SRC_DIR = ./main
HTML_HEADER_FILES = main/webinterface_html.h main/koenigslied_html.h

UPLOAD_PORT = /dev/ttyUSB0
BOARD = generic

LIBS = ~/.arduino15/packages/esp8266/hardware/esp8266/2.3.0/libraries ../libraries/FastLED
MAKEESPARDUINO_PATH = $(HOME)/dev/arduino/makeEspArduino/makeEspArduino.mk

# -----------------------------------------------------------------


# fix SD lib problem
BUILD_EXTRA_FLAGS = -D__arm__

# activate debug flags
#BUILD_EXTRA_FLAGS += -ggdb -Og -DGDBSTUB_REDIRECT_CONSOLE_OUTPUT -DGDBSTUB_CTRLC_BREAK -DGDBSTUB_BREAK_ON_INIT
#BUILD_EXTRA_FLAGS += -DDEBUG_ESP_CORE
#BUILD_EXTRA_FLAGS += -DDEBUG_ESP_H
#BUILD_EXTRA_FLAGS += -DDEBUG_ESP_WIFI
#BUILD_EXTRA_FLAGS += -DDEBUG_ESP_PORT=Serial
#VERBOSE = 1


# targets to convert html files to header files with byte arrays
$(SRC_DIR)/%_html.h: %.html
	# convert 
	echo convert $< to header file $@
  # add NULL byte and PROGMEM
	xxd -i $< | sed 's/^\(  \(0x[0-9a-f]\{2\}, \)\+0x[0-9a-f]\{2\}\)$$/\1, 0x00/m' | sed 's/\(.*\) = {/static \1 PROGMEM = {/m' > $@

%.html: %.htm
	# minify html file
	echo minify $
	css-html-js-minify.py $<

html: $(HTML_HEADER_FILES)
	echo "preparing html files"


# include makeEspArduino Makefile
include $(MAKEESPARDUINO_PATH)
