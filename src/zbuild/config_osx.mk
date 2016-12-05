# Copyright (c) 2014 - 2016 Roger Hanna
# For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

APPEXT = .elf

MM = /usr/bin/g++ -c
CCPP = /usr/bin/g++ -c
CC = /usr/bin/gcc -c
LD = /usr/bin/g++
DSYM = dsymutil

CPPFLAGS += -fno-strict-aliasing
CPPFLAGS += -fno-strict-overflow
CFLAGS += -arch i386
LFLAGS += -arch i386

$(TEMPDIR)/$(RPROJNAME)/%.o: %.mm
	@$(ECHO) $< | sed "s/^/CC $(RPROJNAME)\//"
	@$(MM) $(CFLAGS) -o $@ $< -MMD -MF $(subst .o,.d,$@)

%.elf: serialsubs serialobjs
	@$(MKDIR) -p ../../$(TARGETROOT)
	@$(ECHO) LD $@
	@$(LD) $(OBJS) $(LFLAGS) -o $@
ifdef ZM_DEBUG
	@$(DSYM) $@
endif
	@$(STRIP) $@
