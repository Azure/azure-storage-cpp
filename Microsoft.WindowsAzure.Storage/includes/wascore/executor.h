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
        
        static pplx::task<istream_descriptor> create(concurrency::streams::istream stream, bool calculate_md5 = false, utility::size64_t length = std::numeric_limits<utility::size64_t>::max(), utility::size64_t max_length = std::numeric_limits<utility::size64_t>::max())
        {
            if (length == std::numeric_limits<utility::size64_t>::max())
            {
                length = get_remaining_stream_length(stream);
            }

            if (length != std::numeric_limits<utility::size64_t>::max() && length > max_length)
            {
                throw std::invalid_argument(protocol::error_stream_length);
            }

            if (!calculate_md5 && stream.can_seek())
            {
                return pplx::task_from_result(istream_descriptor(stream, length, utility::string_t()));
            }

            hash_provider provider = calculate_md5 ? core::hash_provider::create_md5_hash_provider() : core::hash_provider();
            concurrency::streams::container_buffer<std::vector<uint8_t>> temp_buffer;
            concurrency::streams::ostream temp_stream;

            if (calculate_md5)
            {
                temp_stream = hash_wrapper_streambuf<concurrency::streams::ostream::traits::char_type>(temp_buffer, provider).create_ostream();
            }
            else
            {
                temp_stream = temp_buffer.create_ostream();
            }

            return stream_copy_async(stream, temp_stream, length, max_length).then([temp_buffer, provider] (pplx::task<utility::size64_t> buffer_task) mutable -> istream_descriptor
            {
                provider.close();
                return istream_descriptor(concurrency::streams::container_stream<std::vector<uint8_t>>::open_istream(temp_buffer.collection()), buffer_task.get(), provider.hash());
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

        const utility::string_t& content_md5() const
        {
            return m_content_md5;
        }

        void rewind()
        {
            m_stream.seek(m_offset);
        }

    private:
        
        istream_descriptor(concurrency::streams::istream stream, utility::size64_t length, utility::string_t content_md5)
            : m_stream(stream), m_offset(stream.tell()), m_length(length), m_content_md5(std::move(content_md5))
        {
        }

        concurrency::streams::istream m_stream;
        concurrency::streams::istream::pos_type m_offset;
        utility::string_t m_content_md5;
        utility::size64_t m_length;
    };

    class ostream_descriptor
    {
    public:

        ostream_descriptor()
            : m_length(std::numeric_limits<utility::size64_t>::max())
        {
        }

        ostream_descriptor(utility::size64_t length, utility::string_t content_md5)
            : m_length(length), m_content_md5(std::move(content_md5))
        {
        }

        utility::size64_t length() const
        {
            return m_length;
        }

        const utility::string_t& content_md5() const
        {
            return m_content_md5;
        }

    private:
        
        utility::string_t m_content_md5;
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

        explicit storage_command_base(const storage_uri& request_uri)
            : m_request_uri(request_uri), m_location_mode(command_location_mode::primary_only)
        {
        }

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

        void set_calculate_response_body_md5(bool value)
        {
            m_calculate_response_body_md5 = value;
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

    private:

        virtual void preprocess_response(const web::http::http_response&, const request_result&, operation_context) = 0;
        virtual pplx::task<void> postprocess_response(const web::http::http_response&, const request_result&, const ostream_descriptor&, operation_context) = 0;

        storage_uri m_request_uri;
        istream_descriptor m_request_body;
        concurrency::streams::ostream m_destination_stream;
        bool m_calculate_response_body_md5;
        command_location_mode m_location_mode;

        std::function<web::http::http_request(web::http::uri_builder&, const std::chrono::seconds&, operation_context)> m_build_request;
        std::function<void(web::http::http_request&, operation_context)> m_sign_request;
        std::function<bool(utility::size64_t, operation_context)> m_recover_request;

        friend class executor_impl;
    };

    template<typename T>
    class storage_command : public storage_command_base
    {
    public:

        explicit storage_command(const storage_uri& request_uri)
            : storage_command_base(request_uri)
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

        explicit storage_command(const storage_uri& request_uri)
            : storage_command_base(request_uri)
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

        static pplx::task<void> execute_async(std::shared_ptr<storage_command_base> command, const request_options& options, operation_context context)
        {
            if (!context.start_time().is_initialized())
            {
                context.set_start_time(utility::datetime::utc_now());
            }

            // TODO: Use "it" variable name for iterators in for loops
            // TODO: Reduce usage of auto variable types

            auto instance = std::make_shared<executor_impl>(command, options, context);
            return pplx::details::do_while([instance]() -> pplx::task<bool>
            {
                // 0. Begin request 
                instance->validate_location_mode();

                // 1. Build request
                instance->m_start_time = utility::datetime::utc_now();
                instance->m_uri_builder = web::http::uri_builder(instance->m_command->m_request_uri.get_location_uri(instance->m_current_location));
                instance->m_request = instance->m_command->m_build_request(instance->m_uri_builder, instance->m_request_options.server_timeout(), instance->m_context);
                instance->m_request_result = request_result(instance->m_start_time, instance->m_current_location);

                if (logger::instance().should_log(instance->m_context, client_log_level::log_level_informational))
                {
                    utility::string_t str;
                    str.reserve(256);
                    str.append(_XPLATSTR("Starting ")).append(instance->m_request.method()).append(_XPLATSTR(" request to ")).append(instance->m_request.request_uri().to_string());
                    logger::instance().log(instance->m_context, client_log_level::log_level_informational, str);
                }

                // 2. Set Headers
                auto& client_request_id = instance->m_context.client_request_id();
                if (!client_request_id.empty())
                {
                    instance->add_request_header(protocol::ms_header_client_request_id, client_request_id);
                }

                auto& user_headers = instance->m_context.user_headers();
                for (auto iter = user_headers.begin(); iter != user_headers.end(); ++iter)
                {
                    instance->add_request_header(iter->first, iter->second);
                }

                // If the command provided a request body, set it on the http_request object
                if (instance->m_command->m_request_body.is_valid())
                {
                    instance->m_command->m_request_body.rewind();
                    instance->m_request.set_body(instance->m_command->m_request_body.stream(), instance->m_command->m_request_body.length(), utility::string_t());
                }

                // If the command wants to copy the response body to a stream, set it
                // on the http_request object
                if (instance->m_command->m_destination_stream)
                {
                    // Calculate the length and MD5 hash if needed as the incoming data is read
                    if (!instance->m_is_hashing_started)
                    {
                        if (instance->m_command->m_calculate_response_body_md5)
                        {
                            instance->m_hash_provider = hash_provider::create_md5_hash_provider();
                        }

                        instance->m_total_downloaded = 0;
                        instance->m_is_hashing_started = true;
                        instance->m_should_restart_hash_provider = false;

                        // TODO: Consider using hash_provider::is_enabled instead of m_is_hashing_started to signal when the hash provider has been closed
                    }

                    if (instance->m_should_restart_hash_provider)
                    {
                        if (instance->m_command->m_calculate_response_body_md5)
                        {
                            instance->m_hash_provider = hash_provider::create_md5_hash_provider();
                        }
                        instance->m_should_restart_hash_provider = false;
                    }

                    instance->m_response_streambuf = hash_wrapper_streambuf<concurrency::streams::ostream::traits::char_type>(instance->m_command->m_destination_stream.streambuf(), instance->m_hash_provider);
                    instance->m_request.set_response_stream(instance->m_response_streambuf.create_ostream());
                }

                // Let the user know we are ready to send
                auto sending_request = instance->m_context._get_impl()->sending_request();
                if (sending_request)
                {
                    sending_request(instance->m_request, instance->m_context);
                }

                // 3. Sign Request
                instance->m_command->m_sign_request(instance->m_request, instance->m_context);

                // 4. Set HTTP client configuration
                web::http::client::http_client_config config;
                if (instance->m_context.proxy().is_specified())
                {
                    config.set_proxy(instance->m_context.proxy());
                }

                instance->remaining_time();
                config.set_timeout(instance->m_request_options.noactivity_timeout());

                size_t http_buffer_size = instance->m_request_options.http_buffer_size();
                if (http_buffer_size > 0)
                {
                    config.set_chunksize(http_buffer_size);
                }

                // 5-6. Potentially upload data and get response
#ifdef _WIN32
                web::http::client::http_client client(instance->m_request.request_uri().authority(), config);
                return client.request(instance->m_request).then([instance](pplx::task<web::http::http_response> get_headers_task)->pplx::task<web::http::http_response>
#else
                std::shared_ptr<web::http::client::http_client> client = core::http_client_reusable::get_http_client(instance->m_request.request_uri().authority(), config);
                return client->request(instance->m_request).then([instance](pplx::task<web::http::http_response> get_headers_task) -> pplx::task<web::http::http_response>
#endif // _WIN32
                {
                    // Headers are ready. It should be noted that http_client will
                    // continue to download the response body in parallel.
                    auto response = get_headers_task.get();

                    if (logger::instance().should_log(instance->m_context, client_log_level::log_level_informational))
                    {
                        utility::string_t str;
                        str.reserve(128);
                        str.append(_XPLATSTR("Response received. Status code = ")).append(utility::conversions::print_string(response.status_code())).append(_XPLATSTR(". Reason = ")).append(response.reason_phrase());
                        logger::instance().log(instance->m_context, client_log_level::log_level_informational, str);
                    }

                    try
                    {
                        // Let the user know we received response
                        auto response_received = instance->m_context._get_impl()->response_received();
                        if (response_received)
                        {
                            response_received(instance->m_request, response, instance->m_context);
                        }

                        // 7. Do Response parsing (headers etc, no stream available here)
                        // This is when the status code will be checked and m_preprocess_response
                        // will throw a storage_exception if it is not expected.
                        instance->m_request_result = request_result(instance->m_start_time, instance->m_current_location, response, false);
                        instance->m_command->preprocess_response(response, instance->m_request_result, instance->m_context);

                        if (logger::instance().should_log(instance->m_context, client_log_level::log_level_informational))
                        {
                            logger::instance().log(instance->m_context, client_log_level::log_level_informational, _XPLATSTR("Successful request ID = ") + instance->m_request_result.service_request_id());
                        }

                        // 8. Potentially download data
                        return response.content_ready();
                    }
                    catch (const storage_exception& e)
                    {
                        // If the exception already contains an error message, the issue is not with
                        // the response, so rethrowing is the right thing.
                        if (e.what() != NULL && e.what()[0] != '\0')
                        {
                            throw;
                        }

                        // Otherwise, response body might contain an error coming from the Storage service.

                        return response.content_ready().then([instance](pplx::task<web::http::http_response> get_error_body_task) -> web::http::http_response
                        {
                            auto response = get_error_body_task.get();

                            if (!instance->m_command->m_destination_stream)
                            {
                                // However, if the command has a destination stream, there is no guarantee that it
                                // is seekable and thus it cannot be read back to parse the error.
                                instance->m_request_result = request_result(instance->m_start_time, instance->m_current_location, response, true);
                            }
                            else
                            {
                                // Command has a destination stream. In this case, error information
                                // contained in response body might have been written into the destination
                                // stream. Need recreate the hash_provider since a retry might be needed.
                                instance->m_is_hashing_started = false;
                            }

                            if (logger::instance().should_log(instance->m_context, client_log_level::log_level_warning))
                            {
                                logger::instance().log(instance->m_context, client_log_level::log_level_warning, _XPLATSTR("Failed request ID = ") + instance->m_request_result.service_request_id());
                            }

                            throw storage_exception(utility::conversions::to_utf8string(response.reason_phrase()));
                        });
                    }
                }).then([instance](pplx::task<web::http::http_response> get_body_task) -> pplx::task<void>
                {
                    // 9. Evaluate response & parse results
                    auto response = get_body_task.get();

                    if (instance->m_command->m_destination_stream)
                    {
                        utility::size64_t current_total_downloaded = instance->m_response_streambuf.total_written();
                        utility::size64_t content_length = instance->m_request_result.content_length();
                        if (content_length != -1 && current_total_downloaded != content_length)
                        {
                            // The download was interrupted before it could complete
                            throw storage_exception(protocol::error_incorrect_length);
                        }
                    }

                    // It is now time to call m_postprocess_response
                    // Finish the MD5 hash if MD5 was being calculated
                    instance->m_hash_provider.close();
                    instance->m_is_hashing_started = false;

                    ostream_descriptor descriptor;
                    if (instance->m_response_streambuf)
                    {
                        utility::size64_t total_downloaded = instance->m_total_downloaded + instance->m_response_streambuf.total_written();
                        descriptor = ostream_descriptor(total_downloaded, instance->m_hash_provider.hash());
                    }

                    return instance->m_command->postprocess_response(response, instance->m_request_result, descriptor, instance->m_context).then([instance](pplx::task<void> result_task)
                    {
                        try
                        {
                            result_task.get();
                        }
                        catch (const storage_exception& e)
                        {
                            if (e.result().is_response_available())
                            {
                                instance->m_request_result.set_http_status_code(e.result().http_status_code());
                                instance->m_request_result.set_extended_error(e.result().extended_error());
                            }

                            throw;
                        }

                    });
                }).then([instance](pplx::task<void> final_task) -> pplx::task<bool>
                {
                    bool retryable_exception = true;
                    instance->m_context._get_impl()->add_request_result(instance->m_request_result);

                    try
                    {
                        try
                        {
                            final_task.wait();
                        }
                        catch (const storage_exception& e)
                        {
                            retryable_exception = e.retryable();
                            throw;
                        }
                    }
                    catch (const std::exception& e)
                    {
                        //
                        // exception thrown by previous steps are handled here below
                        //

                        if (logger::instance().should_log(instance->m_context, client_log_level::log_level_warning))
                        {
                            logger::instance().log(instance->m_context, client_log_level::log_level_warning, _XPLATSTR("Exception thrown while processing response: ") + utility::conversions::to_string_t(e.what()));
                        }

                        if (!retryable_exception)
                        {
                            if (logger::instance().should_log(instance->m_context, client_log_level::log_level_error))
                            {
                                logger::instance().log(instance->m_context, client_log_level::log_level_error, _XPLATSTR("Exception was not retryable: ") + utility::conversions::to_string_t(e.what()));
                            }

                            throw storage_exception(e.what(), instance->m_request_result, capture_inner_exception(e), false);
                        }

                        // An exception occured and thus the request might be retried. Ask the retry policy.
                        retry_context context(instance->m_retry_count++, instance->m_request_result, instance->get_next_location(), instance->m_current_location_mode);
                        retry_info retry(instance->m_retry_policy.evaluate(context, instance->m_context));
                        if (!retry.should_retry())
                        {
                            if (logger::instance().should_log(instance->m_context, client_log_level::log_level_error))
                            {
                                logger::instance().log(instance->m_context, client_log_level::log_level_error, _XPLATSTR("Retry policy did not allow for a retry, so throwing exception: ") + utility::conversions::to_string_t(e.what()));
                            }

                            throw storage_exception(e.what(), instance->m_request_result, capture_inner_exception(e), false);
                        }

                        instance->m_current_location = retry.target_location();
                        instance->m_current_location_mode = retry.updated_location_mode();

                        // Hash provider may be closed by Casablanca due to stream error. Close hash provider and force to recreation when retry.
                        instance->m_hash_provider.close();
                        instance->m_should_restart_hash_provider = true;

                        if (instance->m_response_streambuf)
                        {
                            instance->m_total_downloaded += instance->m_response_streambuf.total_written();
                        }

                        // Try to recover the request. If it cannot be recovered, it cannot be retried
                        // even if the retry policy allowed for a retry.
                        if (instance->m_command->m_recover_request &&
                            !instance->m_command->m_recover_request(instance->m_total_downloaded, instance->m_context))
                        {
                            if (logger::instance().should_log(instance->m_context, client_log_level::log_level_error))
                            {
                                logger::instance().log(instance->m_context, client_log_level::log_level_error, _XPLATSTR("Cannot recover request for retry, so throwing exception: ") + utility::conversions::to_string_t(e.what()));
                            }

                            throw storage_exception(e.what(), instance->m_request_result, capture_inner_exception(e), false);
                        }

                        if (logger::instance().should_log(instance->m_context, client_log_level::log_level_informational))
                        {
                            utility::string_t str;
                            str.reserve(128);
                            str.append(_XPLATSTR("Retrying failed operation, number of retries: ")).append(utility::conversions::print_string(instance->m_retry_count));
                            logger::instance().log(instance->m_context, client_log_level::log_level_informational, str);
                        }

                        return complete_after(retry.retry_interval()).then([]() -> bool
                        {
                            // Returning true here will tell the outer do_while loop to loop one more time.
                            return true;
                        });
                    }

                    // Returning false here will cause do_while to exit.
                    return pplx::task_from_result<bool>(false);
                });
            }).then([instance](pplx::task<bool> loop_task)
            {
                instance->m_context.set_end_time(utility::datetime::utc_now());
                loop_task.wait();

                if (logger::instance().should_log(instance->m_context, client_log_level::log_level_informational))
                {
                    logger::instance().log(instance->m_context, client_log_level::log_level_informational, _XPLATSTR("Operation completed successfully"));
                }
            });
        }

    private:

        std::chrono::seconds remaining_time() const
        {
            if (m_request_options.operation_expiry_time().is_initialized())
            {
                auto now = utility::datetime::utc_now();
                if (m_request_options.operation_expiry_time().to_interval() > now.to_interval())
                {
                    return std::chrono::seconds(m_request_options.operation_expiry_time() - now);
                }
                else
                {
                    throw storage_exception(protocol::error_client_timeout, false);
                }
            }

            return std::chrono::seconds();
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
