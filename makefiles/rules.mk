# Override default implicit rules

BUILDDIR := .build-$(TARGET)
OBJDIR := $(BUILDDIR)/obj
DEPDIR := $(BUILDDIR)/dep

CFLAGS += $(addprefix -I,$(INCLUDEPATH))
CFLAGS += $(addprefix -D,$(DEFINES))
CXXFLAGS += $(addprefix -I,$(INCLUDEPATH))
CXXFLAGS += $(addprefix -D,$(DEFINES))

SOURCES_C := $(filter %.c,$(SOURCES))
SOURCES_CXX := $(filter %.cpp,$(SOURCES))
OBJECTS := $(addprefix $(OBJDIR)/,$(SOURCES_C:%.c=%.o) $(SOURCES_CXX:%.cpp=%.o))
DEPENDS := $(addprefix $(DEPDIR)/,$(SOURCES_C:%.c=%.d) $(SOURCES_CXX:%.cpp=%.d))

GENERATED := $(TARGET) $(BUILDDIR)

.PHONY: all clean

all: $(TARGET)

clean:
	@echo -e "\tRM\t$(GENERATED)"
	$(A)rm -rf $(GENERATED) 

$(OBJDIR)/%.o: %.c
	@echo -e "\tCC\t$(<F)"
	$(A)$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: %.cpp
	@echo -e "\tCXX\t$(<F)"
	$(A)$(CXX) $(CXXFLAGS) -o $@ -c $<

$(DEPDIR)/%.d: %.c
	@echo -e "\tDEP\t$(<F)"
	$(A)$(CC) $(CFLAGS) -o $@ -MT "$(OBJDIR)/$*.o" -MM $< 

$(DEPDIR)/%.d: %.cpp
	@echo -e "\tDEP\t$(<F)"
	$(A)$(CXX) $(CXXFLAGS) -o $@ -MT "$(OBJDIR)/$*.o" -MM $< 

$(TARGET): $(OBJECTS)
	@echo -e "\tLD\t$(^F)"
	$(A)$(LINK) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(OBJECTS): Makefile Makefile.client Makefile.server | $(OBJDIR)

$(DEPENDS): Makefile Makefile.client Makefile.server | $(DEPDIR)

$(OBJDIR):
	@mkdir -p $@

$(DEPDIR):
	@mkdir -p $@

-include $(DEPENDS)
