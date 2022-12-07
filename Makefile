EXE = roadmap
SOURCES = main.cpp shape_collections.cpp
UTILS_DIR = sdl-opengl-utils
SOURCES += $(UTILS_DIR)/gl_sdl_utils.cpp $(UTILS_DIR)/gl_sdl_2d.cpp 
SOURCES += $(UTILS_DIR)/gl_sdl_shape_obj.cpp $(UTILS_DIR)/gl_sdl_geometry.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL -lGLEW -lSDL2_image

IMGUI_DIR = imgui
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp
SOURCES += $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

SHADERS_DIR = shaders
ASSETS_DIR = assets
STANDARD_SHADERS_DIR = ../shaders
GLM_DIR = /usr/include/glm

INCLUDE_FLAGS = -Iimgui -Isdl-opengl-utils
INCLUDE_FLAGS += -Iimgui/backends

# Just for vscode Intellisense
#INCLUDE_FLAGS += -I/usr/include/SDL2

COMMON_FLAGS = -std=c++14
COMMON_FLAGS += -Og -Wall -Wformat
COMMON_FLAGS += $(INCLUDE_FLAGS)
LIBS =
CXXFLAGS = $(COMMON_FLAGS)

WASM_FLAGS = $(COMMON_FLAGS)
WASM_FLAGS += -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -sASYNCIFY -sASYNCIFY_IMPORTS=[emscripten_sleep]
WASM_FLAGS += -s USE_SDL=2 -s FULL_ES3=1 -s MIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sALLOW_MEMORY_GROWTH
WASM_FLAGS += --preload-file $(STANDARD_SHADERS_DIR) --preload-file $(SHADERS_DIR)
WASM_FLAGS += --preload-file $(ASSETS_DIR)
WASM_FLAGS += -I$(GLM_DIR)
WASM_OUT = docs/index.html
WASM_OUT_FILES = $(WASM_OUT) $(addsuffix .data, $(basename $(WASM_OUT)))
WASM_OUT_FILES += $(addsuffix .js, $(basename $(WASM_OUT)))
WASM_OUT_FILES += $(addsuffix .wasm, $(basename $(WASM_OUT)))

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) -ldl $(shell sdl2-config --libs)

	CXXFLAGS += $(shell sdl2-config --cflags)
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	LIBS += -L/usr/local/lib -L/opt/local/lib

	CXXFLAGS += `sdl2-config --cflags`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
    ECHO_MESSAGE = "MinGW"
    LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

    CXXFLAGS += `pkg-config --cflags sdl2`
    CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(UTILS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

wasm: $(WASM_OUT)
	@echo HTML built

$(WASM_OUT): $(SOURCES)
	emcc -o $@ $^ $(WASM_FLAGS)

clean:
	rm -f $(EXE) $(OBJS)

wasm_clean:
	rm -f $(WASM_OUT_FILES)