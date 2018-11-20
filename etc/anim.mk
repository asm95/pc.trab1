### SUBMODULE: anim
CXX=g++
CFLAGS=--std=c++11
INI_PROJ_PATH=../vendor/anim
OBJ_DIR=obj
LIB_DIR=lib
SRC_FILE_LIST=$(wildcard $(INI_PROJ_PATH)/*.cpp)
OBJ_FILE_LIST=$(SRC_FILE_LIST:$(INI_PROJ_PATH)/%.cpp=$(OBJ_DIR)/%.o)
TARGET_LIB=$(LIB_DIR)/libanim.a

premake:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(LIB_DIR)

$(TARGET_LIB): $(OBJ_FILE_LIST)
	ar rvs $(TARGET_LIB) $^

$(OBJ_DIR)/%.o: $(INI_PROJ_PATH)/%.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

static: premake $(TARGET_LIB)