rem Set build and code folder
rem Sample of the buildroot/coderoot: c:\cpp\
set buildroot=%1
set coderoot=%2

rem Set test_configurations.json's set number. This is for parallel execution. On Jenkins, v120-debug-=1, v120-release=2, v140-debug=3, v140-release=4
set testconfigset=%3

rem Set the toolset
set toolset=%4

rem Get the casablanca version
set casaver=2.10.6

rem Build the packages.config file
@echo 
set packagesfile=%buildroot%packages.config
echo ^<?xml version=^"1.0^" encoding=^"utf-8^"?^> > "%packagesfile%"
echo ^<packages^> >> "%packagesfile%"
if "%toolset%" == "v120" echo   ^<package id=^"casablanca.v120^" version=^"%casaver%^" targetFramework=^"Native^" /^> >> "%packagesfile%"
if "%toolset%" == "v140" echo   ^<package id=^"casablanca.v140^" version=^"%casaver%^" targetFramework=^"Native^" /^> >> "%packagesfile%"
echo ^</packages^> >> "%packagesfile%"

rem Copy the packages.config file to code/test folder
Copy "%buildroot%packages.config" "%coderoot%Microsoft.WindowsAzure.Storage\packages.config"
Copy "%buildroot%packages.config" "%coderoot%Microsoft.WindowsAzure.Storage\tests\packages.config"

rem Copy the test_configurations.json
if exist "%buildroot%test_configurations_%testconfigset%.json" copy "%buildroot%test_configurations_%testconfigset%.json" "%coderoot%Microsoft.WindowsAzure.Storage\tests\test_configurations.json"

rem Inject the package information to the .vcxproj file.

rem v120
if "%toolset%" == "v120" (
  Echo Writing nuget package information to Microsoft.WindowsAzure.Storage.v120.vcxproj file.
  CALL :InjectPackageInfoProd "%coderoot%Microsoft.WindowsAzure.Storage\Microsoft.WindowsAzure.Storage.v120.vcxproj" v120 %casaver% "Microsoft.WindowsAzure.Storage.v120.vcxproj"
  Echo Writing nuget package information to Microsoft.WindowsAzure.Storage.UnitTests.v120.vcxproj file.
  CALL :InjectPackageInfoTest "%coderoot%Microsoft.WindowsAzure.Storage\tests\Microsoft.WindowsAzure.Storage.UnitTests.v120.vcxproj" v120 %casaver% "Microsoft.WindowsAzure.Storage.UnitTests.v120.vcxproj"
)
rem v140
if "%toolset%" == "v140" (
  Echo Writing nuget package information to Microsoft.WindowsAzure.Storage.v140.vcxproj file.
  CALL :InjectPackageInfoProd "%coderoot%Microsoft.WindowsAzure.Storage\Microsoft.WindowsAzure.Storage.v140.vcxproj" v140 %casaver% "Microsoft.WindowsAzure.Storage.v140.vcxproj"
  Echo Writing nuget package information to Microsoft.WindowsAzure.Storage.UnitTests.v140.vcxproj file.
  CALL :InjectPackageInfoTest "%coderoot%Microsoft.WindowsAzure.Storage\tests\Microsoft.WindowsAzure.Storage.UnitTests.v140.vcxproj" v140 %casaver% "Microsoft.WindowsAzure.Storage.UnitTests.v140.vcxproj"
)

rem Copy Unitest++
xcopy /s %buildroot%unittest-cpp_%toolset% %coderoot%Microsoft.WindowsAzure.Storage\tests\Unittest++

GOTO :EOF

rem Function used to inject the package information
:InjectPackageInfoProd
@echo off
setlocal EnableExtensions EnableDelayedExpansion
set outputfile=%~1
set inputfile=%~4.bak
set inputfilefull=%~1.bak
set platform=%~2
set casaversion=%~3
set stringtofind="  <ImportGroup Label="ExtensionTargets">"
ren %outputfile% %inputfile%
(
    echo ^<?xml version=^"1.0^" encoding=^"utf-8^"?^>
    for /f usebackq^ skip^=1^ delims^=^ eol^=  %%a in ("%inputfilefull%") do (
        echo %%a
        if "%%~a" == %stringtofind% echo     ^<Import Project=^"..\..\packages\casablanca.%platform%.%casaversion%\build\native\casablanca.%platform%.targets^" Condition=^"Exists^(^'..\..\packages\casablanca.%platform%.%casaversion%\build\native\casablanca.%platform%.targets^'^)^" ^/^>
))>"%outputfile%" 
@echo on
EXIT /B 0

:InjectPackageInfoTest
@echo off
setlocal EnableExtensions EnableDelayedExpansion
set outputfile=%~1
set inputfile=%~4.bak
set inputfilefull=%~1.bak
set platform=%~2
set casaversion=%~3
set stringtofind="  <ImportGroup Label="ExtensionTargets">"
ren %outputfile% %inputfile%
(
    echo ^<?xml version=^"1.0^" encoding=^"utf-8^"?^>
    for /f usebackq^ skip^=1^ delims^=^ eol^=  %%a in ("%inputfilefull%") do (
        echo %%a
        if "%%~a" == %stringtofind% echo     ^<Import Project=^"..\..\..\packages\casablanca.%platform%.%casaversion%\build\native\casablanca.%platform%.targets^" Condition=^"Exists^(^'..\..\..\packages\casablanca.%platform%.%casaversion%\build\native\casablanca.%platform%.targets^'^)^" ^/^>
))>"%outputfile%" 
@echo on
EXIT /B 0
