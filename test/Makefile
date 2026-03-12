RAYLIB_PATH         ?= C:/raylib/raylib
PLATFORM            ?= PLATFORM_DESKTOP
RAYLIB_LIBTYPE      ?= STATIC
BUILD_MODE          ?= DEBUG
RAYLIB_RELEASE_PATH ?= $(RAYLIB_PATH)/src
STD                 ?= c99

ifeq ($(OS),Windows_NT)
	PLATFORM_OS = WINDOWS
else
	UNAMEOS = $(shell uname)
	ifeq ($(UNAMEOS),Linux)
		PLATFORM_OS = LINUX
	else ifeq ($(UNAMEOS),Darwin)
		PLATFORM_OS = OSX
	endif
endif

ifeq ($(PLATFORM_OS),WINDOWS)
	EXE = test.exe
else
	EXE = test
endif

ifeq ($(STD),c99)
	CC = gcc
else ifeq ($(STD),c++11)
	CC = g++
endif

CFLAGS = -std=$(STD) -Wall -Wno-missing-braces

ifeq ($(STD),c99)
	CFLAGS += -D_DEFAULT_SOURCE
endif

ifeq ($(BUILD_MODE),DEBUG)
	CFLAGS += -O0 -g -DDEBUG
else
	CFLAGS += -O2 -s
endif

INCLUDE_PATHS = -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external -I$(RAYLIB_PATH)/src/extras -I../external -I..

LDFLAGS = -L. -L$(RAYLIB_RELEASE_PATH)
ifeq ($(PLATFORM_OS),WINDOWS)
	ifeq ($(BUILD_MODE),RELEASE)
		LDFLAGS += -Wl,--subsystem,windows
	endif
endif

ifeq ($(PLATFORM_OS),WINDOWS)
	LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
else ifeq ($(PLATFORM_OS),LINUX)
	LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
	ifeq ($(USE_WAYLAND_DISPLAY),TRUE)
		LDLIBS += -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon
	endif
else ifeq ($(PLATFORM_OS),OSX)
	LDLIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
endif

SRCS = test.c
OBJS = $(SRCS:.c=.o)

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

clean:
	rm -fv *.o $(EXE)
