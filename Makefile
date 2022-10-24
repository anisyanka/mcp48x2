C_SOURCES =  mcp48x2.c
C_INCLUDE_FILES = mcp48x2.h

PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
else
CC = $(PREFIX)gcc
endif

# C defines
C_DEFS =  \
-DDAC_TYPE_MCP4822 \

CFLAGS = -mcpu=cortex-m3 -mthumb $(C_DEFS) -Wall 

all: $(C_INCLUDE_FILES)
	$(CC) $(CFLAGS) -c $(C_SOURCES)

clean:
	rm -rf *.o
