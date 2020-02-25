// -----------------------------------------------------------------------------------------
// <copyright file="cloud_file_test.cpp" company="Microsoft">
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
#include "file_test_base.h"
#include "check_macros.h"
#include "blob_test_base.h"

#include "wascore/util.h"

#pragma region Fixture

bool file_test_base::wait_for_copy(azure::storage::cloud_file& file)
{
    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
    } while (file.copy_state().status() == azure::storage::copy_status::pending);

    return file.copy_state().status() == azure::storage::copy_status::success;
}

#pragma endregion

SUITE(File)
{
    TEST_FIXTURE(file_test_base, file_create_delete)
    {
        CHECK(!m_file.exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        CHECK(!m_file.delete_file_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));

        CHECK(m_file.create_if_not_exists(1024U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        CHECK(!m_file.create_if_not_exists(1024U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        CHECK_EQUAL(m_file.properties().length(), 1024U);
        m_file.download_attributes();

        CHECK_EQUAL(m_file.properties().server_encrypted(), true);

        CHECK(m_file.exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));

        CHECK(m_file.delete_file_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        CHECK(!m_file.delete_file_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));

        CHECK(!m_file.exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
    }

    TEST_FIXTURE(file_test_base, file_metadata)
    {
        auto value1 = this->get_random_string();
        auto value2 = this->get_random_string();
        auto value3 = this->get_random_string();
        auto value4 = this->get_random_string();

        // create 2 pairs
        m_file.metadata()[_XPLATSTR("key1")] = value1;
        m_file.metadata()[_XPLATSTR("key2")] = value2;
        m_file.create_if_not_exists(1024U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_UTF8_EQUAL(m_file.metadata()[_XPLATSTR("key1")], value1);
        CHECK_UTF8_EQUAL(m_file.metadata()[_XPLATSTR("key2")], value2);

        auto same_file = m_file.get_parent_share_reference().get_directory_reference(m_directory.name()).get_file_reference(m_file.name());
        CHECK(same_file.metadata().empty());
        same_file.download_attributes();
        CHECK_EQUAL(2U, same_file.metadata().size());
        CHECK_UTF8_EQUAL(value1, same_file.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(value2, same_file.metadata()[_XPLATSTR("key2")]);

        // add 1 pair
        m_file.metadata()[_XPLATSTR("key3")] = value3;
        m_file.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        same_file.metadata().clear();
        CHECK(same_file.metadata().empty());
        same_file.download_attributes();
        CHECK_EQUAL(3U, same_file.metadata().size());
        CHECK_UTF8_EQUAL(value1, same_file.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(value2, same_file.metadata()[_XPLATSTR("key2")]);
        CHECK_UTF8_EQUAL(value3, same_file.metadata()[_XPLATSTR("key3")]);

        // overwrite with 1 pair
        m_file.metadata().clear();
        m_file.metadata()[_XPLATSTR("key4")] = value4;
        m_file.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        same_file.metadata().clear();
        CHECK(same_file.metadata().empty());
        same_file.download_attributes();
        CHECK_EQUAL(1U, same_file.metadata().size());
        CHECK_UTF8_EQUAL(value4, same_file.metadata()[_XPLATSTR("key4")]);

        // clear metadata
        m_file.metadata().clear();
        m_file.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        same_file.metadata().clear();
        CHECK(same_file.metadata().empty());
        same_file.download_attributes();
        CHECK_EQUAL(0U, same_file.metadata().size());
    }

    TEST_FIXTURE(file_test_base, file_properties)
    {
        m_file.create_if_not_exists(1024U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(1024U, m_file.properties().length());

        {
            m_file.properties().set_content_md5(_XPLATSTR(""));
            m_file.upload_properties();
            CHECK(_XPLATSTR("") == m_file.properties().content_md5());

            auto same_file = m_file.get_parent_share_reference().get_directory_reference(m_directory.name()).get_file_reference(m_file.name());
            auto stream = concurrency::streams::container_stream<std::vector<uint8_t>>::open_ostream();
            azure::storage::file_request_options options;
            options.set_use_transactional_md5(true);
            same_file.download_range_to_stream(stream, 0, 128, azure::storage::file_access_condition(), options, m_context);
            CHECK(_XPLATSTR("") == same_file.properties().content_md5());
        }
    }

    TEST_FIXTURE(file_test_base, file_smb_properties)
    {
        m_file.properties().set_permission(azure::storage::cloud_file_properties::inherit);
        m_file.properties().set_attributes(azure::storage::cloud_file_attributes::none);
        m_file.properties().set_creation_time(azure::storage::cloud_file_properties::now);
        m_file.properties().set_last_write_time(azure::storage::cloud_file_properties::now);

        m_file.create(1024);
        auto properties = m_file.properties();
        CHECK(properties.permission().empty());
        CHECK(!properties.permission_key().empty());
        CHECK(properties.attributes() != azure::storage::cloud_file_attributes::none);
        CHECK(properties.creation_time().is_initialized());
        CHECK(properties.last_write_time().is_initialized());
        CHECK(properties.change_time().is_initialized());
        CHECK(!properties.file_id().empty());
        CHECK(!properties.file_parent_id().empty());

        m_file.properties().set_permission(azure::storage::cloud_file_properties::preserve);
        m_file.properties().set_attributes(azure::storage::cloud_file_attributes::preserve);
        m_file.properties().set_creation_time(azure::storage::cloud_file_properties::preserve);
        m_file.properties().set_last_write_time(azure::storage::cloud_file_properties::preserve);

        m_file.upload_properties();
        CHECK(m_file.properties().permission().empty());
        CHECK(m_file.properties().permission_key() == properties.permission_key());
        CHECK(m_file.properties().attributes() == properties.attributes());
        CHECK(m_file.properties().creation_time() == properties.creation_time());
        CHECK(m_file.properties().last_write_time() == properties.last_write_time());
        CHECK(m_file.properties().file_id() == properties.file_id());
        CHECK(m_file.properties().file_parent_id() == properties.file_parent_id());

        utility::string_t permission = m_share.download_file_permission(m_file.properties().permission_key());
        m_file.properties().set_permission(permission);
        m_file.properties().set_attributes(
            azure::storage::cloud_file_attributes::readonly | azure::storage::cloud_file_attributes::hidden | azure::storage::cloud_file_attributes::system |
            azure::storage::cloud_file_attributes::archive | azure::storage::cloud_file_attributes::temporary | azure::storage::cloud_file_attributes::offline |
            azure::storage::cloud_file_attributes::not_content_indexed | azure::storage::cloud_file_attributes::no_scrub_data);
        auto new_attributes = m_file.properties().attributes();
        auto current_time = utility::datetime::utc_now();
        m_file.properties().set_creation_time(current_time);
        m_file.properties().set_last_write_time(current_time);

        m_file.upload_properties();
        CHECK(m_file.properties().permission().empty());
        CHECK(!m_file.properties().permission_key().empty());
        CHECK(m_file.properties().attributes() == new_attributes);
        CHECK(m_file.properties().creation_time() == current_time);
        CHECK(m_file.properties().last_write_time() == current_time);

        m_file.upload_properties();
    }

    TEST_FIXTURE(file_test_base, directory_smb_properties)
    {
        azure::storage::cloud_file_directory directory = m_share.get_directory_reference(get_random_string());
        directory.properties().set_permission(azure::storage::cloud_file_directory_properties::inherit);
        directory.properties().set_attributes(azure::storage::cloud_file_attributes::none);
        directory.properties().set_creation_time(azure::storage::cloud_file_directory_properties::now);
        directory.properties().set_last_write_time(azure::storage::cloud_file_directory_properties::now);
        directory.create();

        auto properties = directory.properties();
        CHECK(properties.permission().empty());
        CHECK(!properties.permission_key().empty());
        CHECK(properties.attributes() != azure::storage::cloud_file_attributes::none);
        CHECK(properties.creation_time().is_initialized());
        CHECK(properties.last_write_time().is_initialized());
        CHECK(!properties.file_id().empty());

        directory.properties().set_permission(azure::storage::cloud_file_directory_properties::preserve);
        directory.properties().set_attributes(azure::storage::cloud_file_attributes::preserve);
        directory.properties().set_creation_time(azure::storage::cloud_file_directory_properties::preserve);
        directory.properties().set_last_write_time(azure::storage::cloud_file_directory_properties::preserve);

        directory.upload_properties();
        CHECK(directory.properties().permission().empty());
        CHECK(directory.properties().permission_key() == properties.permission_key());
        CHECK(directory.properties().attributes() == properties.attributes());
        CHECK(directory.properties().creation_time() == properties.creation_time());
        CHECK(directory.properties().last_write_time() == properties.last_write_time());
        CHECK(directory.properties().file_id() == properties.file_id());
        CHECK(directory.properties().file_parent_id() == properties.file_parent_id());

        utility::string_t permission = m_share.download_file_permission(directory.properties().permission_key());
        directory.properties().set_permission(permission);
        directory.properties().set_attributes(
            azure::storage::cloud_file_attributes::readonly | azure::storage::cloud_file_attributes::hidden | azure::storage::cloud_file_attributes::system |
            azure::storage::cloud_file_attributes::directory | azure::storage::cloud_file_attributes::offline | azure::storage::cloud_file_attributes::not_content_indexed |
            azure::storage::cloud_file_attributes::no_scrub_data);
        auto new_attributes = directory.properties().attributes();
        auto current_time = utility::datetime::utc_now();
        directory.properties().set_creation_time(current_time);
        directory.properties().set_last_write_time(current_time);

        directory.upload_properties();
        CHECK(directory.properties().permission().empty());
        CHECK(!directory.properties().permission_key().empty());
        CHECK(directory.properties().attributes() == new_attributes);
        CHECK(directory.properties().creation_time() == current_time);
        CHECK(directory.properties().last_write_time() == current_time);

        directory.upload_properties();
    }

    TEST_FIXTURE(file_test_base, file_properties_resize_wont_work)
    {
        m_file.create_if_not_exists(1024U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(1024U, m_file.properties().length());

        //Using newly created properties does not set the size of file back to zero.
        m_file.properties() = azure::storage::cloud_file_properties();
        m_file.upload_properties();
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(1024U, m_file.properties().length());

        //Using the properties that explicitly equals to zero to upload properties will not set the size of the file back to zero.
        m_file.resize(0U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(0U, m_file.properties().length());
        auto zero_properties = m_file.properties();
        m_file.resize(1024U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(1024U, m_file.properties().length());
        m_file.properties() = zero_properties;
        m_file.upload_properties();
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(1024U, m_file.properties().length());

        //Using the properties that explicitly equals to non-zero will not set the size of the file to this non-zero value.
        auto non_zero_properties = m_file.properties();
        m_file.resize(0U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(0U, m_file.properties().length());
        m_file.properties() = non_zero_properties;
        m_file.upload_properties();
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(0U, m_file.properties().length());
    }

    TEST_FIXTURE(file_test_base, file_resize)
    {
        m_file.create_if_not_exists(1024U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(1024U, m_file.properties().length());

        auto length = get_random_int32() % 5120 + 1;
        if (length < 0)
            length = -length;
        length = length % 5120 + 1;

        m_file.resize(length, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(length, m_file.properties().length());
        //Setting the length back to zero and see that it works.
        m_file.resize(0U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(0U, m_file.properties().length());
    }

    TEST_FIXTURE(file_test_base, file_get_parent_directory_ref)
    {
        m_file.create_if_not_exists(1024U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto directory = m_file.get_parent_directory_reference();

        check_equal(m_directory, directory);

        CHECK(!directory.uri().primary_uri().is_empty());
        CHECK(directory.metadata().empty());
        CHECK(!directory.properties().etag().empty());

        CHECK(directory.properties().last_modified().is_initialized());
    }

    TEST_FIXTURE(file_test_base, file_get_parent_share_ref)
    {
        m_file.create_if_not_exists(1024U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto share = m_file.get_parent_share_reference();

        check_equal(m_share, share);

        CHECK(!share.uri().primary_uri().is_empty());
        CHECK(share.metadata().empty());
        CHECK(!share.properties().etag().empty());

        CHECK(share.properties().last_modified().is_initialized());
    }

    TEST_FIXTURE(file_test_base, file_copy)
    {
        auto file = m_directory.get_file_reference(_XPLATSTR("file"));
        file.create(1);
        file.upload_text(_XPLATSTR("1"), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        auto copy = m_directory.get_file_reference(_XPLATSTR("copy"));
        auto copy_id = copy.start_copy((file.uri().primary_uri()), azure::storage::file_access_condition(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(wait_for_copy(copy));
        CHECK_THROW(copy.abort_copy(copy_id, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context), azure::storage::storage_exception);
        CHECK_EQUAL(web::http::status_codes::Conflict, m_context.request_results().back().http_status_code());

        // copy from cloud_file object within same account using shared key.
        auto copy2 = m_directory.get_file_reference(_XPLATSTR("copy2"));
        copy2.start_copy(file, azure::storage::file_access_condition(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(wait_for_copy(copy2));
    }

    /// <summary>
    /// Test file copy from a cloud_file object using sas token.
    /// </summary>
    TEST_FIXTURE(file_test_base, file_copy_with_sas_token)
    {
        azure::storage::file_shared_access_policy read_policy(utility::datetime::utc_now() + utility::datetime::from_minutes(10), azure::storage::file_shared_access_policy::permissions::read);
        azure::storage::file_shared_access_policy write_policy(utility::datetime::utc_now() + utility::datetime::from_minutes(10), azure::storage::file_shared_access_policy::permissions::write);

        for (size_t i = 0; i < 2; ++i)
        {
            auto source_file_name = this->get_random_string();
            auto source = m_directory.get_file_reference(source_file_name);
            source.create(10);
            source.upload_text(_XPLATSTR("1"), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

            /// create source file with specified sas credentials, only read access to source file.
            auto source_sas = source.get_shared_access_signature(read_policy);
            azure::storage::cloud_file source_file = azure::storage::cloud_file(source.uri(), azure::storage::storage_credentials(source_sas));

            source_file.download_text(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

            /// create dest files with specified sas credentials, only read access to dest read file and only write access to dest write file.
            auto dest_file_name = this->get_random_string();
            auto dest = m_directory.get_file_reference(dest_file_name);

            auto dest_read_sas = dest.get_shared_access_signature(read_policy);
            azure::storage::cloud_file dest_read_file = azure::storage::cloud_file(dest.uri(), azure::storage::storage_credentials(dest_read_sas));

            auto dest_write_sas = dest.get_shared_access_signature(write_policy);
            azure::storage::cloud_file dest_write_file = azure::storage::cloud_file(dest.uri(), azure::storage::storage_credentials(dest_write_sas));

            /// try to copy from source file to dest file, use dest_read_file to check copy stats.
            auto copy_id = dest_write_file.start_copy(source_file, azure::storage::file_access_condition(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
            CHECK(wait_for_copy(dest_read_file));
            CHECK_THROW(dest_write_file.abort_copy(copy_id, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context), azure::storage::storage_exception);
            CHECK_EQUAL(web::http::status_codes::Conflict, m_context.request_results().back().http_status_code());
        }
    }

    /// <summary>
    /// Test file copy from a cloud_blob object.
    /// </summary>
    TEST_FIXTURE(file_test_base, file_copy_from_blob)
    {
        azure::storage::blob_shared_access_policy read_policy(utility::datetime::utc_now() + utility::datetime::from_minutes(10), azure::storage::blob_shared_access_policy::permissions::read);
        azure::storage::file_shared_access_policy write_policy(utility::datetime::utc_now() + utility::datetime::from_minutes(10), azure::storage::file_shared_access_policy::permissions::write);

        for (size_t i = 0; i < 2; ++i)
        {
            auto blob_name = this->get_random_string();
            auto container = test_config::instance().account().create_cloud_blob_client().get_container_reference(_XPLATSTR("container") + get_random_string());
            container.create_if_not_exists();

            auto source = container.get_block_blob_reference(blob_name);
            source.upload_text(_XPLATSTR("1"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

            /// create source blob with specified sas credentials, only read access to source file.
            auto source_sas = source.get_shared_access_signature(read_policy);
            azure::storage::cloud_block_blob source_blob = azure::storage::cloud_blob(source.uri(), azure::storage::storage_credentials(source_sas));

            source_blob.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

            /// create dest files with specified sas credentials, only read access to dest read file and only write access to dest write file.
            auto dest_file_name = this->get_random_string();
            auto dest = m_directory.get_file_reference(dest_file_name);

            /// try to copy from source blob to dest file, use dest_read_file to check copy stats.
            auto copy_id = dest.start_copy(source_blob, azure::storage::access_condition(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
            CHECK(wait_for_copy(dest));
            CHECK_THROW(dest.abort_copy(copy_id, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context), azure::storage::storage_exception);
            CHECK_EQUAL(web::http::status_codes::Conflict, m_context.request_results().back().http_status_code());

            container.delete_container();
        }
    }

    // file level sas test
    TEST_FIXTURE(file_test_base, file_sas_token)
    {
        utility::size64_t length = 512;
        utility::string_t content = _XPLATSTR("testtargetfile");
        auto content_length = content.length();

        azure::storage::file_shared_access_policy create_policy;
        create_policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
        create_policy.set_permissions(azure::storage::file_shared_access_policy::permissions::create);
        auto create_sas = m_file.get_shared_access_signature(create_policy);
        auto create_file = azure::storage::cloud_file(m_file.uri(), azure::storage::storage_credentials(create_sas));
        create_file.create(length, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        azure::storage::file_shared_access_policy write_policy;
        write_policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
        write_policy.set_permissions(azure::storage::file_shared_access_policy::permissions::write);
        auto write_sas = m_file.get_shared_access_signature(write_policy);
        auto write_file = azure::storage::cloud_file(m_file.uri(), azure::storage::storage_credentials(write_sas));
        write_file.upload_text(content, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        azure::storage::file_shared_access_policy read_policy;
        read_policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
        read_policy.set_permissions(azure::storage::file_shared_access_policy::permissions::read);
        auto read_sas = m_file.get_shared_access_signature(read_policy);
        auto read_file = azure::storage::cloud_file(m_file.uri(), azure::storage::storage_credentials(read_sas));
        read_file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(read_file.properties().length(), content_length);
        auto download_content = read_file.download_text(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(download_content == content);

        azure::storage::file_shared_access_policy delete_policy;
        delete_policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
        delete_policy.set_permissions(azure::storage::file_shared_access_policy::permissions::del);
        auto delete_sas = m_file.get_shared_access_signature(delete_policy);
        auto delete_file = azure::storage::cloud_file(m_file.uri(), azure::storage::storage_credentials(delete_sas));
        delete_file.delete_file(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
    }

    TEST_FIXTURE(file_test_base, file_text_upload_download)
    {
        utility::string_t content = _XPLATSTR("content");
        m_file.create(content.length(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(!m_file.properties().server_encrypted());
        m_file.upload_text(content, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto download_content = m_file.download_text(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(content == download_content);
        CHECK(m_file.properties().server_encrypted());
    }

    TEST_FIXTURE(file_test_base, file_upload_download_from_file)
    {
        temp_file file(1000);
        m_file.upload_from_file(file.path(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        temp_file file2(0);
        m_file.download_to_file(file2.path(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        concurrency::streams::container_buffer<std::vector<uint8_t>> original_file_buffer;
        auto original_file = concurrency::streams::file_stream<uint8_t>::open_istream(file.path()).get();
        original_file.read_to_end(original_file_buffer).wait();
        original_file.close().wait();

        concurrency::streams::container_buffer<std::vector<uint8_t>> downloaded_file_buffer;
        auto downloaded_file = concurrency::streams::file_stream<uint8_t>::open_istream(file2.path()).get();
        downloaded_file.read_to_end(downloaded_file_buffer).wait();
        downloaded_file.close().wait();

        CHECK_EQUAL(original_file_buffer.collection().size(), downloaded_file_buffer.collection().size());
        CHECK_ARRAY_EQUAL(original_file_buffer.collection(), downloaded_file_buffer.collection(), (int)downloaded_file_buffer.collection().size());
    }

    TEST_FIXTURE(file_test_base, file_range)
    {
        m_file.create_if_not_exists(2048, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto ranges0 = m_file.list_ranges(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges0.size() == 0);

        utility::string_t content1 = _XPLATSTR("content1");
        m_file.write_range(concurrency::streams::bytestream::open_istream(std::move(utility::conversions::to_utf8string(content1))), 0, utility::string_t(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto ranges1 = m_file.list_ranges(0, 2048, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges1.size() == 1);
        if (ranges1.size() == 1)
        {
            auto range = ranges1.at(0);
            CHECK(range.start_offset() == 0);
            CHECK(range.end_offset() == 511);
        }

        utility::string_t content2 = _XPLATSTR("content2");
        m_file.write_range(concurrency::streams::bytestream::open_istream(std::move(utility::conversions::to_utf8string(content2))), 1024, utility::string_t(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto ranges2 = m_file.list_ranges(0, 2048, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges2.size() == 2);
        if (ranges1.size() == 2)
        {
            auto range0 = ranges1.at(0);
            CHECK(range0.start_offset() == 0);
            CHECK(range0.end_offset() == 511);

            auto range1 = ranges1.at(1);
            CHECK(range1.start_offset() == 1024);
            CHECK(range1.end_offset() == 1535);
        }
    }

    TEST_FIXTURE(file_test_base, file_write_range)
    {
        // check range with certain length
        utility::string_t content = _XPLATSTR("content");

        m_file.create_if_not_exists(content.length(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto ranges0 = m_file.list_ranges(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges0.size() == 0);

        m_file.write_range(concurrency::streams::bytestream::open_istream(std::move(utility::conversions::to_utf8string(content))), 0, utility::string_t(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto ranges1 = m_file.list_ranges(0, 2048, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges1.size() == 1);
        if (ranges1.size() == 1)
        {
            auto range = ranges1.at(0);
            CHECK(range.start_offset() == 0);
            CHECK(size_t(range.end_offset() - range.start_offset() + 1) == content.length());
        }
        m_file.clear_range(0, content.length());
        auto ranges_clear = m_file.list_ranges(0, 2048, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        // for ranges not aligned to 512 bytes, only write the content to 0, but not release the range.
        CHECK(ranges_clear.size() == 1);
        if (ranges1.size() == 1)
        {
            auto range = ranges1.at(0);
            CHECK(range.start_offset() == 0);
            CHECK(size_t(range.end_offset() - range.start_offset() + 1) == content.length());
        }

        // verify write range with total length larger than the content.
        m_file.create(1024, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        ranges0 = m_file.list_ranges(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges0.size() == 0);

        m_file.write_range(concurrency::streams::bytestream::open_istream(std::move(utility::conversions::to_utf8string(content))), 0, utility::string_t(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        ranges1 = m_file.list_ranges(0, 2048, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges1.size() == 1);
        if (ranges1.size() == 1)
        {
            auto range = ranges1.at(0);
            CHECK(range.start_offset() == 0);
            CHECK(range.end_offset() == 511);
        }
        m_file.clear_range(0, 1024);
        ranges_clear = m_file.list_ranges(0, 2048, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges_clear.size() == 0);

        // verify write range with start start_offset not zero.
        m_file.create(1024, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        ranges0 = m_file.list_ranges(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges0.size() == 0);

        m_file.write_range(concurrency::streams::bytestream::open_istream(std::move(utility::conversions::to_utf8string(content))), 512, utility::string_t(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        ranges1 = m_file.list_ranges(0, 2048, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges1.size() == 1);
        if (ranges1.size() == 1)
        {
            auto range = ranges1.at(0);
            CHECK(range.start_offset() == 512);
            CHECK(range.end_offset() == 1023);
        }
        m_file.clear_range(0, 1024);
        ranges_clear = m_file.list_ranges(0, 2048, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges_clear.size() == 0);
    }

    TEST_FIXTURE(file_test_base, file_write_range_with_md5)
    {
        utility::string_t content = _XPLATSTR("content");

        m_file.create_if_not_exists(content.length(), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto ranges0 = m_file.list_ranges(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(ranges0.size() == 0);

        CHECK_THROW(m_file.write_range(concurrency::streams::bytestream::open_istream(std::move(utility::conversions::to_utf8string(content))), 0, _XPLATSTR("md5"), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context), azure::storage::storage_exception);
    }

    TEST_FIXTURE(file_test_base, file_sas_combinations)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            auto permissions = i;

            azure::storage::file_shared_access_policy policy;
            policy.set_permissions(permissions);
            policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
            policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));

            auto file = m_share.get_root_directory_reference().get_file_reference(_XPLATSTR("file") + azure::storage::core::convert_to_string((int)i));
            file.create_if_not_exists(512U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
            file.properties().set_cache_control(_XPLATSTR("no-transform"));
            file.properties().set_content_disposition(_XPLATSTR("attachment"));
            file.properties().set_content_encoding(_XPLATSTR("gzip"));
            file.properties().set_content_language(_XPLATSTR("tr,en"));
            file.properties().set_content_type(_XPLATSTR("text/html"));
            file.upload_text(_XPLATSTR("test"), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

            auto sas_token = file.get_shared_access_signature(policy);
            check_access(sas_token, permissions, azure::storage::cloud_file_shared_access_headers(), file);
        }
    }

    TEST_FIXTURE(file_test_base, file_md5)
    {
        utility::string_t content[5], filename[5];
        for (size_t i = 0; i < 5; ++i)
        {
            filename[i] = get_random_string(10);
            content[i] = get_random_string(1024);
        }

        auto options = azure::storage::file_request_options();
        options.set_disable_content_md5_validation(false);
        options.set_use_transactional_md5(true);
        options.set_parallelism_factor(10);
        options.set_store_file_content_md5(true);

        for (size_t i = 0; i < 5; ++i)
        {
            auto file = m_directory.get_file_reference(filename[i]);
            file.upload_text(content[i], azure::storage::file_access_condition(), options, m_context);
            auto download_content = file.download_text(azure::storage::file_access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(content[i], download_content);
            file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
            CHECK(!file.properties().content_md5().empty());

            auto same_file = m_directory.get_file_reference(filename[i]);
            concurrency::streams::container_buffer<std::vector<uint8_t>> buff;
            same_file.download_range_to_stream(buff.create_ostream(), 0, content[i].length() / 2);
            std::vector<uint8_t>& data = buff.collection();
            std::string download_partial_content(data.begin(), data.end());
            CHECK_UTF8_EQUAL(content[i].substr(0, content[i].length() / 2), download_partial_content);
            CHECK_UTF8_EQUAL(file.properties().content_md5(), same_file.properties().content_md5());
        }
    }

    /// <summary>
    /// Test parallel download
    /// </summary>
    TEST_FIXTURE(file_test_base, parallel_download)
    {
        // download file smaller than 32MB.
        {
            auto file_name = get_random_string(20);
            auto file = m_share.get_root_directory_reference().get_file_reference(file_name);
            size_t target_length = 31 * 1024 * 1024;
            azure::storage::file_request_options option;
            option.set_parallelism_factor(2);
            option.set_use_transactional_md5(false);
            std::vector<uint8_t> data;
            data.resize(target_length);
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            file.upload_from_stream(upload_buffer.create_istream(), azure::storage::file_access_condition(), option, m_context);

            // download target file in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            file.download_to_stream(download_buffer.create_ostream(), azure::storage::file_access_condition(), option, context);

            check_parallelism(context, 1);
            CHECK(file.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == target_length);
            CHECK(std::equal(data.begin(), data.end(), download_buffer.collection().begin()));
        }

        // file with size larger than 32MB.
        {
            auto file_name = get_random_string(20);
            auto file = m_share.get_root_directory_reference().get_file_reference(file_name);
            size_t target_length = 100 * 1024 * 1024;
            azure::storage::file_request_options option;
            option.set_parallelism_factor(2);
            option.set_use_transactional_md5(false);
            std::vector<uint8_t> data;
            data.resize(target_length);
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            file.upload_from_stream(upload_buffer.create_istream(), azure::storage::file_access_condition(), option, m_context);

            // download target file in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            file.download_to_stream(download_buffer.create_ostream(), azure::storage::file_access_condition(), option, context);

            check_parallelism(context, 2);
            CHECK(file.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == target_length);
            CHECK(std::equal(data.begin(), data.end(), download_buffer.collection().begin()));
        }
    }

    /// <summary>
    /// Test parallel download wit offset
    /// </summary>
    TEST_FIXTURE(file_test_base, parallel_download_with_offset)
    {
        // file with size larger than 32MB.
        // With offset not zero.
        {
            auto file_name = get_random_string(20);
            auto file = m_directory.get_file_reference(file_name);
            size_t target_length = 100 * 1024 * 1024;
            azure::storage::file_request_options option;
            option.set_parallelism_factor(2);
            std::vector<uint8_t> data;
            data.resize(target_length);
            fill_buffer(data);
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            file.upload_from_stream(upload_buffer.create_istream(), azure::storage::file_access_condition(), option, m_context);

            // download target file in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;

            utility::size64_t actual_offset = get_random_int32() % 255 + 1;
            utility::size64_t actual_length = target_length - actual_offset;
            file.download_range_to_stream(download_buffer.create_ostream(), actual_offset, actual_length, azure::storage::file_access_condition(), option, context);

            check_parallelism(context, 2);
            CHECK(file.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == actual_length);
            CHECK(std::equal(data.begin() + actual_offset, data.end(), download_buffer.collection().begin()));
        }

        // file with size larger than 32MB.
        // With offset not zero, length = max.
        {
            auto file_name = get_random_string(20);
            auto file = m_directory.get_file_reference(file_name);
            size_t target_length = 100 * 1024 * 1024;
            azure::storage::file_request_options option;
            option.set_parallelism_factor(2);
            std::vector<uint8_t> data;
            data.resize(target_length);
            fill_buffer(data);
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            file.upload_from_stream(upload_buffer.create_istream(), azure::storage::file_access_condition(), option, m_context);

            // download target file in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;

            utility::size64_t actual_offset = get_random_int32() % 255 + 1;
            utility::size64_t actual_length = target_length - actual_offset;
            file.download_range_to_stream(download_buffer.create_ostream(), actual_offset, std::numeric_limits<utility::size64_t>::max(), azure::storage::file_access_condition(), option, context);

            check_parallelism(context, 2);
            CHECK(file.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == actual_length);
            CHECK(std::equal(data.begin() + actual_offset, data.end(), download_buffer.collection().begin()));
        }
    }

    /// <summary>
    /// Test parallel download wit length too large
    /// </summary>
    TEST_FIXTURE(file_test_base, parallel_download_with_length_too_large)
    {
        // file with size larger than 32MB.
        // With offset not zero.
        {
            auto file_name = get_random_string(20);
            auto file = m_directory.get_file_reference(file_name);
            size_t target_length = 100 * 1024 * 1024;
            azure::storage::file_request_options option;
            option.set_parallelism_factor(10);
            std::vector<uint8_t> data;
            data.resize(target_length);
            fill_buffer(data);
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            file.upload_from_stream(upload_buffer.create_istream(), azure::storage::file_access_condition(), option, m_context);

            // download target file in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;

            utility::size64_t actual_offset = get_random_int32() % 255 + 1;
            utility::size64_t actual_length = target_length - actual_offset;
            file.download_range_to_stream(download_buffer.create_ostream(), actual_offset, actual_length * 2, azure::storage::file_access_condition(), option, context);

            check_parallelism(context, 10);
            CHECK(file.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == actual_length);
            CHECK(std::equal(data.begin() + actual_offset, data.end(), download_buffer.collection().begin()));
        }
    }

    TEST_FIXTURE(file_test_base, parallel_download_with_md5)
    {
        // transactional md5 enabled.
        // download file smaller than 4MB.
        {
            auto file_name = get_random_string(20);
            auto file = m_share.get_root_directory_reference().get_file_reference(file_name);
            size_t target_length = 1 * 1024 * 1024;
            azure::storage::file_request_options option;
            option.set_parallelism_factor(2);
            option.set_use_transactional_md5(true);
            std::vector<uint8_t> data;
            data.resize(target_length);
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            file.upload_from_stream(upload_buffer.create_istream(), azure::storage::file_access_condition(), option, m_context);

            // download target file in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            file.download_to_stream(download_buffer.create_ostream(), azure::storage::file_access_condition(), option, context);

            check_parallelism(context, 1);
            CHECK(file.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == target_length);
            CHECK(std::equal(data.begin(), data.end(), download_buffer.collection().begin()));
        }

        // download file larger than 4MB.
        {
            auto file_name = get_random_string(20);
            auto file = m_share.get_root_directory_reference().get_file_reference(file_name);
            size_t target_length = 21 * 1024 * 1024;
            azure::storage::file_request_options option;
            option.set_parallelism_factor(2);
            option.set_use_transactional_md5(true);
            std::vector<uint8_t> data;
            data.resize(target_length);
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            file.upload_from_stream(upload_buffer.create_istream(), azure::storage::file_access_condition(), option, m_context);

            // download target file in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            file.download_to_stream(download_buffer.create_ostream(), azure::storage::file_access_condition(), option, context);

            check_parallelism(context, 2);
            CHECK(file.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == target_length);
            CHECK(std::equal(data.begin(), data.end(), download_buffer.collection().begin()));
        }
    }

    TEST_FIXTURE(file_test_base, parallel_download_empty_file)
    {
        auto file_name = get_random_string(20);
        auto file = m_share.get_root_directory_reference().get_file_reference(file_name);
        size_t target_length = 0;
        azure::storage::file_request_options option;
        option.set_parallelism_factor(2);
        option.set_use_transactional_md5(true);
        std::vector<uint8_t> data;
        data.resize(target_length);
        concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
        file.upload_from_stream(upload_buffer.create_istream(), azure::storage::file_access_condition(), option, m_context);

        // download target file in parallel.
        azure::storage::operation_context context;
        concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
        file.download_to_stream(download_buffer.create_ostream(), azure::storage::file_access_condition(), option, context);

        check_parallelism(context, 1);
        CHECK(file.properties().size() == target_length);
    }

    TEST_FIXTURE(file_test_base, file_lease)
    {
        m_file.create(1024);
        CHECK(azure::storage::lease_status::unspecified == m_file.properties().lease_status());
        CHECK(azure::storage::lease_state::unspecified == m_file.properties().lease_state());
        CHECK(azure::storage::lease_duration::unspecified == m_file.properties().lease_duration());
        m_file.download_attributes();
        CHECK(azure::storage::lease_status::unlocked == m_file.properties().lease_status());
        CHECK(azure::storage::lease_state::available == m_file.properties().lease_state());
        CHECK(azure::storage::lease_duration::unspecified == m_file.properties().lease_duration());

        // Acquire
        utility::string_t lease_id = m_file.acquire_lease(utility::string_t());
        CHECK(azure::storage::lease_status::unspecified == m_file.properties().lease_status());
        CHECK(azure::storage::lease_state::unspecified == m_file.properties().lease_state());
        CHECK(azure::storage::lease_duration::unspecified == m_file.properties().lease_duration());
        m_file.download_attributes();
        CHECK(azure::storage::lease_status::locked == m_file.properties().lease_status());
        CHECK(azure::storage::lease_state::leased == m_file.properties().lease_state());
        CHECK(azure::storage::lease_duration::infinite == m_file.properties().lease_duration());

        // Change
        utility::string_t lease_id2 = utility::uuid_to_string(utility::new_uuid());
        azure::storage::file_access_condition condition;
        condition.set_lease_id(lease_id);
        lease_id = m_file.change_lease(lease_id2, condition);
        utility::details::inplace_tolower(lease_id);
        utility::details::inplace_tolower(lease_id2);
        CHECK(lease_id == lease_id2);
        CHECK(azure::storage::lease_status::unspecified == m_file.properties().lease_status());
        CHECK(azure::storage::lease_state::unspecified == m_file.properties().lease_state());
        CHECK(azure::storage::lease_duration::unspecified == m_file.properties().lease_duration());

        // Break
        m_file.break_lease();
        CHECK(azure::storage::lease_status::unspecified == m_file.properties().lease_status());
        CHECK(azure::storage::lease_state::unspecified == m_file.properties().lease_state());
        CHECK(azure::storage::lease_duration::unspecified == m_file.properties().lease_duration());
        m_file.download_attributes();
        CHECK(azure::storage::lease_status::unlocked == m_file.properties().lease_status());
        CHECK(azure::storage::lease_state::broken == m_file.properties().lease_state());
        CHECK(azure::storage::lease_duration::unspecified == m_file.properties().lease_duration());

        lease_id = m_file.acquire_lease(utility::string_t());
        condition.set_lease_id(lease_id);
        m_file.break_lease(condition, azure::storage::file_request_options(), m_context);

        // Acquire with proposed lease id
        lease_id2 = utility::uuid_to_string(utility::new_uuid());
        lease_id = m_file.acquire_lease(lease_id2);
        utility::details::inplace_tolower(lease_id);
        utility::details::inplace_tolower(lease_id2);
        CHECK(lease_id == lease_id2);

        // Release
        CHECK_THROW(m_file.release_lease(condition), azure::storage::storage_exception);
        condition.set_lease_id(lease_id);
        m_file.release_lease(condition);
        CHECK(azure::storage::lease_status::unspecified == m_file.properties().lease_status());
        CHECK(azure::storage::lease_state::unspecified == m_file.properties().lease_state());
        CHECK(azure::storage::lease_duration::unspecified == m_file.properties().lease_duration());
        m_file.download_attributes();
        CHECK(azure::storage::lease_status::unlocked == m_file.properties().lease_status());
        CHECK(azure::storage::lease_state::available == m_file.properties().lease_state());
        CHECK(azure::storage::lease_duration::unspecified == m_file.properties().lease_duration());
    }

    TEST_FIXTURE(file_test_base, file_operations_with_lease)
    {
        m_file.create(1024);
        utility::string_t lease_id = m_file.acquire_lease(utility::string_t());

        azure::storage::file_access_condition lease_condition;
        lease_condition.set_lease_id(lease_id);
        azure::storage::file_access_condition wrong_condition;
        wrong_condition.set_lease_id(utility::uuid_to_string(utility::new_uuid()));
        azure::storage::file_access_condition empty_condition;

        std::vector<azure::storage::file_access_condition> conditions =
        {
            empty_condition, wrong_condition, lease_condition
        };

        utility::string_t upload_content = _XPLATSTR("content");
        concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
        auto copy_src = m_directory.get_file_reference(_XPLATSTR("copy_src"));
        copy_src.create(1024);
        copy_src.upload_text(upload_content);
        utility::string_t copy_id;
        std::vector<std::function<void(azure::storage::file_access_condition)>> funcs =
        {
            // Create
            [&](azure::storage::file_access_condition condition) { m_file.create(2048, condition, azure::storage::file_request_options(), m_context); },
            // Create if not exists
            [&](azure::storage::file_access_condition condition) { m_file.create_if_not_exists(2048, condition, azure::storage::file_request_options(), m_context); },
            // Download attributes
            [&](azure::storage::file_access_condition condition) { m_file.download_attributes(condition, azure::storage::file_request_options(), m_context); },
            // Exist
            [&](azure::storage::file_access_condition condition) { m_file.exists(condition, azure::storage::file_request_options(), m_context); },
            // Upload properties
            [&](azure::storage::file_access_condition condition) { m_file.upload_properties(condition, azure::storage::file_request_options(), m_context); },
            // Upload metadata
            [&](azure::storage::file_access_condition condition) { m_file.upload_metadata(condition, azure::storage::file_request_options(), m_context); },
            // Resize
            [&](azure::storage::file_access_condition condition) { m_file.resize(4096, condition, azure::storage::file_request_options(), m_context); },
            // Upload from stream
            [&](azure::storage::file_access_condition condition) { m_file.upload_from_stream(concurrency::streams::bytestream::open_istream(utility::conversions::to_utf8string(upload_content)), condition, azure::storage::file_request_options(), m_context); },
            // Write range
            [&](azure::storage::file_access_condition condition) { m_file.write_range(concurrency::streams::bytestream::open_istream(utility::conversions::to_utf8string(upload_content)), 0, utility::string_t(), condition, azure::storage::file_request_options(), m_context); },
            // List ranges
            [&](azure::storage::file_access_condition condition) { m_file.list_ranges(0, 4096, condition, azure::storage::file_request_options(), m_context); },
            // Download range
            [&](azure::storage::file_access_condition condition) { m_file.download_to_stream(download_buffer.create_ostream(), condition, azure::storage::file_request_options(), m_context); },
            // Clear range
            [&](azure::storage::file_access_condition condition) { m_file.clear_range(0, 1, condition, azure::storage::file_request_options(), m_context); },
            // Start copy
            [&](azure::storage::file_access_condition condition)
            {
                auto id = m_file.start_copy(copy_src.uri().primary_uri(), azure::storage::access_condition(), condition, azure::storage::file_request_options(), m_context);
                copy_id = id.empty() ? copy_id : id;
            },
            // Abort copy
            [&](azure::storage::file_access_condition condition) { m_file.abort_copy(copy_id, condition, azure::storage::file_request_options(), m_context); },
            // Delete if exists
            [&](azure::storage::file_access_condition condition) { m_file.delete_file_if_exists(condition, azure::storage::file_request_options(), m_context); },
        };

        std::vector<std::vector<bool>> expected_results =
        {
            // Create
            {0, 0, 1},
            // Create if not exists
            {0, 0, 1},
            // Download Attributes
            {1, 0, 1},
            // Exist
            {1, 0, 1},
            // Upload properties
            {0, 0, 1},
            // Upload metadata
            {0, 0, 1},
            // Resize
            {0, 0, 1},
            // Upload from stream
            {0, 0, 1},
            // Write range
            {0, 0, 1},
            // List ranges
            {1, 0, 1},
            // Download range
            {1, 0, 1},
            // Clear range
            {0, 0, 1},
            // Start copy
            {0, 0, 1},
            // Abort copy
            {0, 0, 1},
            // Delete if exists
            {0, 0, 1},
        };
        CHECK_EQUAL(funcs.size(), expected_results.size());

        for (int i = 0; i < funcs.size(); ++i)
        {
            for (int j = 0; j < conditions.size(); ++j)
            {
                try
                {
                    funcs[i](conditions[j]);
                }
                catch (azure::storage::storage_exception& e)
                {
                    if (expected_results[i][j] == true && e.result().http_status_code() == web::http::status_codes::PreconditionFailed)
                    {
                        throw;
                    }
                }
            }
        }
    }
}
