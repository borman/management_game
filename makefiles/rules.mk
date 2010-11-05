# Override default implicit rules

OBJDIR := build/obj
DEPDIR := build/dep

CFLAGS += $(addprefix -I,$(INCLUDEPATH))
CFLAGS += $(addprefix -D,$(DEFINES))
OBJECTS := $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,$(DEPDIR)/%.d,$(SOURCES))

GENERATED := $(TARGET) 

.PHONY: all clean

all: $(TARGET)

clean:
	@echo -e "\tRM\t$(GENERATED)"
	$(A)rm -f $(GENERATED) 
	@echo -e "\tRM\tbuild"
	$(A)rm -rf build 

$(OBJDIR)/%.o: %.c
	@echo -e "\tCC\t$<"
	$(A)$(CC) $(CFLAGS) -o $@ -c $<

$(DEPDIR)/%.d: %.c
	@echo -e "\tDEP\t$<"
	$(A)$(CC) $(CFLAGS) -o $@ -MT "$(OBJDIR)/$*.o" -MM $< 

$(TARGET): $(OBJECTS)
	@echo -e "\tLD\t$^"
	$(A)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(OBJECTS): Makefile | $(OBJDIR)

$(DEPENDS): Makefile | $(DEPDIR)

$(OBJDIR):
	@mkdir -p $@

$(DEPDIR):
	@mkdir -p $@

-include $(DEPENDS)
