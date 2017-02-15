// -----------------------------------------------------------------------------------------
// <copyright file="check_macros.h" company="Microsoft">
//    Copyright 2013 Microsoft Corporation
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//      http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
// </copyright>
// -----------------------------------------------------------------------------------------

#pragma once

#define CHECK_UTF8_EQUAL(expected, actual) CHECK_EQUAL(utility::conversions::to_utf8string(expected), utility::conversions::to_utf8string(actual))

#define CHECK_STORAGE_EXCEPTION(expression, expected_message) \
    do \
        { \
        bool caught_ = false; \
        std::string returned_message; \
        try { expression; } \
        catch (const azure::storage::storage_exception& ex) { \
            returned_message = ex.what(); \
            caught_ = true; \
        } \
        catch (...) {} \
        if (!caught_) \
            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__), "storage_exception not thrown"); \
        else { \
            try { \
                UnitTest::CheckEqual(*UnitTest::CurrentTest::Results(), expected_message, returned_message, UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__)); \
            } \
            catch (...) { \
                UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__), \
                    "Unhandled exception in CHECK_STORAGE_EXCEPTION"); \
            } \
        } \
   } while(0)

#define CHECK_NOTHROW(expression) \
    do \
    { \
        bool exception_thrown = false; \
        try { expression; } \
        catch (...) { exception_thrown = true; } \
        if (exception_thrown) \
            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__), "Expected no exception thrown: \"" #expression "\"."); \
    } while(0)
