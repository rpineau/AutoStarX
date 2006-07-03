# Microsoft Developer Studio Project File - Name="AU2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=AU2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AU2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AU2.mak" CFG="AU2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AU2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "AU2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/AU2", MXDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AU2 - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Zp1 /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "AU2 - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /Zp1 /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "AU2 - Win32 Release"
# Name "AU2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Autostar\Asteroid.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\AstroBody.cpp
# End Source File
# Begin Source File

SOURCE=.\AU2.cpp
# End Source File
# Begin Source File

SOURCE=.\AU2.rc
# End Source File
# Begin Source File

SOURCE=.\AU2Doc.cpp
# End Source File
# Begin Source File

SOURCE=.\AU2FileDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\AU2ListView.cpp
# End Source File
# Begin Source File

SOURCE=.\AU2ReplaceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AU2View.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\Autostar.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\AutostarModel.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\AutostarStat.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\BILDTOUR.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Autostar\BodyData.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\BodyDataCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\BodyDataFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\BodyDataMaker.cpp
# End Source File
# Begin Source File

SOURCE=.\colorbtn.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\Comet.cpp
# End Source File
# Begin Source File

SOURCE=.\ComPortDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DefineCatalog.cpp
# End Source File
# Begin Source File

SOURCE=.\ErrorLogDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HTTPDownload.cpp
# End Source File
# Begin Source File

SOURCE=.\HTTPDownloadStat.cpp
# End Source File
# Begin Source File

SOURCE=.\Image.cpp
# End Source File
# Begin Source File

SOURCE=.\Label.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\LandMark.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MaskedBitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\Model494.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\Model494_497.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\Model497.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\ModelLX.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\PECTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\Persist.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\Satellite.cpp
# End Source File
# Begin Source File

SOURCE=.\SatelliteDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\SerialPort.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\Site.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Autostar\Tour.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\UserInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\UserObj.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\UserObjEx.cpp
# End Source File
# Begin Source File

SOURCE=.\UserObjSelectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Autostar\UserSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\UserSiteDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Autostar\Asteroid.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\AstroBody.h
# End Source File
# Begin Source File

SOURCE=.\AU2.h
# End Source File
# Begin Source File

SOURCE=.\AU2Doc.h
# End Source File
# Begin Source File

SOURCE=.\AU2FileDialog.h
# End Source File
# Begin Source File

SOURCE=.\AU2ListView.h
# End Source File
# Begin Source File

SOURCE=.\AU2ReplaceDlg.h
# End Source File
# Begin Source File

SOURCE=.\AU2View.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\Autostar.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\AutostarModel.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\AutostarStat.h
# End Source File
# Begin Source File

SOURCE=.\BitmapDemo.h
# End Source File
# Begin Source File

SOURCE="..\Preliminary Objects\BitmapDemo.h"
# End Source File
# Begin Source File

SOURCE=.\Autostar\BodyData.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\BodyDataCollection.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\BodyDataFactory.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\BodyDataMaker.h
# End Source File
# Begin Source File

SOURCE=.\colorbtn.h
# End Source File
# Begin Source File

SOURCE=.\ColorCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\Comet.h
# End Source File
# Begin Source File

SOURCE=.\ComPortDlg.h
# End Source File
# Begin Source File

SOURCE=.\DefineCatalog.h
# End Source File
# Begin Source File

SOURCE=.\ErrorLogDlg.h
# End Source File
# Begin Source File

SOURCE=.\HTTPDownload.h
# End Source File
# Begin Source File

SOURCE=.\HTTPDownloadStat.h
# End Source File
# Begin Source File

SOURCE=.\Image.h
# End Source File
# Begin Source File

SOURCE=.\Label.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\LandMark.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MaskedBitmap.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\Model494.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\Model494_497.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\Model497.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\ModelLX.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\PECTable.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\Persist.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\Satellite.h
# End Source File
# Begin Source File

SOURCE=.\SatelliteDlg.h
# End Source File
# Begin Source File

SOURCE=.\SelectDlg.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\SerialPort.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\Site.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\Tour.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\UserInfo.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\UserObj.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\UserObjEx.h
# End Source File
# Begin Source File

SOURCE=.\UserObjSelectDlg.h
# End Source File
# Begin Source File

SOURCE=.\Autostar\UserSettings.h
# End Source File
# Begin Source File

SOURCE=.\UserSiteDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\AU2.ico
# End Source File
# Begin Source File

SOURCE=.\res\AU2.rc2
# End Source File
# Begin Source File

SOURCE=.\res\AU2Doc.ico
# End Source File
# Begin Source File

SOURCE=.\res\ETX.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\rosette.jpg
# End Source File
# Begin Source File

SOURCE=.\res\S_S_Setup.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Title.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter ".rtf;.reg;.cnt;.hlp;.log;.ph"
# Begin Source File

SOURCE=".\hlp\505-1.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\505-1hires.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\505-2.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\505-2hires.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\506-1.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\506-1hires.bmp"
# End Source File
# Begin Source File

SOURCE=.\hlp\AfxCore.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AppExit.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\AU2.cnt
# End Source File
# Begin Source File

SOURCE=.\hlp\AU2.FTS
# End Source File
# Begin Source File

SOURCE=.\hlp\AU2.gid
# End Source File
# Begin Source File

SOURCE=.\hlp\AU2.hlp
# End Source File
# Begin Source File

SOURCE=.\hlp\AU2.hm
# End Source File
# Begin Source File

SOURCE=.\hlp\AU2.hpj
# End Source File
# Begin Source File

SOURCE=.\hlp\AU2.LOG
# End Source File
# Begin Source File

SOURCE=.\hlp\AU2.ph
# End Source File
# Begin Source File

SOURCE=.\hlp\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw2.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw4.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCopy.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCut.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditPast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditUndo.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileNew.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FilePrnt.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileSave.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpSBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpTBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\main_dialog.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\main_dialog_hires.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\main_dialog_notext.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecFirst.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecLast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecNext.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecPrev.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmax.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\ScMenu.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmin.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\notes.txt
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\release_notes.txt
# End Source File
# Begin Source File

SOURCE=.\testcriteria.txt
# End Source File
# End Target
# End Project
