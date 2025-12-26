SHELL := /bin/bash
# ========================
#   COMPILERS & FLAGS
# ========================
CXX := g++

# [FIX] Define Include paths separately (for Compiler)
INCLUDES := -Ivendors/fmt/include -Ivendors/nlohmann_json -Ivendors/gtest/googletest/include

# [FIX] Add $(INCLUDES) to CXXFLAGS
CXXFLAGS := -std=c++20 -Wall -Wextra -O3 -MMD -MP $(INCLUDES)

# ========================
#   STATIC LINKING
# ========================
# -static: Forces linking of all libraries statically (if available).
# -static-libgcc -static-libstdc++: Ensures standard C++ libs are inside the .exe
STATIC_FLAGS := -static -static-libgcc -static-libstdc++

# [FIX] LDFLAGS is for linking. Added STATIC_FLAGS here.
LDFLAGS := -Lvendors/fmt/build -lfmt $(STATIC_FLAGS) -lstdc++ -lpthread

# For Coverage
COV_FLAGS := -fprofile-arcs -ftest-coverage
COV_LIBS := -lgcov

GTEST_LIBS := -Lvendors/gtest/build/lib -lgtest -lgtest_main -pthread

# ========================
#   DIRECTORIES
# ========================
SRC_DIR := src
TEST_UNIT_DIR := tests/unit
TEST_INTEGRATION_DIR := tests/integration

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

OBJ_MAIN_DIR := $(OBJ_DIR)/cpp
OBJ_IMPL_DIR := $(OBJ_DIR)/cxx
OBJ_UNIT_DIR := $(OBJ_DIR)/unit
OBJ_INT_DIR := $(OBJ_DIR)/integration

TARGET := app
TEST_UNIT_TARGET := unitTest
TEST_INT_TARGET := integrationTest

# ========================
#   VENDOR BUILD
# ========================

vendor-build:
	# CMake build googletest
	mkdir -p vendors/gtest/build
	cd vendors/gtest/build && cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
	make -C vendors/gtest/build -j

	# CMake build fmt
	mkdir -p vendors/fmt/build
	cd vendors/fmt/build && cmake ..
	make -C vendors/fmt/build -j



