# Unit Tests for Azure Storage Client Library for C++

## Prerequisites
Run following commands under root of this repository to get UnitTest++.
```bash
git submodule init
git submodule update
```

## Running the tests

The unit tests run against real Azure services. If you have an Azure Storage
account, change the values in `test_configurations.json` to point to it. *Do
not push these changes to GitHub, as doing so will expose your account key
to everyone.*

Alternatively, you can change the "target" in `test_configurations.json` to
"devstore". Then the tests will be run against the
[Azure Storage Emulator](https://azure.microsoft.com/en-us/documentation/articles/storage-use-emulator/)
(which can be installed as part of the
[Azure SDK](https://azure.microsoft.com/downloads/)). Some tests will fail
under this configuration, as the emulator doesn't support all of the
features the tests use.
