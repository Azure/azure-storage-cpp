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

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "was/blob.h"

#ifndef _WIN32

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#endif


class custom_test_reporter : public UnitTest::TestReporter
{
public:
    custom_test_reporter() : m_reporter(std::make_shared<UnitTest::TestReporterStdout>()) {}
    ~custom_test_reporter() override {}

    void ReportTestStart(const UnitTest::TestDetails& test) override
    {
        m_reporter->ReportTestStart(test);
    }

    void ReportFailure(const UnitTest::TestDetails& test, char const* failure) override
    {
        std::string suite_name = test.suiteName;
        std::string test_name = test.testName;
        std::string full_name = suite_name + ":" + test_name;
        if (m_failed_tests.empty() || m_failed_tests.back() != full_name)
        {
            m_failed_tests.emplace_back(full_name);
        }
        m_reporter->ReportFailure(test, failure);
    }

    void ReportTestFinish(const UnitTest::TestDetails& test, float secondsElapsed) override
    {
        m_reporter->ReportTestFinish(test, secondsElapsed);
    }

    void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) override
    {
        m_reporter->ReportSummary(totalTestCount, failedTestCount, failureCount, secondsElapsed);
    }

    const std::vector<std::string>& GetFailedTests() const
    {
        return m_failed_tests;
    }

private:
    // Since UnitTest::TestReporterStdout privatized all methods, we cannot directly inherit from it. Use this as a workaround.
    std::shared_ptr<UnitTest::TestReporter> m_reporter;
    std::vector<std::string> m_failed_tests;
};

struct retry_policy
{
    // If m out of n retries succeed, this test case is considered passed.
    int m{ 0 };
    int n{ 0 };
};

int run_tests(const std::unordered_set<std::string>& included_cases, const std::unordered_set<std::string>& excluded_cases, retry_policy retry_policy, const std::string& warning_message_prefix)
{
    std::unordered_map<std::string, int> failed_testcases;
    {
        custom_test_reporter reporter;
        UnitTest::TestRunner runner(reporter);

        auto match = [](const std::unordered_set<std::string>& all_cases, const std::string& suite_name, const std::string& test_name)
        {
            return all_cases.find(suite_name) != all_cases.end() || all_cases.find(suite_name + ":" + test_name) != all_cases.end();
        };

        runner.RunTestsIf(UnitTest::Test::GetTestList(), nullptr, [match, included_cases, excluded_cases](const UnitTest::Test* test) -> bool
        {
            std::string suite_name = test->m_details.suiteName;
            std::string test_name = test->m_details.testName;

            return !match(excluded_cases, suite_name, test_name) && (included_cases.empty() || match(included_cases, suite_name, test_name));
        }, 0);

        if (retry_policy.m > 0)
        {
            for (const auto& t : reporter.GetFailedTests())
            {
                failed_testcases.emplace(t, 0);
            }
        }
    }

    for (int i = 0; i < retry_policy.n && !failed_testcases.empty(); ++i)
    {
        custom_test_reporter reporter;
        UnitTest::TestRunner runner(reporter);

        runner.RunTestsIf(UnitTest::Test::GetTestList(), nullptr, [&failed_testcases, i, retry_policy](const UnitTest::Test* test) -> bool
        {
            std::string suite_name = test->m_details.suiteName;
            std::string test_name = test->m_details.testName;
            std::string full_name = suite_name + ":" + test_name;

            auto ite = failed_testcases.find(full_name);
            if (ite != failed_testcases.end())
            {
                int failed_count = ite->second;
                int successful_count = i - failed_count;
                return successful_count < retry_policy.m && failed_count < retry_policy.n - retry_policy.m + 1;
            }
            return false;
        }, 0);

        for (const auto& t : reporter.GetFailedTests())
        {
            ++failed_testcases[t];
        }
    }

    int num_failed = 0;
    for (const auto& p : failed_testcases)
    {
        fprintf(stderr, "%s%s failed %d time(s)\n", warning_message_prefix.data(), p.first.data(), p.second + 1);

        if (p.second > retry_policy.n - retry_policy.m)
        {
            ++num_failed;
        }
    }

    return num_failed;
}

int main(int argc, const char** argv)
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

    std::unordered_set<std::string> included_cases;
    std::unordered_set<std::string> excluded_cases;
    retry_policy retry_policy;
    std::string warning_message_prefix;

    auto starts_with = [](const std::string& str, const std::string& prefix)
    {
        size_t i = 0;
        while (i < str.length() && i < prefix.length() && prefix[i] == str[i]) ++i;
        return i == prefix.length();
    };

    auto add_to = [](std::unordered_set<std::string>& all_cases, const std::string& name)
    {
        auto colon_pos = name.find(":");
        std::string suite_name = name.substr(0, colon_pos);
        if (suite_name.empty())
        {
            throw std::invalid_argument("Invalid test case \"" + name + "\".");
        }
        all_cases.emplace(name);
    };

    for (int i = 1; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if (starts_with(arg, "--retry-policy="))
        {
            size_t pos;
            std::string policy_str = arg.substr(arg.find('=') + 1);
            retry_policy.m = std::stoi(policy_str, &pos);
            if (pos != policy_str.size())
            {
                retry_policy.n = std::stoi(policy_str.substr(pos + 1));
            }
            else
            {
                retry_policy.n = retry_policy.m;
            }
        }
        else if (starts_with(arg, "--warning-message="))
        {
            warning_message_prefix = arg.substr(arg.find('=') + 1);
        }
        else if (starts_with(arg, "--exclude="))
        {
            add_to(excluded_cases, arg.substr(arg.find('=') + 1));
        }
        else
        {
            add_to(included_cases, arg);
        }
    }

    return run_tests(included_cases, excluded_cases, retry_policy, warning_message_prefix);
}
