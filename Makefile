PLAT= none

PLATS= aix bsd c89 freebsd generic linux macosx mingw posix solaris

# Targets start here.
all:	$(PLAT)

$(PLATS) clean:
	cd 3rd/lua-5.3.5 && $(MAKE) $@ && $(MAKE) install

none:
	@echo "Please do 'make PLATFORM' where PLATFORM is one of these:"
	@echo "   $(PLATS)"
	@echo "See doc/readme.html for complete instructions."