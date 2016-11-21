// -----------------------------------------------------------------------------------------
// <copyright file="main.cpp" company="Microsoft">
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

#include "stdafx.h"

#include "was/blob.h"

#ifndef _WIN32

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#endif


int run_tests(const char* suite_name, const char* test_name)
{
    UnitTest::TestReporterStdout reporter;
    UnitTest::TestRunner runner(reporter);
    return runner.RunTestsIf(UnitTest::Test::GetTestList(), suite_name, [test_name] (UnitTest::Test* test) -> bool
    {
        return (test_name == NULL) || (!strcmp(test_name, test->m_details.testName));
    }, 0);
}

int main(int argc, const char* argv[])
{
    azure::storage::operation_context::set_default_log_level(azure::storage::client_log_level::log_level_verbose);

#ifndef _WIN32
    boost::log::add_common_attributes();
    boost::log::add_file_log
    (
        boost::log::keywords::file_name = "test_log.log",
        boost::log::keywords::format = 
        (
            boost::log::expressions::stream << "<Sev: " << boost::log::trivial::severity
            << "> " << boost::log::expressions::smessage
        )
    );

#endif

    int failure_count;
    if (argc == 1)
    {
        failure_count = run_tests(NULL, NULL);
    }
    else
    {
        failure_count = 0;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg(argv[i]);
            auto colon = arg.find(':');
            if (colon == std::string::npos)
            {
                failure_count += run_tests(argv[i], NULL);
            }
            else
            {
                auto suite_name = arg.substr(0, colon);
                auto test_name = arg.substr(colon + 1);
                failure_count += run_tests(suite_name.c_str(), test_name.c_str());
            }
        }
    }

    return failure_count;
}
