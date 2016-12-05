# Copyright (c) 2014 - 2016 Roger Hanna
# For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

# this is the makefile data for the root folder

default: app

dozmake:
ifeq (,$(wildcard ../src/zbuild/zmake/zmake.elf))
	@echo warning: zmake.elf is missing
else
	@echo Zmake Command
	@cd zbuild/zmake;./zmake.elf zmake-config.ztxt --make
endif

debug: dozmake
	@cd zmake;make CONFIG=debug debug

clean:
	@echo Cleaning root
	@rm -f *~ \#* *.orig
	@rm -f -r ../.zproj/temp
	@rm -f -r ../.zproj/temp_gch
	@rm -f -r zbuild/zmake/zmake.elf

app: dozmake
	@cd link_zmake;make
