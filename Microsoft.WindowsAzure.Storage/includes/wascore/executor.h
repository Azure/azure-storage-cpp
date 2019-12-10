// -----------------------------------------------------------------------------------------
// <copyright file="executor.h" company="Microsoft">
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

#include "cpprest/http_client.h"

#include "basic_types.h"
#include "logging.h"
#include "util.h"
#include "streams.h"
#include "was/auth.h"
#include "wascore/constants.h"
#include "wascore/resources.h"
#include "wascore/timer_handler.h"

#pragma push_macro("max")
#undef max

namespace azure { namespace storage { namespace core {

    class istream_descriptor
    {
    public:

        istream_descriptor()
            : m_offset(std::numeric_limits<concurrency::streams::istream::pos_type>::max()), m_length(std::numeric_limits<utility::size64_t>::max())
        {
        }
        
        static pplx::task<istream_descriptor> create(concurrency::streams::istream stream, checksum_type calculate_checksum = checksum_type::none, utility::size64_t length = std::numeric_limits<utility::size64_t>::max(), utility::size64_t max_length = std::numeric_limits<utility::size64_t>::max(), const pplx::cancellation_token& cancellation_token = pplx::cancellation_token::none())
        {
            if (length == std::numeric_limits<utility::size64_t>::max())
            {
                length = get_remaining_stream_length(stream);
            }

            if (length != std::numeric_limits<utility::size64_t>::max() && length > max_length)
            {
                throw std::invalid_argument(protocol::error_stream_length);
            }

            if (calculate_checksum == checksum_type::none && stream.can_seek())
            {
                return pplx::task_from_result(istream_descriptor(stream, length, checksum(checksum_none)));
            }

            hash_provider provider = core::hash_provider();

            if (calculate_checksum == checksum_type::md5)
            {
                provider = core::hash_provider::create_md5_hash_provider();
            }
            else if (calculate_checksum == checksum_type::crc64)
            {
                provider = core::hash_provider::create_crc64_hash_provider();
            }
            concurrency::streams::container_buffer<std::vector<uint8_t>> temp_buffer;
            concurrency::streams::ostream temp_stream;

            if (calculate_checksum != checksum_type::none)
            {
                temp_stream = hash_wrapper_streambuf<concurrency::streams::ostream::traits::char_type>(temp_buffer, provider).create_ostream();
            }
            else
            {
                temp_stream = temp_buffer.create_ostream();
            }

            return stream_copy_async(stream, temp_stream, length, max_length, cancellation_token).then([temp_buffer, provider] (pplx::task<utility::size64_t> buffer_task) mutable -> istream_descriptor
            {
                auto length = buffer_task.get();
                provider.close();
                return istream_descriptor(concurrency::streams::container_stream<std::vector<uint8_t>>::open_istream(temp_buffer.collection()), length, provider.hash());
            });
        }

        bool is_valid() const
        {
            return m_stream.is_valid();
        }

        concurrency::streams::istream stream() const
        {
            return m_stream;
        }

        utility::size64_t length() const
        {
            return m_length;
        }

        const checksum& content_checksum() const
        {
            return m_content_checksum;
        }

        void rewind()
        {
            m_stream.seek(m_offset);
        }

    private:
        
        istream_descriptor(concurrency::streams::istream stream, utility::size64_t length, checksum content_checksum)
            : m_stream(stream), m_offset(stream.tell()), m_length(length), m_content_checksum(std::move(content_checksum))
        {
        }

        concurrency::streams::istream m_stream;
        concurrency::streams::istream::pos_type m_offset;
        checksum m_content_checksum;
        utility::size64_t m_length;
    };

    class ostream_descriptor
    {
    public:

        ostream_descriptor()
            : m_length(std::numeric_limits<utility::size64_t>::max())
        {
        }

        ostream_descriptor(utility::size64_t length, checksum content_checksum)
            : m_length(length), m_content_checksum(std::move(content_checksum))
        {
        }

        utility::size64_t length() const
        {
            return m_length;
        }

        const checksum& content_checksum() const
        {
            return m_content_checksum;
        }

    private:
        
        checksum m_content_checksum;
        utility::size64_t m_length;
    };

    enum class command_location_mode
    {
        primary_only,
        secondary_only,
        primary_or_secondary,
    };

