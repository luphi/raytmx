# Define required environment variables
#-----------------------------------------------------------------------------------------------------------------------
# Path to raylib's folder with headers and binaries
RAYLIB_PATH         ?= C:/raylib/raylib
# Define target platform: PLATFORM_DESKTOP, PLATFORM_RPI, PLATFORM_DRM, PLATFORM_ANDROID, PLATFORM_WEB
PLATFORM            ?= PLATFORM_DESKTOP
# Library type compilation: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE      ?= STATIC
# Build mode for project: DEBUG or RELEASE
BUILD_MODE          ?= DEBUG
# Define raylib release directory for compiled library
RAYLIB_RELEASE_PATH ?= $(RAYLIB_PATH)/src
# Define the C(++) standard: c99 or c++11 expected
STD                 ?= c99

# Determine the OS: PLATFORM_OS
#-----------------------------------------------------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
	PLATFORM_OS = WINDOWS
else
	# Use the 'uname' tool to get the kernel's name
	UNAMEOS = $(shell uname)
	# If one of the many flavors of Linux
	ifeq ($(UNAMEOS),Linux)
		PLATFORM_OS = LINUX
	# If macOS 10+
	else ifeq ($(UNAMEOS),Darwin)
		PLATFORM_OS = OSX
	endif
endif

# Name of the resulting executable: EXE
#-----------------------------------------------------------------------------------------------------------------------
# *.exe for Windows, extensionless for everything else
ifeq ($(PLATFORM_OS),WINDOWS)
	EXE = raytmx-example.exe
else
	EXE = raytmx-example
endif

# Define default C compiler: CC
#-----------------------------------------------------------------------------------------------------------------------
ifeq ($(STD),c99)
	CC = gcc
else ifeq ($(STD),c++11)
	CC = g++
endif

# Define compiler flags: CFLAGS
#-----------------------------------------------------------------------------------------------------------------------
# -std=$(STD)           defines the C or C++ language mode (
# -Wall                 turns on most, but not all, compiler warnings
# -Wno-missing-braces   ignore invalid warning (GCC bug 53119)
# -O0, -O2              defines optimization level
CFLAGS = -std=$(STD) -Wall -Wno-missing-braces

ifeq ($(STD),c99)
	# -D_DEFAULT_SOURCE   use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
	CFLAGS += -D_DEFAULT_SOURCE
endif

ifeq ($(BUILD_MODE),DEBUG)
	# -g        include debug symbols
	# -DDEBUG   equivalent to "#define DEBUG"
	CFLAGS += -O0 -g -DDEBUG
else
	# -s   strip unnecessary data from build
	CFLAGS += -O2 -s
endif

# Define include paths for required headers: INCLUDE_PATHS
#-----------------------------------------------------------------------------------------------------------------------
INCLUDE_PATHS = -I$(RAYLIB_PATH)/src \
				-I$(RAYLIB_PATH)/src/external \
				-I$(RAYLIB_PATH)/src/extras \
				-I../external \
				-I..

# Define library paths containing required libs: LDFLAGS
#-----------------------------------------------------------------------------------------------------------------------
LDFLAGS = -L. -L$(RAYLIB_RELEASE_PATH)
ifeq ($(PLATFORM_OS),WINDOWS)
	# NOTE: The resource .rc file contains windows executable icon and properties
	#LDFLAGS += $(RAYLIB_PATH)/src/raylib.rc.data
	ifeq ($(BUILD_MODE),RELEASE)
		# -Wl,--subsystem,windows hides the console window
		LDFLAGS += -Wl,--subsystem,windows
	endif
endif

# Define libraries required on linking: LDLIBS
# NOTE: To link libraries (lib<name>.so or lib<name>.a), use -l<name>
#-----------------------------------------------------------------------------------------------------------------------
ifeq ($(PLATFORM_OS),WINDOWS)
	# Libraries for Windows desktop compilation
	# NOTE: WinMM library required to set high-res timer resolution
	LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
else ifeq ($(PLATFORM_OS),LINUX)
	# Libraries for Debian GNU/Linux desktop compiling
	# NOTE: Required packages: libegl1-mesa-dev
	LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
	# On Wayland windowing system, additional libraries are required
	ifeq ($(USE_WAYLAND_DISPLAY),TRUE)
		LDLIBS += -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon
	endif
else ifeq ($(PLATFORM_OS),OSX)
	# Libraries for macOS 10+ desktop compiling
	# NOTE: Required packages: libopenal-dev libegl1-mesa-dev
	LDLIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
endif

# Define source files and the object files to build from them
#-----------------------------------------------------------------------------------------------------------------------
SRCS = raytmx-example.c
OBJS = $(SRCS:.c=.o)

# Targets
#-----------------------------------------------------------------------------------------------------------------------
# Default target entry
all: $(EXE)

# Executable, depends on objects
$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)

# Compile source file(s)
# NOTE: This pattern will compile every module defined on $(OBJS)
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Remove the products of this build script
clean:
	rm -fv *.o $(EXE)
