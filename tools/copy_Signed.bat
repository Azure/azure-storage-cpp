@echo off

pushd %~dp0

if NOT EXIST %1\Signed echo %1\Signed not exists && goto copyfailed

echo Copying signed DLLs and the pdbs to the final drop location...
copy /y %1\Signed\v140_Win32_Debug_wastoraged.dll ..\Microsoft.WindowsAzure.Storage\v140\Win32\Debug\wastoraged.dll
copy /y %1\Signed\v140_x64_Debug_wastoraged.dll ..\Microsoft.WindowsAzure.Storage\v140\x64\Debug\wastoraged.dll
copy /y %1\Signed\v140_Win32_Release_wastorage.dll ..\Microsoft.WindowsAzure.Storage\v140\Win32\Release\wastorage.dll
copy /y %1\Signed\v140_x64_Release_wastorage.dll ..\Microsoft.WindowsAzure.Storage\v140\x64\Release\wastorage.dll
copy /y %1\Signed\v120_Win32_Debug_wastoraged.dll ..\Microsoft.WindowsAzure.Storage\v120\Win32\Debug\wastoraged.dll
copy /y %1\Signed\v120_x64_Debug_wastoraged.dll ..\Microsoft.WindowsAzure.Storage\v120\x64\Debug\wastoraged.dll
copy /y %1\Signed\v120_Win32_Release_wastorage.dll ..\Microsoft.WindowsAzure.Storage\v120\Win32\Release\wastorage.dll
copy /y %1\Signed\v120_x64_Release_wastorage.dll ..\Microsoft.WindowsAzure.Storage\v120\x64\Release\wastorage.dll
if %ERRORLEVEL% neq 0 goto copyfailed
echo OK

popd
exit /b 0

:copyfailed

echo FAILED. Unable to copy DLLs

popd
exit /b -1