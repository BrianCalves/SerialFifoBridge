

ARFLAGS	=	r

CPPFLAGS=	-g -D MAC_OS_X

SRCS	=	\

INCDIRS	=	
LIBDIRS	=	-L.
OBJS	=	$(subst .cc,.o,$(SRCS))
#DEPS	=	$(subst .cc,.d,$(SRCS))
DEPS	=	$(subst .cc,.d,$(SRCS)) $(patsubst %,%.d,$(BINS))
LIB	=	libSerialFifoBridge.a

# LIBS	=	-lSerialFifoBridge -lstdc++ -lm
LIBS	=	-lstdc++ -lm
BINS	=	serial2fifo

.SUFFIXES: .d
.PRECIOUS: $(OBJS) $(DEPS) $(BINS)
.PHONY:	all tidy clean

# all:	$(LIB) $(BINS)
all:	$(BINS)

$(LIB):	$(OBJS)
	@echo "    o Archiving $@"
	@$(AR) $(ARFLAGS) $@ $^ >& /dev/null

tidy:
	@echo "    o Tidying"
	@$(RM) $(OBJS) $(DEPS)

clean:
	@echo "    o Cleaning"
	@$(RM) $(LIB) $(BINS) $(patsubst %,%.o,$(BINS)) $(OBJS) $(DEPS)

%.d:	%.cc
	@$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

%.o:	%.cc
	@echo "    o Compiling $@"
	@$(COMPILE.cc) $(OUTPUT_OPTION) $<

# %: %.o $(LIB)
%: %.o
	@echo "    o Linking $@"
	@$(LINK.o) $^ $(LIBDIRS) -o $@ $(LIBS)

-include $(DEPS)

