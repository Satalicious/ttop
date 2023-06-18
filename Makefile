WX_CONFIG := wx-config
WX_CXXFLAGS := $(shell $(WX_CONFIG) --cxxflags)
WX_LIBS := $(shell $(WX_CONFIG) --libs)

APPLICATION := ttop
OBJECTS := mainframe.o app.o resultwindow.o

LIBS += -lcurl -ljsoncpp `wx-config --libs net`
LIBS += /usr/lib/x86_64-linux-gnu/libsqlite3.so.0

CXXFLAGS += `wx-config --cxxflags net`

all: $(APPLICATION)

$(APPLICATION): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) $(WX_LIBS) $(LIBS)

$(OBJECTS): %.o: %.cpp
	$(CXX) -c -o $@ $(WX_CXXFLAGS) $(CXXFLAGS) $<

.PHONY: clean
clean:
	find . -name '*~' -o -name '*.o' -o -name $(APPLICATION) | xargs rm


TEST_EXEC := mainframe_tests
TEST_OBJECTS := tests/mainframe_tests.o

.PHONY: test
test: $(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJECTS) mainframe.o resultwindow.o catch_amalgamated.o
	$(CXX) -o $@ $(TEST_OBJECTS) mainframe.o resultwindow.o catch_amalgamated.o $(LDFLAGS) $(WX_LIBS) $(LIBS)

catch_amalgamated.o: tests/catch_amalgamated.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

$(TEST_OBJECTS): tests/%.o: tests/%.cpp
	$(CXX) -c -o $@ $(WX_CXXFLAGS) $(CXXFLAGS) -I./tests $<


.PHONY: clean_test
clean_test:
	rm -f tests/mainframe_tests.o catch_amalgamated.o mainframe_tests
