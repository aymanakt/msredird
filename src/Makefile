DEP_DIR    := .dep
OBJDIR     := obj
BINDIR     := ../bin

EFILE      := msredird
INCLUDES   := -I./include
SHELL      := /bin/sh
CFLAGS     := $(INCLUDES) -ggdb3
VERSION    := 0.2
CC         := gcc
DEFS       := -DHAVE_CONFIG_H
LIBS       := -lpthread
STRIP      :=


vpath %.h include

SOURCES      := $(wildcard *.c)
OBJECTS      := $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES))
DEP_FILES    := $(patsubst %.c,$(DEP_DIR)/%.d,$(SOURCES))

#
# Cancel built-in implicit rule for %.o
#
%.o : %.c

#
#Redifine the implicit rule fo %.o
#
$(OBJDIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $(DEFS) $< -o $@


.PHONY: all
all : whatsup version $(EFILE)


$(EFILE): $(OBJECTS)
	@echo
	@echo " Linking object files..."
	@echo
	$(CC)  $^ -o $(BINDIR)/$(EFILE) $(LIBS) $(STRIP)


include $(DEP_FILES)


.PHONY: whatsup
whatsup:
	@echo
	@echo " `date '+ %A, %d %B %Y'`"
	@echo " Generating object files for $(EFILE) $(VERSION)..."
	@echo


$(DEP_DIR)/%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< \
       | sed '\''s/\($*\)\.o[ :]*/$(OBJDIR)\/\1.o $(@D)\/$(@F) : /g'\'' > $@; \
       [ -s $@ ] || rm -f $@'


.PHONY: version
version:
	@chmod 755 version.c.in; ./version.c.in


.PHONY: tags
tags:
	@echo "Updating tags file..."
	@ctags --recurse=yes --sort=yes *.[ch]

.PHONY: clean
clean:
	rm -rf obj/*.o
	rm -rf .dep/*.d
	rm -rf ../bin/*

.PHONY: install
install:
	@if [ ! -r /etc/msredird ]; then \
	echo "Creating configuration directory /etc/msredird..."; \
	mkdir -p /etc/msredird/log; \
	cp ../etc/msredird.conf /etc/msredird; \
	fi
	@echo "Copy the resulting binary to your favourite bin dir."