    class storage_command_base
    {
    public:

        explicit storage_command_base(const storage_uri& request_uri, const pplx::cancellation_token& cancellation_token, const bool use_timeout, std::shared_ptr<core::timer_handler> timer_handler)
            : m_request_uri(request_uri), m_location_mode(command_location_mode::primary_only),
            m_cancellation_token(cancellation_token), m_calculate_response_body_checksum(checksum_type::none), m_use_timeout(use_timeout), m_timer_handler(timer_handler)
        {
            if (m_use_timeout)
            {
                m_timer_handler = std::make_shared<core::timer_handler>(m_cancellation_token);
            }
        }

#if defined(_MSC_VER) && _MSC_VER < 1900

        // Prevents the compiler from generating default assignment operator.
        storage_command_base& operator=(storage_command_base& other) = delete;

#endif

        void set_request_body(istream_descriptor value)
        {
            m_request_body = value;
        }

        void set_destination_stream(concurrency::streams::ostream value)
        {
            if (!value)
            {
                throw std::invalid_argument("stream");
            }

            m_destination_stream = value;
        }

        void set_calculate_response_body_checksum(checksum_type value)
        {
            m_calculate_response_body_checksum = value;
        }

        void set_build_request(std::function<web::http::http_request(web::http::uri_builder&, const std::chrono::seconds&, operation_context)> value)
        {
            m_build_request = value;
        }

        void set_custom_sign_request(std::function<void(web::http::http_request &, operation_context)> value)
        {
            m_sign_request = value;
        }

        void set_authentication_handler(std::shared_ptr<protocol::authentication_handler> handler)
        {
            set_custom_sign_request(std::bind(&protocol::authentication_handler::sign_request, handler, std::placeholders::_1, std::placeholders::_2));
        }

        void set_recover_request(std::function<bool(utility::size64_t, operation_context)> value)
        {
            m_recover_request = value;
        }

        void set_location_mode(command_location_mode value, storage_location lock_location = storage_location::unspecified)
        {
            switch (lock_location)
            {
            case storage_location::primary:
                if (value == command_location_mode::secondary_only)
                {
                    throw storage_exception(protocol::error_secondary_only_command, false);
                }

                m_location_mode = command_location_mode::primary_only;
                break;

            case storage_location::secondary:
                if (value == command_location_mode::primary_only)
                {
                    throw storage_exception(protocol::error_primary_only_command, false);
                }

                m_location_mode = command_location_mode::secondary_only;
                break;

            default:
                m_location_mode = value;
                break;
            }
        }

        bool is_canceled()
        {
            if (m_use_timeout)
            {
                return m_timer_handler->is_canceled();
            }
            else
            {
                return m_cancellation_token.is_canceled();
            }
        }

        const pplx::cancellation_token get_cancellation_token() const
        {
            if (m_use_timeout)
            {
                return m_timer_handler->get_cancellation_token();
            }
            else
            {
                return m_cancellation_token;
            }
        }

    private:

        virtual void preprocess_response(const web::http::http_response&, const request_result&, operation_context) = 0;
        virtual pplx::task<void> postprocess_response(const web::http::http_response&, const request_result&, const ostream_descriptor&, operation_context) = 0;

        storage_uri m_request_uri;
        istream_descriptor m_request_body;
        concurrency::streams::ostream m_destination_stream;
        checksum_type m_calculate_response_body_checksum;
        command_location_mode m_location_mode;

        const pplx::cancellation_token m_cancellation_token;
        std::shared_ptr<timer_handler> m_timer_handler;
        bool m_use_timeout;

        std::function<web::http::http_request(web::http::uri_builder&, const std::chrono::seconds&, operation_context)> m_build_request;
        std::function<void(web::http::http_request&, operation_context)> m_sign_request;
        std::function<bool(utility::size64_t, operation_context)> m_recover_request;

        friend class executor_impl;
    };

    template<typename T>
    class storage_command : public storage_command_base
    {
    public:

        explicit storage_command(const storage_uri& request_uri, const pplx::cancellation_token& cancellation_token = pplx::cancellation_token::none(), const bool use_timeout = false, std::shared_ptr<core::timer_handler> timer_handler = nullptr)
            : storage_command_base(request_uri, cancellation_token, use_timeout, timer_handler)
        {
        }

