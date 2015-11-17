CXX=g++
LIBS=gl glu sdl2 glew
LDLIBS=$(shell pkg-config --libs-only-l $(LIBS))
LDFLAGS=$(shell pkg-config --libs-only-L --libs-only-other $(LIBS))
CXXFLAGS=--std=c++11 -g -Wall $(shell pkg-config --cflags $(LIBS))
BUILDDIR=build
BINDIR=bin
OBJ=$(addprefix $(BUILDDIR)/, Main.o)

RM=rm -rf
MKDIR=mkdir

$(BINDIR)/ray-marching: $(OBJ) | $(BINDIR)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(OBJ) $(LDLIBS) -o $@

$(BUILDDIR)/%.o: src/%.cpp | $(BUILDDIR)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILDDIR):
	$(MKDIR) $@

$(BINDIR):
	$(MKDIR) $@

clean:
	$(RM) $(BUILDDIR) $(BINDIR)
