# Microsoft Developer Studio Project File - Name="ilc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ilc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ilc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ilc.mak" CFG="ilc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ilc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ilc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ilc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=copy Release\ilc.exe ..\bin\win32\ilc.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ilc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ilc - Win32 Release"
# Name "ilc - Win32 Debug"
# Begin Source File

SOURCE=.\asmop.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\asmop.h
# End Source File
# Begin Source File

SOURCE=.\cexpr.c
# End Source File
# Begin Source File

SOURCE=.\cexpr.h
# End Source File
# Begin Source File

SOURCE=.\compiler.c
# End Source File
# Begin Source File

SOURCE=.\compiler.h
# End Source File
# Begin Source File

SOURCE=.\conf.h
# End Source File
# Begin Source File

SOURCE=.\debug.c
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\emitter.c
# End Source File
# Begin Source File

SOURCE=.\emitter.h
# End Source File
# Begin Source File

SOURCE=.\errlist.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\errlist.h
# End Source File
# Begin Source File

SOURCE=.\errs.c
# End Source File
# Begin Source File

SOURCE=.\errs.h
# End Source File
# Begin Source File

SOURCE=.\ilc.h
# End Source File
# Begin Source File

SOURCE=.\keywords.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\msd.h
# End Source File
# Begin Source File

SOURCE=.\parser.c
# End Source File
# Begin Source File

SOURCE=.\parser.h
# End Source File
# Begin Source File

SOURCE=.\parser_op.c
# End Source File
# Begin Source File

SOURCE=.\parser_p.h
# End Source File
# Begin Source File

SOURCE=.\pass1.c
# End Source File
# Begin Source File

SOURCE=.\pass1.h
# End Source File
# Begin Source File

SOURCE=.\pass2.c
# End Source File
# Begin Source File

SOURCE=.\pass2.h
# End Source File
# Begin Source File

SOURCE=.\rdfile.c
# End Source File
# Begin Source File

SOURCE=.\rdfile.h
# End Source File
# Begin Source File

SOURCE=.\scanner.c
# End Source File
# Begin Source File

SOURCE=.\scanner.h
# End Source File
# Begin Source File

SOURCE=.\scanner_p.h
# End Source File
# Begin Source File

SOURCE=.\symtab.c
# End Source File
# Begin Source File

SOURCE=.\symtab.h
# End Source File
# Begin Source File

SOURCE=.\sysdep.h
# End Source File
# Begin Source File

SOURCE=.\util.c
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\ver.c
# End Source File
# End Target
# End Project
