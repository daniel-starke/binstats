APPS = binstats

binstats_version = 1.2.0
binstats_version_date = 2019-01-04
binstats_author = Daniel Starke

CPPFLAGS += '-DBINSTATS_VERSION="$(binstats_version) ($(binstats_version_date))"' '-DBINSTATS_AUTHOR="$(binstats_author)"'

binstats_obj = \
	libpcf/fdio \
	libpcf/fdios \
	libpcf/fdious \
	pcf/gui/DroppableReadOnlyInput \
	pcf/gui/SymbolViewer \
	pcf/gui/Utility \
	binstats

binstats_lib = \
	libfltk \
	libgdi32 \
	libole32 \
	libuuid \
	libcomctl32 \
	libcomdlg32

all: $(DSTDIR) $(LIBDIR) $(INCDIR) $(addprefix $(DSTDIR)/,$(addsuffix $(BINEXT),$(APPS)))

.PHONY: $(DSTDIR)
$(DSTDIR):
	mkdir -p $(DSTDIR)

.PHONY: $(LIBDIR)
$(LIBDIR):
	mkdir -p $(LIBDIR)

.PHONY: $(INCDIR)
$(INCDIR):
	mkdir -p $(INCDIR)

.PHONY: distclean
distclean:
	$(RM) $(DSTDIR)/*$(OBJEXT)
	$(RM) $(addprefix $(DSTDIR)/,$(addsuffix $(BINEXT),$(APPS)))
	$(RM) -r $(DSTDIR) $(LIBDIR) $(INCDIR)

.PHONY: clean
clean:
	$(RM) $(DSTDIR)/*$(OBJEXT)
	$(RM) $(addprefix $(DSTDIR)/,$(addsuffix $(BINEXT),$(APPS)))
	$(RM) -r $(DSTDIR)

$(DSTDIR)/binstats$(BINEXT): $(addprefix $(DSTDIR)/,$(addsuffix $(OBJEXT),$(binstats_obj)))
	$(AR) rs $(DSTDIR)/binstats.a $+
	$(LD) $(LDFLAGS) -mwindows -o $@ $(DSTDIR)/binstats.a $(binstats_lib:lib%=-l%)

$(DSTDIR)/%$(OBJEXT): $(SRCDIR)/%$(CEXT)
	mkdir -p "$(dir $@)"
	$(CC) $(CWFLAGS) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(DSTDIR)/%$(OBJEXT): $(SRCDIR)/%$(CXXEXT)
	mkdir -p "$(dir $@)"
	$(CXX) $(CWFLAGS) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(SRCDIR)/license.hpp: $(DOCDIR)/COPYING $(SCRIPTDIR)/convert-license.sh
	binstats_author="$(binstats_author)" $(SCRIPTDIR)/convert-license.sh $(DOCDIR)/COPYING $@

# dependencies
$(DSTDIR)/binstats$(OBJEXT): \
	$(SRCDIR)/pcf/gui/DroppableReadOnlyInput.hpp \
	$(SRCDIR)/pcf/gui/SymbolViewer.hpp \
	$(SRCDIR)/pcf/gui/Utility.hpp \
	$(SRCDIR)/pcf/Utility.hpp \
	$(SRCDIR)/binstats.hpp
$(DSTDIR)/pcf/gui/SymbolViewer$(OBJEXT): \
	$(SRCDIR)/pcf/gui/DroppableReadOnlyInput.hpp \
	$(SRCDIR)/pcf/gui/SymbolViewer.hpp \
	$(SRCDIR)/pcf/gui/Utility.hpp \
	$(SRCDIR)/pcf/Utility.hpp \
	$(SRCDIR)/license.hpp