        void set_preprocess_response(std::function<T(const web::http::http_response&, const request_result&, operation_context)> value)
        {
            m_preprocess_response = value;
        }

        void set_postprocess_response(std::function<pplx::task<T>(const web::http::http_response&, const request_result&, const ostream_descriptor&, operation_context)> value)
        {
            m_postprocess_response = value;
        }

        T&& result()
        {
            return std::move(m_result);
        }

    private:

        virtual void preprocess_response(const web::http::http_response& response, const request_result& result, operation_context context)
        {
            if (m_preprocess_response != nullptr)
            {
                m_result = m_preprocess_response(response, result, context);
            }
        }

        virtual pplx::task<void> postprocess_response(const web::http::http_response& response, const request_result& result, const ostream_descriptor& descriptor, operation_context context)
        {
            if (m_postprocess_response != nullptr)
            {
                return m_postprocess_response(response, result, descriptor, context).then([this](pplx::task<T> task)
                {
                    m_result = task.get();
                });
            }

            return pplx::task_from_result();
        }

        std::function<T (const web::http::http_response&, const request_result&, operation_context)> m_preprocess_response;
        std::function<pplx::task<T> (const web::http::http_response&, const request_result&, const ostream_descriptor&, operation_context)> m_postprocess_response;
        T m_result;
    };

    template<>
    class storage_command<void> : public storage_command_base
    {
    public:

        explicit storage_command(const storage_uri& request_uri, const pplx::cancellation_token& cancellation_token = pplx::cancellation_token::none(), const bool use_timeout = false, std::shared_ptr<core::timer_handler> timer_handler = nullptr)
            : storage_command_base(request_uri, cancellation_token, use_timeout, timer_handler)
        {
        }

        void set_preprocess_response(std::function<void(const web::http::http_response&, const request_result&, operation_context)> value)
        {
            m_preprocess_response = value;
        }

        void set_postprocess_response(std::function<pplx::task<void>(const web::http::http_response&, const request_result&, const ostream_descriptor&, operation_context)> value)
        {
            m_postprocess_response = value;
        }

    private:
        virtual void preprocess_response(const web::http::http_response& response, const request_result& result, operation_context context)
        {
            if (m_preprocess_response != nullptr)
            {
                m_preprocess_response(response, result, context);
            }
        }

        virtual pplx::task<void> postprocess_response(const web::http::http_response& response, const request_result& result, const ostream_descriptor& descriptor, operation_context context)
        {
            if (m_postprocess_response != nullptr)
            {
                return m_postprocess_response(response, result, descriptor, context);
            }

            return pplx::task_from_result();
        }

        std::function<void(const web::http::http_response&, const request_result&, operation_context)> m_preprocess_response;
        std::function<pplx::task<void>(const web::http::http_response&, const request_result&, const ostream_descriptor&, operation_context)> m_postprocess_response;
    };

    class executor_impl
    {
    public:

        executor_impl(std::shared_ptr<storage_command_base> command, const request_options& options, operation_context context)
            : m_command(command), m_request_options(options), m_context(context), m_is_hashing_started(false),
            m_total_downloaded(0), m_retry_count(0), m_current_location(get_first_location(options.location_mode())),
            m_current_location_mode(options.location_mode()), m_retry_policy(options.retry_policy().clone()),
            m_should_restart_hash_provider(false)
        {
        }

        WASTORAGE_API static pplx::task<void> execute_async(std::shared_ptr<storage_command_base> command, const request_options& options, operation_context context);
 
    private:

        std::chrono::milliseconds remaining_time() const
        {
            if (m_request_options.operation_expiry_time().time_since_epoch().count())
            {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(m_request_options.operation_expiry_time() - std::chrono::system_clock::now());
                if (duration.count() > 0)
                {
                    return duration;
                }
                else
                {
                    throw storage_exception(protocol::error_client_timeout, false);
                }
            }

            return std::chrono::milliseconds();
        }

        void assert_canceled() const
        {
            //Throw timeout if timeout is the reason of canceling.
            core::assert_timed_out_by_timer(m_command->m_timer_handler);
            if (m_command->is_canceled())
            {
                throw storage_exception(protocol::error_operation_canceled);
            }
        }

