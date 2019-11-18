// -----------------------------------------------------------------------------------------
// <copyright file="executor.cpp" company="Microsoft">
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

#include "wascore/executor.h"

namespace azure { namespace storage { namespace core {
    pplx::task<void> executor_impl::execute_async(std::shared_ptr<storage_command_base> command, const request_options& options, operation_context context)
    {
        if (!context.start_time().is_initialized())
        {
            context.set_start_time(utility::datetime::utc_now());
        }

        // TODO: Use "it" variable name for iterators in for loops
        // TODO: Reduce usage of auto variable types
        auto instance = std::make_shared<executor_impl>(command, options, context);
        return pplx::details::_do_while([instance]() -> pplx::task<bool>
        {
            // Start the timer to track timeout.
            if (instance->m_command->m_use_timeout && !instance->m_command->m_timer_handler->timer_started())
            {
                // Timer will be stopped when instance is out of scope, so no need to stop here.
                instance->m_command->m_timer_handler->start_timer(instance->m_request_options.maximum_execution_time());
            }
            // 0. Begin request 
            instance->assert_canceled();
            instance->validate_location_mode();

            // 1. Build request
            instance->assert_canceled();
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
            instance->assert_canceled();
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
                    if (instance->m_command->m_calculate_response_body_checksum == checksum_type::md5)
                    {
                        instance->m_hash_provider = hash_provider::create_md5_hash_provider();
                    }
                    else if (instance->m_command->m_calculate_response_body_checksum == checksum_type::crc64)
                    {
                        instance->m_hash_provider = hash_provider::create_crc64_hash_provider();
                    }

                    instance->m_total_downloaded = 0;
                    instance->m_is_hashing_started = true;
                    instance->m_should_restart_hash_provider = false;

                    // TODO: Consider using hash_provider::is_enabled instead of m_is_hashing_started to signal when the hash provider has been closed
                }

                if (instance->m_should_restart_hash_provider)
                {
                    if (instance->m_command->m_calculate_response_body_checksum == checksum_type::md5)
                    {
                        instance->m_hash_provider = hash_provider::create_md5_hash_provider();
                    }
                    else if (instance->m_command->m_calculate_response_body_checksum == checksum_type::crc64)
                    {
                        instance->m_hash_provider = hash_provider::create_crc64_hash_provider();
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
            instance->assert_canceled();
            instance->m_command->m_sign_request(instance->m_request, instance->m_context);

            // 4. Set HTTP client configuration
            instance->assert_canceled();
            web::http::client::http_client_config config;

            config.set_proxy(instance->m_context.proxy());

            instance->remaining_time();
            config.set_timeout(instance->m_request_options.noactivity_timeout());

            size_t http_buffer_size = instance->m_request_options.http_buffer_size();
            if (http_buffer_size > 0)
            {
                config.set_chunksize(http_buffer_size);
            }
#ifndef _WIN32
            if (instance->m_context._get_impl()->get_ssl_context_callback() != nullptr)
            {
                config.set_ssl_context_callback(instance->m_context._get_impl()->get_ssl_context_callback());
            }
#endif

            config.set_validate_certificates(instance->m_request_options.validate_certificates());

            // 5-6. Potentially upload data and get response
            instance->assert_canceled();
#ifdef _WIN32
            web::http::client::http_client client(instance->m_request.request_uri().authority(), config);
            return client.request(instance->m_request, instance->m_command->get_cancellation_token()).then([instance](pplx::task<web::http::http_response> get_headers_task)->pplx::task<web::http::http_response>
#else
            std::shared_ptr<web::http::client::http_client> client = core::http_client_reusable::get_http_client(instance->m_request.request_uri().authority(), config);
            return client->request(instance->m_request, instance->m_command->get_cancellation_token()).then([instance](pplx::task<web::http::http_response> get_headers_task)->pplx::task<web::http::http_response>
#endif // _WIN32
            {
                // Headers are ready. It should be noted that http_client will
                // continue to download the response body in parallel.
                web::http::http_response response = get_headers_task.get();

                if (logger::instance().should_log(instance->m_context, client_log_level::log_level_informational))
                {
                    utility::string_t str;
                    str.reserve(128);
                    str.append(_XPLATSTR("Response received. Status code = ")).append(core::convert_to_string(response.status_code())).append(_XPLATSTR(". Reason = ")).append(response.reason_phrase());
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
                    // the response, so re-throwing is the right thing.
                    if (e.what() != NULL && e.what()[0] != '\0')
                    {
                        instance->assert_canceled();
                        throw;
                    }

                    // Otherwise, response body might contain an error coming from the Storage service.

                    return response.content_ready().then([instance](pplx::task<web::http::http_response> get_error_body_task) -> web::http::http_response
                    {
                        auto response = get_error_body_task.get();

                        if (!instance->m_command->m_destination_stream)
                        {
                            // However, if the command has a destination stream, there is no guarantee that it
                            // is seek-able and thus it cannot be read back to parse the error.
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

                        instance->assert_canceled();
                        throw storage_exception(utility::conversions::to_utf8string(response.reason_phrase()));
                    });
                }
            }).then([instance](pplx::task<web::http::http_response> get_body_task) -> pplx::task<void>
            {
                // 9. Evaluate response & parse results
                web::http::http_response response = get_body_task.get();

                if (instance->m_command->m_destination_stream)
                {
                    utility::size64_t current_total_downloaded = instance->m_response_streambuf.total_written();
                    utility::size64_t content_length = instance->m_request_result.content_length();
                    if (content_length != std::numeric_limits<utility::size64_t>::max() && current_total_downloaded != content_length)
                    {
                        // The download was interrupted before it could complete
                        instance->assert_canceled();
                        throw storage_exception(protocol::error_incorrect_length);
                    }
                }

                // It is now time to call m_postprocess_response
                // Finish the checksum hash if checksum was being calculated
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
                // Currently this holds exception pointer to non storage exceptions (exceptions thrown from cpp_rest)
                std::exception_ptr nonstorage_ex_ptr = nullptr;

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
                    catch (...) {
                        nonstorage_ex_ptr = std::current_exception();
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

                    // If operation is canceled.
                    if (instance->m_command->get_cancellation_token().is_canceled())
                    {
                        if (logger::instance().should_log(instance->m_context, client_log_level::log_level_informational))
                        {
                            logger::instance().log(instance->m_context, client_log_level::log_level_informational, _XPLATSTR("Exception thrown while operation canceled: ") + utility::conversions::to_string_t(e.what()));
                        }

                        // deal with canceling situation if the exception is protocol::error_operation_canceled, while throwing exception that is already thrown before canceling.
                        if (std::string(e.what()) == protocol::error_operation_canceled)
                        {
                            instance->assert_canceled();
                        }
                        throw storage_exception(e.what(), instance->m_request_result, capture_inner_exception(e), false);
                    }

                    // An exception occurred and thus the request might be retried. Ask the retry policy.
                    retry_context context(instance->m_retry_count++, instance->m_request_result, instance->get_next_location(), instance->m_current_location_mode, nonstorage_ex_ptr);
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
                        str.append(_XPLATSTR("Retrying failed operation, number of retries: ")).append(core::convert_to_string(instance->m_retry_count));
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

}}} // namespace azure::storage::core
