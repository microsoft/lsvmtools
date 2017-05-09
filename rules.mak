##==============================================================================
##
## all: jump to target rule
##
##==============================================================================

top: all

##==============================================================================
##
## __OBJECTS: full object paths:
##
##==============================================================================

ifndef DIRECTORY
$(error "please define DIRECTORY variable")
endif

SUBOBJDIR=$(OBJDIR)/$(DIRECTORY)

__OBJECTS = $(addprefix $(SUBOBJDIR)/,$(OBJECTS))

##==============================================================================
##
## %.o: build and object file
##
##==============================================================================

$(SUBOBJDIR)/%.o: %.c
	mkdir -p $(SUBOBJDIR)
	gcc -c $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

##==============================================================================
##
## depend: build dependencies
##
##==============================================================================

define NL


endef

depend:
	@ rm -f depend.mak
	@ $(foreach i, $(SOURCES), gcc -M -MG $(DEFINES) $(INCLUDES) $(i) -MT $(SUBOBJDIR)/$(i:.c=.o) >> depend.mak $(NL) )

##==============================================================================
##
## $(ARCHIVE): build archive (.a file)
##
##==============================================================================

ifdef ARCHIVE

__ARCHIVE = $(addprefix $(LIBDIR)/,$(ARCHIVE))

TARGET = $(__ARCHIVE)

__MAKEFILENAME = $(firstword $(MAKEFILE_LIST))

all: depend
	$(MAKE) -f $(__MAKEFILENAME) target

target: $(__ARCHIVE) done

$(__ARCHIVE): $(__OBJECTS)
	mkdir -p $(LIBDIR)
	ar rv $(__ARCHIVE) $(__OBJECTS)

endif

##==============================================================================
##
## $(PROGRAM): build a C program
##
##==============================================================================

ifdef PROGRAM

__PROGRAM = $(addprefix $(BINDIR)/,$(PROGRAM))

TARGET = $(__PROGRAM)

__MAKEFILENAME = $(firstword $(MAKEFILE_LIST))

all: depend
	$(MAKE) -f $(__MAKEFILENAME) target

target: $(__PROGRAM) done

$(__PROGRAM): $(__OBJECTS) $(LIBDEPENDS)
	mkdir -p $(BINDIR)
	gcc -o $(__PROGRAM) $(LINKFLAGS) $(__OBJECTS) $(LIBRARIES)

endif

##==============================================================================
##
## Create TMPDIR:
##
##==============================================================================

$(shell mkdir -p $(TMPDIR))

##==============================================================================
##
## clean: clean build files
##
##==============================================================================

clean:
	rm -rf $(TARGET) $(__OBJECTS) $(CLEAN)

##==============================================================================
##
## done:
##
##==============================================================================

done:
