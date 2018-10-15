# Microsoft Developer Studio Generated NMAKE File, Based on simple.dsp
!IF "$(CFG)" == ""
CFG=SimpH323 - Win32 Release
!MESSAGE No configuration specified. Defaulting to SimpH323 - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "SimpH323 - Win32 Release" && "$(CFG)" != "SimpH323 - Win32 Debug" && "$(CFG)" != "SimpH323 - Win32 No Trace"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simple.mak" CFG="SimpH323 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SimpH323 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SimpH323 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "SimpH323 - Win32 No Trace" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "SimpH323 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\simple.exe"

!ELSE 

ALL : "OpenH323dll - Win32 Release" "$(OUTDIR)\simple.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"OpenH323dll - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\precompile.obj"
	-@erase "$(INTDIR)\simple.pch"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\simple.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W4 /GX /O2 /Ob2 /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D "PTRACING" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\simple.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simple.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=openh323.lib ptclib.lib ptlib.lib comdlg32.lib winspool.lib wsock32.lib mpr.lib kernel32.lib user32.lib gdi32.lib shell32.lib advapi32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\simple.pdb" /machine:I386 /out:"$(OUTDIR)\simple.exe" 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\precompile.obj" \
	"..\..\lib\OpenH323.lib"

"$(OUTDIR)\simple.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SimpH323 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\simple.exe"

!ELSE 

ALL : "OpenH323dll - Win32 Debug" "$(OUTDIR)\simple.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"OpenH323dll - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\precompile.obj"
	-@erase "$(INTDIR)\simple.pch"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\simple.exe"
	-@erase "$(OUTDIR)\simple.ilk"
	-@erase "$(OUTDIR)\simple.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D "PTRACING" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\simple.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simple.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=openh323d.lib ptclibd.lib ptlibd.lib comdlg32.lib winspool.lib wsock32.lib mpr.lib kernel32.lib user32.lib gdi32.lib shell32.lib advapi32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\simple.pdb" /debug /machine:I386 /out:"$(OUTDIR)\simple.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\precompile.obj" \
	"..\..\lib\OpenH323d.lib"

"$(OUTDIR)\simple.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SimpH323 - Win32 No Trace"

OUTDIR=.\NoTrace
INTDIR=.\NoTrace
# Begin Custom Macros
OutDir=.\NoTrace
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\simple.exe"

!ELSE 

ALL : "OpenH323dll - Win32 No Trace" "$(OUTDIR)\simple.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"OpenH323dll - Win32 No TraceCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\precompile.obj"
	-@erase "$(INTDIR)\simple.pch"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\simple.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W4 /GX /O1 /Ob2 /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D "PASN_NOPRINTON" /D "PASN_LEANANDMEAN" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\simple.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simple.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=openh323n.lib ptclib.lib ptlib.lib comdlg32.lib winspool.lib wsock32.lib mpr.lib kernel32.lib user32.lib gdi32.lib shell32.lib advapi32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\simple.pdb" /machine:I386 /out:"$(OUTDIR)\simple.exe" 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\precompile.obj" \
	"..\..\lib\OpenH323n.lib"

"$(OUTDIR)\simple.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("simple.dep")
!INCLUDE "simple.dep"
!ELSE 
!MESSAGE Warning: cannot find "simple.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "SimpH323 - Win32 Release" || "$(CFG)" == "SimpH323 - Win32 Debug" || "$(CFG)" == "SimpH323 - Win32 No Trace"
SOURCE=.\main.cxx

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\simple.pch"


SOURCE=.\precompile.cxx

!IF  "$(CFG)" == "SimpH323 - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /O2 /Ob2 /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D "PTRACING" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\simple.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\precompile.obj"	"$(INTDIR)\simple.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "SimpH323 - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D "PTRACING" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\simple.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\precompile.obj"	"$(INTDIR)\simple.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "SimpH323 - Win32 No Trace"

CPP_SWITCHES=/nologo /MD /W4 /GX /O1 /Ob2 /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D "PASN_NOPRINTON" /D "PASN_LEANANDMEAN" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\simple.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\precompile.obj"	"$(INTDIR)\simple.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

!IF  "$(CFG)" == "SimpH323 - Win32 Release"

"OpenH323dll - Win32 Release" : 
   cd "\Work\openh323"
   $(MAKE) /$(MAKEFLAGS) /F .\OpenH323dll.mak CFG="OpenH323dll - Win32 Release" 
   cd ".\samples\simple"

"OpenH323dll - Win32 ReleaseCLEAN" : 
   cd "\Work\openh323"
   $(MAKE) /$(MAKEFLAGS) /F .\OpenH323dll.mak CFG="OpenH323dll - Win32 Release" RECURSE=1 CLEAN 
   cd ".\samples\simple"

!ELSEIF  "$(CFG)" == "SimpH323 - Win32 Debug"

"OpenH323dll - Win32 Debug" : 
   cd "\Work\openh323"
   $(MAKE) /$(MAKEFLAGS) /F .\OpenH323dll.mak CFG="OpenH323dll - Win32 Debug" 
   cd ".\samples\simple"

"OpenH323dll - Win32 DebugCLEAN" : 
   cd "\Work\openh323"
   $(MAKE) /$(MAKEFLAGS) /F .\OpenH323dll.mak CFG="OpenH323dll - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\samples\simple"

!ELSEIF  "$(CFG)" == "SimpH323 - Win32 No Trace"

"OpenH323dll - Win32 No Trace" : 
   cd "\Work\openh323"
   $(MAKE) /$(MAKEFLAGS) /F .\OpenH323dll.mak CFG="OpenH323dll - Win32 No Trace" 
   cd ".\samples\simple"

"OpenH323dll - Win32 No TraceCLEAN" : 
   cd "\Work\openh323"
   $(MAKE) /$(MAKEFLAGS) /F .\OpenH323dll.mak CFG="OpenH323dll - Win32 No Trace" RECURSE=1 CLEAN 
   cd ".\samples\simple"

!ENDIF 


!ENDIF 

