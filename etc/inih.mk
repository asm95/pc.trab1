### SUBMODULE: inih

INI_PROJ_PATH=../vendor/inih
OBJ_DIR=obj
LIB_DIR=lib

premake:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(LIB_DIR)

$(OBJ_DIR)/ini.o: $(INI_PROJ_PATH)/ini.c $(INI_PROJ_PATH)/ini.h
	$(CXX) $(CFLAGS) -c $(INI_PROJ_PATH)/ini.c -o $(OBJ_DIR)/ini.o

$(OBJ_DIR)/INIReader.o: $(INI_PROJ_PATH)/cpp/INIReader.cpp $(INI_PROJ_PATH)/cpp/INIReader.h
	$(CXX) $(CFLAGS) -c $(INI_PROJ_PATH)/cpp/INIReader.cpp -o $(OBJ_DIR)/INIReader.o

$(LIB_DIR)/libinih.a: $(OBJ_DIR)/ini.o $(OBJ_DIR)/INIReader.o
	ar rvs $(LIB_DIR)/libinih.a $(OBJ_DIR)/ini.o $(OBJ_DIR)/INIReader.o

static: premake $(LIB_DIR)/libinih.a