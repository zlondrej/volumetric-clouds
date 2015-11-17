CXX=g++
LDLIBS=$(shell pkg-config --libs GL SLD2 glut)
# -lGL -lglut -lGLEW -lGLU -ljpeg -L./ -ljread
CXXFLAGS=-g -Wall
BUILDDIR=build
BINDIR=bin
OBJ=$(addprefix $(BUILDDIR)/, Main.o)

RM=rm -rf
MKDIR=mkdir

$(BINDIR)/ray-marching: $(OBJ) | $(BINDIR)
	$(CXX) $(OBJ) $(CXXFLAGS) $(LDFLAGS) -o $@

$(BUILDDIR)/%.o: src/%.cpp | $(BUILDDIR)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILDDIR):
	$(MKDIR) $@

$(BINDIR):
	$(MKDIR) $@

clean:
	$(RM) $(BUILDDIR) $(BINDIR)
