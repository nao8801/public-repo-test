# Makefile for M88 SDL2 Prototype Tests

CXX = g++
CXXFLAGS = -Wall -O2 -std=c++11 $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2) -lm

TARGETS = test_graphics test_audio test_audio_debug test_audio_terminal hal_demo

all: $(TARGETS)

test_graphics: test_graphics.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test_audio: test_audio.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test_audio_debug: test_audio_debug.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test_audio_terminal: test_audio_terminal.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

hal_demo: hal_demo.cpp hal_sdl2_impl.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
