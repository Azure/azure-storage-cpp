@echo off

pushd %~dp0

echo Clean up the existing ToSign folder
if EXIST %1\ToSign rmdir /s /q %1\ToSign
mkdir %1\ToSign
if %ERRORLEVEL% neq 0 goto copyfailed
echo OK

echo Copying DLLs to signing source directory
copy /y ..\Microsoft.WindowsAzure.Storage\v140\Win32\Debug\wastorage.dll %1\ToSign\v140_Win32_Debug_wastorage.dll
copy /y ..\Microsoft.WindowsAzure.Storage\v140\x64\Debug\wastorage.dll %1\ToSign\v140_x64_Debug_wastorage.dll
copy /y ..\Microsoft.WindowsAzure.Storage\v140\Win32\Release\wastorage.dll %1\ToSign\v140_Win32_Release_wastorage.dll
copy /y ..\Microsoft.WindowsAzure.Storage\v140\x64\Release\wastorage.dll %1\ToSign\v140_x64_Release_wastorage.dll
copy /y ..\Microsoft.WindowsAzure.Storage\v120\Win32\Debug\wastorage.dll %1\ToSign\v120_Win32_Debug_wastorage.dll
copy /y ..\Microsoft.WindowsAzure.Storage\v120\x64\Debug\wastorage.dll %1\ToSign\v120_x64_Debug_wastorage.dll
copy /y ..\Microsoft.WindowsAzure.Storage\v120\Win32\Release\wastorage.dll %1\ToSign\v120_Win32_Release_wastorage.dll
copy /y ..\Microsoft.WindowsAzure.Storage\v120\x64\Release\wastorage.dll %1\ToSign\v120_x64_Release_wastorage.dll
if %ERRORLEVEL% neq 0 goto copyfailed
echo OK

popd
exit /b 0

:copyfailed

echo FAILED. Unable to copy DLLs
popd
exit /b -1