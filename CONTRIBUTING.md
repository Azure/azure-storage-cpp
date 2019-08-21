If you intend to contribute to the project, please make sure you've followed
the instructions provided in the [Azure Projects Contribution Guidelines]
(http://azure.github.io/guidelines/).

## Project Setup on Windows
On Windows, the Azure Storage development team uses Visual Studio so
instructions will be tailored to that preference. However, any preferred IDE or
other toolset should be usable.

### Install
* Visual Studio 2015 or Visual Studio 2013 with C++ toolsets.
* Clone the source code from GitHub.

#### Open Solution
Open the project from Visual Studio using **File->Open->Project/Solution...**
and navigate to the `Microsoft.WindowsAzure.Storage.v140.sln` (for Visual
Studio 2015) or `Microsoft.WindowsAzure.Storage.v120.sln` (for Visual Studio
2013) solution file in the repo base folder. The dependent library Casablanca
will be installed by NuGet upon building.

### Tests

#### Add Unit Test Project
Use Visual Studio menu **File->Add->Existing Project...** and navigate to the
folder `Microsoft.WindowsAzure.Storage\tests`. Select
`Microsoft.WindowsAzure.Storage.UnitTests.v140.vcxproj` (for Visual Studio 2015)
or `Microsoft.WindowsAzure.Storage.UnitTests.v120.vcxproj` (for Visual Studio
2013).

#### Install UnitTest++
* Fetch source code of UnitTest++ from its [GitHub repo]
(https://github.com/unittest-cpp/unittest-cpp)
* Checkout version 1.4
```bash
git checkout v1.4
```
* Create a folder `UnitTest++` under `Microsoft.WindowsAzure.Storage\tests` and
copy all contents of `unittest-cpp` to it.
* Add another existing project to the Visual Studio via **File->Add->Existing
Project...**. Navigate to `Microsoft.WindowsAzure.Storage\tests\UnitTest++`, and
select `UnitTest++.vsnet2005.vcproj`.
* A "Review Project And Solution Changes" dialog popups. Choose **OK**, and a
new `UnitTest++.vsnet2005.vcxproj` is generated and added to the solution.

#### Configuration
The only step to configure testing is to change the `test_configuration.json`
file in `Microsoft.WindowsAzure.Storage\tests` folder. You should insert your
storage account information into the file. If you want to run the tests against
Azure Storage Emulator, change `target` to `devstore`. If you want to run the
tests against real Azure Storage, use real connection string in `production`.

#### Running
Set `Microsoft.WindowsAzure.Storage.UnitTests` as startup project. You can specify
a subset of tests to run on the command line before running the project. Go to the
project properties for the unit test project, select **Configuration Properties->
Debugging->Command Arguments**. Enter a space-separated list of `SUITE:TEST` and/or
`SUITE`. For example: `Queue:Queue_Messages`, `Core`, or `Table TableClient`.

### Debug
To use Fiddler, you need to set the system winhttp proxy. Open an administrator
command prompt. Run `netsh.exe`. Set the proxy by executing the command: `winhttp
set proxy localhost:8888`. Clear the proxy by executing the command: `winhttp reset
proxy`.

## Project Setup on Linux
The Azure Storage development team uses Ubuntu 14.04 LTS as the main development
platform, so instructions will be tailored to that. However, you can refer to other
platforms' documentation to install the dependent libraries and preferred IDE or
other toolset.

### Casablanca
Azure Storage Client Library for C++ depends on Casablanca. Follow [these
instructions](https://github.com/Microsoft/cpprestsdk/wiki/How-to-build-for-Linux)
to compile and install it.

### Additional Dependencies
```bash
sudo apt-get install libxml++2.6-dev libxml++2.6-doc uuid-dev
```

### Build
```bash
cd azure-storage-cpp/Microsoft.WIndowsAzure.Storage
mkdir build.release
cd build.release
CASABLANCA_DIR=<path to Casablanca> CXX=g++-5.1 cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
In the above command, replace `<path to Casablanca>` to point to your local
installation of Casablanca. For example, if the file `libcpprest.so` exists at
location `~/Github/Casablanca/cpprestsdk/Release/build.release/Binaries/libcpprest.so`,
then your `cmake` command should be:
```bash
CASABLANCA_DIR=~/Github/Casablanca/cpprestsdk CXX=g++-5.1 cmake .. -DCMAKE_BUILD_TYPE=Release
```
The library is generated under
`azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.release/Binaries/`.

### Tests

#### Install UnitTest++
```bash
sudo apt-get install libunittest++-dev
```

#### Build the Test Code
```bash
CASABLANCA_DIR=<path to Casablanca> CXX=g++-5.1 cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
make
```
The test binary `azurestoragetest` and `test_configuration.json` are generated under
the same directory as `libazurestorage.so`.

#### Configuration
The only step to configure testing is to change the `test_configuration.json`
file in `Microsoft.WindowsAzure.Storage/tests` folder. You should insert your
storage account information into the file. If you want to run the tests against
Azure Storage Emulator, change `target` to `devstore`. If you want to run the
tests against real Azure Storage, use real connection string in `production`.

#### Running
```bash
cd Binaries
./azurestoragetest [<SUITE>|<SUITE:TEST>]*
```

### Samples
```bash
CASABLANCA_DIR=<path to Casablanca> CXX=g++-5.1 cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON
make
```

```bash
cd Binaries
./samplesblobs            # run the blobs sample
./samplestables           # run the tables sample
./samplesjson             # run the tables sample with json_no_metadata to reduce payload size
./samplesqueues           # run the queues sample
```

## Pull Requests

### Guidelines
The following are the minimum requirements for any pull request that must be met
before contributions can be accepted.
* Make sure you've signed the CLA before you start working on any change.
* Discuss any proposed contribution with the team via a GitHub issue **before**
starting development.
* Code must be professional quality.
	* You should strive to mimic the style with which we have written the library.
	* Clean, well-commented, well-designed code.
	* Try to limit the number of commits for a feature to 1-2. If you end up having
	too many we may ask you to squash your changes into fewer commits.
* [Changelog.txt](Changelog.txt) needs to be updated describing the new change.
* [BreakingChanges.txt](BreakingChanges.txt) contains changes that break
backward-compatibility.
* Thoroughly test your feature.

### Testing Features
As you develop a feature, you'll need to write tests to ensure quality. You should
also run existing tests related to your change to address any unexpected breaks.

### Branching Policy
Changes should be based on the `dev` branch. We're following [semver](http://semver.org/).
We generally release any breaking changes in the next major version (e.g. 1.0, 2.0)
and non-breaking changes in the next minor or major version (e.g. 2.1, 2.2).

### Adding Features for All Platforms
We strive to release each new feature for each of our environments at the same time.
Therefore, we ask that all contributions be written for both Window and Linux. This
includes testing work for both platforms as well. Because most of our code is written using
standard C++11 and upon a cross-platform library Casablanca, we expect contributions are
also using standard language and cross-platform libraries, so that it won't cause much effort
for cross-platform support.

### Review Process
We expect all guidelines to be met before accepting a pull request. As such, we will
work with you to address issues we find by leaving comments in your code. Please
understand that it may take a few iterations before the code is accepted as we maintain
high standards on code quality. Once we feel comfortable with a contribution, we will
validate the change and accept the pull request.

# Thank you for any contributions!
Please let the team know if you have any questions or concerns about our contribution policy.
