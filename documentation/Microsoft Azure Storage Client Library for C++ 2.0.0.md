# Azure Storage Client Library for C++ 2.0.0: What’s new and sample code

This article introduces the changes in Microsoft Azure Storage Client Library for C++ 2.0.0 and provides some code samples. You can also check the GitHub [readme.md](https://github.com/Azure/azure-storage-cpp/blob/master/README.md) and [changelog.log](https://github.com/Azure/azure-storage-cpp/blob/master/Changelog.txt) files for more details.

## New storage blob type

By default, Azure Storage Client Library for C++ 2.0.0 uses REST API version 2015-2-21. This version of the Azure Storage service includes several important features that you can find at [2015-2 version blog link]. The C++ client library supports one key feature, the new storage blob type called *Append Blob*.

Append Blob is optimized for fast append operations, making it ideal for scenarios where the data must be added to an existing blob without modifying the existing contents of that blob (for example, logging or auditing). For more details, go to [Introducing Azure Storage Append Blob](http://blogs.msdn.com/b/windowsazurestorage/archive/2015/04/13/introducing-azure-storage-append-blob.aspx).

The following sample code shows the use of Append Blob.

	try
	        {
	            // Initialize Storage account
	            azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);
	
	            // Create a blob container
	            azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();
	            azure::storage::cloud_blob_container container = blob_client.get_container_reference(U("my-sample-container"));
	           
	            // Return value is true if the container did not exist and was successfully created
	            container.create_if_not_exists();
	
	            // Create Append Blob
	            azure::storage::cloud_append_blob append_blob = container.get_append_blob_reference(U("my-append-1"));
	            append_blob.properties().set_content_type(U("text/plain; charset=utf-8"));
	            append_blob.create_or_replace();
	
	            // Append blocks in different ways:
	            // 1. Append one block
	            concurrency::streams::istream append_input_stream1 = concurrency::streams::bytestream::open_istream(utility::conversions::to_utf8string(U("block text.")));
	            append_blob.append_block(append_input_stream1, utility::string_t());
	            append_input_stream1.close().wait();
	            // 2. Append from stream
	            concurrency::streams::istream append_input_stream2 = concurrency::streams::bytestream::open_istream(utility::conversions::to_utf8string(U("stream text.")));
	            append_blob.append_from_stream(append_input_stream2);
	            append_input_stream2.close().wait();
	
	            // 3. Append from text
	            append_blob.append_text(U("more text."));
	
	            // Download Append Blob as text
	            utility::string_t append_text = append_blob.download_text();
	            ucout << U("Append Text: ") << append_text << std::endl;
	            
	            // Delete the blob
	            append_blob.delete_blob();
	
	            // Delete the blob container
	            container.delete_container_if_exists();
	        }
	        catch (const azure::storage::storage_exception& e)
	        {
	            ucout << U("Error: ") << e.what() << std::endl;
	
	            azure::storage::request_result result = e.result();
	            azure::storage::storage_extended_error extended_error = result.extended_error();
	            if (!extended_error.message().empty())
	            {
	                ucout << extended_error.message() << std::endl;
	            }
	        }
	        catch (const std::exception& e)
	        {
	            ucout << U("Error: ") << e.what() << std::endl;
	        }

## Range-based for-loop to list objects

For versions earlier than 2.0.0, you can list blobs via the following method.

        // List blobs in the blob container
        azure::storage::list_blob_item_iterator end_of_results;
        for (auto it = container.list_blobs(); it != end_of_results; ++it)
        {
            if (it->is_blob())
            {
                process_blob(it->as_blob());
            }
            else
            {
                process_directory(it->as_directory());
            }
			}

With version 2.0.0, you can also use a range-based for-loop to list blobs.

		// List blobs in the blob container
	     for (auto& item : container.list_blobs())
	     {
	         if (item.is_blob())
	         {
	             process_blob(item.as_blob());
	         }
	         else
	         {
	             process_directory(item.as_directory());
	         }
	     }

For more details about listing APIs of the C++ client library, visit [Efficiently use Listing APIs in Microsoft Azure Storage Client Library for C++](https://azure.microsoft.com/documentation/articles/storage-c-plus-plus-enumeration/).

## Handling query parameters in the resource URI

With versions earlier than 2.0.0, the C++ client library will *keep* only the following parameters and ignore the others when handling the Azure Storage resource URI:

<table>
<tr>
    <td><b>Field Name</b></td>
    <td><b>Query Parameter</b></td>
</tr>
<tr>
    <td>signedversion</td>
    <td>sv</td>
</tr>
<tr>
    <td>signedresource</td>
    <td>sr</td>
</tr>
<tr>
    <td>tablename</td>
    <td>tn</td>
</tr>
<tr>
    <td>signedstart</td>
    <td>st</td>
</tr>
<tr>
    <td>signedexpiry</td>
    <td>se</td>
</tr>
<tr>
    <td>signedpermissions</td>
    <td>sp</td>
</tr>
<tr>
    <td>startpk</td>
    <td>spk</td>
</tr>
<tr>
    <td>startrk</td>
    <td>srk</td>
</tr>
<tr>
    <td>endpk</td>
    <td>epk</td>
</tr>
<tr>
    <td>endrk</td>
    <td>erk</td>
</tr>
<tr>
    <td>signedidentifier</td>
    <td>si</td>
</tr>
<tr>
    <td>Cache-Control</td>
    <td>rscc</td>
</tr>
<tr>
    <td>Content-Disposition</td>
    <td>rscd</td>
</tr>
<tr>
    <td>Content-Encoding</td>
    <td>rsce</td>
</tr>
<tr>
    <td>Content-Language</td>
    <td>rscl</td>
</tr>
<tr>
    <td>Content-Type</td>
    <td>rsct</td>
</tr>
<tr>
    <td>signature</td>
    <td>sig</td>
</tr>
<tr>
    <td>api-version</td>
    <td>api-version</td>
</tr>
</table>

With version 2.0.0, the C++ client library will ignore only the following parameters:

<table>
<tr>
    <td><b>Field Name</b></td>
    <td><b>Query Parameter</b></td>
</tr>
<tr>
    <td>resoucetype</td>
    <td>restype</td>
</tr>
<tr>
    <td>component</td>
    <td>comp</td>
</tr>
<tr>
    <td>snapshot</td>
    <td>snapshot</td>
</tr>
<tr>
    <td>api-version</td>
    <td>api-version</td>
</tr>
</table>

As a result of this change, you can write code like this:

        azure::storage::storage_uri sas_uri(
            web::http::uri(U("https://myaccount.queue.core.windows.net/myqueue?sp=raup&sv=2012-02-12&se=2013-05-14T18%3A23%3A15Z&st=2013-05-14T17%3A23%3A15Z&sig=mysignature")),
            web::http::uri(U("https://myaccount-secondary.queue.core.windows.net/myqueue?sp=raup&sv=2012-02-12&se=2013-05-14T18%3A23%3A15Z&st=2013-05-14T17%3A23%3A15Z&sig=mysignature")));
        azure::storage::cloud_queue queue2(sas_uri);

Note that with this behavior change, the C++ client library will throw an `std::invalid_argument` error if multiple sources identified the same parameter. For example, the C++ client library will report an error if you try to specify *sas_credentials* with `azure::storage::storage_credentials` and the SAS URI at the same time, as shown in the following code.

        utility::string_t sas_token(U("sv=2012-02-12&st=2013-05-14T17%3A23%3A15Z&se=2013-05-14T18%3A23%3A15Z&sp=raup&sig=mysignature"));
        azure::storage::storage_credentials sas_credentials(sas_token);
        azure::storage::storage_uri sas_uri(
            web::http::uri(U("https://myaccount.queue.core.windows.net/myqueue?sp=raup&sv=2012-02-12&se=2013-05-14T18%3A23%3A15Z&st=2013-05-14T17%3A23%3A15Z&sig=mysignature")),
            web::http::uri(U("https://myaccount-secondary.queue.core.windows.net/myqueue?sp=raup&sv=2012-02-12&se=2013-05-14T18%3A23%3A15Z&st=2013-05-14T17%3A23%3A15Z&sig=mysignature")));
        azure::storage::cloud_queue queue2(sas_uri, sas_credentials);

## Renamed interfaces

Azure Storage Client Library for C++ 2.0.0 changes interfaces as follows:
-  Renames `cloud_blob::start_copy_from_blob` to `cloud_blob::start_copy`
-  Renames `cloud_blob::start_copy_from_blob_async` to `cloud_blob::start_copy_async`

## Bug fixes

You can find the bug fixes at [changelog.txt](https://github.com/Azure/azure-storage-cpp/blob/master/Changelog.txt).

## Next steps

For more information about Azure Storage and the client library for C++, see the following resources:
-  [How to use Blob Storage from C++](http://azure.microsoft.com/documentation/articles/storage-c-plus-plus-how-to-use-blobs/)
-  [How to use Table Storage from C++](https://azure.microsoft.com/documentation/articles/storage-c-plus-plus-how-to-use-tables/)
-  [How to use Queue Storage from C++](http://azure.microsoft.com/documentation/articles/storage-c-plus-plus-how-to-use-queues/)
-  [Azure Storage Client Library for C++ API documentation](http://azure.github.io/azure-storage-cpp/)
-  [Azure Storage Team Blog](http://blogs.msdn.com/b/windowsazurestorage/)
-  [Azure Storage documentation](http://azure.microsoft.com/documentation/services/storage/)

