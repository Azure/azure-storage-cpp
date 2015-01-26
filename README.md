# Azure Storage Client Library for C++

The Azure Storage Client Library for C++ allows you to build applications against Microsoft Azure Storage. For an overview of Azure Storage, see [Introduction to Microsoft Azure Storage](http://azure.microsoft.com/en-us/documentation/articles/storage-introduction/).

Note that the Azure Storage Client Library for C++ library is a CTP (Community Technology Preview) release.

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

# Getting started

For the best development experience, we recommend that developers use the official Microsoft NuGet packages for libraries. NuGet packages are regularly updated with new functionality and hotfixes. 
Download the [NuGet Package](http://www.nuget.org/packages/wastorage).

## Requirements

To call Azure services, you must first have an Azure subscription. Sign up for a [free trial](/en-us/pricing/free-trial/) or use your [MSDN subscriber benefits](/en-us/pricing/member-offers/msdn-benefits-details/).

## Need Help?

Be sure to check out the [Azure Storage Forum](https://social.msdn.microsoft.com/Forums/azure/en-US/home?forum=windowsazuredata) on MSDN if you need help, or use [StackOverflow](http://stackoverflow.com/questions/tagged/azure).

## Collaborate & Contribute

We gladly accept community contributions.

- **Issues:** Report bugs on the [Issues page](https://github.com/Azure/azure-storage-cpp/issues) in GitHub.
- **Forums:** Communicate with the Azure Storage development team on the [Azure Storage Forum](https://social.msdn.microsoft.com/Forums/azure/en-US/home?forum=windowsazuredata) or [StackOverflow](http://stackoverflow.com/questions/tagged/azure).
- **Source Code Contributions:** Please follow the [contribution guidelines for Azure open source](http://azure.github.io/guidelines.html) for instructions about contributing to the source project. 

For general suggestions about Azure, use our [Azure feedback forum](http://feedback.azure.com/forums/34192--general-feedback).

## Download & Install

### Via Git

To create a local clone of the source for the Azure Storage Client Library for C++ via `git`, type:

```bash
git clone https://github.com/Azure/azure-storage-cpp.git
cd azure-storage-cpp
```

### Via NuGet

To install the binaries for the Azure Storage Client Library for C++, type the following into the [NuGet Package Manager console](http://docs.nuget.org/docs/start-here/using-the-package-manager-console):

`Install-Package wastorage -Pre`

## Dependencies

### C++ REST SDK

The Azure Storage Client Library for C++ depends on the C++ REST SDK (codename "Casablanca") 2.3.0. It can be installed through [NuGet](http://www.nuget.org/packages/cpprestsdk/2.3.0) or downloaded directly from [CodePlex](http://casablanca.codeplex.com/releases/view/129408).

## Code Samples

How-to topics focused around accomplishing specific tasks are available in the [samples folder](https://github.com/Azure/azure-storage-cpp/tree/master/Microsoft.WindowsAzure.Storage).

## Getting Started on Linux
As mentioned above, the Azure Storage Client Library for C++ depends on Casablanca. Follow [these instructions](https://casablanca.codeplex.com/wikipage?title=Setup%20and%20Build%20on%20Linux&referringTitle=Documentation) to compile it. Current version of the library depends on Casablanca version 2.3.0.

Once this is complete, then:

- Clone the project using git:
```bash
git clone https://github.com/Azure/azure-storage-cpp.git
```
The project is cloned to a folder called `azure-storage-cpp`. Always use the master branch, which contains the latest release.
- Install additional dependencies:
```bash
sudo apt-get install libxml++2.6-dev libxml++2.6-doc uuid-dev
```
- Build the SDK for Release:
```bash
cd azure-storage-cpp/Microsoft.WIndowsAzure.Storage
mkdir build.release
cd build.release
CASABLANCA_DIR=<path to Casablanca> CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
In the above command, replace `<path to Casablanca>` to point to your local installation of Casablanca. For example, if the file `libcpprest.so` exists at location `~/Github/Casablanca/casablanca/Release/build.release/Binaries/libcpprest.so`, then your `cmake` command should be:
```bash
CASABLANCA_DIR=~/Github/Casablanca/casablanca CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release
```
The library is generated under `azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.release/Binaries/`.

Once you have built the library, the samples should work equally well for Windows and Linux. You can build the samples as well:
```bash
cd ../samples
vi SamplesCommon/samples_common.h – edit this file to include your storage account name and key
mkdir build.release
cd build.release
CASABLANCA_DIR=<path to Casablanca> CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release
Make
```
To run the samples:
```bash
cd Binaries
cp ../../BlobsGettingStarted/DataFile.txt .            (this is required to run the blobs sample)
./samplesblobs            (run the blobs sample)
./samplestables           (run the tables sample)
./samplesqueues           (run the queues sample)
```
