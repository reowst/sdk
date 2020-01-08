/*
    Network Next SDK 3.3.4

    Copyright © 2017 - 2020 Network Next, Inc.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following 
    conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
       and the following disclaimer in the documentation and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote 
       products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NEXT_H
#define NEXT_H

#define __STDC_FORMAT_MACROS

#include <stdint.h>
#include <stddef.h>

#define NEXT_VERSION_FULL                                   "3.3.4"
#define NEXT_VERSION_MAJOR                                      "3"
#define NEXT_VERSION_MINOR                                      "3"
#define NEXT_VERSION_PATCH                                      "4"
#define NEXT_VERSION_GITHUB                             "9afdbd27d"

#define NEXT_OK                                                   0
#define NEXT_ERROR                                               -1

#define NEXT_MTU                                               1300
#define NEXT_HEADER_BYTES                                        35

#define NEXT_LOG_LEVEL_NONE                                       0
#define NEXT_LOG_LEVEL_ERROR                                      1
#define NEXT_LOG_LEVEL_INFO                                       2
#define NEXT_LOG_LEVEL_WARN                                       3
#define NEXT_LOG_LEVEL_DEBUG                                      4

#define NEXT_ADDRESS_NONE                                         0
#define NEXT_ADDRESS_IPV4                                         1
#define NEXT_ADDRESS_IPV6                                         2

#define NEXT_MAX_ADDRESS_STRING_LENGTH                          256

#define NEXT_CONNECTION_TYPE_UNKNOWN                              0
#define NEXT_CONNECTION_TYPE_WIRED                                1
#define NEXT_CONNECTION_TYPE_WIFI                                 2
#define NEXT_CONNECTION_TYPE_CELLULAR                             3

#define NEXT_PLATFORM_WINDOWS                                     1
#define NEXT_PLATFORM_MAC                                         2
#define NEXT_PLATFORM_LINUX                                       3
#define NEXT_PLATFORM_SWITCH                                      4
#define NEXT_PLATFORM_PS4                                         5
#define NEXT_PLATFORM_IOS                                         6
#define NEXT_PLATFORM_XBOX_ONE                                    7

#define NEXT_PLATFORM_STEAM                                  (1<<9)
#define NEXT_PLATFORM_EGS                                   (1<<10)

#define NEXT_FLAGS_BAD_ROUTE_TOKEN                           (1<<0)
#define NEXT_FLAGS_NO_ROUTE_TO_CONTINUE                      (1<<1)
#define NEXT_FLAGS_PREVIOUS_UPDATE_STILL_PENDING             (1<<2)
#define NEXT_FLAGS_BAD_CONTINUE_TOKEN                        (1<<3)
#define NEXT_FLAGS_ROUTE_EXPIRED                             (1<<4)
#define NEXT_FLAGS_ROUTE_REQUEST_TIMED_OUT                   (1<<5)
#define NEXT_FLAGS_CONTINUE_REQUEST_TIMED_OUT                (1<<6)
#define NEXT_FLAGS_CLIENT_TIMED_OUT                          (1<<7)
#define NEXT_FLAGS_TRY_BEFORE_YOU_BUY_ABORT                  (1<<8)
#define NEXT_FLAGS_DIRECT_ROUTE_EXPIRED                      (1<<9)
#define NEXT_FLAGS_COUNT                                         10

#if defined(_WIN32)
#define NOMINMAX
#endif

#if defined( NEXT_SHARED )
    #if defined( _WIN32 ) || defined( __ORBIS__ )
        #ifdef NEXT_EXPORT
            #define NEXT_EXPORT_FUNC extern "C" __declspec(dllexport)
        #else
            #define NEXT_EXPORT_FUNC extern "C" __declspec(dllimport)
        #endif
    #else
        #define NEXT_EXPORT_FUNC extern "C"
    #endif
#else
    #define NEXT_EXPORT_FUNC extern
#endif

#if defined(NN_NINTENDO_SDK)
    #define NEXT_PLATFORM NEXT_PLATFORM_SWITCH
#elif defined(__ORBIS__)
    #define NEXT_PLATFORM NEXT_PLATFORM_PS4
#elif defined(_XBOX_ONE)
    #define NEXT_PLATFORM NEXT_PLATFORM_XBOX_ONE
#elif defined(_WIN32)
    #define NEXT_PLATFORM NEXT_PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
        #define NEXT_PLATFORM NEXT_PLATFORM_IOS
    #else
        #define NEXT_PLATFORM NEXT_PLATFORM_MAC
    #endif
#else
    #define NEXT_PLATFORM NEXT_PLATFORM_LINUX
#endif

#if defined( _MSC_VER )
#define NEXT_PACK_PUSH() __pragma( pack( push, 8 ) )
#define NEXT_PACK_POP() __pragma( pack( pop ) )
#elif defined( __llvm__ ) || defined( __clang__ ) || defined( __GNUC__ )
#define NEXT_PACK_PUSH() _Pragma( "pack( push, 8 )" )
#define NEXT_PACK_POP() _Pragma( "pack( pop )" )
#else
#error unsupported compiler
#endif

NEXT_PACK_PUSH()

// -----------------------------------------

struct next_config_t
{
    char customer_public_key[256];
    char customer_private_key[256];
    int socket_send_buffer_size;
    int socket_receive_buffer_size;
    bool disable_network_next;
    bool try_before_you_buy;
    bool high_priority_server_thread;
};

NEXT_EXPORT_FUNC void next_default_config( next_config_t * config );

NEXT_EXPORT_FUNC int next_init( void * context, next_config_t * config );

NEXT_EXPORT_FUNC void next_term();

// -----------------------------------------

NEXT_EXPORT_FUNC double next_time();

NEXT_EXPORT_FUNC void next_sleep( double time_seconds );

NEXT_EXPORT_FUNC void next_printf( int level, const char * format, ... );

extern void (*next_assert_function_pointer)( const char * condition, const char * function, const char * file, int line );

#ifndef NDEBUG
#define next_assert( condition )                                                            \
do                                                                                          \
{                                                                                           \
    if ( !(condition) )                                                                     \
    {                                                                                       \
        next_assert_function_pointer( #condition, __FUNCTION__, __FILE__, __LINE__ );       \
    }                                                                                       \
} while(0)
#else
#define next_assert( ignore ) ((void)0)
#endif

NEXT_EXPORT_FUNC void next_log_level( int level );

NEXT_EXPORT_FUNC void next_log_function( void (*function)( int level, const char * format, ... ) );

NEXT_EXPORT_FUNC void next_assert_function( void (*function)( const char * condition, const char * function, const char * file, int line ) );

NEXT_EXPORT_FUNC void next_allocator( void * (*malloc_function)( void * context, size_t bytes ), void (*free_function)( void * context, void * p ) );

NEXT_EXPORT_FUNC const char * next_user_id_string( uint64_t user_id, char * buffer );

// -----------------------------------------

struct next_address_t
{
    union { uint8_t ipv4[4]; uint16_t ipv6[8]; } data;
    uint16_t port;
    uint8_t type;
};

NEXT_EXPORT_FUNC int next_address_parse( next_address_t * address, const char * address_string_in );

NEXT_EXPORT_FUNC const char * next_address_to_string( const next_address_t * address, char * buffer );

NEXT_EXPORT_FUNC int next_address_equal( const next_address_t * a, const next_address_t * b );

// -----------------------------------------

struct next_client_stats_t
{
    uint64_t flags;
    uint64_t platform_id;
    int connection_type;
    bool try_before_you_buy;
    bool multipath;
    bool flagged;
    bool next;
    float next_min_rtt;
    float next_max_rtt;
    float next_mean_rtt;
    float next_jitter;
    float next_packet_loss;
    float direct_min_rtt;
    float direct_max_rtt;
    float direct_mean_rtt;
    float direct_jitter;
    float direct_packet_loss;
    float kbps_up;
    float kbps_down;
    uint64_t packets_lost_client_to_server;
    uint64_t packets_lost_server_to_client;
};

// -----------------------------------------

struct next_client_t;

NEXT_EXPORT_FUNC next_client_t * next_client_create( void * context, void (*packet_received_callback)( next_client_t * client, void * context, const uint8_t * packet_data, int packet_bytes ) );

NEXT_EXPORT_FUNC void next_client_open_session( next_client_t * client, const char * server_address );

NEXT_EXPORT_FUNC void next_client_close_session( next_client_t * client );

NEXT_EXPORT_FUNC void next_client_update( next_client_t * client );

NEXT_EXPORT_FUNC void next_client_send_packet( next_client_t * client, const uint8_t * packet_data, int packet_bytes );

NEXT_EXPORT_FUNC void next_client_send_packet_direct( next_client_t * client, const uint8_t * packet_data, int packet_bytes );

NEXT_EXPORT_FUNC void next_client_flag_session( next_client_t * client );

NEXT_EXPORT_FUNC uint64_t next_client_session_id( next_client_t * client );

NEXT_EXPORT_FUNC const next_client_stats_t * next_client_stats( next_client_t * client );

NEXT_EXPORT_FUNC void next_client_destroy( next_client_t * client );

// -----------------------------------------

struct next_server_t;

NEXT_EXPORT_FUNC next_server_t * next_server_create( void * context, const char * server_address, const char * bind_address, const char * datacenter, void (*packet_received_callback)( next_server_t * server, void * context, const next_address_t * from, const uint8_t * packet_data, int packet_bytes ) );

NEXT_EXPORT_FUNC uint16_t next_server_port( next_server_t * server );

NEXT_EXPORT_FUNC void next_server_update( next_server_t * server );

NEXT_EXPORT_FUNC uint64_t next_server_upgrade_session( next_server_t * server, const next_address_t * address, const char * user_id, uint32_t platform_id, const char * tag );

NEXT_EXPORT_FUNC void next_server_tag_session( next_server_t * server, const next_address_t * address, const char * tag );

NEXT_EXPORT_FUNC int next_server_session_upgraded( next_server_t * server, const next_address_t * address );

NEXT_EXPORT_FUNC void next_server_send_packet( next_server_t * server, const next_address_t * to_address, const uint8_t * packet_data, int packet_bytes );

NEXT_EXPORT_FUNC void next_server_send_packet_direct( next_server_t * server, const next_address_t * to_address, const uint8_t * packet_data, int packet_bytes );

NEXT_EXPORT_FUNC void next_server_destroy( next_server_t * server );

// -----------------------------------------

NEXT_EXPORT_FUNC void next_test();

NEXT_PACK_POP()

#endif // #ifndef NEXT_H
