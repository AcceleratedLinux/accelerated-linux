
# quieter output, disable with V=1
Q = $(if $(V:0=),,@)
BRIEF      = $(if $(V:0=),,@printf '  %-7s %s\n' $1 "$(patsubst $(CURDIR)/%,%,$(or $2,$@))";)
COMPILE.c  = $(call BRIEF,CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cc = $(call BRIEF,CXX)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
LINK.c     = $(call BRIEF,CCLD)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)
LINK.cc    = $(call BRIEF,CXXLD)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)