# ========================
#   SOURCE COLLECTION
# ========================
SRCS_CPP := $(wildcard $(SRC_DIR)/*.cpp)
SRCS_CXX := $(wildcard $(SRC_DIR)/*.cxx)

SRCS_TEST_UNIT := $(wildcard $(TEST_UNIT_DIR)/*.cc)
SRCS_TEST_INT := $(wildcard $(TEST_INTEGRATION_DIR)/*.c++)

OBJS_CPP := $(patsubst $(SRC_DIR)/%,$(OBJ_MAIN_DIR)/%,$(SRCS_CPP:.cpp=.o))
OBJS_CXX := $(patsubst $(SRC_DIR)/%,$(OBJ_IMPL_DIR)/%,$(SRCS_CXX:.cxx=.o))

OBJS_TEST_UNIT := $(patsubst $(TEST_UNIT_DIR)/%,$(OBJ_UNIT_DIR)/%,$(SRCS_TEST_UNIT:.cc=.o))
OBJS_TEST_INT := $(patsubst $(TEST_INTEGRATION_DIR)/%,$(OBJ_INT_DIR)/%,$(SRCS_TEST_INT:.c++=.o))

DEPS := $(OBJS_CPP:.o=.d) $(OBJS_CXX:.o=.d) \
        $(OBJS_TEST_UNIT:.o=.d) $(OBJS_TEST_INT:.o=.d)


# ========================
#   COLORS
# ========================
GREEN ?=
MAGENTA ?=
CYAN ?=
YELLOW ?=
RED ?=
RESET ?=


# ========================
#   DEFAULT BUILD
# ========================
build: $(BIN_DIR)/$(TARGET)

all: build


# ========================
#   RUN TARGET
# ========================
run:
	./$(BIN_DIR)/$(TARGET)

# ========================
#   MAIN PROGRAM LINK
# ========================
$(BIN_DIR)/$(TARGET): $(OBJS_CPP) $(OBJS_CXX) | $(BIN_DIR)
	@echo $(GREEN)Linking main:$(RESET)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


# ========================
#   RULES FOR SOURCES
# ========================
$(OBJ_MAIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_MAIN_DIR)
	@echo $(GREEN)Compiling cpp:$(RESET) $<
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_IMPL_DIR)/%.o: $(SRC_DIR)/%.cxx | $(OBJ_IMPL_DIR)
	@echo $(GREEN)Compiling cxx:$(RESET) $<
	$(CXX) $(CXXFLAGS) -c $< -o $@


# ========================
#   UNIT TESTS
# ========================
build-unit: $(BIN_DIR)/$(TEST_UNIT_TARGET)

$(BIN_DIR)/$(TEST_UNIT_TARGET): $(OBJS_TEST_UNIT) $(OBJS_CPP) | $(BIN_DIR)
	@echo $(GREEN)Linking Unit Tests:$(RESET)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GTEST_LIBS) $(LDFLAGS)

run-unit: build-unit
	@echo $(MAGENTA)Running Unit Tests:$(RESET)
	$(BIN_DIR)/$(TEST_UNIT_TARGET) --gtest_output=xml:./build/test-results/unit.xml


# ========================
#   INTEGRATION TESTS
# ========================
build-int: $(BIN_DIR)/$(TEST_INT_TARGET)

$(BIN_DIR)/$(TEST_INT_TARGET): $(OBJS_TEST_INT) $(OBJS_CPP) | $(BIN_DIR)
	@echo $(GREEN)Linking Integration Tests:$(RESET)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GTEST_LIBS) $(LDFLAGS)

run-int: build-int
	@echo $(MAGENTA)Running Integration Tests:$(RESET)
	$(BIN_DIR)/$(TEST_INT_TARGET)


# ========================
#   COVERAGE
# ========================
coverage: CXXFLAGS += $(COV_FLAGS)
coverage: LDFLAGS += $(COV_LIBS)
coverage: clean $(BIN_DIR)/$(TEST_UNIT_TARGET)
	@echo $(CYAN)Running tests for coverage...$(RESET)
	$(BIN_DIR)/$(TEST_UNIT_TARGET)
	@echo $(CYAN)Generating coverage report...$(RESET)
	gcov -o $(OBJ_MAIN_DIR) $(SRCS_CPP)
	gcov -o $(OBJ_IMPL_DIR) $(SRCS_CXX)
	gcov -o $(OBJ_UNIT_DIR) $(SRCS_TEST_UNIT)


# ========================
#   TEST SOURCE RULES
# ========================
$(OBJ_UNIT_DIR)/%.o: $(TEST_UNIT_DIR)/%.cc | $(OBJ_UNIT_DIR)
	@echo $(YELLOW)Compiling unit test:$(RESET) $<
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_INT_DIR)/%.o: $(TEST_INTEGRATION_DIR)/%.c++ | $(OBJ_INT_DIR)
	@echo $(YELLOW)Compiling integration test:$(RESET) $<
	$(CXX) $(CXXFLAGS) -c $< -o $@


# ========================
#   DIRECTORY CREATION (WINDOWS FIXED)
# ========================
$(OBJ_MAIN_DIR) $(OBJ_IMPL_DIR) $(OBJ_UNIT_DIR) $(OBJ_INT_DIR) $(BIN_DIR):
	@echo $(CYAN)Creating directory:$(RESET) $@
	@mkdir -p $@


# ========================
#   CLEAN
# ========================
clean:
	@echo $(RED)Cleaning...$(RESET)
	@rm -rf $(BUILD_DIR)


# Include auto-deps
-include $(DEPS)