        static storage_location get_first_location(location_mode mode)
        {
            switch (mode)
            {
            case location_mode::primary_only:
            case location_mode::primary_then_secondary:
                return storage_location::primary;

            case location_mode::secondary_only:
            case location_mode::secondary_then_primary:
                return storage_location::secondary;
            }

            throw std::invalid_argument("mode");
        }

        storage_location get_next_location() const
        {
            switch (m_current_location_mode)
            {
            case location_mode::primary_only:
                return storage_location::primary;

            case location_mode::secondary_only:
                return storage_location::secondary;

            case location_mode::primary_then_secondary:
            case location_mode::secondary_then_primary:
                return m_current_location == storage_location::primary ?
                    storage_location::secondary :
                    storage_location::primary;
            }

            throw std::invalid_argument("mode");
        }

        void validate_location_mode()
        {
            bool is_valid;
            switch (m_current_location_mode)
            {
            case location_mode::primary_only:
                is_valid = !m_command->m_request_uri.primary_uri().is_empty();
                break;

            case location_mode::secondary_only:
                is_valid = !m_command->m_request_uri.secondary_uri().is_empty();
                break;

            default:
                is_valid = !m_command->m_request_uri.primary_uri().is_empty() &&
                    !m_command->m_request_uri.secondary_uri().is_empty();
                break;
            }

            if (!is_valid)
            {
                throw storage_exception(protocol::error_uri_missing_location, false);
            }

            switch (m_command->m_location_mode)
            {
            case command_location_mode::primary_only:
                if (m_current_location_mode == location_mode::secondary_only)
                {
                    throw storage_exception(protocol::error_primary_only_command, false);
                }

                if (logger::instance().should_log(m_context, client_log_level::log_level_verbose))
                {
                    logger::instance().log(m_context, client_log_level::log_level_verbose, protocol::error_primary_only_command);
                }

                m_current_location_mode = location_mode::primary_only;
                m_current_location = storage_location::primary;
                break;

            case command_location_mode::secondary_only:
                if (m_current_location_mode == location_mode::primary_only)
                {
                    throw storage_exception(protocol::error_secondary_only_command, false);
                }

                if (logger::instance().should_log(m_context, client_log_level::log_level_verbose))
                {
                    logger::instance().log(m_context, client_log_level::log_level_verbose, protocol::error_secondary_only_command);
                }

                m_current_location_mode = location_mode::secondary_only;
                m_current_location = storage_location::secondary;
                break;
            }
        }

        template<typename Value>
        void add_request_header(const web::http::http_headers::key_type& name, const Value& value)
        {
            auto& headers = m_request.headers();
            if (headers.has(name))
            {
                headers.remove(name);
            }

            headers.add(name, value);
        }

        static std::exception_ptr capture_inner_exception(const std::exception& exception)
        {
            if (nullptr == dynamic_cast<const storage_exception*>(&exception))
            {
                // exception other than storage_exception is captured as inner exception.
                return std::current_exception();
            }

            return nullptr;
        }

        std::shared_ptr<storage_command_base> m_command;
        request_options m_request_options;
        operation_context m_context;
        utility::datetime m_start_time;
        web::http::uri_builder m_uri_builder;
        web::http::http_request m_request;
        request_result m_request_result;
        bool m_is_hashing_started;
        bool m_should_restart_hash_provider;
        hash_provider m_hash_provider;
        hash_wrapper_streambuf<concurrency::streams::ostream::traits::char_type> m_response_streambuf;
        utility::size64_t m_total_downloaded;
        retry_policy m_retry_policy;
        int m_retry_count;
        storage_location m_current_location;
        location_mode m_current_location_mode;
    };

    template<typename T>
    class executor
    {
    public:
        static pplx::task<T> execute_async(std::shared_ptr<storage_command<T>> command, const request_options& options, operation_context context)
        {
            return executor_impl::execute_async(command, options, context).then([command](pplx::task<void> task) -> T
            {
                task.get();
                return command->result();
            });
        }
    };

    template<>
    class executor<void>
    {
    public:
        static pplx::task<void> execute_async(std::shared_ptr<storage_command<void>> command, const request_options& options, operation_context context)
        {
            return executor_impl::execute_async(command, options, context);
        }
    };

}}} // namespace azure::storage::core

#pragma pop_macro("max")
