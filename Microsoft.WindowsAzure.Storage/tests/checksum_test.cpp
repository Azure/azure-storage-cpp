// -----------------------------------------------------------------------------------------
// <copyright file="unicode_test.cpp" company="Microsoft">
//    Copyright 2019 Microsoft Corporation
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
#include "check_macros.h"
#include "was/core.h"

SUITE(Core)
{
    TEST(checksum_class)
    {
        CHECK(azure::storage::checksum_type::none == azure::storage::checksum_none_t::value);
        CHECK(azure::storage::checksum_type::md5 == azure::storage::checksum_md5_t::value);
        CHECK(azure::storage::checksum_type::crc64 == azure::storage::checksum_crc64_t::value);
        CHECK(azure::storage::checksum_type::hmac_sha256 == azure::storage::checksum_hmac_sha256_t::value);
        CHECK(azure::storage::checksum_type::none == azure::storage::checksum_none.value);
        CHECK(azure::storage::checksum_type::md5 == azure::storage::checksum_md5.value);
        CHECK(azure::storage::checksum_type::crc64 == azure::storage::checksum_crc64.value);
        CHECK(azure::storage::checksum_type::hmac_sha256 == azure::storage::checksum_hmac_sha256.value);

        const utility::char_t* md5_cstr = _XPLATSTR("1B2M2Y8AsgTpgAmY7PhCfg==");
        const utility::string_t md5_str(md5_cstr);
        const uint64_t crc64_val = 0x0;
        const utility::string_t crc64_str(_XPLATSTR("AAAAAAAAAAA="));
        const utility::string_t hmac_sha256_str(_XPLATSTR("H3MaxXPHmTz2iCz6XIggaMFNXVI0gCqYsU/BChVkrHE="));

        {
            azure::storage::checksum cs;
            CHECK(!cs.is_md5());
            CHECK(!cs.is_crc64());
            CHECK(!cs.is_hmac_sha256());
            CHECK(cs.empty());
        }
        {
            azure::storage::checksum cs(azure::storage::checksum_none);
            CHECK(!cs.is_md5());
            CHECK(!cs.is_crc64());
            CHECK(!cs.is_hmac_sha256());
            CHECK(cs.empty());
        }
        {
            // For backward compatibility.
            utility::string_t empty_string;
            azure::storage::checksum cs(empty_string);
            CHECK(!cs.is_md5());
            CHECK(!cs.is_crc64());
            CHECK(!cs.is_hmac_sha256());
            CHECK(cs.empty());
        }
        {
            azure::storage::checksum cs(md5_str);
            CHECK(cs.is_md5());
            CHECK(!cs.is_crc64());
            CHECK(!cs.is_hmac_sha256());
            CHECK(!cs.empty());
            CHECK_UTF8_EQUAL(cs.md5(), md5_str);
        }
        {
            azure::storage::checksum cs(md5_cstr);
            CHECK(cs.is_md5());
            CHECK(!cs.is_crc64());
            CHECK(!cs.is_hmac_sha256());
            CHECK(!cs.empty());
            CHECK_UTF8_EQUAL(cs.md5(), md5_str);
        }
        {
            azure::storage::checksum cs(azure::storage::checksum_md5, md5_str);
            CHECK(cs.is_md5());
            CHECK(!cs.is_crc64());
            CHECK(!cs.is_hmac_sha256());
            CHECK(!cs.empty());
            CHECK_UTF8_EQUAL(cs.md5(), md5_str);
        }
        {
            azure::storage::checksum cs(crc64_val);
            CHECK(!cs.is_md5());
            CHECK(cs.is_crc64());
            CHECK(!cs.is_hmac_sha256());
            CHECK(!cs.empty());
            CHECK_UTF8_EQUAL(cs.crc64(), crc64_str);
        }
        {
            azure::storage::checksum cs(azure::storage::checksum_crc64, crc64_val);
            CHECK(!cs.is_md5());
            CHECK(cs.is_crc64());
            CHECK(!cs.is_hmac_sha256());
            CHECK(!cs.empty());
            CHECK_UTF8_EQUAL(cs.crc64(), crc64_str);
        }
        {
            azure::storage::checksum cs(azure::storage::checksum_hmac_sha256, hmac_sha256_str);
            CHECK(!cs.is_md5());
            CHECK(!cs.is_crc64());
            CHECK(cs.is_hmac_sha256());
            CHECK(!cs.empty());
            CHECK_UTF8_EQUAL(cs.hmac_sha256(), hmac_sha256_str);
        }
    }
}