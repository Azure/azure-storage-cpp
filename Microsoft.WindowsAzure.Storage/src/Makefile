LIBRARY=azurestorage.so

SOURCES = \
	xmlhelpers.cpp \
	util_windows.cpp \
	response_parsers.cpp \
	request_result.cpp \
	request_factory.cpp \
	protocol_xml.cpp \
	navigation.cpp \
	async_semaphore.cpp \
	util.cpp \
	table_request_factory.cpp \
	table_response_parsers.cpp \
	table_query.cpp \
	entity_property.cpp \
	shared_access_signature.cpp \
	retry_policies.cpp \
	queue_request_factory.cpp \
	protocol_json.cpp \
	operation_context.cpp \
	mime_multipart_helper.cpp \
	logging_windows.cpp \
	hash_windows.cpp \
	cloud_table_client.cpp \
	cloud_table.cpp \
	cloud_storage_account.cpp \
	cloud_queue_message.cpp \
	cloud_queue_client.cpp \
	cloud_queue.cpp \
	cloud_page_blob.cpp \
	cloud_core.cpp \
	cloud_client.cpp \
	cloud_block_blob.cpp \
	cloud_blob_directory.cpp \
	cloud_blob_container.cpp \
	cloud_blob_shared.cpp \
	cloud_blob_ostreambuf.cpp \
	cloud_blob_istreambuf.cpp \
	cloud_blob_client.cpp \
	cloud_blob.cpp \
	blob_response_parsers.cpp \
	blob_request_factory.cpp \
	basic_types.cpp \
	authentication.cpp \
	cloud_common.cpp

ifeq ($(UNAME),APPLE)
	# SOURCES += apple.cpp
else
	# SOURCES += linux.cpp
endif


HEADERS = \
	../includes/wascore/async_semaphore.h \
	../includes/wascore/basic_types.h \
	../includes/wascore/blobstreams.h \
	../includes/wascore/constants.h \
	../includes/wascore/executor.h \
	../includes/wascore/hash_linux.h \
	../includes/wascore/logging.h \
	../includes/wascore/protocol_json.h \
	../includes/wascore/protocol_xml.h \
	../includes/wascore/protocol.h \
	../includes/wascore/resources.h \
	../includes/wascore/streambuf.h \
	../includes/wascore/streams.h \
	../includes/wascore/util.h \
	../includes/wascore/xmlhelpers.h \
	../includes/wascore/xmlstream.h \
	../includes/was/auth.h \
	../includes/was/blob.h \
	../includes/was/common.h \
	../includes/was/core.h \
	../includes/was/queue.h \
	../includes/was/retry_policies.h \
	../includes/was/service_client.h \
	../includes/was/storage_account.h \
	../includes/was/table.h \
	../includes/targetver.h \
	../includes/stdafx.h

ifeq ($(UNAME),APPLE)
	# HEADERS += ../include/compat/apple_compat.h
else
	# HEADERS += ../include/compat/linux_compat.h
endif

PCH = ../includes/stdafx.h

PRECOMPILED_HEADERS = $(addsuffix .gch, $(PCH))
OBJECTS = $(addsuffix .$(MODE).o, $(basename $(SOURCES)))
CPP_DEPENDENCIES = $(addsuffix .d, $(basename $(SOURCES)))

# flags for the C++ compiler

ifeq ($(UNAME),LINUX)
PKGCONFIG_CFLAGS = $(shell pkg-config libxml++-2.6 --cflags)
PKGCONFIG_LIBS = $(shell pkg-config libxml++-2.6 --libs) $(shell pkg-config libssl --libs)
endif

ifeq ($(UNAME),APPLE)
PLATFORM_FLAGS = -framework CoreFoundation
endif
ifeq ($(UNAME),LINUX)
PLATFORM_FLAGS = -pthread
endif

#-fpch-deps -MMD not useful with a single monolithic PCH

CXXFLAGS = -fPIC -std=c++11 $(STRICT_BASE_CXXFLAGS) -I../includes -I./pch $(WARNINGS) $(PKGCONFIG_CFLAGS)
LIBS = $(PKGCONFIG_LIBS) $(PLATFORM_FLAGS) -l$(BOOST_SYSTEM) -l$(BOOST_THREAD) -l$(BOOST_LOCALE) -l$(BOOST_REGEX) -lstdc++ -lm # these are explicit for clang
LDFLAGS = $(BASE_LDFLAGS)

ifeq ($(OUTPUT_DIR),)
  LIBRARY_DST=$(MODE)/$(LIBRARY)
else
  LIBRARY_DST=$(OUTPUT_DIR)/$(LIBRARY)
endif

all:  $(LIBRARY_DST)

%.h.gch: %.h
	@echo "Building precompiled header $(@)"
	@$(CXX) -x c++-header $(CXXFLAGS) -c $< -o $@

-include $(CPP_DEPENDENCIES)

%.$(MODE).o: %.cpp
	@echo "Compiling $< -> $@"
	@$(CXX) -MMD -MF $*.d $(CXXFLAGS) -c $< -o $@

$(PRECOMPILED_HEADERS): $(HEADERS)

$(LIBRARY_DST): $(OBJECTS)
	@echo "Building library $@"
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $^ -o $@ $(LIBS)

#DEPS := $(OBJECTS:.o=.d)
#-include $(DEPS)

clean:
	rm $(OBJECTS) $(PRECOMPILED_HEADERS) $(LIBRARY) $(CPP_DEPENDENCIES)

.SILENT:clean