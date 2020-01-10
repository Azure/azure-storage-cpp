// -----------------------------------------------------------------------------------------
// <copyright file="BlobsGettingStarted.cpp" company="Microsoft">
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

#include "samples_common.h"

#include <thread>
#include <chrono>
#include <random>
#include <algorithm>

#include <was/storage_account.h>
#include <was/blob.h>
#include <cpprest/filestream.h>
#include <cpprest/containerstream.h>

namespace azure { namespace storage { namespace samples {

    SAMPLE(BlobsPerformanceBenchmark, blobs_performance_benchmark)
    void blobs_performance_benchmark()
    {
        try
        {
            azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);

            azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();
            azure::storage::cloud_blob_container container = blob_client.get_container_reference(_XPLATSTR("benchmark-container"));

            container.create_if_not_exists();

            const uint64_t blob_size = 1024 * 1024 * 1024;
            const int parallelism = 20;

            std::vector<uint8_t> buffer;
            buffer.resize(blob_size);

            std::mt19937_64 rand_engine(std::random_device{}());
            std::uniform_int_distribution<uint64_t> dist;
            std::generate(reinterpret_cast<uint64_t*>(buffer.data()), reinterpret_cast<uint64_t*>(buffer.data() + buffer.size()), [&dist, &rand_engine]() { return dist(rand_engine); });

            azure::storage::blob_request_options options;
            options.set_parallelism_factor(8);
            options.set_use_transactional_crc64(false);
            options.set_use_transactional_md5(false);
            options.set_store_blob_content_md5(false);
            options.set_stream_write_size_in_bytes(32 * 1024 * 1024);

            std::vector<pplx::task<void>> tasks;

            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < parallelism; ++i)
            {
                auto blob = container.get_block_blob_reference(_XPLATSTR("blob") + utility::conversions::to_string_t(std::to_string(i)));
                auto task = blob.upload_from_stream_async(concurrency::streams::container_buffer<std::vector<uint8_t>>(buffer).create_istream(), blob_size, azure::storage::access_condition(), options, azure::storage::operation_context());
                tasks.emplace_back(std::move(task));
            }
            for (auto& t : tasks)
            {
                t.get();
            }
            auto end = std::chrono::steady_clock::now();

            double elapsed_s = double(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000;
            uint64_t data_mb = blob_size * parallelism / 1024 / 1024;

            std::cout << "Uploaded " << data_mb << "MB in " << elapsed_s << " seconds, throughput " << data_mb / elapsed_s << "MBps" << std::endl;

            tasks.clear();
            start = std::chrono::steady_clock::now();
            {
                std::vector<concurrency::streams::container_buffer<std::vector<uint8_t>>> download_buffers;
                for (int i = 0; i < parallelism; ++i)
                {
                    auto blob = container.get_block_blob_reference(_XPLATSTR("blob") + utility::conversions::to_string_t(std::to_string(i)));
                    download_buffers.emplace_back(concurrency::streams::container_buffer<std::vector<uint8_t>>());
                    auto task = blob.download_to_stream_async(download_buffers.back().create_ostream(), azure::storage::access_condition(), options, azure::storage::operation_context());
                    tasks.emplace_back(std::move(task));
                }
                for (auto& t : tasks)
                {
                    t.get();
                }
                end = std::chrono::steady_clock::now();
            }

            elapsed_s = double(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000;

            std::cout << "Downloaded " << data_mb << "MB in " << elapsed_s << " seconds, throughput " << data_mb / elapsed_s << "MBps" << std::endl;
        }
        catch (const azure::storage::storage_exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

}}}  // namespace azure::storage::samples
