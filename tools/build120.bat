CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\VsDevCmd.bat"
msbuild /p:Configuration=Debug;Platform=x86 Microsoft.WindowsAzure.Storage\Microsoft.WindowsAzure.Storage.v120.vcxproj
msbuild /p:Configuration=Debug;Platform=x64 Microsoft.WindowsAzure.Storage\Microsoft.WindowsAzure.Storage.v120.vcxproj
msbuild /p:Configuration=Release;Platform=x86 Microsoft.WindowsAzure.Storage\Microsoft.WindowsAzure.Storage.v120.vcxproj
msbuild /p:Configuration=Release;Platform=x64 Microsoft.WindowsAzure.Storage\Microsoft.WindowsAzure.Storage.v120.vcxproj