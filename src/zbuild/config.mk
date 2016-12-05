# Copyright (c) 2014 - 2016 Roger Hanna
# For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h


#todo: fix dependency failure on deletion of header

default: all

PROJDIR   = $(shell pwd | sed "s/\/src\/.*//")
RPROJNAME = $(shell pwd | sed "s/.*\/src\///")
PROJNAME  = lib
TEMPDIR = $(PROJDIR)/.zproj/temp/$(PLATFORM)_$(CONFIG)
TEMPDIRG = $(PROJDIR)/.zproj/temp_gch/$(PLATFORM)_$(CONFIG)

ifeq ($(CONFIG),debug)
  ZM_DEBUG = 1
endif

ECHO  = echo
RM    = rm
MKDIR = mkdir
CD    = cd
STRIP = strip

ifdef ZM_DEBUG
   CFLAGS += -DZM_DEBUG=1
   CFLAGS += -g
else
   CFLAGS += -O3 -ffast-math
#-msse2
#-mfpmath=sse
   LFLAGS += -dead_strip
   CFLAGS += -fomit-frame-pointer
   CFLAGS += -Wno-unused-parameter
   CFLAGS += -Wno-unused-variable
   CFLAGS += -fno-exceptions
# TODO
#LFLAGS += -gc-sections
#CFLAGS += -ffunction-sections -fdata-sections
#CFLAGS += -flto
endif

include $(PROJDIR)/src/zbuild/config_$(PLATFORM).mk

# generate OBJS from SRC
OBJS = $(patsubst %,$(TEMPDIR)/$(RPROJNAME)/%.o,$(basename $(SRC)))
GCH = $(patsubst %,$(TEMPDIRG)/$(RPROJNAME)/%.h.gch,$(basename $(PCH)))
DEPS = $(subst .o,.d,$(OBJS))
DEPS += $(subst .h.gch,.d,$(GCH))
SUBMAKES = $(patsubst %,$(PROJDIR)/src/%/make,$(PROJS))
SUBMAKESCLEAN = $(patsubst %,$(PROJDIR)/src/%/makeclean,$(PROJS))
AROBJS = $(patsubst %,$(TEMPDIR)/%.a,$(PROJS))
LFLAGS += $(AROBJS) $(AROBJS)
CFLAGS += -Wall
CFLAGS += -I$(TEMPDIRG)/
CFLAGS += -I$(PROJDIR)/src/
CFLAGS += -I$(PROJDIR)/src/examples/

CFLAGS += -Wwrite-strings -Wredundant-decls
CFLAGS += -Wextra -Wshadow

CPPFLAGS += -Wno-invalid-offsetof -Wcast-qual
ARFLAGS = -rc

#flags that seem like they would be nice:
#-Wundef - doesn't work because of undefined values:
#    __STDC_VERSION__, TARGET_OS_MAC, _FILE_OFFSET_BITS, _LARGEFILE64_SOURCE
#-O4

LIBPATH = $(TEMPDIR)/$(RPROJNAME).a
ifdef TARGET
   TARGETPATH = ../../$(TARGETROOT)$(TARGET)$(APPEXT)
   EXTRACLEAN = ../../*~ ../../*/*~ ../../*/*/*~ ../../*/*/*/*~
   EXTRACLEAN += ../../*.orig ../../*/*.orig ../../*/*/*.orig ../../*/*/*/*.orig

run: all
	@$(CD) ../../data; ../$(TARGETROOT)$(TARGET)$(APPEXT) $(RUNPARAMS)

debug: all
	@$(CD) ../../data; gdb -se ../$(TARGETROOT)$(TARGET)$(APPEXT) -q

print:
	@$(ECHO) $< | sed "s#^#make $(RPROJNAME) $(CONFIG) $(PLATFORM)#"

else
   TARGETPATH = $(LIBPATH)

print:

endif

serialgch: $(GCH)

all: print serialgch $(TEMPDIR)/$(RPROJNAME) $(TARGETPATH)
	@echo > /dev/null

$(TEMPDIR)/$(RPROJNAME):
	@$(MKDIR) -p $(TEMPDIR)/$(RPROJNAME)

serialobjs:
	@$(MAKE) -j parallelobjs --no-print-directory

#TODO only exes want submakes
serialsubs:
	@$(MAKE) -j 2 parallelsubs --no-print-directory

parallelobjs: $(OBJS)
	@echo > /dev/null

parallelsubs: $(SUBMAKES)
	@echo > /dev/null

$(SUBMAKES):
	@$(MAKE) -C $(@D) --no-print-directory 

$(SUBMAKESCLEAN):
	@$(MAKE) clean -C $(@D) --no-print-directory 

$(TEMPDIR)/$(RPROJNAME)/%.o: %.c
	@$(ECHO) $< | sed "s#^#CC $(RPROJNAME)\/#"
	@$(MKDIR) -p $(dir $@)
	@$(CC) $(CFLAGS) -o $@ $< -MMD -MF $(subst .o,.d,$@)

$(TEMPDIR)/$(RPROJNAME)/%.o: %.cpp $(GCH)
	@$(ECHO) $< | sed "s#^#CC $(RPROJNAME)\/#"
	@$(MKDIR) -p $(dir $@)
	@$(CCPP) $(CFLAGS) $(CPPFLAGS) -o $@ $< -MMD -MF $(subst .o,.d,$@)

$(TEMPDIRG)/$(RPROJNAME)/%.h.gch: %.h
	@$(MKDIR) -p $(TEMPDIRG)/$(RPROJNAME)
	@$(ECHO) $< | sed "s#^#PCH $(RPROJNAME)\/#"
	@$(CCPP) $(CFLAGS) $(CPPFLAGS) -x c++-header -o $@ $< -MMD -MF $(subst .h.gch,.d,$@)

#TODO: $(OBJS), but not in parallel with serialobjs
$(TEMPDIR)/$(RPROJNAME).a: serialobjs
	@$(ECHO) AR $(RPROJNAME)
	@$(RM) -f $@
	@$(AR) $(ARFLAGS) $@ $(OBJS)

clean: $(SUBMAKESCLEAN) cleantarget
	@$(ECHO) Cleaning $(RPROJNAME)
	@$(RM) -f $(OBJS) $(DEPS) $(GCH) *~ \#* *.orig .\#*
	@$(RM) -f -r $(TEMPDIR)

cleantarget:
	@$(RM) -f $(TARGETPATH) $(EXTRACLEAN)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif
