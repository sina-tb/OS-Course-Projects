CC = g++
BUILD_DIR = build
INCLUDE_DIR = includes
CFLAGS = -std=c++17 -lstdc++fs

MAIN_EXECUTABLE_FILE = ClubsAgeStats.out
COUNTRY_EXECUTABLE_FILE = country.out
CLUB_EXECUTABLE_FILE = club.out
REDUCE_EXECUTABLE_FILE = reduce.out

MAIN_OBJECTS = \
	$(BUILD_DIR)/main.o

COUNTRY_OBJECTS = \
	$(BUILD_DIR)/country.o

CLUB_OBJECTS = \
	$(BUILD_DIR)/club.o

REDUCE_OBJECTS = \
	$(BUILD_DIR)/reduce.o


MainSensitivityList = \
	main.cpp

ReduceSensitivityList = \
	PositionsProcess.cpp

CountrySensitivityList = \
	CountriesProcess.cpp

ClubsSensitivityList = \
	ClubsProcess.cpp

all: $(BUILD_DIR) $(MAIN_EXECUTABLE_FILE) $(COUNTRY_EXECUTABLE_FILE) $(CLUB_EXECUTABLE_FILE) $(REDUCE_EXECUTABLE_FILE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(MAIN_EXECUTABLE_FILE): $(MAIN_OBJECTS)
	$(CC) $(MAIN_OBJECTS) $(CFLAGS) -o $(MAIN_EXECUTABLE_FILE)

$(COUNTRY_EXECUTABLE_FILE): $(COUNTRY_OBJECTS)
	$(CC) $(COUNTRY_OBJECTS) $(CFLAGS) -o $(COUNTRY_EXECUTABLE_FILE)

$(CLUB_EXECUTABLE_FILE): $(CLUB_OBJECTS)
	$(CC) $(CLUB_OBJECTS) $(CFLAGS) -o $(CLUB_EXECUTABLE_FILE)

$(REDUCE_EXECUTABLE_FILE): $(REDUCE_OBJECTS)
	$(CC) $(REDUCE_OBJECTS) $(CFLAGS) -o $(REDUCE_EXECUTABLE_FILE)

$(BUILD_DIR)/main.o: $(MainSensitivityList)
	$(CC) -c main.cpp $(CFLAGS) -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/reduce.o: $(ReduceSensitivityList)
	$(CC) -c PositionsProcess.cpp $(CFLAGS) -o $(BUILD_DIR)/reduce.o

$(BUILD_DIR)/country.o: $(CountrySensitivityList)
	$(CC) -c CountriesProcess.cpp $(CFLAGS) -o $(BUILD_DIR)/country.o

$(BUILD_DIR)/club.o: $(ClubsSensitivityList)
	$(CC) -c ClubsProcess.cpp $(CFLAGS) -o $(BUILD_DIR)/club.o


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) *.o *.out \