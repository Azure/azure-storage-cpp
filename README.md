# Azure Storage Client Library for C++ (6.0.0)

The Azure Storage Client Library for C++ allows you to build applications against Microsoft Azure Storage. For an overview of Azure Storage, see [Introduction to Microsoft Azure Storage](http://azure.microsoft.com/en-us/documentation/articles/storage-introduction/).

There is an alternative client library that requires minimum dependency, which provides basic object storage that Blob service offers. Please see [azure-storage-cpplite](https://github.com/Azure/azure-storage-cpplite) for more information.

# Features

- Tables
    - Create/Delete Tables
    - Query/Create/Read/Update/Delete Entities
- Blobs
    - Create/Read/Update/Delete Blobs
- Queues
    - Create/Delete Queues
    - Insert/Peek Queue Messages
    - Advanced Queue Operations
- Files
    - Create/Delete/Resize Shares
    - Create/Delete Directories
    - Create/Read/Update/Delete Files

# Getting started

For the best development experience, we recommend that developers use the [Vcpkg](https://github.com/Microsoft/vcpkg) as the cross-platform library manager.

## Requirements

To call Azure services, you must first have an Azure subscription. Sign up for a [free trial](https://azure.microsoft.com/en-us/pricing/free-trial/) or use your [MSDN subscriber benefits](https://azure.microsoft.com/en-us/pricing/member-offers/msdn-benefits-details/).

## Need Help?

Be sure to check out the [Azure Storage Forum](https://social.msdn.microsoft.com/Forums/azure/en-US/home?forum=windowsazuredata) on MSDN if you need help, or use [StackOverflow](http://stackoverflow.com/questions/tagged/azure).

## Collaborate & Contribute

We gladly accept community contributions.

- **Issues:** Report bugs on the [Issues page](https://github.com/Azure/azure-storage-cpp/issues) in GitHub.
- **Forums:** Communicate with the Azure Storage development team on the [Azure Storage Forum](https://social.msdn.microsoft.com/Forums/azure/en-US/home?forum=windowsazuredata) or [StackOverflow](http://stackoverflow.com/questions/tagged/azure).
- **Source Code Contributions:** Please follow the [contribution guidelines for Azure open source](http://azure.github.io/guidelines/) for instructions about contributing to the source project.

For general suggestions about Azure, use our [Azure feedback forum](http://feedback.azure.com/forums/34192--general-feedback).

## Download & Install

### Via Git

To create a local clone of the source for the Azure Storage Client Library for C++ via `git`, type:

```bash
git clone https://github.com/Azure/azure-storage-cpp.git
cd azure-storage-cpp
```

To build with source code, there are three ways:

**Via Vcpkg**
You can manage the dependencies with Vcpkg, and use Visual Studio 2015 update 3 or Visual Studio 2017 for development environment. Simply install Casablanca via Vcpkg will setup everything needed.
```
C:\src\vcpkg> .\vcpkg install cpprestsdk
```

**Via NuGet**
Because Casablanca does not release NuGet packages anywhere anymore, Starting from 5.1.0, this repository cannot be built with pre-built Casablanca NuGet packages. However, you can export your own version of Casablanca NuGet packages to install dependencies of this project:
```
C:\src\vcpkg> .\vcpkg install cpprestsdk
C:\src\vcpkg> .\vcpkg export --nuget cpprestsdk --nuget-id=Casablanca --nuget-version=2.10.10
```

**Manage dependencies by yourself**
It is not recommended to manage dependencies by yourself. However, you can still build Casablanca by yourself and specify the include directories and link binaries.

### Via NuGet

To install the binaries for the Azure Storage Client Library for C++, you can export a NuGet package with Vcpkg and put it into your local NuGet feed. For more information about how to export a NuGet package, please see [Binary Export](https://github.com/Microsoft/vcpkg/blob/master/docs/specifications/export-command.md).

Normally, exporting NuGet package is done with the following command:
```
C:\src\vcpkg> .\vcpkg export --nuget azure-storage-cpp --nuget-id=Microsoft.Azure.Storage.CPP --nuget-version=6.0.0
```

### Via Vcpkg

To install the Azure Storage Client Library for C++ through Vcpkg, you need Vcpkg installed first. Please follow the instructions(https://github.com/Microsoft/vcpkg#quick-start) to install Vcpkg.

install package with:
```
C:\src\vcpkg> .\vcpkg install azure-storage-cpp
```

#### Visual Studio Version
Starting from version 6.1.0, Azure Storage Client Library for C++ supports Visual Studio 2015 and Visual Studio 2017. In case you have the need to use Visual Studio 2013, please get [version 6.0.0](https://github.com/Azure/azure-storage-cpp/releases/tag/v6.0.0), to use Visual Studio 2012, please get [version 2.0.0](http://www.nuget.org/packages/wastorage/2.0.0).

## Dependencies

### C++ REST SDK

The Azure Storage Client Library for C++ depends on the C++ REST SDK (codename "Casablanca") It can be installed through Vcpkg (vcpkg install cpprestsdk) or downloaded directly from [GitHub](https://github.com/Microsoft/cpprestsdk/releases/).

The validated Casablanca version for each major or recent release on different platforms can be found in the following chart:


| azure-storage-cpp's version | Casablanca version for Windows | Casablanca version for Linux |
|-----------------------------|--------------------------------|------------------------------|
| 1.0.0                       | 2.4.0                          | 2.4.0                        |
| 2.0.0                       | 2.4.0                          | 2.4.0                        |
| 3.0.0                       | 2.9.1                          | 2.9.1                        |
| 4.0.0                       | 2.9.1                          | 2.9.1                        |
| 5.0.0                       | 2.9.1                          | 2.9.1                        |
| 5.0.1                       | 2.9.1                          | 2.9.1                        |
| 5.1.0                       | 2.10.6                         | 2.10.3                       |
| 5.1.1                       | 2.10.6                         | 2.10.3                       |
| 5.2.0                       | 2.10.6                         | 2.10.3                       |
| 6.0.0                       | 2.10.10                        | 2.10.10                      |

## Code Samples

To get started with the coding, please visit the following articles:
- [How to use Blob Storage from C++](https://azure.microsoft.com/documentation/articles/storage-c-plus-plus-how-to-use-blobs/)
- [How to use Table Storage from C++](https://azure.microsoft.com/documentation/articles/storage-c-plus-plus-how-to-use-tables/)
- [How to use Queue Storage from C++](https://azure.microsoft.com/documentation/articles/storage-c-plus-plus-how-to-use-queues)

To accomplish specific tasks, please find the code samples at [samples folder](Microsoft.WindowsAzure.Storage/samples).

## Getting Started on Linux

### Getting Started on Ubuntu
As mentioned above, the Azure Storage Client Library for C++ depends on Casablanca. Follow [these instructions](https://github.com/Microsoft/cpprestsdk/wiki/How-to-build-for-Linux) to compile it.

Once this is complete, then:

- Clone the project using git:
```bash
git clone https://github.com/Azure/azure-storage-cpp.git
```
The project is cloned to a folder called `azure-storage-cpp`. Always use the master branch, which contains the latest release.
- Install additional dependencies:
```bash
sudo apt-get install libxml2-dev uuid-dev
```
- Build the SDK for Release:
```bash
cd azure-storage-cpp/Microsoft.WindowsAzure.Storage
mkdir build.release
cd build.release
CASABLANCA_DIR=<path to Casablanca> CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
In the above command, replace `<path to Casablanca>` to point to your local installation of Casablanca. For example, if the file `libcpprest.so` exists at location `~/Github/Casablanca/cpprestsdk/Release/build.release/Binaries/libcpprest.so`, then your `cmake` command should be:
```bash
CASABLANCA_DIR=~/Github/Casablanca/cpprestsdk CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release
```
The library is generated under `azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.release/Binaries/`.

To build and run unit tests:
- Install UnitTest++ library:
```bash
sudo apt-get install libunittest++-dev
```
- Build the test code:
```bash
CASABLANCA_DIR=<path to Casablanca> CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
make
```
- Run unit tests
```bash
cd Binaries
vi test_configurations.json # modify test config file to include your storage account credentials
./azurestoragetest
```

To build sample code:
```bash
CASABLANCA_DIR=<path to Casablanca> CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON
make
```
To run the samples:
```bash
cd Binaries
vi ../../samples/SamplesCommon/samples_common.h # modify connection string to include your storage account credentials
./samplesblobs            # run the blobs sample
./samplesjson             # run the tables sample with JSON payload
./samplestables           # run the tables sample
./samplesqueues           # run the queues sample
```

Please note the current build script is only tested on Ubuntu 16.04. Please update the script accordingly for other distributions.

Please note that starting from 2.10.0, Casablanca requires a minimum version of CMake v3.1, so the default CMake on Ubuntu 14.04 cannot support Casablanca build. User can upgrade CMake by themselves to build Casablanca. If default CMake (2.8) for Ubuntu 14.04 must be used, 5.0.1 with Casablanca version v2.9.1 is recommended.

### Getting Started on SLES12

*Please note the following build script is only tested on SLES12 SP3. The script may need to be updated accordingly for other distributions.*

Before building the Azure Storage Client Library on C++, some prerequisites need to be installed first:
- Install prerequisites:
```bash
sudo zypper install git gcc-c++ boost-devel cmake libopenssl-devel libxml2-devel libuuid-devel
```

The Azure Storage Client Library for C++ depends on Casablanca, following are instructions to build and install Casablanca:

- Clone the project using git:
```bash
git clone https://github.com/Microsoft/cpprestsdk.git
```

- Checkout the version on which Azure Storage Client Library for C++ depends:
```bash
git checkout tags/v2.10.10 -b v2.10.10
```

- Build the project in Release mode
```bash
cd cpprestsdk/Release
git submodule update --init
mkdir build.release
cd build.release
CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release -DWERROR=OFF -DBUILD_SAMPLES=OFF -DBUILD_TESTS=OFF
sudo make install
```

To build the Azure Storage Client Library for C++ project:

- Clone the project using git:
```bash
git clone https://github.com/Azure/azure-storage-cpp.git
```
The project is cloned to a folder called `azure-storage-cpp`. Always use the master branch, which contains the latest release.

- Build the SDK in Release mode:
```bash
cd azure-storage-cpp/Microsoft.WindowsAzure.Storage
mkdir build.release
cd build.release
CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

The library is generated under `azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.release/Binaries/`.

The Azure Storage Client Library for C++ project depends on Unitest++ for unit test:

To build and install Unitest++:
- Clone the project using git: 
```bash
git clone https://github.com/unittest-cpp/unittest-cpp.git
```

- Build and install the project:
```bash
cd unittest-cpp/builds/
CXX=g++-4.8 cmake ..
sudo make install
```

Build and run unit test against Azure Storage Client Library for C++:
- Build the test code:
```bash
CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
make
```
- Run unit tests
```bash
cd Binaries
vi test_configurations.json # modify test config file to include your storage account credentials
./azurestoragetest
```

To build sample code:
```bash
CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON
make
```
To run the samples:
```bash
cd Binaries
vi ../../samples/SamplesCommon/samples_common.h # modify connection string to include your storage account credentials
./samplesblobs            # run the blobs sample
./samplesjson             # run the tables sample with JSON payload
./samplestables           # run the tables sample
./samplesqueues           # run the queues sample
```

### Getting Started on RHEL 7

*Please note the following build script is only tested on RHEL 7.5. The script may need to be updated accordingly for other distributions.*

Before building the Azure Storage Client Library on C++, some prerequisites need to be installed first:
- Install prerequisites:
```bash
sudo yum install git gcc-c++ openssl-devel libxml2-devel libuuid-devel
```

- Download and install cmake3:
```bash
wget -O cmakeinstall.sh https://cmake.org/files/v3.12/cmake-3.12.3-Linux-x86_64.sh
chmod a+x cmakeinstall.sh
sudo ./cmakeinstall.sh --prefix=/usr
```

- Download and install boost
```bash
wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz
tar xvf boost_1_68_0.tar.gz
cd boost_1_68_0
./bootstrap.sh
sudo ./b2 install
```

The Azure Storage Client Library for C++ depends on Casablanca, following are instructions to build and install Casablanca:

- Clone the project using git:
```bash
git clone https://github.com/Microsoft/cpprestsdk.git
```

- Checkout the version on which Azure Storage Client Library for C++ depends:
```bash
git checkout tags/v2.10.10 -b v2.10.10
```

- Build the project in Release mode
```bash
cd cpprestsdk/Release
git submodule update --init
mkdir build.release
cd build.release
cmake .. -DCMAKE_BUILD_TYPE=Release -DWERROR=OFF -DBUILD_SAMPLES=OFF -DBUILD_TESTS=OFF
sudo make install
```

To build the Azure Storage Client Library for C++ project:

- Clone the project using git:
```bash
git clone https://github.com/Azure/azure-storage-cpp.git
```
The project is cloned to a folder called `azure-storage-cpp`. Always use the master branch, which contains the latest release.

- Build the SDK in Release mode:
```bash
cd azure-storage-cpp/Microsoft.WindowsAzure.Storage
mkdir build.release
cd build.release
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

The library is generated under `azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.release/Binaries/`.

The Azure Storage Client Library for C++ project depends on Unitest++ for unit test:

To build and install Unitest++:
- Clone the project using git: 
```bash
git clone https://github.com/unittest-cpp/unittest-cpp.git
```

- Build and install the project:
```bash
cd unittest-cpp/builds/
cmake ..
sudo make install
```

Build and run unit test against Azure Storage Client Library for C++:
- Build the test code:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
make
```
- Run unit tests
```bash
cd Binaries
vi test_configurations.json # modify test config file to include your storage account credentials
./azurestoragetest
```

To build sample code:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON
make
```
To run the samples:
```bash
cd Binaries
vi ../../samples/SamplesCommon/samples_common.h # modify connection string to include your storage account credentials
./samplesblobs            # run the blobs sample
./samplesjson             # run the tables sample with JSON payload
./samplestables           # run the tables sample
./samplesqueues           # run the queues sample
```

### Getting Started on RHEL 6

*Please note the following build script is only tested on RHEL 6.9. The script may need to be updated accordingly for other distributions.*

Before building the Azure Storage Client Library on C++, some prerequisites need to be installed first:
- Install prerequisites:
```bash
sudo yum install git openssl-devel libxml2-devel libuuid-devel
```

- Install and enable to use gcc-c++
```bash
sudo yum install devtoolset-3-gcc-c++.x86_64
scl enable devtoolset-3 bash
```

- Download and install cmake3:
```bash
wget -O cmakeinstall.sh https://cmake.org/files/v3.12/cmake-3.12.3-Linux-x86_64.sh
chmod a+x cmakeinstall.sh
sudo ./cmakeinstall.sh --prefix=/usr
```

- Download and install boost
```bash
wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz
tar xvf boost_1_68_0.tar.gz
cd boost_1_68_0
./bootstrap.sh
sudo ./b2 install
```

The Azure Storage Client Library for C++ depends on Casablanca, following are instructions to build and install Casablanca:

- Clone the project using git:
```bash
git clone https://github.com/Microsoft/cpprestsdk.git
```

- Checkout the version on which Azure Storage Client Library for C++ depends:
```bash
git checkout tags/v2.10.10 -b v2.10.10
```

- Build the project in Release mode
```bash
cd cpprestsdk/Release
git submodule update --init
mkdir build.release
cd build.release
cmake .. -DCMAKE_BUILD_TYPE=Release -DWERROR=OFF -DBUILD_SAMPLES=OFF -DBUILD_TESTS=OFF -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
sudo make install
```

To build the Azure Storage Client Library for C++ project:

- Clone the project using git:
```bash
git clone https://github.com/Azure/azure-storage-cpp.git
```
The project is cloned to a folder called `azure-storage-cpp`. Always use the master branch, which contains the latest release.

- Build the SDK in Release mode:
```bash
cd azure-storage-cpp/Microsoft.WindowsAzure.Storage
mkdir build.release
cd build.release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
make
```

The library is generated under `azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.release/Binaries/`.

The Azure Storage Client Library for C++ project depends on Unitest++ for unit test:

To build and install Unitest++:
- Clone the project using git: 
```bash
git clone https://github.com/unittest-cpp/unittest-cpp.git
```

- Build and install the project:
```bash
cd unittest-cpp/builds/
cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
sudo make install
```

Build and run unit test against Azure Storage Client Library for C++:
- Build the test code:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
make
```
- Run unit tests
```bash
cd Binaries
vi test_configurations.json # modify test config file to include your storage account credentials
./azurestoragetest
```

To build sample code:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
make
```
To run the samples:
```bash
cd Binaries
vi ../../samples/SamplesCommon/samples_common.h # modify connection string to include your storage account credentials
./samplesblobs            # run the blobs sample
./samplesjson             # run the tables sample with JSON payload
./samplestables           # run the tables sample
./samplesqueues           # run the queues sample
```

## Getting Started on OSX

*Note that OSX is not officially supported yet, but it has been seen to work, YMMV. This build has been tested to work when the dependencies are installed via homebrew, YMMV if using FINK or MacPorts*

Install dependecies with homebrew:

```
brew install libxml2 ossp-uuid openssl
```

As mentioned above, the Azure Storage Client Library for C++ depends on Casablanca.
If you are using homebrew you can install it from there:
```
brew install cpprestsdk
```

Otherwise, you may need to build it. Follow [these instructions](https://github.com/Microsoft/cpprestsdk/wiki/How-to-build-for-Mac-OS-X) to compile it.

Once this is complete, then:

- Clone the project using git:
```bash
git clone https://github.com/Azure/azure-storage-cpp.git
```
The project is cloned to a folder called `azure-storage-cpp`. Always use the master branch, which contains the latest release.

**Some notes about building**:
- If you're using homebrew, there seems to be an issue with the pkg-config files, which means that, by default, a -L flag to tell the linker where libintl lives is left out. We've accounted for this in our CMAKE file, by looking in the usual directory that homebrew puts those libs. If you are not using homebrew, you will get an error stating that you need to tell us where those libs live.
- Similarly, for openssl, you don't want to use the version that comes with OSX, it is old. We've accounted for this in the CMAKE script by setting the search paths to where homebrew puts openssl, so if you're not using homebrew you'll need to tell us where a more recent version of openssl lives.

- Build the SDK for Release if you are using hombrew:
```bash
cd azure-storage-cpp/Microsoft.WIndowsAzure.Storage
mkdir build.release
cd build.release
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

- OR, Build the SDK for Release if you are not using homebrew

```bash
cd azure-storage-cpp/Microsoft.WindowsAzure.Storage
mkdir build.release
cd build.release
CASABLANCA_DIR=<path to casablanca> cmake .. -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR=<path to openssl> -DGETTEXT_LIB_DIR=<path to gettext lib dir>
make
```

In the above command, replace:
- `<path to Casablanca>` to point to your local installation of Casablanca. For example, if the file `libcpprest.so` exists at location `~/Github/Casablanca/cpprestsdk/Release/build.release/Binaries/libcpprest.dylib`, then <path to casablanca> should be `~/Github/Casablanca/cpprestsdk`
- `<path to openssl>` to your local openssl, it is recommended not to use the version that comes with OSX, rather use one from Homebrew or the like. This should be the path that contains the `lib` and `include` directories
- `<path to gettext lib dir>` is the directory which contains `libintl.dylib`

For example you might use:
```bash
CASABLANCA_DIR=~/Github/Casablanca/cpprestsdk cmake .. -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DGETTEXT_LIB_DIR=/usr/local/opt/gettext/lib
```
The library is generated under `azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.release/Binaries/`.

To build and run unit tests:
- Install UnitTest++ library:
```bash
brew install unittest-cpp
```
- Build the test code:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
make
```
- Run unit tests
```bash
cd Binaries
vi test_configurations.json # modify test config file to include your storage account credentials
./azurestoragetest
```

# Learn More
- [Microsoft Azure Storage Client Library for C++ v2.0.0](documentation/Microsoft%20Azure%20Storage%20Client%20Library%20for%20C%2B%2B%202.0.0.md)
- [Microsoft Azure Storage Client Library for C++ v1.0.0 (General Availability)](http://blogs.msdn.com/b/windowsazurestorage/archive/2015/04/29/microsoft-azure-storage-client-library-for-c-v1-0-0-general-availability.aspx)
