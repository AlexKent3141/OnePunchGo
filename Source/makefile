APP=OPG.out
CXXFLAGS=-Wall -std=c++11 -O3
LDFLAGS=-lpthread

src=$(shell find ./ -type f -name '*.cpp')
obj=$(src:.cpp=.o)

build_all: $(obj)
	$(CXX) $^ -o $(APP) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(APP)
