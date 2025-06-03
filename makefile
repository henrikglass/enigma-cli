
TARGET := enigma-cli

C_WARNINGS := -Werror -Wall -Wlogical-op -Wextra -Wvla -Wnull-dereference \
			  -Wswitch-enum -Wno-deprecated -Wduplicated-cond -Wduplicated-branches \
			  -Wshadow -Wpointer-arith -Wcast-qual -Winit-self -Wuninitialized \
			  -Wcast-align -Wstrict-aliasing -Wformat=2 -Wmissing-declarations \
			  -Wmissing-prototypes -Wstrict-prototypes -Wwrite-strings \
			  -Wunused-parameter -Wshadow -Wdouble-promotion -Wfloat-equal \
			  -Wno-unused-function \
			  -Wno-error=cpp 
C_INCLUDES := -I. -Iinclude
C_FLAGS    := $(C_WARNINGS) $(C_INCLUDES) --std=c17 -O0 -ggdb3 

all:
	$(CC) $(C_FLAGS) src/enigma_cli.c -o $(TARGET)

clean:
	-rm $(TARGET)

