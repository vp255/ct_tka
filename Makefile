CXX                     := -g++ -std=c++2a -g
CXXFLAGS                := -pedantic-errors -Wall -Wextra -Werror -static-libgcc
LDFLAGS                 := -L/usr/lib -lstdc++ -lm -pthread
BUILD                   := ./build
TEST_OBJ_DIR            := $(BUILD)/timer_service_test_objects
APP_DIR                 := $(BUILD)/apps
LIB_DIR                 := $(BUILD)/libraries
TEST_TARGET             := timer_service_tests
TIMER_SERVICE_TESTS_SRC := $(wildcard ./timer_service_tests/*.cpp)
TIMER_SERVICE_SRC       := $(wildcard ./timer_service/*.cpp)

all: build $(APP_DIR)/$(TEST_TARGET)

TIMER_SERVICE_OBJS         := $(TIMER_SERVICE_SRC:%.cpp=$(TEST_OBJ_DIR)/%.o)

timer_service_lib: $(TIMER_SERVICE_OBJS)
	ar -crs $(LIB_DIR)/libtimer_service.a $^

$(TIMER_SERVICE)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@ $(LDFLAGS)


TEST_OBJS                  := $(TIMER_SERVICE_TESTS_SRC:%.cpp=$(TEST_OBJ_DIR)/%.o)

$(TEST_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I. -c $< -o $@ $(LDFLAGS)

$(APP_DIR)/$(TEST_TARGET): timer_service_lib $(TIMER_SERVICE_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TEST_TARGET) $(TEST_OBJS) $(LDFLAGS) -L$(LIB_DIR) -ltimer_service

.PHONY: all build clean debug tests

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(TEST_OBJ_DIR)
	@mkdir -p $(LIB_DIR)
	
debug: CXXFLAGS += -DDEBUG -g
debug: all

clean:
	-@rm -rvf $(BUILD)

