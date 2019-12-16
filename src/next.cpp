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

#include "next.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <inttypes.h>

#define NEXT_HOSTNAME                                "v3.networknext.com"
#define NEXT_PORT                                                 "40000"
#define NEXT_MAX_PACKET_BYTES                                        1500
#define NEXT_ADDRESS_BYTES                                             19
#define NEXT_ADDRESS_BUFFER_SAFETY                                     32
#define NEXT_DEFAULT_SOCKET_SEND_BUFFER_SIZE                      1000000
#define NEXT_DEFAULT_SOCKET_RECEIVE_BUFFER_SIZE                   1000000
#define NEXT_REPLAY_PROTECTION_BUFFER_SIZE                            256
#define NEXT_PING_HISTORY_ENTRY_COUNT                                 256
#define NEXT_CLIENT_STATS_WINDOW                                     10.0
#define NEXT_PING_SAFETY                                              1.0
#define NEXT_UPGRADE_TIMEOUT                                         10.0
#define NEXT_SESSION_TIMEOUT                                         10.0
#define NEXT_INITIAL_PENDING_SESSION_SIZE                              64
#define NEXT_INITIAL_SESSION_SIZE                                      64
#define NEXT_PINGS_PER_SECOND                                          10
#define NEXT_DIRECT_PINGS_PER_SECOND                                   10
#define NEXT_COMMAND_QUEUE_LENGTH                                    1024
#define NEXT_NOTIFY_QUEUE_LENGTH                                     1024
#define NEXT_CLIENT_STATS_UPDATES_PER_SECOND                           10
#define NEXT_SECONDS_BETWEEN_SERVER_UPDATES                            10
#define NEXT_SECONDS_BETWEEN_SESSION_UPDATES                           10
#define NEXT_UPGRADE_TOKEN_BYTES                                      128
#define NEXT_MAX_NEAR_RELAYS                                           32
#define NEXT_RELAY_PING_TIME                                          1.0
#define NEXT_ROUTE_TOKEN_BYTES                                         77
#define NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES                              117
#define NEXT_CONTINUE_TOKEN_BYTES                                      18
#define NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES                            58
#define NEXT_UPDATE_TYPE_DIRECT                                         0
#define NEXT_UPDATE_TYPE_ROUTE                                          1
#define NEXT_UPDATE_TYPE_CONTINUE                                       2
#define NEXT_MAX_TOKENS                                                 7
#define NEXT_UPDATE_SEND_TIME                                        0.25
#define NEXT_ROUTE_REQUEST_SEND_TIME                                 0.25
#define NEXT_CONTINUE_REQUEST_SEND_TIME                              0.25
#define NEXT_SLICE_SECONDS                                             10
#define NEXT_ROUTE_REQUEST_TIMEOUT                                      5
#define NEXT_CONTINUE_REQUEST_TIMEOUT                                   5
#define NEXT_SESSION_UPDATE_RESEND_TIME                               1.0
#define NEXT_SESSION_UPDATE_TIMEOUT                                     5
#define NEXT_VERSION_MAJOR_MAX                                        254
#define NEXT_VERSION_MINOR_MAX                                       1023
#define NEXT_VERSION_PATCH_MAX                                        254
#define NEXT_BANDWIDTH_LIMITER_INTERVAL                               1.0
#define NEXT_TRY_BEFORE_YOU_BUY_MIN_EVAL_TIME                         5.0
#define NEXT_TRY_BEFORE_YOU_BUY_ABORT_TIME                           25.0
#define NEXT_DIRECT_ROUTE_EXPIRE_TIME                                30.0

#ifdef NEXT_VERSION_IS_PRESENT
#define NEXT_VERSION_MAJOR_INT                      3
#define NEXT_VERSION_MINOR_INT                      3
#define NEXT_VERSION_PATCH_INT                      4
#else
#define NEXT_VERSION_MAJOR_INT                                          0
#define NEXT_VERSION_MINOR_INT                                          0
#define NEXT_VERSION_PATCH_INT                                          0
#endif

#define NEXT_STATS_ENABLED                                              0

#define NEXT_CLIENT_COUNTER_OPEN_SESSION                                0
#define NEXT_CLIENT_COUNTER_CLOSE_SESSION                               1
#define NEXT_CLIENT_COUNTER_UPGRADE_SESSION                             2
#define NEXT_CLIENT_COUNTER_FALLBACK_TO_DIRECT                          3
#define NEXT_CLIENT_COUNTER_PACKET_SENT_DIRECT                          4
#define NEXT_CLIENT_COUNTER_PACKET_RECEIVED_DIRECT                      5
#define NEXT_CLIENT_COUNTER_PACKET_SENT_NEXT                            6
#define NEXT_CLIENT_COUNTER_PACKET_RECEIVED_NEXT                        7
#define NEXT_CLIENT_COUNTER_TRY_BEFORE_YOU_BUY_COMPLETED                8
#define NEXT_CLIENT_COUNTER_TRY_BEFORE_YOU_BUY_ABORT                    9
#define NEXT_CLIENT_COUNTER_MULTIPATH                                  10
#define NEXT_CLIENT_COUNTER_PACKETS_LOST_CLIENT_TO_SERVER              11
#define NEXT_CLIENT_COUNTER_PACKETS_LOST_SERVER_TO_CLIENT              12

#define NEXT_CLIENT_COUNTER_MAX                                        64

#define NEXT_PACKET_LOSS_TRACKER_HISTORY                             1024
#define NEXT_PACKET_LOSS_TRACKER_SAFETY                                30
#define NEXT_SECONDS_BETWEEN_PACKET_LOSS_UPDATES                      1.0

static const uint8_t next_backend_public_key[] = 
{ 
     76,  97, 202, 140,  71, 135,  62, 212, 
    160, 181, 151, 195, 202, 224, 207, 113, 
      8,  45,  37,  60, 145,  14, 212, 111, 
     25,  34, 175, 186,  37, 150, 163,  64 
};

static const uint8_t next_router_public_key[] = 
{ 
    0x49, 0x2e, 0x79, 0x74, 0x49, 0x7d, 0x9d, 0x34, 
    0xa7, 0x55, 0x50, 0xeb, 0xab, 0x03, 0xde, 0xa9, 
    0x1b, 0xff, 0x61, 0xc6, 0x0e, 0x65, 0x92, 0xd7, 
    0x09, 0x64, 0xe9, 0x34, 0x12, 0x32, 0x5f, 0x46 
};

#if !defined (NEXT_LITTLE_ENDIAN ) && !defined( NEXT_BIG_ENDIAN )

  #ifdef __BYTE_ORDER__
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      #define NEXT_LITTLE_ENDIAN 1
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      #define NEXT_BIG_ENDIAN 1
    #else
      #error Unknown machine endianess detected. Please define NEXT_LITTLE_ENDIAN or NEXT_BIG_ENDIAN. 
    #endif // __BYTE_ORDER__
  
  // Detect with GLIBC's endian.h
  #elif defined(__GLIBC__)
    #include <endian.h>
    #if (__BYTE_ORDER == __LITTLE_ENDIAN)
      #define NEXT_LITTLE_ENDIAN 1
    #elif (__BYTE_ORDER == __BIG_ENDIAN)
      #define NEXT_BIG_ENDIAN 1
    #else
      #error Unknown machine endianess detected. Please define NEXT_LITTLE_ENDIAN or NEXT_BIG_ENDIAN. 
    #endif // __BYTE_ORDER
  
  // Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro
  #elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
    #define NEXT_LITTLE_ENDIAN 1
  #elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
    #define NEXT_BIG_ENDIAN 1
  
  // Detect with architecture macros
  #elif    defined(__sparc)     || defined(__sparc__)                           \
        || defined(_POWER)      || defined(__powerpc__)                         \
        || defined(__ppc__)     || defined(__hpux)      || defined(__hppa)      \
        || defined(_MIPSEB)     || defined(_POWER)      || defined(__s390__)
    #define NEXT_BIG_ENDIAN 1
  #elif    defined(__i386__)    || defined(__alpha__)   || defined(__ia64)      \
        || defined(__ia64__)    || defined(_M_IX86)     || defined(_M_IA64)     \
        || defined(_M_ALPHA)    || defined(__amd64)     || defined(__amd64__)   \
        || defined(_M_AMD64)    || defined(__x86_64)    || defined(__x86_64__)  \
        || defined(_M_X64)      || defined(__bfin__)
    #define NEXT_LITTLE_ENDIAN 1
  #elif defined(_MSC_VER) && defined(_M_ARM)
    #define NEXT_LITTLE_ENDIAN 1
  #else
    #error Unknown machine endianess detected. Please define NEXT_LITTLE_ENDIAN or NEXT_BIG_ENDIAN. 
  #endif

#endif

#if defined( _MSC_VER ) && _MSC_VER < 1700
typedef __int32 int32_t;
typedef __int64 int64_t;
#define PRId64 "I64d"
#define SCNd64 "I64d"
#define PRIx64 "I64x"
#define SCNx64 "I64x"
#else
#include <inttypes.h>
#endif

#if defined( _MSC_VER )
#pragma warning(disable:4127)
#pragma warning(disable:4244)
#endif

static void default_assert_function( const char * condition, const char * function, const char * file, int line )
{
    printf( "assert failed: ( %s ), function %s, file %s, line %d\n", condition, function, file, line );
    fflush( stdout );
    #if defined(_MSC_VER)
        __debugbreak();
    #elif defined(__ORBIS__)
        __builtin_trap();
    #elif defined(__clang__)
        __builtin_debugtrap();
    #elif defined(__GNUC__)
        __builtin_trap();
    #elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__APPLE__)
        raise(SIGTRAP);
    #else
        #error "asserts not supported on this platform!"
    #endif
}

#if defined( _MSC_VER ) && _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}

#endif

uint16_t next_ntohs( uint16_t in )
{
    return (uint16_t)( ( ( in << 8 ) & 0xFF00 ) | ( ( in >> 8 ) & 0x00FF ) );
}

uint16_t next_htons( uint16_t in )
{
    return (uint16_t)( ( ( in << 8 ) & 0xFF00 ) | ( ( in >> 8 ) & 0x00FF ) );
}

#if NEXT_PLATFORM == NEXT_PLATFORM_WINDOWS
#include "next_windows.h"
#elif NEXT_PLATFORM == NEXT_PLATFORM_MAC
#include "next_mac.h"
#elif NEXT_PLATFORM == NEXT_PLATFORM_LINUX
#include "next_linux.h"
#elif NEXT_PLATFORM == NEXT_PLATFORM_SWITCH
#include "next_switch.h"
#elif NEXT_PLATFORM == NEXT_PLATFORM_PS4
#include "next_ps4.h"
#elif NEXT_PLATFORM == NEXT_PLATFORM_IOS 
#include "next_ios.h"
#elif NEXT_PLATFORM == NEXT_PLATFORM_XBOX_ONE
#include "next_xboxone.h"
#endif

NEXT_PACK_PUSH()

extern int next_platform_init();

extern void next_platform_term();

extern const char * next_platform_getenv( const char * );

extern double next_platform_time();

extern void next_platform_sleep( double seconds );

extern int next_platform_inet_pton4( const char * address_string, uint32_t * address_out );

extern int next_platform_inet_pton6( const char * address_string, uint16_t * address_out );

extern int next_platform_inet_ntop6( const uint16_t * address, char * address_string, size_t address_string_size );

extern next_platform_socket_t * next_platform_socket_create( void * context, next_address_t * address, int socket_type, float timeout_seconds, int send_buffer_size, int receive_buffer_size );

extern void next_platform_socket_destroy( next_platform_socket_t * socket );

extern void next_platform_socket_send_packet( next_platform_socket_t * socket, const next_address_t * to, const void * packet_data, int packet_bytes );

extern int next_platform_socket_receive_packet( next_platform_socket_t * socket, next_address_t * from, void * packet_data, int max_packet_size );

extern int next_platform_id();

extern int next_platform_connection_type();

extern int next_platform_hostname_resolve( const char * hostname, const char * port, next_address_t * address );

extern next_platform_thread_t * next_platform_thread_create( void * context, next_platform_thread_func_t * func, void * arg );

extern void next_platform_thread_join( next_platform_thread_t * thread );

extern void next_platform_thread_destroy( next_platform_thread_t * thread );

extern void next_platform_thread_set_sched_max( next_platform_thread_t * thread );

extern next_platform_mutex_t * next_platform_mutex_create( void * context );

extern void next_platform_mutex_acquire( next_platform_mutex_t * mutex );

extern void next_platform_mutex_release( next_platform_mutex_t * mutex );

extern void next_platform_mutex_destroy( next_platform_mutex_t * mutex );

struct next_mutex_helper_t
{
    next_platform_mutex_t * mutex;
    next_mutex_helper_t( next_platform_mutex_t * mutex );
    ~next_mutex_helper_t();
};

#define next_mutex_guard( _mutex ) next_mutex_helper_t __mutex_helper( _mutex )

next_mutex_helper_t::next_mutex_helper_t( next_platform_mutex_t * mutex ) : mutex( mutex )
{
    next_assert( mutex );
    next_platform_mutex_acquire( mutex );
}

next_mutex_helper_t::~next_mutex_helper_t()
{
    next_assert( mutex );
    next_platform_mutex_release( mutex );
    mutex = NULL;
}

// -------------------------------------------------------------

bool next_platform_getenv_bool(const char *name)
{
    const char *v = next_platform_getenv(name);
    if (v != NULL && v[0]) {
        return v[0] == '1' || v[0] == 't' || v[0] == 'T';
    }
    return false;
}

double next_time()
{
    return next_platform_time();
}

void next_sleep( double time_seconds )
{
    next_platform_sleep( time_seconds );
}

static int log_level = NEXT_LOG_LEVEL_INFO;

void next_log_level( int level )
{
    log_level = level;
}

void (*next_assert_function_pointer)( const char * condition, const char * function, const char * file, int line ) = default_assert_function;

void next_assert_function( void (*function)( const char * condition, const char * function, const char * file, int line ) )
{
    next_assert_function_pointer = function;
}

const char * next_log_level_string( int level )
{
    if ( level == NEXT_LOG_LEVEL_DEBUG )
        return "debug";
    else if ( level == NEXT_LOG_LEVEL_INFO )
        return "info";
    else if ( level == NEXT_LOG_LEVEL_ERROR )
        return "error";
    else if ( level == NEXT_LOG_LEVEL_WARN )
        return "warning";
    else
        return "???";
}

static void default_log_function( int level, const char * format, ... ) 
{
    va_list args;
    va_start( args, format );
    char buffer[1024];
    vsnprintf( buffer, sizeof( buffer ), format, args );
    const char * level_string = next_log_level_string( level );
    printf( "%.6f: %s: %s\n", next_time(), level_string, buffer );
    va_end( args );
    fflush( stdout );
}

static void (*log_function)( int level, const char * format, ... ) = default_log_function;

void next_log_function( void (*function)( int level, const char * format, ... ) )
{
    log_function = function;
}

static void * next_default_malloc_function( void * context, size_t bytes )
{
    (void) context;
    return malloc( bytes );
}

static void next_default_free_function( void * context, void * p )
{
    (void) context;
    free( p );
}

static void * (*next_malloc_function)( void * context, size_t bytes ) = next_default_malloc_function;
static void (*next_free_function)( void * context, void * p ) = next_default_free_function;

void next_allocator( void * (*malloc_function)( void * context, size_t bytes ), void (*free_function)( void * context, void * p ) )
{
    next_assert( malloc_function );
    next_assert( free_function );
    next_malloc_function = malloc_function;
    next_free_function = free_function;
}

void * next_malloc( void * context, size_t bytes )
{
    next_assert( next_malloc_function );
    return next_malloc_function( context, bytes );
}

void next_free( void * context, void * p )
{
    next_assert( next_free_function );
    return next_free_function( context, p );
}

__inline void clear_and_free( void * context, void * p, size_t p_size )
{
    memset( p, 0, p_size );
    next_free( context, p );
}

void next_printf( int level, const char * format, ... ) 
{
    if ( level > log_level )
        return;
    va_list args;
    va_start( args, format );
    char buffer[1024];
    vsnprintf( buffer, sizeof( buffer ), format, args );
    log_function( level, "%s", buffer );
    va_end( args );
}

void next_print_bytes( const char * label, const uint8_t * data, int data_bytes )
{
    printf( "%s: ", label );
    for ( int i = 0; i < data_bytes; ++i )
    {
        if ( i != data_bytes - 1 )
        {
            printf( "0x%02x,", (int) data[i] );
        }
        else
        {
            printf( "0x%02x", (int) data[i] );
        }
    }
    printf( " (%d bytes)\n", data_bytes );
}

const char * next_user_id_string( uint64_t user_id, char * buffer )
{
    sprintf( buffer, "%" PRIx64, user_id );
    return buffer;
}

// -------------------------------------------------------------

#if NEXT_STATS_ENABLED

struct next_stats_t
{
    void * context;
    uint64_t session_id;
    next_platform_mutex_t * mutex;
    FILE * file;
};

void next_stats_destroy( next_stats_t * stats );

next_stats_t * next_stats_create( void * context )
{
    next_stats_t * stats = (next_stats_t*) next_malloc( context, sizeof( next_stats_t ) );
    memset( stats, 0, sizeof(next_stats_t) );
    stats->mutex = next_platform_mutex_create( context );
    if ( !stats->mutex )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "failed to create stats mutex" );
    }
    return stats;
}

void next_stats_destroy( next_stats_t * stats )
{
    next_assert( stats );
    if ( stats->mutex )
    {
        next_platform_mutex_destroy( stats->mutex );
    }
    memset( stats, 0, sizeof(next_stats_t) );
    next_free( stats->context, stats );
}

void next_stats_session_changed( next_stats_t * stats, uint64_t session_id )
{
    if ( stats == NULL )
        return;

    next_platform_mutex_acquire( stats->mutex );
    if ( session_id != stats->session_id )
    {
        if ( session_id != 0 )
        {
            // open file
            char filename[256];
            sprintf( filename, "%" PRIx64 ".log", session_id );
            stats->file = fopen( filename, "w" );
        }
        else
        {
            // close file
            if ( stats->file )
            {
                fclose( stats->file );
                stats->file = NULL;
            }
        }
    }
    next_platform_mutex_release( stats->mutex );
}

static void next_stats( next_stats_t * stats, const char * format, ... ) 
{
    if ( stats == NULL )
        return;

    va_list args;
    va_start( args, format );
    char buffer[1024];
    vsnprintf( buffer, sizeof( buffer ), format, args );
    va_end( args );

    next_platform_mutex_acquire( stats->mutex );
    if ( stats->file )
    {
        fprintf( stats->file, "%.6f: %s\n", next_time(), buffer );
    }
    next_platform_mutex_release( stats->mutex );
}

#else // #ifndef NEXT_STATS_ENABLED

struct next_stats_t {};

void next_stats_session_changed( next_stats_t * stats, uint64_t session_id )
{
    (void) stats;
    (void) session_id;
}

static void next_stats( next_stats_t * stats, const char * format, ... ) 
{
    (void) stats;
    (void) format;
}

#endif // #ifndef NEXT_STATS_ENABLED

// -------------------------------------------------------------

void next_write_uint8( uint8_t ** p, uint8_t value )
{
    **p = value;
    ++(*p);
}

void next_write_uint16( uint8_t ** p, uint16_t value )
{
    (*p)[0] = value & 0xFF;
    (*p)[1] = value >> 8;
    *p += 2;
}

void next_write_uint32( uint8_t ** p, uint32_t value )
{
    (*p)[0] = value & 0xFF;
    (*p)[1] = ( value >> 8  ) & 0xFF;
    (*p)[2] = ( value >> 16 ) & 0xFF;
    (*p)[3] = value >> 24;
    *p += 4;
}

void next_write_uint64( uint8_t ** p, uint64_t value )
{
    (*p)[0] = value & 0xFF;
    (*p)[1] = ( value >> 8  ) & 0xFF;
    (*p)[2] = ( value >> 16 ) & 0xFF;
    (*p)[3] = ( value >> 24 ) & 0xFF;
    (*p)[4] = ( value >> 32 ) & 0xFF;
    (*p)[5] = ( value >> 40 ) & 0xFF;
    (*p)[6] = ( value >> 48 ) & 0xFF;
    (*p)[7] = value >> 56;
    *p += 8;
}

void next_write_float32( uint8_t ** p, float value )
{
    uint32_t value_int = 0;
    char * p_value = (char*)(&value);
    char * p_value_int = (char*)(&value_int);
    memcpy(p_value_int, p_value, sizeof(uint32_t));
    next_write_uint32( p, value_int);
}

void next_write_float64( uint8_t ** p, double value )
{
    uint64_t value_int = 0;
    char * p_value = (char *)(&value);
    char * p_value_int = (char *)(&value_int);
    memcpy(p_value_int, p_value, sizeof(uint64_t));
    next_write_uint64( p, value_int);
}

void next_write_bytes( uint8_t ** p, const uint8_t * byte_array, int num_bytes )
{
    for ( int i = 0; i < num_bytes; ++i )
    {
        next_write_uint8( p, byte_array[i] );
    }
}

uint8_t next_read_uint8( const uint8_t ** p )
{
    uint8_t value = **p;
    ++(*p);
    return value;
}

uint16_t next_read_uint16( const uint8_t ** p )
{
    uint16_t value;
    value = (*p)[0];
    value |= ( ( (uint16_t)( (*p)[1] ) ) << 8 );
    *p += 2;
    return value;
}

uint32_t next_read_uint32( const uint8_t ** p )
{
    uint32_t value;
    value  = (*p)[0];
    value |= ( ( (uint32_t)( (*p)[1] ) ) << 8 );
    value |= ( ( (uint32_t)( (*p)[2] ) ) << 16 );
    value |= ( ( (uint32_t)( (*p)[3] ) ) << 24 );
    *p += 4;
    return value;
}

uint64_t next_read_uint64( const uint8_t ** p )
{
    uint64_t value;
    value  = (*p)[0];
    value |= ( ( (uint64_t)( (*p)[1] ) ) << 8  );
    value |= ( ( (uint64_t)( (*p)[2] ) ) << 16 );
    value |= ( ( (uint64_t)( (*p)[3] ) ) << 24 );
    value |= ( ( (uint64_t)( (*p)[4] ) ) << 32 );
    value |= ( ( (uint64_t)( (*p)[5] ) ) << 40 );
    value |= ( ( (uint64_t)( (*p)[6] ) ) << 48 );
    value |= ( ( (uint64_t)( (*p)[7] ) ) << 56 );
    *p += 8;
    return value;
}

float next_read_float32( const uint8_t ** p )
{
    uint32_t value_int = next_read_uint32( p );
    float value_float = 0.0f;
    uint8_t * pointer_int = (uint8_t *)( &value_int );
    uint8_t * pointer_float = (uint8_t *)( &value_float );
    memcpy( pointer_float, pointer_int, sizeof( value_int ) );
    return value_float;
}

double next_read_float64( const uint8_t ** p )
{
    uint64_t value_int = next_read_uint64( p );
    double value_float = 0.0;
    uint8_t * pointer_int = (uint8_t *)( &value_int );
    uint8_t * pointer_float = (uint8_t *)( &value_float );
    memcpy( pointer_float, pointer_int, sizeof( value_int ) );
    return value_float;
}

void next_read_bytes( const uint8_t ** p, uint8_t * byte_array, int num_bytes )
{
    for ( int i = 0; i < num_bytes; ++i )
    {
        byte_array[i] = next_read_uint8( p );
    }
}

// ------------------------------------------------------------

void next_write_address( uint8_t ** buffer, const next_address_t * address )
{
    next_assert( buffer );
    next_assert( *buffer );
    next_assert( address );

    uint8_t * start = *buffer;

    (void) buffer;

    if ( address->type == NEXT_ADDRESS_IPV4 )
    {
        next_write_uint8( buffer, NEXT_ADDRESS_IPV4 );
        for ( int i = 0; i < 4; ++i )
        {
            next_write_uint8( buffer, address->data.ipv4[i] );
        }
        next_write_uint16( buffer, address->port );
        for ( int i = 0; i < 12; ++i )
        {
            next_write_uint8( buffer, 0 );
        }
    }
    else if ( address->type == NEXT_ADDRESS_IPV6 )
    {
        next_write_uint8( buffer, NEXT_ADDRESS_IPV6 );
        for ( int i = 0; i < 8; ++i )
        {
            next_write_uint16( buffer, address->data.ipv6[i] );
        }
        next_write_uint16( buffer, address->port );
    }
    else
    {
        for ( int i = 0; i < NEXT_ADDRESS_BYTES; ++i )
        {
            next_write_uint8( buffer, 0 );
        }
    }

    (void) start;

    next_assert( *buffer - start == NEXT_ADDRESS_BYTES );
}

void next_read_address( const uint8_t ** buffer, next_address_t * address )
{
    const uint8_t * start = *buffer;

    address->type = next_read_uint8( buffer );

    if ( address->type == NEXT_ADDRESS_IPV4 )
    {
        for ( int j = 0; j < 4; ++j )
        {
            address->data.ipv4[j] = next_read_uint8( buffer );
        }
        address->port = next_read_uint16( buffer );
        for ( int i = 0; i < 12; ++i )
        {
            uint8_t dummy = next_read_uint8( buffer ); (void) dummy;
        }
    }
    else if ( address->type == NEXT_ADDRESS_IPV6 )
    {
        for ( int j = 0; j < 8; ++j )
        {
            address->data.ipv6[j] = next_read_uint16( buffer );
        }
        address->port = next_read_uint16( buffer );
    }
    else
    {
        for ( int i = 0; i < NEXT_ADDRESS_BYTES - 1; ++i )
        {
            uint8_t dummy = next_read_uint8( buffer ); (void) dummy;
        }
    }

    (void) start;

    next_assert( *buffer - start == NEXT_ADDRESS_BYTES );
}

// -------------------------------------------------------------

static const unsigned char base64_table_encode[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int next_base64_encode_data( const uint8_t * input, size_t input_length, char * output, size_t output_size )
{
    next_assert( input );
    next_assert( output );
    next_assert( output_size > 0 );

    char * pos;
    const uint8_t * end;
    const uint8_t * in;

    size_t output_length = 4 * ( ( input_length + 2 ) / 3 ); // 3-byte blocks to 4-byte

    if ( output_length < input_length )
    {
        return -1; // integer overflow
    }

    if ( output_length >= output_size )
    {
        return -1; // not enough room in output buffer
    }

    end = input + input_length;
    in = input;
    pos = output;
    while ( end - in >= 3 )
    {
        *pos++ = base64_table_encode[in[0] >> 2];
        *pos++ = base64_table_encode[( ( in[0] & 0x03 ) << 4 ) | ( in[1] >> 4 )];
        *pos++ = base64_table_encode[( ( in[1] & 0x0f ) << 2 ) | ( in[2] >> 6 )];
        *pos++ = base64_table_encode[in[2] & 0x3f];
        in += 3;
    }

    if ( end - in )
    {
        *pos++ = base64_table_encode[in[0] >> 2];
        if (end - in == 1)
        {
            *pos++ = base64_table_encode[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else
        {
            *pos++ = base64_table_encode[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = base64_table_encode[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    output[output_length] = '\0';

    return int( output_length );
}

static const int base64_table_decode[256] =
{
    0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
    7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
    0,  0,  0,  63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
};

int next_base64_decode_data( const char * input, uint8_t * output, size_t output_size )
{
    next_assert( input );
    next_assert( output );
    next_assert( output_size > 0 );

    size_t input_length = strlen( input );
    int pad = input_length > 0 && ( input_length % 4 || input[input_length - 1] == '=' );
    size_t L = ( ( input_length + 3 ) / 4 - pad ) * 4;
    size_t output_length = L / 4 * 3 + pad;

    if ( output_length > output_size )
    {
        return 0;
    }

    for ( size_t i = 0, j = 0; i < L; i += 4 )
    {
        int n = base64_table_decode[int( input[i] )] << 18 | base64_table_decode[int( input[i + 1] )] << 12 | base64_table_decode[int( input[i + 2] )] << 6 | base64_table_decode[int( input[i + 3] )];
        output[j++] = uint8_t( n >> 16 );
        output[j++] = uint8_t( n >> 8 & 0xFF );
        output[j++] = uint8_t( n & 0xFF );
    }

    if ( pad )
    {
        int n = base64_table_decode[int( input[L] )] << 18 | base64_table_decode[int( input[L + 1] )] << 12;
        output[output_length - 1] = uint8_t( n >> 16 );

        if (input_length > L + 2 && input[L + 2] != '=')
        {
            n |= base64_table_decode[int( input[L + 2] )] << 6;
            output_length += 1;
            if ( output_length > output_size )
            {
                return 0;
            }
            output[output_length - 1] = uint8_t( n >> 8 & 0xFF );
        }
    }

    return int( output_length );
}

int next_base64_encode_string( const char * input, char * output, size_t output_size )
{
    next_assert( input );
    next_assert( output );
    next_assert( output_size > 0 );

    return next_base64_encode_data( (const uint8_t *)( input ), strlen( input ), output, output_size );
}

int next_base64_decode_string( const char * input, char * output, size_t output_size )
{
    next_assert( input );
    next_assert( output );
    next_assert( output_size > 0 );

    int output_length = next_base64_decode_data( input, (uint8_t *)( output ), output_size - 1 );
    if ( output_length < 0 )
    {
        return 0;
    }

    output[output_length] = '\0';

    return output_length;
}

// -------------------------------------------------------------

typedef uint64_t next_fnv_t;

void next_fnv_init( next_fnv_t * fnv )
{
    *fnv = 0xCBF29CE484222325;
}

void next_fnv_write( next_fnv_t * fnv, const uint8_t * data, size_t size )
{
    for ( size_t i = 0; i < size; i++ )
    {
        (*fnv) ^= data[i];
        (*fnv) *= 0x00000100000001B3;
    }
}

uint64_t next_fnv_finalize( next_fnv_t * fnv )
{
    return *fnv;
}

uint64_t next_hash_string( const char * string )
{
    next_fnv_t fnv;
    next_fnv_init( &fnv );
    next_fnv_write( &fnv, (uint8_t *)( string ), strlen( string ) );
    return next_fnv_finalize( &fnv );
}

uint64_t next_tag_id( const char * name )
{
    if ( name == NULL || name[0] == '\0' || strcmp( name, "default" ) == 0 || strcmp( name, "none" ) == 0 )
        return 0;
    else
        return next_hash_string( name );
}

uint64_t next_relay_id( const char * name )
{
    return next_hash_string( name );
}

uint64_t next_datacenter_id( const char * name )
{
    return next_hash_string( name );
}

uint64_t next_protocol_version()
{
    #ifdef NEXT_VERSION_IS_PRESENT
    return next_hash_string( NEXT_VERSION_FULL );
    #else
    return 0;
    #endif
}

// -------------------------------------------------------------

namespace next
{
    /**
        Calculates the population count of an unsigned 32 bit integer at compile time.
        See "Hacker's Delight" and http://www.hackersdelight.org/hdcodetxt/popArrayHS.c.txt
     */

    template <uint32_t x> struct PopCount
    {
        enum {   a = x - ( ( x >> 1 )       & 0x55555555 ),
                 b =   ( ( ( a >> 2 )       & 0x33333333 ) + ( a & 0x33333333 ) ),
                 c =   ( ( ( b >> 4 ) + b ) & 0x0f0f0f0f ),
                 d =   c + ( c >> 8 ),
                 e =   d + ( d >> 16 ),

            result = e & 0x0000003f 
        };
    };

    /**
        Calculates the log 2 of an unsigned 32 bit integer at compile time.
     */

    template <uint32_t x> struct Log2
    {
        enum {   a = x | ( x >> 1 ),
                 b = a | ( a >> 2 ),
                 c = b | ( b >> 4 ),
                 d = c | ( c >> 8 ),
                 e = d | ( d >> 16 ),
                 f = e >> 1,

            result = PopCount<f>::result
        };
    };

    /**
        Calculates the number of bits required to serialize an integer value in [min,max] at compile time.
     */

    template <int64_t min, int64_t max> struct BitsRequired
    {
        static const uint32_t result = ( min == max ) ? 0 : ( Log2<uint32_t(max-min)>::result + 1 );
    };

    /**
        Calculates the population count of an unsigned 32 bit integer.
        The population count is the number of bits in the integer set to 1.
        @param x The input integer value.
        @returns The number of bits set to 1 in the input value.
     */

    inline uint32_t popcount( uint32_t x )
    {
    #ifdef __GNUC__
        return __builtin_popcount( x );
    #else // #ifdef __GNUC__
        const uint32_t a = x - ( ( x >> 1 )       & 0x55555555 );
        const uint32_t b =   ( ( ( a >> 2 )       & 0x33333333 ) + ( a & 0x33333333 ) );
        const uint32_t c =   ( ( ( b >> 4 ) + b ) & 0x0f0f0f0f );
        const uint32_t d =   c + ( c >> 8 );
        const uint32_t e =   d + ( d >> 16 );
        const uint32_t result = e & 0x0000003f;
        return result;
    #endif // #ifdef __GNUC__
    }

    /**
        Calculates the log base 2 of an unsigned 32 bit integer.
        @param x The input integer value.
        @returns The log base 2 of the input.
     */

    inline uint32_t log2( uint32_t x )
    {
        const uint32_t a = x | ( x >> 1 );
        const uint32_t b = a | ( a >> 2 );
        const uint32_t c = b | ( b >> 4 );
        const uint32_t d = c | ( c >> 8 );
        const uint32_t e = d | ( d >> 16 );
        const uint32_t f = e >> 1;
        return popcount( f );
    }

    /**
        Calculates the number of bits required to serialize an integer in range [min,max].
        @param min The minimum value.
        @param max The maximum value.
        @returns The number of bits required to serialize the integer.
     */

    inline int bits_required( uint32_t min, uint32_t max )
    {
    #ifdef __GNUC__
        return ( min == max ) ? 0 : 32 - __builtin_clz( max - min );
    #else // #ifdef __GNUC__
        return ( min == max ) ? 0 : log2( max - min ) + 1;
    #endif // #ifdef __GNUC__
    }

    /**
        Reverse the order of bytes in a 64 bit integer.
        @param value The input value.
        @returns The input value with the byte order reversed.
     */

    inline uint64_t bswap( uint64_t value )
    {
    #ifdef __GNUC__
        return __builtin_bswap64( value );
    #else // #ifdef __GNUC__
        value = ( value & 0x00000000FFFFFFFF ) << 32 | ( value & 0xFFFFFFFF00000000 ) >> 32;
        value = ( value & 0x0000FFFF0000FFFF ) << 16 | ( value & 0xFFFF0000FFFF0000 ) >> 16;
        value = ( value & 0x00FF00FF00FF00FF ) << 8  | ( value & 0xFF00FF00FF00FF00 ) >> 8;
        return value;
    #endif // #ifdef __GNUC__
    }

    /**
        Reverse the order of bytes in a 32 bit integer.
        @param value The input value.
        @returns The input value with the byte order reversed.
     */

    inline uint32_t bswap( uint32_t value )
    {
    #ifdef __GNUC__
        return __builtin_bswap32( value );
    #else // #ifdef __GNUC__
        return ( value & 0x000000ff ) << 24 | ( value & 0x0000ff00 ) << 8 | ( value & 0x00ff0000 ) >> 8 | ( value & 0xff000000 ) >> 24;
    #endif // #ifdef __GNUC__
    }

    /**
        Reverse the order of bytes in a 16 bit integer.
        @param value The input value.
        @returns The input value with the byte order reversed.
     */

    inline uint16_t bswap( uint16_t value )
    {
        return ( value & 0x00ff ) << 8 | ( value & 0xff00 ) >> 8;
    }

    /**
        Template to convert an integer value from local byte order to network byte order.
        IMPORTANT: Because most machines running next are little endian, next defines network byte order to be little endian.
        @param value The input value in local byte order. Supported integer types: uint64_t, uint32_t, uint16_t.
        @returns The input value converted to network byte order. If this processor is little endian the output is the same as the input. If the processor is big endian, the output is the input byte swapped.
     */

    template <typename T> T host_to_network( T value )
    {
    #if next_BIG_ENDIAN
        return bswap( value );
    #else // #if next_BIG_ENDIAN
        return value;
    #endif // #if next_BIG_ENDIAN
    }

    /**
        Template to convert an integer value from network byte order to local byte order.
        IMPORTANT: Because most machines running next are little endian, next defines network byte order to be little endian.
        @param value The input value in network byte order. Supported integer types: uint64_t, uint32_t, uint16_t.
        @returns The input value converted to local byte order. If this processor is little endian the output is the same as the input. If the processor is big endian, the output is the input byte swapped.
     */

    template <typename T> T network_to_host( T value )
    {
    #if NEXT_BIG_ENDIAN
        return bswap( value );
    #else // #if NEXT_BIG_ENDIAN
        return value;
    #endif // #if NEXT_BIG_ENDIAN
    }

    /** 
        Compares two 16 bit sequence numbers and returns true if the first one is greater than the second (considering wrapping).
        IMPORTANT: This is not the same as s1 > s2!
        Greater than is defined specially to handle wrapping sequence numbers. 
        If the two sequence numbers are close together, it is as normal, but they are far apart, it is assumed that they have wrapped around.
        Thus, sequence_greater_than( 1, 0 ) returns true, and so does sequence_greater_than( 0, 65535 )!
        @param s1 The first sequence number.
        @param s2 The second sequence number.
        @returns True if the s1 is greater than s2, with sequence number wrapping considered.
     */

    inline bool sequence_greater_than( uint16_t s1, uint16_t s2 )
    {
        return ( ( s1 > s2 ) && ( s1 - s2 <= 32768 ) ) || 
               ( ( s1 < s2 ) && ( s2 - s1  > 32768 ) );
    }

    /** 
        Compares two 16 bit sequence numbers and returns true if the first one is less than the second (considering wrapping).
        IMPORTANT: This is not the same as s1 < s2!
        Greater than is defined specially to handle wrapping sequence numbers. 
        If the two sequence numbers are close together, it is as normal, but they are far apart, it is assumed that they have wrapped around.
        Thus, sequence_less_than( 0, 1 ) returns true, and so does sequence_greater_than( 65535, 0 )!
        @param s1 The first sequence number.
        @param s2 The second sequence number.
        @returns True if the s1 is less than s2, with sequence number wrapping considered.
     */

    inline bool sequence_less_than( uint16_t s1, uint16_t s2 )
    {
        return sequence_greater_than( s2, s1 );
    }

    /**
        Bitpacks unsigned integer values to a buffer.
        Integer bit values are written to a 64 bit scratch value from right to left.
        Once the low 32 bits of the scratch is filled with bits it is flushed to memory as a dword and the scratch value is shifted right by 32.
        The bit stream is written to memory in little endian order, which is considered network byte order for this library.
     */

    class BitWriter
    {
    public:

        /**
            Bit writer constructor.
            Creates a bit writer object to write to the specified buffer. 
            @param data The pointer to the buffer to fill with bitpacked data.
            @param bytes The size of the buffer in bytes. Must be a multiple of 4, because the bitpacker reads and writes memory as dwords, not bytes.
         */

        BitWriter( void * data, int bytes ) : m_data( (uint32_t*) data ), m_numWords( bytes / 4 )
        {
            next_assert( data );
            next_assert( ( bytes % 4 ) == 0 );
            m_numBits = m_numWords * 32;
            m_bitsWritten = 0;
            m_wordIndex = 0;
            m_scratch = 0;
            m_scratchBits = 0;
        }

        /**
            Write bits to the buffer.
            Bits are written to the buffer as-is, without padding to nearest byte. Will assert if you try to write past the end of the buffer.
            A boolean value writes just 1 bit to the buffer, a value in range [0,31] can be written with just 5 bits and so on.
            IMPORTANT: When you have finished writing to your buffer, take care to call BitWrite::FlushBits, otherwise the last dword of data will not get flushed to memory!
            @param value The integer value to write to the buffer. Must be in [0,(1<<bits)-1].
            @param bits The number of bits to encode in [1,32].
         */

        void WriteBits( uint32_t value, int bits )
        {
            next_assert( bits > 0 );
            next_assert( bits <= 32 );
            next_assert( m_bitsWritten + bits <= m_numBits );
            next_assert( uint64_t( value ) <= ( ( 1ULL << bits ) - 1 ) );

            m_scratch |= uint64_t( value ) << m_scratchBits;

            m_scratchBits += bits;

            if ( m_scratchBits >= 32 )
            {
                next_assert( m_wordIndex < m_numWords );
                m_data[m_wordIndex] = host_to_network( uint32_t( m_scratch & 0xFFFFFFFF ) );
                m_scratch >>= 32;
                m_scratchBits -= 32;
                m_wordIndex++;
            }

            m_bitsWritten += bits;
        }

        /**
            Write an alignment to the bit stream, padding zeros so the bit index becomes is a multiple of 8.
            This is useful if you want to write some data to a packet that should be byte aligned. For example, an array of bytes, or a string.
            IMPORTANT: If the current bit index is already a multiple of 8, nothing is written.
         */

        void WriteAlign()
        {
            const int remainderBits = m_bitsWritten % 8;

            if ( remainderBits != 0 )
            {
                uint32_t zero = 0;
                WriteBits( zero, 8 - remainderBits );
                next_assert( ( m_bitsWritten % 8 ) == 0 );
            }
        }

        /**
            Write an array of bytes to the bit stream.
            Use this when you have to copy a large block of data into your bitstream.
            Faster than just writing each byte to the bit stream via BitWriter::WriteBits( value, 8 ), because it aligns to byte index and copies into the buffer without bitpacking.
            @param data The byte array data to write to the bit stream.
            @param bytes The number of bytes to write.
         */

        void WriteBytes( const uint8_t * data, int bytes )
        {
            next_assert( GetAlignBits() == 0 );
            next_assert( m_bitsWritten + bytes * 8 <= m_numBits );
            next_assert( ( m_bitsWritten % 32 ) == 0 || ( m_bitsWritten % 32 ) == 8 || ( m_bitsWritten % 32 ) == 16 || ( m_bitsWritten % 32 ) == 24 );

            int headBytes = ( 4 - ( m_bitsWritten % 32 ) / 8 ) % 4;
            if ( headBytes > bytes )
                headBytes = bytes;
            for ( int i = 0; i < headBytes; ++i )
                WriteBits( data[i], 8 );
            if ( headBytes == bytes )
                return;

            FlushBits();

            next_assert( GetAlignBits() == 0 );

            int numWords = ( bytes - headBytes ) / 4;
            if ( numWords > 0 )
            {
                next_assert( ( m_bitsWritten % 32 ) == 0 );
                memcpy( &m_data[m_wordIndex], data + headBytes, numWords * 4 );
                m_bitsWritten += numWords * 32;
                m_wordIndex += numWords;
                m_scratch = 0;
            }

            next_assert( GetAlignBits() == 0 );

            int tailStart = headBytes + numWords * 4;
            int tailBytes = bytes - tailStart;
            next_assert( tailBytes >= 0 && tailBytes < 4 );
            for ( int i = 0; i < tailBytes; ++i )
                WriteBits( data[tailStart+i], 8 );

            next_assert( GetAlignBits() == 0 );

            next_assert( headBytes + numWords * 4 + tailBytes == bytes );
        }

        /**
            Flush any remaining bits to memory.
            Call this once after you've finished writing bits to flush the last dword of scratch to memory!
         */

        void FlushBits()
        {
            if ( m_scratchBits != 0 )
            {
                next_assert( m_scratchBits <= 32 );
                next_assert( m_wordIndex < m_numWords );
                m_data[m_wordIndex] = host_to_network( uint32_t( m_scratch & 0xFFFFFFFF ) );
                m_scratch >>= 32;
                m_scratchBits = 0;
                m_wordIndex++;                
            }
        }

        /**
            How many align bits would be written, if we were to write an align right now?
            @returns Result in [0,7], where 0 is zero bits required to align (already aligned) and 7 is worst case.
         */

        int GetAlignBits() const
        {
            return ( 8 - ( m_bitsWritten % 8 ) ) % 8;
        }

        /** 
            How many bits have we written so far?
            @returns The number of bits written to the bit buffer.
         */

        int GetBitsWritten() const
        {
            return m_bitsWritten;
        }

        /**
            How many bits are still available to write?
            For example, if the buffer size is 4, we have 32 bits available to write, if we have already written 10 bytes then 22 are still available to write.
            @returns The number of bits available to write.
         */

        int GetBitsAvailable() const
        {
            return m_numBits - m_bitsWritten;
        }
        
        /**
            Get a pointer to the data written by the bit writer.
            Corresponds to the data block passed in to the constructor.
            @returns Pointer to the data written by the bit writer.
         */

        const uint8_t * GetData() const
        {
            return (uint8_t*) m_data;
        }

        /**
            The number of bytes flushed to memory.
            This is effectively the size of the packet that you should send after you have finished bitpacking values with this class.
            The returned value is not always a multiple of 4, even though we flush dwords to memory. You won't miss any data in this case because the order of bits written is designed to work with the little endian memory layout.
            IMPORTANT: Make sure you call BitWriter::FlushBits before calling this method, otherwise you risk missing the last dword of data.
         */

        int GetBytesWritten() const
        {
            return ( m_bitsWritten + 7 ) / 8;
        }

    private:

        uint32_t * m_data;              ///< The buffer we are writing to, as a uint32_t * because we're writing dwords at a time.
        uint64_t m_scratch;             ///< The scratch value where we write bits to (right to left). 64 bit for overflow. Once # of bits in scratch is >= 32, the low 32 bits are flushed to memory.
        int m_numBits;                  ///< The number of bits in the buffer. This is equivalent to the size of the buffer in bytes multiplied by 8. Note that the buffer size must always be a multiple of 4.
        int m_numWords;                 ///< The number of words in the buffer. This is equivalent to the size of the buffer in bytes divided by 4. Note that the buffer size must always be a multiple of 4.
        int m_bitsWritten;              ///< The number of bits written so far.
        int m_wordIndex;                ///< The current word index. The next word flushed to memory will be at this index in m_data.
        int m_scratchBits;              ///< The number of bits in scratch. When this is >= 32, the low 32 bits of scratch is flushed to memory as a dword and scratch is shifted right by 32.
    };

    /**
        Reads bit packed integer values from a buffer.
        Relies on the user reconstructing the exact same set of bit reads as bit writes when the buffer was written. This is an unattributed bitpacked binary stream!
        Implementation: 32 bit dwords are read in from memory to the high bits of a scratch value as required. The user reads off bit values from the scratch value from the right, after which the scratch value is shifted by the same number of bits.
     */

    class BitReader
    {
    public:

        /**
            Bit reader constructor.
            Non-multiples of four buffer sizes are supported, as this naturally tends to occur when packets are read from the network.
            However, actual buffer allocated for the packet data must round up at least to the next 4 bytes in memory, because the bit reader reads dwords from memory not bytes.
            @param data Pointer to the bitpacked data to read.
            @param bytes The number of bytes of bitpacked data to read.
         */

    #ifndef NDEBUG
        BitReader( const void * data, int bytes ) : m_data( (const uint32_t*) data ), m_numBytes( bytes ), m_numWords( ( bytes + 3 ) / 4)
    #else // #ifndef NDEBUG
        BitReader( const void * data, int bytes ) : m_data( (const uint32_t*) data ), m_numBytes( bytes )
    #endif // #ifndef NDEBUG
        {
            next_assert( data );
            m_numBits = m_numBytes * 8;
            m_bitsRead = 0;
            m_scratch = 0;
            m_scratchBits = 0;
            m_wordIndex = 0;
        }

        /**
            Would the bit reader would read past the end of the buffer if it read this many bits?
            @param bits The number of bits that would be read.
            @returns True if reading the number of bits would read past the end of the buffer.
         */

        bool WouldReadPastEnd( int bits ) const
        {
            return m_bitsRead + bits > m_numBits;
        }

        /**
            Read bits from the bit buffer.
            This function will assert in debug builds if this read would read past the end of the buffer.
            In production situations, the higher level ReadStream takes care of checking all packet data and never calling this function if it would read past the end of the buffer.
            @param bits The number of bits to read in [1,32].
            @returns The integer value read in range [0,(1<<bits)-1].
         */

        uint32_t ReadBits( int bits )
        {
            next_assert( bits > 0 );
            next_assert( bits <= 32 );
            next_assert( m_bitsRead + bits <= m_numBits );

            m_bitsRead += bits;

            next_assert( m_scratchBits >= 0 && m_scratchBits <= 64 );

            if ( m_scratchBits < bits )
            {
                next_assert( m_wordIndex < m_numWords );
                m_scratch |= uint64_t( network_to_host( m_data[m_wordIndex] ) ) << m_scratchBits;
                m_scratchBits += 32;
                m_wordIndex++;
            }

            next_assert( m_scratchBits >= bits );

            const uint32_t output = m_scratch & ( (uint64_t(1)<<bits) - 1 );

            m_scratch >>= bits;
            m_scratchBits -= bits;

            return output;
        }

        /**
            Read an align.
            Call this on read to correspond to a WriteAlign call when the bitpacked buffer was written. 
            This makes sure we skip ahead to the next aligned byte index. As a safety check, we verify that the padding to next byte is zero bits and return false if that's not the case. 
            This will typically abort packet read. Just another safety measure...
            @returns True if we successfully read an align and skipped ahead past zero pad, false otherwise (probably means, no align was written to the stream).
         */

        bool ReadAlign()
        {
            const int remainderBits = m_bitsRead % 8;
            if ( remainderBits != 0 )
            {
                uint32_t value = ReadBits( 8 - remainderBits );
                next_assert( m_bitsRead % 8 == 0 );
                if ( value != 0 )
                    return false;
            }
            return true;
        }

        /**
            Read bytes from the bitpacked data.
         */

        void ReadBytes( uint8_t * data, int bytes )
        {
            next_assert( GetAlignBits() == 0 );
            next_assert( m_bitsRead + bytes * 8 <= m_numBits );
            next_assert( ( m_bitsRead % 32 ) == 0 || ( m_bitsRead % 32 ) == 8 || ( m_bitsRead % 32 ) == 16 || ( m_bitsRead % 32 ) == 24 );

            int headBytes = ( 4 - ( m_bitsRead % 32 ) / 8 ) % 4;
            if ( headBytes > bytes )
                headBytes = bytes;
            for ( int i = 0; i < headBytes; ++i )
                data[i] = (uint8_t) ReadBits( 8 );
            if ( headBytes == bytes )
                return;

            next_assert( GetAlignBits() == 0 );

            int numWords = ( bytes - headBytes ) / 4;
            if ( numWords > 0 )
            {
                next_assert( ( m_bitsRead % 32 ) == 0 );
                memcpy( data + headBytes, &m_data[m_wordIndex], numWords * 4 );
                m_bitsRead += numWords * 32;
                m_wordIndex += numWords;
                m_scratchBits = 0;
            }

            next_assert( GetAlignBits() == 0 );

            int tailStart = headBytes + numWords * 4;
            int tailBytes = bytes - tailStart;
            next_assert( tailBytes >= 0 && tailBytes < 4 );
            for ( int i = 0; i < tailBytes; ++i )
                data[tailStart+i] = (uint8_t) ReadBits( 8 );

            next_assert( GetAlignBits() == 0 );

            next_assert( headBytes + numWords * 4 + tailBytes == bytes );
        }

        /**
            How many align bits would be read, if we were to read an align right now?
            @returns Result in [0,7], where 0 is zero bits required to align (already aligned) and 7 is worst case.
         */

        int GetAlignBits() const
        {
            return ( 8 - m_bitsRead % 8 ) % 8;
        }

        /** 
            How many bits have we read so far?
            @returns The number of bits read from the bit buffer so far.
         */

        int GetBitsRead() const
        {
            return m_bitsRead;
        }

        /**
            How many bits are still available to read?
            For example, if the buffer size is 4, we have 32 bits available to read, if we have already written 10 bytes then 22 are still available.
            @returns The number of bits available to read.
         */

        int GetBitsRemaining() const
        {
            return m_numBits - m_bitsRead;
        }

    private:

        const uint32_t * m_data;            ///< The bitpacked data we're reading as a dword array.
        uint64_t m_scratch;                 ///< The scratch value. New data is read in 32 bits at a top to the left of this buffer, and data is read off to the right.
        int m_numBits;                      ///< Number of bits to read in the buffer. Of course, we can't *really* know this so it's actually m_numBytes * 8.
        int m_numBytes;                     ///< Number of bytes to read in the buffer. We know this, and this is the non-rounded up version.
    #ifndef NDEBUG
        int m_numWords;                     ///< Number of words to read in the buffer. This is rounded up to the next word if necessary.
    #endif // #ifndef NDEBUG
        int m_bitsRead;                     ///< Number of bits read from the buffer so far.
        int m_scratchBits;                  ///< Number of bits currently in the scratch value. If the user wants to read more bits than this, we have to go fetch another dword from memory.
        int m_wordIndex;                    ///< Index of the next word to read from memory.
    };

    /** 
        Functionality common to all stream classes.
     */

    class BaseStream
    {
    public:

        /**
            Base stream constructor.
         */

        explicit BaseStream() : m_context( NULL ) {}

        /**
            Set a context on the stream.
         */

        void SetContext( void * context )
        {
            m_context = context;
        }

        /**
            Get the context pointer set on the stream.
            @returns The context pointer. May be NULL.
         */

        void * GetContext() const
        {
            return m_context;
        }

    private:

        void * m_context;                           ///< The context pointer set on the stream. May be NULL.
    };

    /**
        Stream class for writing bitpacked data.
        This class is a wrapper around the bit writer class. Its purpose is to provide unified interface for reading and writing.
        You can determine if you are writing to a stream by calling Stream::IsWriting inside your templated serialize method.
        This is evaluated at compile time, letting the compiler generate optimized serialize functions without the hassle of maintaining separate read and write functions.
        IMPORTANT: Generally, you don't call methods on this class directly. Use the serialize_* macros instead. See test/shared.h for some examples.
     */

    class WriteStream : public BaseStream
    {
    public:

        enum { IsWriting = 1 };
        enum { IsReading = 0 };

        /**
            Write stream constructor.
            @param buffer The buffer to write to.
            @param bytes The number of bytes in the buffer. Must be a multiple of four.
            @param allocator The allocator to use for stream allocations. This lets you dynamically allocate memory as you read and write packets.
         */

        WriteStream( uint8_t * buffer, int bytes ) : m_writer( buffer, bytes ) {}

        /**
            Serialize an integer (write).
            @param value The integer value in [min,max].
            @param min The minimum value.
            @param max The maximum value.
            @returns Always returns true. All checking is performed by debug asserts only on write.
         */

        bool SerializeInteger( int32_t value, int32_t min, int32_t max )
        {
            next_assert( min < max );
            next_assert( value >= min );
            next_assert( value <= max );
            const int bits = bits_required( min, max );
            uint32_t unsigned_value = value - min;
            m_writer.WriteBits( unsigned_value, bits );
            return true;
        }

        /**
            Serialize a number of bits (write).
            @param value The unsigned integer value to serialize. Must be in range [0,(1<<bits)-1].
            @param bits The number of bits to write in [1,32].
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeBits( uint32_t value, int bits )
        {
            next_assert( bits > 0 );
            next_assert( bits <= 32 );
            m_writer.WriteBits( value, bits );
            return true;
        }

        /**
            Serialize an array of bytes (write).
            @param data Array of bytes to be written.
            @param bytes The number of bytes to write.
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeBytes( const uint8_t * data, int bytes )
        {
            next_assert( data );
            next_assert( bytes >= 0 );
            SerializeAlign();
            m_writer.WriteBytes( data, bytes );
            return true;
        }

        /**
            Serialize an align (write).
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeAlign()
        {
            m_writer.WriteAlign();
            return true;
        }

        /** 
            If we were to write an align right now, how many bits would be required?
            @returns The number of zero pad bits required to achieve byte alignment in [0,7].
         */

        int GetAlignBits() const
        {
            return m_writer.GetAlignBits();
        }

        /**
            Flush the stream to memory after you finish writing.
            Always call this after you finish writing and before you call WriteStream::GetData, or you'll potentially truncate the last dword of data you wrote.
         */

        void Flush()
        {
            m_writer.FlushBits();
        }

        /**
            Get a pointer to the data written by the stream.
            IMPORTANT: Call WriteStream::Flush before you call this function!
            @returns A pointer to the data written by the stream
         */

        const uint8_t * GetData() const
        {
            return m_writer.GetData();
        }

        /**
            How many bytes have been written so far?
            @returns Number of bytes written. This is effectively the packet size.
         */

        int GetBytesProcessed() const
        {
            return m_writer.GetBytesWritten();
        }

        /**
            Get number of bits written so far.
            @returns Number of bits written.
         */

        int GetBitsProcessed() const
        {
            return m_writer.GetBitsWritten();
        }

    private:

        BitWriter m_writer;                 ///< The bit writer used for all bitpacked write operations.
    };

    /**
        Stream class for reading bitpacked data.
        This class is a wrapper around the bit reader class. Its purpose is to provide unified interface for reading and writing.
        You can determine if you are reading from a stream by calling Stream::IsReading inside your templated serialize method.
        This is evaluated at compile time, letting the compiler generate optimized serialize functions without the hassle of maintaining separate read and write functions.
        IMPORTANT: Generally, you don't call methods on this class directly. Use the serialize_* macros instead. See test/shared.h for some examples.
     */

    class ReadStream : public BaseStream
    {
    public:

        enum { IsWriting = 0 };
        enum { IsReading = 1 };

        /**
            Read stream constructor.
            @param buffer The buffer to read from.
            @param bytes The number of bytes in the buffer. May be a non-multiple of four, however if it is, the underlying buffer allocated should be large enough to read the any remainder bytes as a dword.
            @param allocator The allocator to use for stream allocations. This lets you dynamically allocate memory as you read and write packets.
         */

        ReadStream( const uint8_t * buffer, int bytes ) : BaseStream(), m_reader( buffer, bytes ) {}

        /**
            Serialize an integer (read).
            @param value The integer value read is stored here. It is guaranteed to be in [min,max] if this function succeeds.
            @param min The minimum allowed value.
            @param max The maximum allowed value.
            @returns Returns true if the serialize succeeded and the value is in the correct range. False otherwise.
         */

        bool SerializeInteger( int32_t & value, int32_t min, int32_t max )
        {
            next_assert( min < max );
            const int bits = bits_required( min, max );
            if ( m_reader.WouldReadPastEnd( bits ) )
                return false;
            uint32_t unsigned_value = m_reader.ReadBits( bits );
            value = (int32_t) unsigned_value + min;
            return true;
        }

        /**
            Serialize a number of bits (read).
            @param value The integer value read is stored here. Will be in range [0,(1<<bits)-1].
            @param bits The number of bits to read in [1,32].
            @returns Returns true if the serialize read succeeded, false otherwise.
         */

        bool SerializeBits( uint32_t & value, int bits )
        {
            next_assert( bits > 0 );
            next_assert( bits <= 32 );
            if ( m_reader.WouldReadPastEnd( bits ) )
                return false;
            uint32_t read_value = m_reader.ReadBits( bits );
            value = read_value;
            return true;
        }

        /**
            Serialize an array of bytes (read).
            @param data Array of bytes to read.
            @param bytes The number of bytes to read.
            @returns Returns true if the serialize read succeeded. False otherwise.
         */

        bool SerializeBytes( uint8_t * data, int bytes )
        {
            if ( !SerializeAlign() )
                return false;
            if ( m_reader.WouldReadPastEnd( bytes * 8 ) )
                return false;
            m_reader.ReadBytes( data, bytes );
            return true;
        }

        /**
            Serialize an align (read).
            @returns Returns true if the serialize read succeeded. False otherwise.
         */

        bool SerializeAlign()
        {
            const int alignBits = m_reader.GetAlignBits();
            if ( m_reader.WouldReadPastEnd( alignBits ) )
                return false;
            if ( !m_reader.ReadAlign() )
                return false;
            return true;
        }

        /** 
            If we were to read an align right now, how many bits would we need to read?
            @returns The number of zero pad bits required to achieve byte alignment in [0,7].
         */

        int GetAlignBits() const
        {
            return m_reader.GetAlignBits();
        }

        /**
            Get number of bits read so far.
            @returns Number of bits read.
         */

        int GetBitsProcessed() const
        {
            return m_reader.GetBitsRead();
        }

        /**
            How many bytes have been read so far?
            @returns Number of bytes read. Effectively this is the number of bits read, rounded up to the next byte where necessary.
         */

        int GetBytesProcessed() const
        {
            return ( m_reader.GetBitsRead() + 7 ) / 8;
        }

    private:

        BitReader m_reader;             ///< The bit reader used for all bitpacked read operations.
    };

    /**
        Serialize integer value (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param value The integer value to serialize in [min,max].
        @param min The minimum value.
        @param max The maximum value.
     */

    #define serialize_int( stream, value, min, max )                    \
        do                                                              \
        {                                                               \
            next_assert( min < max );                                   \
            int32_t int32_value = 0;                                    \
            if ( Stream::IsWriting )                                    \
            {                                                           \
                next_assert( int64_t(value) >= int64_t(min) );          \
                next_assert( int64_t(value) <= int64_t(max) );          \
                int32_value = (int32_t) value;                          \
            }                                                           \
            if ( !stream.SerializeInteger( int32_value, min, max ) )    \
            {                                                           \
                return false;                                           \
            }                                                           \
            if ( Stream::IsReading )                                    \
            {                                                           \
                value = int32_value;                                    \
                if ( int64_t(value) < int64_t(min) ||                   \
                     int64_t(value) > int64_t(max) )                    \
                {                                                       \
                    return false;                                       \
                }                                                       \
            }                                                           \
        } while (0)

    /**
        Serialize bits to the stream (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param value The unsigned integer value to serialize.
        @param bits The number of bits to serialize in [1,32].
     */

    #define serialize_bits( stream, value, bits )                       \
        do                                                              \
        {                                                               \
            next_assert( bits > 0 );                                    \
            next_assert( bits <= 32 );                                  \
            uint32_t uint32_value = 0;                                  \
            if ( Stream::IsWriting )                                    \
            {                                                           \
                uint32_value = (uint32_t) value;                        \
            }                                                           \
            if ( !stream.SerializeBits( uint32_value, bits ) )          \
            {                                                           \
                return false;                                           \
            }                                                           \
            if ( Stream::IsReading )                                    \
            {                                                           \
                value = uint32_value;                                   \
            }                                                           \
        } while (0)

    /**
        Serialize a boolean value to the stream (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param value The boolean value to serialize.
     */

    #define serialize_bool( stream, value )                             \
        do                                                              \
        {                                                               \
            uint32_t uint32_bool_value = 0;                             \
            if ( Stream::IsWriting )                                    \
            {                                                           \
                uint32_bool_value = value ? 1 : 0;                      \
            }                                                           \
            serialize_bits( stream, uint32_bool_value, 1 );             \
            if ( Stream::IsReading )                                    \
            {                                                           \
                value = uint32_bool_value ? true : false;               \
            }                                                           \
        } while (0)

    template <typename Stream> bool serialize_float_internal( Stream & stream, float & value )
    {
        uint32_t int_value;
        if ( Stream::IsWriting )
        {
            memcpy( &int_value, &value, 4 );
        }
        bool result = stream.SerializeBits( int_value, 32 );
        if ( Stream::IsReading && result )
        {
            memcpy( &value, &int_value, 4 );
        }
        return result;
    }

    /**
        Serialize floating point value (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param value The float value to serialize.
     */

    #define serialize_float( stream, value )                                        \
        do                                                                          \
        {                                                                           \
            if ( !next::serialize_float_internal( stream, value ) )                 \
            {                                                                       \
                return false;                                                       \
            }                                                                       \
        } while (0)

    /**
        Serialize a 32 bit unsigned integer to the stream (read/write).
        This is a helper macro to make unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param value The unsigned 32 bit integer value to serialize.
     */

    #define serialize_uint32( stream, value ) serialize_bits( stream, value, 32 );

    template <typename Stream> bool serialize_uint64_internal( Stream & stream, uint64_t & value )
    {
        uint32_t hi = 0, lo = 0;
        if ( Stream::IsWriting )
        {
            lo = value & 0xFFFFFFFF;
            hi = value >> 32;
        }
        serialize_bits( stream, lo, 32 );
        serialize_bits( stream, hi, 32 );
        if ( Stream::IsReading )
        {
            value = ( uint64_t(hi) << 32 ) | lo;
        }
        return true;
    }

    /**
        Serialize a 64 bit unsigned integer to the stream (read/write).
        This is a helper macro to make unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param value The unsigned 64 bit integer value to serialize.
     */

    #define serialize_uint64( stream, value )                                       \
        do                                                                          \
        {                                                                           \
            if ( !next::serialize_uint64_internal( stream, value ) )                \
                return false;                                                       \
        } while (0)

    template <typename Stream> bool serialize_double_internal( Stream & stream, double & value )
    {
        union DoubleInt
        {
            double double_value;
            uint64_t int_value;
        };
        DoubleInt tmp = { 0 };
        if ( Stream::IsWriting )
        {
            tmp.double_value = value;
        }
        serialize_uint64( stream, tmp.int_value );
        if ( Stream::IsReading )
        {
            value = tmp.double_value;
        }
        return true;
    }

    /**
        Serialize double precision floating point value to the stream (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param value The double precision floating point value to serialize.
     */

    #define serialize_double( stream, value )                                       \
        do                                                                          \
        {                                                                           \
            if ( !next::serialize_double_internal( stream, value ) )                \
            {                                                                       \
                return false;                                                       \
            }                                                                       \
        } while (0)

    template <typename Stream> bool serialize_bytes_internal( Stream & stream, uint8_t * data, int bytes )
    {
        return stream.SerializeBytes( data, bytes );
    }

    /**
        Serialize an array of bytes to the stream (read/write).
        This is a helper macro to make unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param data Pointer to the data to be serialized.
        @param bytes The number of bytes to serialize.
     */

    #define serialize_bytes( stream, data, bytes )                                  \
        do                                                                          \
        {                                                                           \
            if ( !next::serialize_bytes_internal( stream, data, bytes ) )           \
            {                                                                       \
                return false;                                                       \
            }                                                                       \
        } while (0)

    template <typename Stream> bool serialize_string_internal( Stream & stream, char * string, int buffer_size )
    {
        int length = 0;
        if ( Stream::IsWriting )
        {
            length = (int) strlen( string );
            next_assert( length < buffer_size );
        }
        serialize_int( stream, length, 0, buffer_size - 1 );
        serialize_bytes( stream, (uint8_t*)string, length );
        if ( Stream::IsReading )
        {
            string[length] = '\0';
        }
        return true;
    }

    /**
        Serialize a string to the stream (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param string The string to serialize write. Pointer to buffer to be filled on read.
        @param buffer_size The size of the string buffer. String with terminating null character must fit into this buffer.
     */

    #define serialize_string( stream, string, buffer_size )                                 \
        do                                                                                  \
        {                                                                                   \
            if ( !next::serialize_string_internal( stream, string, buffer_size ) )          \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    /**
        Serialize an alignment to the stream (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
     */

    #define serialize_align( stream )                                                       \
        do                                                                                  \
        {                                                                                   \
            if ( !stream.SerializeAlign() )                                                 \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    /**
        Serialize an object to the stream (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param object The object to serialize. Must have a serialize method on it.
     */

    #define serialize_object( stream, object )                                              \
        do                                                                                  \
        {                                                                                   \
            if ( !object.Serialize( stream ) )                                              \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        }                                                                                   \
        while(0)

    template <typename Stream, typename T> bool serialize_int_relative_internal( Stream & stream, T previous, T & current )
    {
        uint32_t difference = 0;
        if ( Stream::IsWriting )
        {
            next_assert( previous < current );
            difference = current - previous;
        }

        bool oneBit = false;
        if ( Stream::IsWriting )
        {
            oneBit = difference == 1;
        }
        serialize_bool( stream, oneBit );
        if ( oneBit )
        {
            if ( Stream::IsReading )
            {
                current = previous + 1;
            }
            return true;
        }
        
        bool twoBits = false;
        if ( Stream::IsWriting )
        {
            twoBits = difference <= 6;
        }
        serialize_bool( stream, twoBits );
        if ( twoBits )
        {
            serialize_int( stream, difference, 2, 6 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }
        
        bool fourBits = false;
        if ( Stream::IsWriting )
        {
            fourBits = difference <= 23;
        }
        serialize_bool( stream, fourBits );
        if ( fourBits )
        {
            serialize_int( stream, difference, 7, 23 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }

        bool eightBits = false;
        if ( Stream::IsWriting )
        {
            eightBits = difference <= 280;
        }
        serialize_bool( stream, eightBits );
        if ( eightBits )
        {
            serialize_int( stream, difference, 24, 280 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }

        bool twelveBits = false;
        if ( Stream::IsWriting )
        {
            twelveBits = difference <= 4377;
        }
        serialize_bool( stream, twelveBits );
        if ( twelveBits )
        {
            serialize_int( stream, difference, 281, 4377 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }

        bool sixteenBits = false;
        if ( Stream::IsWriting )
        {
            sixteenBits = difference <= 69914;
        }
        serialize_bool( stream, sixteenBits );
        if ( sixteenBits )
        {
            serialize_int( stream, difference, 4378, 69914 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }

        uint32_t value = current;
        serialize_uint32( stream, value );
        if ( Stream::IsReading )
        {
            current = value;
        }

        return true;
    }

    /**
        Serialize an integer value relative to another (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param previous The previous integer value.
        @param current The current integer value.
     */

    #define serialize_int_relative( stream, previous, current )                             \
        do                                                                                  \
        {                                                                                   \
            if ( !next::serialize_int_relative_internal( stream, previous, current ) )      \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    template <typename Stream> bool serialize_ack_relative_internal( Stream & stream, uint16_t sequence, uint16_t & ack )
    {
        int ack_delta = 0;
        bool ack_in_range = false;
        if ( Stream::IsWriting )
        {
            if ( ack < sequence )
            {
                ack_delta = sequence - ack;
            }
            else
            {
                ack_delta = (int)sequence + 65536 - ack;
            }
            next_assert( ack_delta > 0 );
            next_assert( uint16_t( sequence - ack_delta ) == ack );
            ack_in_range = ack_delta <= 64;
        }
        serialize_bool( stream, ack_in_range );
        if ( ack_in_range )
        {
            serialize_int( stream, ack_delta, 1, 64 );
            if ( Stream::IsReading )
            {
                ack = sequence - ack_delta;
            }
        }
        else
        {
            serialize_bits( stream, ack, 16 );
        }
        return true;
    }

    /**
        Serialize an ack relative to the current sequence number (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param sequence The current sequence number.
        @param ack The ack sequence number, which is typically near the current sequence number.
     */

    #define serialize_ack_relative( stream, sequence, ack  )                                        \
        do                                                                                          \
        {                                                                                           \
            if ( !next::serialize_ack_relative_internal( stream, sequence, ack ) )                  \
            {                                                                                       \
                return false;                                                                       \
            }                                                                                       \
        } while (0)

    template <typename Stream> bool serialize_sequence_relative_internal( Stream & stream, uint16_t sequence1, uint16_t & sequence2 )
    {
        if ( Stream::IsWriting )
        {
            uint32_t a = sequence1;
            uint32_t b = sequence2 + ( ( sequence1 > sequence2 ) ? 65536 : 0 );
            serialize_int_relative( stream, a, b );
        }
        else
        {
            uint32_t a = sequence1;
            uint32_t b = 0;
            serialize_int_relative( stream, a, b );
            if ( b >= 65536 )
            {
                b -= 65536;
            }
            sequence2 = uint16_t( b );
        }

        return true;
    }

    /**
        Serialize a sequence number relative to another (read/write).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read or write stream.
        @param sequence1 The first sequence number to serialize relative to.
        @param sequence2 The second sequence number to be encoded relative to the first.
     */

    #define serialize_sequence_relative( stream, sequence1, sequence2 )                             \
        do                                                                                          \
        {                                                                                           \
            if ( !next::serialize_sequence_relative_internal( stream, sequence1, sequence2 ) )      \
            {                                                                                       \
                return false;                                                                       \
            }                                                                                       \
        } while (0)

    template <typename Stream> bool serialize_address_internal( Stream & stream, next_address_t & address )
    {
        serialize_bits( stream, address.type, 2 );
        if ( address.type == NEXT_ADDRESS_IPV4 )
        {
            serialize_bytes( stream, address.data.ipv4, 4 );
            serialize_bits( stream, address.port, 16 );
        }
        else if ( address.type == NEXT_ADDRESS_IPV6 )
        {
            for ( int i = 0; i < 8; ++i )
            {
                serialize_bits( stream, address.data.ipv6[i], 16 );
            }
            serialize_bits( stream, address.port, 16 );
        }
        else 
        {
            if ( Stream::IsReading )
            {
                memset( &address, 0, sizeof(next_address_t) );
            }
        }
        return true;
    }

    #define serialize_address( stream, address )                                                    \
        do                                                                                          \
        {                                                                                           \
            if ( !next::serialize_address_internal( stream, address ) )                             \
            {                                                                                       \
                return false;                                                                       \
            }                                                                                       \
        } while (0)
}

// -------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(push)
#pragma warning(disable:4324)
#endif // #ifdef _MSC_VER

#include <sodium.h>

#if SODIUM_LIBRARY_VERSION_MAJOR <= 7 && ( SODIUM_LIBRARY_VERSION_MAJOR && SODIUM_LIBRARY_VERSION_MINOR < 3 )
#error this version of sodium does not support overlapping buffers. please upgrade your libsodium!
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

void next_random_bytes( uint8_t * buffer, int bytes )
{
    randombytes_buf( buffer, bytes );
}

float next_random_float()
{
    uint32_t uint32_value;
    next_random_bytes( (uint8_t*)&uint32_value, sizeof(uint32_value) );
    uint64_t uint64_value = uint64_t(uint32_value);
    double double_value = double(uint64_value) / 0xFFFFFFFF;
    return float(double_value);
}

// -------------------------------------------------------------

struct NextUpgradeToken
{
    uint64_t session_id;
    uint64_t expire_timestamp;
    next_address_t client_address;
    next_address_t server_address;

    NextUpgradeToken()
    {
        session_id = 0;
        expire_timestamp = 0;
        memset( &client_address, 0, sizeof(next_address_t) );
        memset( &server_address, 0, sizeof(next_address_t) );
    }

    int Write( uint8_t * buffer, const uint8_t * private_key )
    {
        next_assert( buffer );
        next_assert( private_key );

        memset( buffer, 0, NEXT_UPGRADE_TOKEN_BYTES );

        uint8_t * nonce = buffer;
        next_random_bytes( nonce, crypto_secretbox_NONCEBYTES );
        buffer += crypto_secretbox_NONCEBYTES;

        uint8_t * p = buffer;
        
        next_write_uint64( &p, session_id );
        next_write_uint64( &p, expire_timestamp );
        next_write_address( &p, &client_address );
        next_write_address( &p, &server_address );
        
        int bytes_written = p - buffer;

        crypto_secretbox_easy( buffer, buffer, NEXT_UPGRADE_TOKEN_BYTES - crypto_secretbox_NONCEBYTES - crypto_secretbox_MACBYTES, nonce, private_key );

        next_assert( crypto_secretbox_NONCEBYTES + bytes_written + crypto_secretbox_MACBYTES <= NEXT_UPGRADE_TOKEN_BYTES );

        return crypto_secretbox_NONCEBYTES + bytes_written + crypto_secretbox_MACBYTES;
    }

    bool Read( const uint8_t * buffer, const uint8_t * private_key )
    {
        next_assert( buffer );
        next_assert( private_key );

        const uint8_t * nonce = buffer;

        uint8_t decrypted[NEXT_UPGRADE_TOKEN_BYTES];
        memcpy( decrypted, buffer + crypto_secretbox_NONCEBYTES, NEXT_UPGRADE_TOKEN_BYTES - crypto_secretbox_NONCEBYTES );

        if ( crypto_secretbox_open_easy( decrypted, decrypted, NEXT_UPGRADE_TOKEN_BYTES - crypto_secretbox_NONCEBYTES, nonce, private_key ) != 0 )
            return false;

        const uint8_t * p = decrypted;
        
        session_id = next_read_uint64( &p );
        expire_timestamp = next_read_uint64( &p );
        next_read_address( &p, &client_address );
        next_read_address( &p, &server_address );

        return true;
    }
};

// ---------------------------------------------------------------

struct next_replay_protection_t
{
    uint64_t most_recent_sequence;
    uint64_t received_packet[NEXT_REPLAY_PROTECTION_BUFFER_SIZE];
};

void next_replay_protection_reset( next_replay_protection_t * replay_protection )
{
    next_assert( replay_protection );
    replay_protection->most_recent_sequence = 0;
    memset( replay_protection->received_packet, 0xFF, sizeof( replay_protection->received_packet ) );
}

int next_replay_protection_already_received( next_replay_protection_t * replay_protection, uint64_t sequence )
{
    next_assert( replay_protection );

    if ( sequence + NEXT_REPLAY_PROTECTION_BUFFER_SIZE <= replay_protection->most_recent_sequence )
        return 1;
    
    int index = (int) ( sequence % NEXT_REPLAY_PROTECTION_BUFFER_SIZE );

    if ( replay_protection->received_packet[index] == 0xFFFFFFFFFFFFFFFFLL )
        return 0;

    if ( replay_protection->received_packet[index] >= sequence )
        return 1;

    return 0;
}

void next_replay_protection_advance_sequence( next_replay_protection_t * replay_protection, uint64_t sequence )
{
    next_assert( replay_protection );

    if ( sequence > replay_protection->most_recent_sequence )
    {
        replay_protection->most_recent_sequence = sequence;
    }

    int index = (int) ( sequence % NEXT_REPLAY_PROTECTION_BUFFER_SIZE );

    replay_protection->received_packet[index] = sequence;
}

// -------------------------------------------------------------

int next_wire_packet_bits( int packet_bytes )
{
    return ( 14 + 20 + 8 + packet_bytes + 4 ) * 8;
}

struct next_bandwidth_limiter_t
{
    uint64_t bits_sent;
    double last_check_time;
    double average_kbps;
};

void next_bandwidth_limiter_reset( next_bandwidth_limiter_t * bandwidth_limiter )
{
    next_assert( bandwidth_limiter );
    bandwidth_limiter->last_check_time = -100.0;
    bandwidth_limiter->bits_sent = 0;
    bandwidth_limiter->average_kbps = 0.0;
}

bool next_bandwidth_limiter_add_packet( next_bandwidth_limiter_t * bandwidth_limiter, double current_time, uint32_t kbps_allowed, uint32_t packet_bits )
{
    next_assert( bandwidth_limiter );
    const bool invalid = bandwidth_limiter->last_check_time < 0.0;
    if ( invalid || current_time - bandwidth_limiter->last_check_time >= NEXT_BANDWIDTH_LIMITER_INTERVAL - 0.001f )
    {
        bandwidth_limiter->bits_sent = 0;
        bandwidth_limiter->last_check_time = current_time;
    }
    bandwidth_limiter->bits_sent += packet_bits;
    return bandwidth_limiter->bits_sent > (uint64_t) ( kbps_allowed * 1000 * NEXT_BANDWIDTH_LIMITER_INTERVAL );
}

void next_bandwidth_limiter_add_sample( next_bandwidth_limiter_t * bandwidth_limiter, double kbps )
{
    if ( bandwidth_limiter->average_kbps == 0.0 && kbps != 0.0 )
    {
        bandwidth_limiter->average_kbps = kbps;
        return;
    }

    if ( bandwidth_limiter->average_kbps != 0.0 && kbps == 0.0 )
    {
        bandwidth_limiter->average_kbps = 0.0;
        return;
    }

    const double delta = kbps - bandwidth_limiter->average_kbps;

    if ( delta < 0.000001f )
    {
        bandwidth_limiter->average_kbps = kbps;
        return;
    }

    bandwidth_limiter->average_kbps += delta * 0.1f;
}

double next_bandwidth_limiter_usage_kbps( next_bandwidth_limiter_t * bandwidth_limiter, double current_time )
{
    next_assert( bandwidth_limiter );
    const bool invalid = bandwidth_limiter->last_check_time < 0.0;
    if ( !invalid )
    {
        const double delta_time = current_time - bandwidth_limiter->last_check_time;
        if ( delta_time > 0.1f )
        {
            const double kbps = bandwidth_limiter->bits_sent / delta_time / 1000.0;
            next_bandwidth_limiter_add_sample( bandwidth_limiter, kbps );
        }
    }
    return bandwidth_limiter->average_kbps;
}

// -------------------------------------------------------------

struct next_packet_loss_tracker_t
{
    void * context;
    uint64_t last_packet_processed;
    uint64_t most_recent_packet_received;
    uint64_t received_packets[NEXT_PACKET_LOSS_TRACKER_HISTORY];
};

void next_packet_loss_tracker_reset( next_packet_loss_tracker_t * tracker )
{
    next_assert( tracker );
    memset( tracker, 0, sizeof(next_packet_loss_tracker_t) );
}

void next_packet_loss_tracker_packet_received( next_packet_loss_tracker_t * tracker, uint64_t sequence )
{
    next_assert( tracker );
    sequence++;
    const int index = int( sequence % NEXT_PACKET_LOSS_TRACKER_HISTORY );
    tracker->received_packets[index] = sequence;
    tracker->most_recent_packet_received = sequence;
}

int next_packet_loss_tracker_update( next_packet_loss_tracker_t * tracker, next_stats_t * stats )
{
    next_assert( tracker );
    int lost_packets = 0;
    uint64_t start = tracker->last_packet_processed + 1;
    uint64_t finish = ( tracker->most_recent_packet_received > NEXT_PACKET_LOSS_TRACKER_SAFETY ) ? ( tracker->most_recent_packet_received - NEXT_PACKET_LOSS_TRACKER_SAFETY ) : 0;
    if ( finish > start && finish - start > NEXT_PACKET_LOSS_TRACKER_HISTORY )
    {
        tracker->last_packet_processed = tracker->most_recent_packet_received;
        return 0;
    }
    for ( uint64_t sequence = start; sequence <= finish; ++sequence )
    {
        const int index = int( sequence % NEXT_PACKET_LOSS_TRACKER_HISTORY );
        if ( tracker->received_packets[index] != sequence )
        {
            next_stats( stats, "server -> client packet lost %" PRIu64, sequence - 1 );
            lost_packets++;
        }
    }
    tracker->last_packet_processed = finish;
    return lost_packets;
}

// -------------------------------------------------------------

#define NEXT_ROUTE_REQUEST_PACKET                            1
#define NEXT_ROUTE_RESPONSE_PACKET                           2
#define NEXT_CLIENT_TO_SERVER_PACKET                         3
#define NEXT_SERVER_TO_CLIENT_PACKET                         4
#define NEXT_PING_PACKET                                    11
#define NEXT_PONG_PACKET                                    12
#define NEXT_CONTINUE_REQUEST_PACKET                        13
#define NEXT_CONTINUE_RESPONSE_PACKET                       14
#define NEXT_UPGRADE_REQUEST_PACKET                         64
#define NEXT_UPGRADE_RESPONSE_PACKET                        65
#define NEXT_UPGRADE_CONFIRM_PACKET                         66
#define NEXT_DIRECT_PING_PACKET                             68
#define NEXT_DIRECT_PONG_PACKET                             69
#define NEXT_CLIENT_STATS_PACKET                            70
#define NEXT_ROUTE_UPDATE_PACKET                            71
#define NEXT_ROUTE_UPDATE_ACK_PACKET                        72
#define NEXT_RELAY_PING_PACKET                              73
#define NEXT_RELAY_PONG_PACKET                              74

struct NextUpgradeRequestPacket
{
    uint64_t protocol_version;
    uint64_t session_id;
    next_address_t server_address;
    uint8_t server_kx_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t upgrade_token[NEXT_UPGRADE_TOKEN_BYTES];
    uint8_t signature[crypto_sign_BYTES];

    NextUpgradeRequestPacket()
    {
        memset( this, 0, sizeof(NextUpgradeRequestPacket) );
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, protocol_version );
        serialize_uint64( stream, session_id );
        serialize_address( stream, server_address );
        serialize_bytes( stream, server_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        serialize_bytes( stream, upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );
        serialize_bytes( stream, signature, crypto_sign_BYTES );
        return true;
    }

    int GetSignData( uint8_t * buffer, int buffer_size )
    {
        uint8_t * p = buffer;
        next_write_uint64( &p, protocol_version );
        next_write_uint64( &p, session_id );
        next_write_address( &p, &server_address );
        next_write_bytes( &p, server_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        next_write_bytes( &p, upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );
        next_assert( p - buffer <= buffer_size );
        (void) buffer_size;
        return p - buffer;
    }

    void Sign( const uint8_t * private_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[1024];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        crypto_sign_final_create( &state, signature, NULL, private_key );
    }

    bool Verify( const uint8_t * public_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[1024];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        return crypto_sign_final_verify( &state, signature, public_key ) == 0;
    }
};

struct NextUpgradeResponsePacket
{
    uint8_t client_open_session_sequence;
    uint8_t client_kx_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t client_route_public_key[crypto_box_PUBLICKEYBYTES];
    uint8_t upgrade_token[NEXT_UPGRADE_TOKEN_BYTES];

    NextUpgradeResponsePacket()
    {
        memset( this, 0, sizeof(NextUpgradeResponsePacket) );
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_bits( stream, client_open_session_sequence, 8 );
        serialize_bytes( stream, client_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        serialize_bytes( stream, client_route_public_key, crypto_box_PUBLICKEYBYTES );
        serialize_bytes( stream, upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );
        return true;
    }
};

struct NextUpgradeConfirmPacket
{
    uint64_t upgrade_sequence;
    uint64_t session_id;
    next_address_t server_address;
    uint8_t client_kx_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t server_kx_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t signature[crypto_sign_BYTES];

    NextUpgradeConfirmPacket()
    {
        memset( this, 0, sizeof(NextUpgradeConfirmPacket) );
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, upgrade_sequence );
        serialize_uint64( stream, session_id );
        serialize_address( stream, server_address );
        serialize_bytes( stream, client_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        serialize_bytes( stream, server_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        serialize_bytes( stream, signature, crypto_sign_BYTES );
        return true;
    }

    int GetSignData( uint8_t * buffer, int buffer_length )
    {
        uint8_t * p = buffer;
        next_write_uint64( &p, upgrade_sequence );
        next_write_uint64( &p, session_id );
        next_write_address( &p, &server_address );
        next_write_bytes( &p, client_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        next_write_bytes( &p, server_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        next_assert( p - buffer <= buffer_length );
        (void) buffer_length;
        return p - buffer;
    }

    void Sign( const uint8_t * private_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[1024];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        crypto_sign_final_create( &state, signature, NULL, private_key );
    }

    bool Verify( const uint8_t * public_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[1024];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        return crypto_sign_final_verify( &state, signature, public_key ) == 0;
    }
};

struct NextDirectPingPacket
{
    uint64_t ping_sequence;

    NextDirectPingPacket()
    {
        ping_sequence = 0;
    }
    
    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, ping_sequence );
        return true;
    }
};

struct NextDirectPongPacket
{
    uint64_t ping_sequence;

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, ping_sequence );
        return true;
    }
};

struct NextClientStatsPacket
{
    bool flagged;
    bool fallback_to_direct;
    bool try_before_you_buy;
    bool multipath;
    uint64_t flags;
    uint64_t platform_id;
    int connection_type;
    float kbps_up;
    float kbps_down;
    float direct_min_rtt;
    float direct_max_rtt;
    float direct_mean_rtt;
    float direct_jitter;
    float direct_packet_loss;
    bool next;
    float next_min_rtt;
    float next_max_rtt;
    float next_mean_rtt;
    float next_jitter;
    float next_packet_loss;
    int num_near_relays;
    uint64_t near_relay_ids[NEXT_MAX_NEAR_RELAYS];
    float near_relay_min_rtt[NEXT_MAX_NEAR_RELAYS];
    float near_relay_max_rtt[NEXT_MAX_NEAR_RELAYS];
    float near_relay_mean_rtt[NEXT_MAX_NEAR_RELAYS];
    float near_relay_jitter[NEXT_MAX_NEAR_RELAYS];
    float near_relay_packet_loss[NEXT_MAX_NEAR_RELAYS];
    uint64_t packets_lost_server_to_client;

    NextClientStatsPacket()
    {
        memset( this, 0, sizeof(NextClientStatsPacket) );
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_bool( stream, flagged );
        serialize_bool( stream, fallback_to_direct );
        serialize_bool( stream, try_before_you_buy );
        serialize_bool( stream, multipath );
        serialize_bits( stream, flags, NEXT_FLAGS_COUNT );
        serialize_uint64( stream, platform_id );
        serialize_int( stream, connection_type, NEXT_CONNECTION_TYPE_UNKNOWN, NEXT_CONNECTION_TYPE_CELLULAR );
        serialize_float( stream, kbps_up );
        serialize_float( stream, kbps_down );
        serialize_float( stream, direct_min_rtt );
        serialize_float( stream, direct_max_rtt );
        serialize_float( stream, direct_mean_rtt );
        serialize_float( stream, direct_jitter );
        serialize_float( stream, direct_packet_loss );
        serialize_bool( stream, next );
        if ( next )
        {
            serialize_float( stream, next_min_rtt );
            serialize_float( stream, next_max_rtt );
            serialize_float( stream, next_mean_rtt );
            serialize_float( stream, next_jitter );
            serialize_float( stream, next_packet_loss );
        }
        serialize_int( stream, num_near_relays, 0, NEXT_MAX_NEAR_RELAYS );
        for ( int i = 0; i < num_near_relays; ++i )
        {
            serialize_uint64( stream, near_relay_ids[i] );
            serialize_float( stream, near_relay_min_rtt[i] );
            serialize_float( stream, near_relay_max_rtt[i] );
            serialize_float( stream, near_relay_mean_rtt[i] );
            serialize_float( stream, near_relay_jitter[i] );
            serialize_float( stream, near_relay_packet_loss[i] );
        }
        serialize_uint64( stream, packets_lost_server_to_client );
        return true;
    }
};

struct NextRouteUpdatePacket
{
    uint64_t sequence;
    bool multipath;
    int num_near_relays;
    uint64_t near_relay_ids[NEXT_MAX_NEAR_RELAYS];
    next_address_t near_relay_addresses[NEXT_MAX_NEAR_RELAYS];
    uint8_t update_type;
    int num_tokens;
    uint8_t tokens[NEXT_MAX_TOKENS*NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES];
    uint64_t packets_lost_client_to_server;

    NextRouteUpdatePacket()
    {
        memset( this, 0, sizeof(NextRouteUpdatePacket ) );
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, sequence );
        serialize_int( stream, num_near_relays, 0, NEXT_MAX_NEAR_RELAYS );
        for ( int i = 0; i < num_near_relays; ++i )
        {
            serialize_uint64( stream, near_relay_ids[i] );
            serialize_address( stream, near_relay_addresses[i] );
        }
        serialize_int( stream, update_type, 0, NEXT_UPDATE_TYPE_CONTINUE );
        if ( update_type != NEXT_UPDATE_TYPE_DIRECT )
        {
            serialize_int( stream, num_tokens, 0, NEXT_MAX_TOKENS );
            serialize_bool( stream, multipath );
        }
        if ( update_type == NEXT_UPDATE_TYPE_ROUTE )
        {
            serialize_bytes( stream, tokens, num_tokens * NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES );
        }
        else if ( update_type == NEXT_UPDATE_TYPE_CONTINUE )
        {
            serialize_bytes( stream, tokens, num_tokens * NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES );
        }
        serialize_uint64( stream, packets_lost_client_to_server );
        return true;
    }
};

struct NextRouteUpdateAckPacket
{
    uint64_t sequence;

    NextRouteUpdateAckPacket()
    {
        sequence = 0;
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, sequence );
        return true;
    }
};

struct NextRelayPingPacket
{
    uint64_t ping_sequence;
    uint64_t session_id;
    uint64_t padding1;
    uint64_t padding2;

    NextRelayPingPacket()
    {
        ping_sequence = 0;
        session_id = 0;
        padding1 = 0;
        padding2 = 0;
    }
    
    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, ping_sequence );
        serialize_uint64( stream, session_id );
        serialize_uint64( stream, padding1 );
        serialize_uint64( stream, padding2 );
        if ( Stream::IsWriting )
        {
            next_assert( padding1 == 0 );
            next_assert( padding2 == 0 );
        }
        if ( Stream::IsReading )
        {
            if ( padding1 != 0 || padding2 != 0 )
                return false;
        }
        return true;
    }
};

struct NextRelayPongPacket
{
    uint64_t ping_sequence;
    uint64_t session_id;

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, ping_sequence );
        serialize_uint64( stream, session_id );
        return true;
    }
};

int next_write_packet( uint8_t packet_id, void * packet_object, uint8_t * packet_data, int * packet_bytes, const int * encrypted_packet, uint64_t * sequence, const uint8_t * private_key )
{
    next_assert( packet_object );
    next_assert( packet_data );
    next_assert( packet_bytes );

    next::WriteStream stream( packet_data, NEXT_MAX_PACKET_BYTES );

    typedef next::WriteStream Stream;

    serialize_bits( stream, packet_id, 8 );

    if ( encrypted_packet && encrypted_packet[packet_id] != 0 )
    {
        next_assert( sequence );
        next_assert( private_key );

        serialize_uint64( stream, *sequence );
    }

    switch ( packet_id )
    {
        case NEXT_UPGRADE_REQUEST_PACKET:
        {
            NextUpgradeRequestPacket * packet = (NextUpgradeRequestPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write upgrade request packet" );
                return NEXT_ERROR;
            }
        }
        break;

        case NEXT_UPGRADE_RESPONSE_PACKET:
        {
            NextUpgradeResponsePacket * packet = (NextUpgradeResponsePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write upgrade response packet" );
                return NEXT_ERROR;
            }
        }
        break;

        case NEXT_UPGRADE_CONFIRM_PACKET:
        {
            NextUpgradeConfirmPacket * packet = (NextUpgradeConfirmPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write upgrade confirm packet" );
                return NEXT_ERROR;
            }
        }
        break;

        case NEXT_DIRECT_PING_PACKET:
        {
            NextDirectPingPacket * packet = (NextDirectPingPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write direct ping packet" );
                return NEXT_ERROR;
            }
        }
        break;
        
        case NEXT_DIRECT_PONG_PACKET:
        {
            NextDirectPongPacket * packet = (NextDirectPongPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write direct pong packet" );
                return NEXT_ERROR;
            }
        }
        break;
        
        case NEXT_CLIENT_STATS_PACKET:
        {
            NextClientStatsPacket * packet = (NextClientStatsPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write client stats packet" );
                return NEXT_ERROR;
            }
        }
        break;

        case NEXT_ROUTE_UPDATE_PACKET:
        {
            NextRouteUpdatePacket * packet = (NextRouteUpdatePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write route update packet" );
                return NEXT_ERROR;
            }
        }
        break;

        case NEXT_ROUTE_UPDATE_ACK_PACKET:
        {
            NextRouteUpdateAckPacket * packet = (NextRouteUpdateAckPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write route update ack packet" );
                return NEXT_ERROR;
            }
        }
        break;

        case NEXT_RELAY_PING_PACKET:
        {
            NextRelayPingPacket * packet = (NextRelayPingPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write relay ping packet" );
                return NEXT_ERROR;
            }
        }
        break;

        case NEXT_RELAY_PONG_PACKET:
        {
            NextRelayPongPacket * packet = (NextRelayPongPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to write relay pong packet" );
                return NEXT_ERROR;
            }
        }
        break;

        default:
            return NEXT_ERROR;
    }

    stream.Flush();

    *packet_bytes = stream.GetBytesProcessed();

    if ( encrypted_packet && encrypted_packet[packet_id] )
    {
        uint8_t * additional = packet_data;
        uint8_t * nonce = packet_data + 1;
        uint8_t * message = packet_data + 1 + 8;
        int message_length = *packet_bytes - 1 - 8;

        unsigned long long encrypted_bytes = 0;

        crypto_aead_chacha20poly1305_encrypt( message, &encrypted_bytes,
                                              message, message_length,
                                              additional, 1,
                                              NULL, nonce, private_key );

        next_assert( encrypted_bytes == message_length + crypto_aead_chacha20poly1305_ABYTES );

        *packet_bytes = 1 + 8 + encrypted_bytes;

        (*sequence)++;
    }

    return NEXT_OK;
}

bool next_is_payload_packet( uint8_t packet_id )
{
    return packet_id == NEXT_CLIENT_TO_SERVER_PACKET || 
           packet_id == NEXT_SERVER_TO_CLIENT_PACKET;
}

uint64_t next_clean_sequence( uint64_t sequence )
{
    uint64_t mask = ~( (1ULL<<63) | (1ULL<<62) );
    return sequence & mask;
}

int next_read_packet( uint8_t * packet_data, int packet_bytes, void * packet_object, const int * encrypted_packet, uint64_t * sequence, const uint8_t * private_key, next_replay_protection_t * replay_protection )
{
    next_assert( packet_data );
    next_assert( packet_object );

    if ( packet_bytes < 1 )
        return NEXT_ERROR;

    uint8_t packet_id = packet_data[0];

    if ( encrypted_packet && encrypted_packet[packet_id] )
    {
        next_assert( sequence );
        next_assert( private_key );
        next_assert( replay_protection );

        if ( packet_bytes <= (int) ( 1 + 8 + crypto_aead_chacha20poly1305_ABYTES ) )
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "encrypted packet is too small to be valid" );
            return NEXT_ERROR;
        }

        const uint8_t * p = &packet_data[1];

        *sequence = next_read_uint64( &p );

        uint64_t clean_sequence = next_clean_sequence( *sequence );

        if ( next_replay_protection_already_received( replay_protection, clean_sequence ) )
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "packet already received: %" PRIu64 " vs. %" PRIu64, clean_sequence, replay_protection->most_recent_sequence );
            return NEXT_ERROR;
        }

        uint8_t * additional = packet_data;
        uint8_t * nonce = packet_data + 1;
        uint8_t * message = packet_data + 1 + 8;
        int message_length = packet_bytes - 1 - 8;

        unsigned long long decrypted_bytes;

        if ( crypto_aead_chacha20poly1305_decrypt( message, &decrypted_bytes,
                                                   NULL,
                                                   message, message_length,
                                                   additional, 1,
                                                   nonce, private_key ) != 0 )
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "encrypted packet failed to decrypt" );
            return NEXT_ERROR;
        }

        next_assert( decrypted_bytes == message_length - crypto_aead_chacha20poly1305_ABYTES );

        packet_data += 1 + 8;
        packet_bytes -= 1 + 8;
    }
    else
    {
        packet_data += 1;
        packet_bytes -= 1;
    }

    next::ReadStream stream( packet_data, packet_bytes );

    switch ( packet_id )
    {
        case NEXT_UPGRADE_REQUEST_PACKET:
        {
            NextUpgradeRequestPacket * packet = (NextUpgradeRequestPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_UPGRADE_RESPONSE_PACKET:
        {
            NextUpgradeResponsePacket * packet = (NextUpgradeResponsePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_UPGRADE_CONFIRM_PACKET:
        {
            NextUpgradeConfirmPacket * packet = (NextUpgradeConfirmPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_DIRECT_PING_PACKET:
        {
            NextDirectPingPacket * packet = (NextDirectPingPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_DIRECT_PONG_PACKET:
        {
            NextDirectPongPacket * packet = (NextDirectPongPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_CLIENT_STATS_PACKET:
        {
            NextClientStatsPacket * packet = (NextClientStatsPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "failed to read client stats packet" );
                return NEXT_ERROR;
            }
        }
        break;

        case NEXT_ROUTE_UPDATE_PACKET:
        {
            NextRouteUpdatePacket * packet = (NextRouteUpdatePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_ROUTE_UPDATE_ACK_PACKET:
        {
            NextRouteUpdateAckPacket * packet = (NextRouteUpdateAckPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_RELAY_PING_PACKET:
        {
            NextRelayPingPacket * packet = (NextRelayPingPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_RELAY_PONG_PACKET:
        {
            NextRelayPongPacket * packet = (NextRelayPongPacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        default:
            return NEXT_ERROR;
    }

    return (int) packet_id;
}

void next_post_validate_packet( uint8_t * packet_data, int packet_bytes, void * packet_object, const int * encrypted_packet, uint64_t * sequence, const uint8_t * private_key, next_replay_protection_t * replay_protection, next_packet_loss_tracker_t * tracker )
{
    (void) packet_bytes;
    (void) packet_object;
    (void) private_key;

    next_assert( packet_bytes >= 1 );

    if ( packet_bytes < 1 )
        return;

    uint8_t packet_id = packet_data[0];

    bool payload_packet = next_is_payload_packet( packet_id );

    if ( encrypted_packet && encrypted_packet[packet_id] && payload_packet )
    {
        next_replay_protection_advance_sequence( replay_protection, *sequence );

        if ( tracker )
        {
            next_packet_loss_tracker_packet_received( tracker, *sequence );
        }
    }
}

// -------------------------------------------------------------

static int next_encrypted_packets[256];

void * next_global_context = NULL;

struct next_config_internal_t
{
    uint64_t customer_id;
    uint8_t customer_public_key[32];
    uint8_t customer_private_key[64];
    bool invalid_customer_private_key;
    int socket_send_buffer_size;
    int socket_receive_buffer_size;
    bool disable_network_next;
    bool try_before_you_buy;
    bool high_priority_server_thread;
};

static next_config_internal_t next_global_config;

void next_default_config( next_config_t * config )
{
    next_assert( config );
    memset( config, 0, sizeof(next_config_t) );
    config->socket_send_buffer_size = NEXT_DEFAULT_SOCKET_SEND_BUFFER_SIZE;
    config->socket_receive_buffer_size = NEXT_DEFAULT_SOCKET_RECEIVE_BUFFER_SIZE;
    config->disable_network_next = false;
    config->try_before_you_buy = true;
    config->high_priority_server_thread = true;
}

int next_init( void * context, next_config_t * config_in )
{
    next_assert( next_global_context == NULL );

    next_global_context = context;

    if ( next_platform_init() != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "failed to initialize platform" );
        return NEXT_ERROR;
    }

    if ( sodium_init() == -1 )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "failed to initialize sodium" );
        return NEXT_ERROR;
    }

    next_config_internal_t config;

    memset( &config, 0, sizeof(next_config_internal_t) );

    config.socket_send_buffer_size = NEXT_DEFAULT_SOCKET_SEND_BUFFER_SIZE;
    config.socket_receive_buffer_size = NEXT_DEFAULT_SOCKET_RECEIVE_BUFFER_SIZE;

    const char * customer_public_key = config_in ? config_in->customer_public_key : next_platform_getenv( "NEXT_CUSTOMER_PUBLIC_KEY" );
    if ( customer_public_key )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "customer public key is '%s'", customer_public_key );
        uint8_t decode_buffer[8+crypto_sign_PUBLICKEYBYTES];
        if ( next_base64_decode_data( customer_public_key, decode_buffer, sizeof(decode_buffer) ) == sizeof(decode_buffer) )
        {
            memcpy( config.customer_public_key, decode_buffer + 8, crypto_sign_PUBLICKEYBYTES );
        }
        else
        {
            if ( customer_public_key[0] != '\0' )
            {
                next_printf( NEXT_LOG_LEVEL_ERROR, "customer public key is invalid: \"%s\"", customer_public_key );
            }
        }
    }

    const char * customer_private_key = config_in ? config_in->customer_private_key : next_platform_getenv( "NEXT_CUSTOMER_PRIVATE_KEY" );
    if ( customer_private_key )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "customer private key is '%s'", customer_private_key );
        uint8_t decode_buffer[8+crypto_sign_SECRETKEYBYTES];
        if ( customer_private_key && next_base64_decode_data( customer_private_key, decode_buffer, sizeof(decode_buffer) ) == sizeof(decode_buffer) )
        {
            const uint8_t * p = decode_buffer;
            config.customer_id = next_read_uint64( &p );
            memcpy( config.customer_private_key, decode_buffer + 8, crypto_sign_SECRETKEYBYTES );
        }
        else
        {
            if ( customer_private_key[0] != '\0' )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "customer private key is invalid: \"%s\"", customer_private_key );
            }
            config.invalid_customer_private_key = true;
        }
    }

    if ( config_in )
    {
        config.socket_send_buffer_size = config_in->socket_send_buffer_size;
        config.socket_receive_buffer_size = config_in->socket_receive_buffer_size;
    }
    else
    {
        const char * socket_send_buffer_size_env = next_platform_getenv( "NEXT_SOCKET_SEND_BUFFER_SIZE" );
        const char * socket_receive_buffer_size_env = next_platform_getenv( "NEXT_SOCKET_RECEIVE_BUFFER_SIZE" );
        if ( socket_send_buffer_size_env != NULL )
        {
            int value = atoi( socket_send_buffer_size_env );
            if ( value > 0 )
            {
                config.socket_send_buffer_size = value;
            }
        }
        if ( socket_receive_buffer_size_env != NULL )
        {
            int value = atoi( socket_receive_buffer_size_env );
            if ( value > 0 )
            {
                config.socket_receive_buffer_size = value;
            }
        }
    }

    if ( config_in )
    {
        config.disable_network_next = config_in->disable_network_next;
        config.try_before_you_buy = config_in->try_before_you_buy;
        config.high_priority_server_thread = config_in->high_priority_server_thread;
    }
    else
    {
        config.disable_network_next = false;
        config.try_before_you_buy = !next_platform_getenv_bool("NEXT_DISABLE_TRY_BEFORE_YOU_BUY");
        config.high_priority_server_thread = true;
    }

    next_global_config = config;

    next_encrypted_packets[NEXT_DIRECT_PING_PACKET] = 1;
    next_encrypted_packets[NEXT_DIRECT_PONG_PACKET] = 1;
    next_encrypted_packets[NEXT_CLIENT_STATS_PACKET] = 1;
    next_encrypted_packets[NEXT_ROUTE_UPDATE_PACKET] = 1;
    next_encrypted_packets[NEXT_ROUTE_UPDATE_ACK_PACKET] = 1;

    const char * log_level_override = next_platform_getenv( "NEXT_LOG_LEVEL" );
    if ( log_level_override )
    {
        log_level = atoi( log_level_override );
    }

    return NEXT_OK;
}

void next_term()
{
    next_platform_term();

    next_global_context = NULL;
}

// ---------------------------------------------------------------

struct next_queue_t
{
    void * context;
    int size;
    int num_entries;
    int start_index;
    void ** entries;
};

void next_queue_destroy( next_queue_t * queue );

next_queue_t * next_queue_create( void * context, int size )
{
    next_queue_t * queue = (next_queue_t*) next_malloc( context, sizeof(next_queue_t) );
    next_assert( queue );
    if ( !queue )
        return NULL;
    queue->context = context;
    queue->size = size;
    queue->num_entries = 0;
    queue->start_index = 0;
    queue->entries = (void**) next_malloc( context, size * sizeof(void*) );
    next_assert( queue->entries );
    if ( !queue->entries )
    {
        next_queue_destroy( queue );
        return NULL;
    }
    return queue;
}

void next_queue_clear( next_queue_t * queue );

void next_queue_destroy( next_queue_t * queue )
{
    next_assert( queue );
    next_queue_clear( queue );
    next_free( queue->context, queue->entries );

    clear_and_free( queue->context, queue, sizeof( queue ) );
}

void next_queue_clear( next_queue_t * queue )
{
    const int queue_size = queue->size;
    const int start_index = queue->start_index;
    for ( int i = 0; i < queue->num_entries; ++i )
    {
        const int index = (start_index + i ) % queue_size;
        next_free( queue->context, queue->entries[index] );
        queue->entries[index] = NULL;
    }
    queue->num_entries = 0;
    queue->start_index = 0;
}

int next_queue_push( next_queue_t * queue, void * entry )
{
    next_assert( queue );
    next_assert( entry );
    if ( queue->num_entries == queue->size )
    {
        next_free( queue->context, entry );
        return NEXT_ERROR;
    }
    int index = ( queue->start_index + queue->num_entries ) % queue->size;
    queue->entries[index] = entry;
    queue->num_entries++;
    return NEXT_OK;
}

void * next_queue_pop( next_queue_t * queue )
{
    if ( queue->num_entries == 0 )
        return NULL;
    void * entry = queue->entries[queue->start_index];
    queue->start_index = ( queue->start_index + 1 ) % queue->size;
    queue->num_entries--;
    return entry;
}

// ---------------------------------------------------------------

struct next_route_stats_t
{
    float min_rtt;
    float max_rtt;
    float mean_rtt;
    float jitter;
    float packet_loss;
};

struct next_ping_history_entry_t
{
    uint64_t sequence;
    double time_ping_sent;
    double time_pong_received;
};

struct next_ping_history_t
{
    uint64_t sequence;
    next_ping_history_entry_t entries[NEXT_PING_HISTORY_ENTRY_COUNT];
};

void next_ping_history_clear( next_ping_history_t * history )
{
    next_assert( history );
    history->sequence = 0;
    for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; ++i )
    {
        history->entries[i].sequence = 0xFFFFFFFFFFFFFFFFULL;
        history->entries[i].time_ping_sent = -1.0;
        history->entries[i].time_pong_received = -1.0;
    }
}

uint64_t next_ping_history_ping_sent( next_ping_history_t * history, double time )
{
    next_assert( history );
    const int index = history->sequence % NEXT_PING_HISTORY_ENTRY_COUNT;
    next_ping_history_entry_t * entry = &history->entries[index];
    entry->sequence = history->sequence;
    entry->time_ping_sent = time;
    entry->time_pong_received = -1.0;
    history->sequence++;
    return entry->sequence;
}

void next_ping_history_pong_received( next_ping_history_t * history, uint64_t sequence, double time )
{
    const int index = sequence % NEXT_PING_HISTORY_ENTRY_COUNT;
    next_ping_history_entry_t * entry = &history->entries[index];
    if ( entry->sequence == sequence )
    {
        entry->time_pong_received = time;
    }
}

void next_route_stats_from_ping_history( const next_ping_history_t * history, double start, double end, next_route_stats_t * stats, double ping_safety )
{
    next_assert( history );
    next_assert( stats );

    next_assert( start < end );

    stats->min_rtt = 0.0f;
    stats->max_rtt = 0.0f;
    stats->mean_rtt = 0.0f;
    stats->jitter = 0.0f;
    stats->packet_loss = 0.0f;

    // calculate packet loss

    int num_pings_sent = 0;
    int num_pongs_received = 0;

    for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; i++ )
    {
        const next_ping_history_entry_t * entry = &history->entries[i];

        if ( entry->time_ping_sent >= start && entry->time_ping_sent <= end - ping_safety )
        {
            num_pings_sent++;

            if ( entry->time_pong_received >= entry->time_ping_sent )
                num_pongs_received++;
        }
    }

    if ( num_pings_sent > 0 )
    {
        stats->packet_loss = (float) ( 100.0 * ( 1.0 - ( double( num_pongs_received ) / double( num_pings_sent ) ) ) );
    }

    // calculate min/max/mean RTT

    double min_rtt = FLT_MAX;
    double max_rtt = 0.0;
    double mean_rtt = 0.0;
    int num_pings = 0;
    int num_pongs = 0;

    for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; i++ )
    {
        const next_ping_history_entry_t * entry = &history->entries[i];

        if ( entry->time_ping_sent >= start && entry->time_ping_sent <= end )
        {
            if ( entry->time_pong_received > entry->time_ping_sent )
            {
                double rtt = 1000.0 * ( entry->time_pong_received - entry->time_ping_sent );
                if ( rtt < min_rtt )
                {
                    min_rtt = rtt;
                }
                if ( rtt > max_rtt )
                {
                    max_rtt = rtt;
                }
                mean_rtt += rtt;
                num_pongs++;
            }
            num_pings++;
        }
    }

    if ( num_pongs > 0 )
    {
        mean_rtt /= num_pongs;
    }
    else
    {
        stats->packet_loss = num_pings > 0 ? 100.0f : 0.0f;
        return;
    }

    next_assert( min_rtt >= 0.0 );
    next_assert( max_rtt >= 0.0 );
    next_assert( mean_rtt >= 0.0 );

    stats->min_rtt = float( min_rtt );
    stats->max_rtt = float( max_rtt );
    stats->mean_rtt = float( mean_rtt );

    // calculate jitter

    int num_jitter_samples = 0;
  
    double stddev_rtt = 0.0;

    for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; i++ )
    {
        const next_ping_history_entry_t * entry = &history->entries[i];

        if ( entry->time_ping_sent >= start && entry->time_ping_sent <= end )
        {
            if ( entry->time_pong_received > entry->time_ping_sent )
            {
                // pong received
                double rtt = 1000.0 * ( entry->time_pong_received - entry->time_ping_sent );
                double error = rtt - min_rtt;
                stddev_rtt += error * error;
                num_jitter_samples++;
            }
        }
    }

    if ( num_jitter_samples > 0 )
    {
        stats->jitter = 3.0f * (float) sqrt( stddev_rtt / num_jitter_samples );
    }
}

// ---------------------------------------------------------------

struct next_relay_stats_t
{
    int num_relays;
    uint64_t relay_ids[NEXT_MAX_NEAR_RELAYS];
    float relay_min_rtt[NEXT_MAX_NEAR_RELAYS];
    float relay_max_rtt[NEXT_MAX_NEAR_RELAYS];
    float relay_mean_rtt[NEXT_MAX_NEAR_RELAYS];
    float relay_jitter[NEXT_MAX_NEAR_RELAYS];
    float relay_packet_loss[NEXT_MAX_NEAR_RELAYS];
};

struct next_relay_manager_t
{
    void * context;
    next_stats_t * stats;
    int num_relays;
    uint64_t relay_ids[NEXT_MAX_NEAR_RELAYS];
    double relay_last_ping_time[NEXT_MAX_NEAR_RELAYS];
    next_address_t relay_addresses[NEXT_MAX_NEAR_RELAYS];
    next_ping_history_t * relay_ping_history[NEXT_MAX_NEAR_RELAYS];
    next_ping_history_t ping_history_array[NEXT_MAX_NEAR_RELAYS];
};

void next_relay_manager_reset( next_relay_manager_t * manager );

next_relay_manager_t * next_relay_manager_create( void * context, next_stats_t * stats )
{
    next_relay_manager_t * manager = (next_relay_manager_t*) next_malloc( context, sizeof(next_relay_manager_t) );
    if ( !manager ) 
        return NULL;
    manager->context = context;
    manager->stats = stats;
    next_relay_manager_reset( manager );
    return manager;
}

void next_relay_manager_reset( next_relay_manager_t * manager )
{
    next_assert( manager );
    manager->num_relays = 0;
    memset( manager->relay_ids, 0, sizeof(manager->relay_ids) );
    memset( manager->relay_last_ping_time, 0, sizeof(manager->relay_last_ping_time) );
    memset( manager->relay_addresses, 0, sizeof(manager->relay_addresses) );
    memset( manager->relay_ping_history, 0, sizeof(manager->relay_ping_history) );
    for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
    {
        next_ping_history_clear( &manager->ping_history_array[i] );
    }    
}

void next_relay_manager_update( next_relay_manager_t * manager, int num_relays, const uint64_t * relay_ids, const next_address_t * relay_addresses )
{
    next_assert( manager );
    next_assert( num_relays >= 0 );
    next_assert( num_relays <= NEXT_MAX_NEAR_RELAYS );
    next_assert( relay_ids );
    next_assert( relay_addresses );

    // first copy all current relays that are also in the updated relay relay list

    bool history_slot_taken[NEXT_MAX_NEAR_RELAYS];
    memset( history_slot_taken, 0, sizeof(history_slot_taken) );

    bool found[NEXT_MAX_NEAR_RELAYS];
    memset( found, 0, sizeof(found) );

    uint64_t new_relay_ids[NEXT_MAX_NEAR_RELAYS];
    double new_relay_last_ping_time[NEXT_MAX_NEAR_RELAYS];
    next_address_t new_relay_addresses[NEXT_MAX_NEAR_RELAYS];
    next_ping_history_t * new_relay_ping_history[NEXT_MAX_NEAR_RELAYS];

    int index = 0;

    for ( int i = 0; i < manager->num_relays; ++i )
    {
        for ( int j = 0; j < num_relays; ++j )
        {
            if ( manager->relay_ids[i] == relay_ids[j] )
            {
                found[j] = true;
                new_relay_ids[index] = manager->relay_ids[i];
                new_relay_last_ping_time[index] = manager->relay_last_ping_time[i];
                new_relay_addresses[index] = manager->relay_addresses[i];
                new_relay_ping_history[index] = manager->relay_ping_history[i];
                const int slot = manager->relay_ping_history[i] - manager->ping_history_array;
                next_assert( slot >= 0 );
                next_assert( slot < NEXT_MAX_NEAR_RELAYS );
                history_slot_taken[slot] = true;
                index++;
                break;
            }
        }
    }

    // now copy all near relays not found in the current relay list

    for ( int i = 0; i < num_relays; ++i )
    {
        if ( !found[i] )
        {
            new_relay_ids[index] = relay_ids[i];
            new_relay_last_ping_time[index] = -10000.0;
            new_relay_addresses[index] = relay_addresses[i];
            new_relay_ping_history[index] = NULL;
            for ( int j = 0; j < NEXT_MAX_NEAR_RELAYS; ++j )
            {
                if ( !history_slot_taken[j] )
                {
                    new_relay_ping_history[index] = &manager->ping_history_array[j];
                    next_ping_history_clear( new_relay_ping_history[index] );
                    history_slot_taken[j] = true;
                    break;
                }
            }
            next_assert( new_relay_ping_history[index] );
            index++;
        }                
    }

    // commit the updated relay array

    manager->num_relays = index;
    memcpy( manager->relay_ids, new_relay_ids, 8 * index );
    memcpy( manager->relay_last_ping_time, new_relay_last_ping_time, 8 * index );
    memcpy( manager->relay_addresses, new_relay_addresses, sizeof(next_address_t) * index );
    memcpy( manager->relay_ping_history, new_relay_ping_history, sizeof(next_ping_history_t*) * index );

    // make sure all ping times are evenly distributed to avoid clusters of ping packets

    double current_time = next_time();

    if ( manager->num_relays > 0 )
    {
        for ( int i = 0; i < manager->num_relays; ++i )
        {
            manager->relay_last_ping_time[i] = current_time - NEXT_RELAY_PING_TIME + i * NEXT_RELAY_PING_TIME / manager->num_relays;
        }
    }

#ifndef NDEBUG

    // make sure everything is correct

    next_assert( num_relays == index );

    int num_found = 0;
    for ( int i = 0; i < num_relays; ++i )
    {
        for ( int j = 0; j < manager->num_relays; ++j )
        {
            if ( relay_ids[i] == manager->relay_ids[j] && next_address_equal( &relay_addresses[i], &manager->relay_addresses[j] ) == 1 )
            {
                num_found++;
                break;
            }
        }
    }
    next_assert( num_found == num_relays );

    for ( int i = 0; i < num_relays; ++i )
    {
        for ( int j = 0; j < num_relays; ++j )
        {
            if ( i == j )
                continue;
            next_assert( manager->relay_ping_history[i] != manager->relay_ping_history[j] );
        }
    }

#endif // #ifndef DEBUG
}

void next_relay_manager_send_pings( next_relay_manager_t * manager, next_platform_socket_t * socket, uint64_t session_id )
{
    next_assert( manager );
    next_assert( socket );

    uint8_t packet_data[NEXT_MAX_PACKET_BYTES];

    double current_time = next_time();

    for ( int i = 0; i < manager->num_relays; ++i )
    {
        if ( manager->relay_last_ping_time[i] + NEXT_RELAY_PING_TIME <= current_time )
        {
            uint64_t ping_sequence = next_ping_history_ping_sent( manager->relay_ping_history[i], next_time() );

            NextRelayPingPacket packet;
            packet.ping_sequence = ping_sequence;
            packet.session_id = session_id;
            
            int packet_bytes = 0;

            if ( next_write_packet( NEXT_RELAY_PING_PACKET, &packet, packet_data, &packet_bytes, NULL, NULL, NULL ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_ERROR, "failed to write ping packet" );
                continue;
            }

            next_platform_socket_send_packet( socket, &manager->relay_addresses[i], packet_data, packet_bytes );

            manager->relay_last_ping_time[i] = current_time;

            char buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
            next_stats( manager->stats, "relay ping %" PRId64 " %s", ping_sequence, next_address_to_string( &manager->relay_addresses[i], buffer ) );
        }
    }
}

bool next_relay_manager_process_pong( next_relay_manager_t * manager, const next_address_t * from, uint64_t sequence )
{
    next_assert( manager );
    next_assert( from );

    for ( int i = 0; i < manager->num_relays; ++i )
    {
        if ( next_address_equal( from, &manager->relay_addresses[i] ) )
        {
            next_ping_history_pong_received( manager->relay_ping_history[i], sequence, next_time() );
            char buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
            next_stats( manager->stats, "relay pong %" PRId64 " %s", sequence, next_address_to_string( from, buffer ) );
            return true;
        }
    }

    return false;
}

void next_relay_manager_get_stats( next_relay_manager_t * manager, next_relay_stats_t * stats )
{
    next_assert( manager );
    next_assert( stats );

    double current_time = next_time();
    
    stats->num_relays = manager->num_relays;
    
    for ( int i = 0; i < stats->num_relays; ++i )
    {        
        next_route_stats_t route_stats;
        next_route_stats_from_ping_history( manager->relay_ping_history[i], current_time - NEXT_CLIENT_STATS_WINDOW, current_time, &route_stats, NEXT_PING_SAFETY );
        stats->relay_ids[i] = manager->relay_ids[i];
        stats->relay_min_rtt[i] = route_stats.min_rtt;
        stats->relay_max_rtt[i] = route_stats.max_rtt;
        stats->relay_mean_rtt[i] = route_stats.mean_rtt;
        stats->relay_jitter[i] = route_stats.jitter;
        stats->relay_packet_loss[i] = route_stats.packet_loss;
    }
}

void next_relay_manager_destroy( next_relay_manager_t * manager )
{
    next_free( manager->context, manager );
}

// ---------------------------------------------------------------

struct next_route_token_t
{
    uint64_t expire_timestamp;
    uint64_t session_id;
    uint8_t session_version;
    uint8_t session_flags;
    int kbps_up;
    int kbps_down;
    next_address_t next_address;
    uint8_t private_key[crypto_box_SECRETKEYBYTES];
};

void next_write_route_token( next_route_token_t * token, uint8_t * buffer, int buffer_length )
{
    (void) buffer_length;

    next_assert( token );
    next_assert( buffer );
    next_assert( buffer_length >= NEXT_ROUTE_TOKEN_BYTES );

    uint8_t * start = buffer;

    (void) start;

    next_write_uint64( &buffer, token->expire_timestamp );
    next_write_uint64( &buffer, token->session_id );
    next_write_uint8( &buffer, token->session_version );
    next_write_uint8( &buffer, token->session_flags );
    next_write_uint32( &buffer, token->kbps_up );
    next_write_uint32( &buffer, token->kbps_down );
    next_write_address( &buffer, &token->next_address );
    next_write_bytes( &buffer, token->private_key, crypto_box_SECRETKEYBYTES );

    next_assert( buffer - start == NEXT_ROUTE_TOKEN_BYTES );
}

void next_read_route_token( next_route_token_t * token, const uint8_t * buffer )
{
    next_assert( token );
    next_assert( buffer );

    const uint8_t * start = buffer;

    (void) start;   

    token->expire_timestamp = next_read_uint64( &buffer );
    token->session_id = next_read_uint64( &buffer );
    token->session_version = next_read_uint8( &buffer );
    token->session_flags = next_read_uint8( &buffer );
    token->kbps_up = next_read_uint32( &buffer );
    token->kbps_down = next_read_uint32( &buffer );
    next_read_address( &buffer, &token->next_address );
    next_read_bytes( &buffer, token->private_key, crypto_box_SECRETKEYBYTES );
    next_assert( buffer - start == NEXT_ROUTE_TOKEN_BYTES );
}

int next_encrypt_route_token( uint8_t * sender_private_key, uint8_t * receiver_public_key, uint8_t * nonce, uint8_t * buffer, int buffer_length )
{
    next_assert( sender_private_key );
    next_assert( receiver_public_key );
    next_assert( buffer );
    next_assert( buffer_length >= (int) ( NEXT_ROUTE_TOKEN_BYTES + crypto_box_MACBYTES ) );

    (void) buffer_length;

    if ( crypto_box_easy( buffer, buffer, NEXT_ROUTE_TOKEN_BYTES, nonce, receiver_public_key, sender_private_key ) != 0 )
    {
        return NEXT_ERROR;
    }

    return NEXT_OK;
}

int next_decrypt_route_token( const uint8_t * sender_public_key, const uint8_t * receiver_private_key, const uint8_t * nonce, uint8_t * buffer )
{
    next_assert( sender_public_key );
    next_assert( receiver_private_key );
    next_assert( buffer );

    if ( crypto_box_open_easy( buffer, buffer, NEXT_ROUTE_TOKEN_BYTES + crypto_box_MACBYTES, nonce, sender_public_key, receiver_private_key ) != 0 )
    {
        return NEXT_ERROR;
    }

    return NEXT_OK;
}

int next_write_encrypted_route_token( uint8_t ** buffer, next_route_token_t * token, uint8_t * sender_private_key, uint8_t * receiver_public_key )
{
    next_assert( buffer );
    next_assert( token );
    next_assert( sender_private_key );
    next_assert( receiver_public_key );

    unsigned char nonce[crypto_box_NONCEBYTES];
    next_random_bytes( nonce, crypto_box_NONCEBYTES );

    uint8_t * start = *buffer;

    (void) start;

    next_write_bytes( buffer, nonce, crypto_box_NONCEBYTES );

    next_write_route_token( token, *buffer, NEXT_ROUTE_TOKEN_BYTES );

    if ( next_encrypt_route_token( sender_private_key, receiver_public_key, nonce, *buffer, NEXT_ROUTE_TOKEN_BYTES + crypto_box_NONCEBYTES ) != NEXT_OK )
        return NEXT_ERROR;

    *buffer += NEXT_ROUTE_TOKEN_BYTES + crypto_box_MACBYTES;

    next_assert( ( *buffer - start ) == NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES );

    return NEXT_OK;
}

int next_read_encrypted_route_token( uint8_t ** buffer, next_route_token_t * token, const uint8_t * sender_public_key, const uint8_t * receiver_private_key )
{
    next_assert( buffer );
    next_assert( token );
    next_assert( sender_public_key );
    next_assert( receiver_private_key );

    const uint8_t * nonce = *buffer;

    *buffer += crypto_box_NONCEBYTES;

    if ( next_decrypt_route_token( sender_public_key, receiver_private_key, nonce, *buffer ) != NEXT_OK )
    {
        return NEXT_ERROR;
    }

    next_read_route_token( token, *buffer );

    *buffer += NEXT_ROUTE_TOKEN_BYTES + crypto_box_MACBYTES;

    return NEXT_OK;
}

// -----------------------------------------------------------

struct next_continue_token_t
{
    uint64_t expire_timestamp;
    uint64_t session_id;
    uint8_t session_version;
    uint8_t session_flags;
};

void next_write_continue_token( next_continue_token_t * token, uint8_t * buffer, int buffer_length )
{
    (void) buffer_length;

    next_assert( token );
    next_assert( buffer );
    next_assert( buffer_length >= NEXT_CONTINUE_TOKEN_BYTES );

    uint8_t * start = buffer;

    (void) start;

    next_write_uint64( &buffer, token->expire_timestamp );
    next_write_uint64( &buffer, token->session_id );
    next_write_uint8( &buffer, token->session_version );
    next_write_uint8( &buffer, token->session_flags );

    next_assert( buffer - start == NEXT_CONTINUE_TOKEN_BYTES );
}

void next_read_continue_token( next_continue_token_t * token, const uint8_t * buffer )
{
    next_assert( token );
    next_assert( buffer );

    const uint8_t * start = buffer;

    (void) start;

    token->expire_timestamp = next_read_uint64( &buffer );
    token->session_id = next_read_uint64( &buffer );
    token->session_version = next_read_uint8( &buffer );
    token->session_flags = next_read_uint8( &buffer );

    next_assert( buffer - start == NEXT_CONTINUE_TOKEN_BYTES );
}

int next_encrypt_continue_token( uint8_t * sender_private_key, uint8_t * receiver_public_key, uint8_t * nonce, uint8_t * buffer, int buffer_length )
{
    next_assert( sender_private_key );
    next_assert( receiver_public_key );
    next_assert( buffer );
    next_assert( buffer_length >= (int) ( NEXT_CONTINUE_TOKEN_BYTES + crypto_box_MACBYTES ) );

    (void) buffer_length;

    if ( crypto_box_easy( buffer, buffer, NEXT_CONTINUE_TOKEN_BYTES, nonce, receiver_public_key, sender_private_key ) != 0 )
    {
        return NEXT_ERROR;
    }

    return NEXT_OK;
}

int next_decrypt_continue_token( const uint8_t * sender_public_key, const uint8_t * receiver_private_key, const uint8_t * nonce, uint8_t * buffer )
{
    next_assert( sender_public_key );
    next_assert( receiver_private_key );
    next_assert( buffer );

    if ( crypto_box_open_easy( buffer, buffer, NEXT_CONTINUE_TOKEN_BYTES + crypto_box_MACBYTES, nonce, sender_public_key, receiver_private_key ) != 0 )
    {
        return NEXT_ERROR;
    }

    return NEXT_OK;
}

int next_write_encrypted_continue_token( uint8_t ** buffer, next_continue_token_t * token, uint8_t * sender_private_key, uint8_t * receiver_public_key )
{
    next_assert( buffer );
    next_assert( token );
    next_assert( sender_private_key );
    next_assert( receiver_public_key );

    unsigned char nonce[crypto_box_NONCEBYTES];
    next_random_bytes( nonce, crypto_box_NONCEBYTES );

    uint8_t * start = *buffer;

    next_write_bytes( buffer, nonce, crypto_box_NONCEBYTES );

    next_write_continue_token( token, *buffer, NEXT_CONTINUE_TOKEN_BYTES );

    if ( next_encrypt_continue_token( sender_private_key, receiver_public_key, nonce, *buffer, NEXT_CONTINUE_TOKEN_BYTES + crypto_box_NONCEBYTES ) != NEXT_OK )
        return NEXT_ERROR;

    *buffer += NEXT_CONTINUE_TOKEN_BYTES + crypto_box_MACBYTES;

    (void) start;

    next_assert( ( *buffer - start ) == NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES );

    return NEXT_OK;
}

int next_read_encrypted_continue_token( uint8_t ** buffer, next_continue_token_t * token, const uint8_t * sender_public_key, const uint8_t * receiver_private_key )
{
    next_assert( buffer );
    next_assert( token );
    next_assert( sender_public_key );
    next_assert( receiver_private_key );

    const uint8_t * nonce = *buffer;

    *buffer += crypto_box_NONCEBYTES;

    if ( next_decrypt_continue_token( sender_public_key, receiver_private_key, nonce, *buffer ) != NEXT_OK )
    {
        return NEXT_ERROR;
    }

    next_read_continue_token( token, *buffer );

    *buffer += NEXT_CONTINUE_TOKEN_BYTES + crypto_box_MACBYTES;

    return NEXT_OK;
}

// ---------------------------------------------------------------

#define NEXT_DIRECTION_CLIENT_TO_SERVER             0
#define NEXT_DIRECTION_SERVER_TO_CLIENT             1

int next_write_header( int direction, uint8_t type, uint64_t sequence, uint64_t session_id, uint8_t session_version, uint8_t session_flags, const uint8_t * private_key, uint8_t * buffer, int buffer_length )
{
    next_assert( private_key );
    next_assert( buffer );
    next_assert( NEXT_HEADER_BYTES <= buffer_length );

    (void) buffer_length;

    uint8_t * start = buffer;

    (void) start;

    if ( direction == NEXT_DIRECTION_SERVER_TO_CLIENT )
    {
        // high bit must be set
        next_assert( sequence & ( 1ULL << 63 ) );
    }
    else
    {
        // high bit must be clear
        next_assert( ( sequence & ( 1ULL << 63 ) ) == 0 );
    }

    if ( type == NEXT_PING_PACKET || type == NEXT_PONG_PACKET || type == NEXT_ROUTE_RESPONSE_PACKET || type == NEXT_CONTINUE_RESPONSE_PACKET )
    {
        // second highest bit must be set
        next_assert( sequence & ( 1ULL << 62 ) );
    }
    else
    {
        // second highest bit must be clear
        next_assert( ( sequence & ( 1ULL << 62 ) ) == 0 );
    }

    next_write_uint8( &buffer, type );

    next_write_uint64( &buffer, sequence );

    uint8_t * additional = buffer;
    const int additional_length = 8 + 2;

    next_write_uint64( &buffer, session_id );
    next_write_uint8( &buffer, session_version );
    next_write_uint8( &buffer, session_flags );

    uint8_t nonce[12];
    {
        uint8_t * p = nonce;
        next_write_uint32( &p, 0 );
        next_write_uint64( &p, sequence );
    }

    unsigned long long encrypted_length = 0;

    int result = crypto_aead_chacha20poly1305_ietf_encrypt( buffer, &encrypted_length,
                                                            buffer, 0,
                                                            additional, (unsigned long long) additional_length,
                                                            NULL, nonce, private_key );

    if ( result != 0 )
        return NEXT_ERROR;

    buffer += encrypted_length;

    next_assert( int( buffer - start ) == NEXT_HEADER_BYTES );

    return NEXT_OK;
}

int next_peek_header( int direction, uint8_t * type, uint64_t * sequence, uint64_t * session_id, uint8_t * session_version, uint8_t * session_flags, const uint8_t * buffer, int buffer_length )
{
    uint8_t packet_type;
    uint64_t packet_sequence;

    next_assert( buffer );

    if ( buffer_length < NEXT_HEADER_BYTES )
        return NEXT_ERROR;

    packet_type = next_read_uint8( &buffer );

    packet_sequence = next_read_uint64( &buffer );

    if ( direction == NEXT_DIRECTION_SERVER_TO_CLIENT )
    {
        // high bit must be set
        if ( !( packet_sequence & ( 1ULL << 63 ) ) )
            return NEXT_ERROR;
    }
    else
    {
        // high bit must be clear
        if ( packet_sequence & ( 1ULL << 63 ) )
            return NEXT_ERROR;
    }

    *type = packet_type;

    if ( *type == NEXT_PING_PACKET || *type == NEXT_PONG_PACKET || *type == NEXT_ROUTE_RESPONSE_PACKET || *type == NEXT_CONTINUE_RESPONSE_PACKET )
    {
        // second highest bit must be set
        next_assert( packet_sequence & ( 1ULL << 62 ) );
    }
    else
    {
        // second highest bit must be clear
        next_assert( ( packet_sequence & ( 1ULL << 62 ) ) == 0 );
    }


    *sequence = packet_sequence;
    *session_id = next_read_uint64( &buffer );
    *session_version = next_read_uint8( &buffer );
    *session_flags = next_read_uint8( &buffer );

    return NEXT_OK;
}

int next_read_header( int direction, uint8_t * type, uint64_t * sequence, uint64_t * session_id, uint8_t * session_version, uint8_t * session_flags, const uint8_t * private_key, uint8_t * buffer, int buffer_length )
{
    next_assert( private_key );
    next_assert( buffer );

    if ( buffer_length < NEXT_HEADER_BYTES )
    {
        return NEXT_ERROR;
    }

    const uint8_t * p = buffer;

    uint8_t packet_type = next_read_uint8( &p );

    uint64_t packet_sequence = next_read_uint64( &p );

    if ( direction == NEXT_DIRECTION_SERVER_TO_CLIENT )
    {
        // high bit must be set
        if ( !( packet_sequence & ( 1ULL <<  63) ) )
        {
            return NEXT_ERROR;
        }
    }
    else
    {
        // high bit must be clear
        if ( packet_sequence & ( 1ULL << 63 ) )
        {
            return NEXT_ERROR;
        }
    }

    if ( packet_type == NEXT_PING_PACKET || packet_type == NEXT_PONG_PACKET || packet_type == NEXT_ROUTE_RESPONSE_PACKET || packet_type == NEXT_CONTINUE_RESPONSE_PACKET )
    {
        // second highest bit must be set
        next_assert( packet_sequence & ( 1ULL << 62 ) );
    }
    else
    {
        // second highest bit must be clear
        next_assert( ( packet_sequence & ( 1ULL << 62 ) ) == 0 );
    }

    const uint8_t * additional = p;

    const int additional_length = 8 + 2;

    uint64_t packet_session_id = next_read_uint64( &p );
    uint8_t packet_session_version = next_read_uint8( &p );
    uint8_t packet_session_flags = next_read_uint8( &p );

    uint8_t nonce[12];
    {
        uint8_t * q = nonce;
        next_write_uint32( &q, 0 );
        next_write_uint64( &q, packet_sequence );
    }

    unsigned long long decrypted_length;

    int result = crypto_aead_chacha20poly1305_ietf_decrypt( buffer + 19, &decrypted_length, NULL,
                                                            buffer + 19, (unsigned long long) crypto_aead_chacha20poly1305_IETF_ABYTES,
                                                            additional, (unsigned long long) additional_length,
                                                            nonce, private_key );

    if ( result != 0 )
    {
        return NEXT_ERROR;
    }

    *type = packet_type;
    *sequence = packet_sequence;
    *session_id = packet_session_id;
    *session_version = packet_session_version;
    *session_flags = packet_session_flags;

    return NEXT_OK;
}

// ---------------------------------------------------------------

struct next_route_data_t
{
    double direct_route_expire_time;

    bool current_route;
    double current_route_expire_time;
    uint64_t current_route_session_id;
    uint8_t current_route_session_version;
    int current_route_kbps_up;
    int current_route_kbps_down;
    next_address_t current_route_next_address;
    uint8_t current_route_private_key[crypto_box_SECRETKEYBYTES];

    bool previous_route;
    uint64_t previous_route_session_id;
    uint8_t previous_route_session_version;
    uint8_t previous_route_private_key[crypto_box_SECRETKEYBYTES];

    bool pending_route;
    double pending_route_start_time;
    double pending_route_last_send_time;
    uint64_t pending_route_session_id;
    uint8_t pending_route_session_version;
    uint8_t pending_route_session_flags;
    int pending_route_kbps_up;
    int pending_route_kbps_down;
    int pending_route_request_packet_bytes;
    next_address_t pending_route_next_address;
    uint8_t pending_route_request_packet_data[NEXT_MAX_PACKET_BYTES];
    uint8_t pending_route_private_key[crypto_box_SECRETKEYBYTES];
    
    bool pending_continue;
    double pending_continue_start_time;
    double pending_continue_last_send_time;
    int pending_continue_request_packet_bytes;
    uint8_t pending_continue_request_packet_data[NEXT_MAX_PACKET_BYTES];
};

struct next_route_manager_t
{
    void * context;
    next_stats_t * stats;
    uint64_t send_sequence;
    bool fallback_to_direct;
    next_route_data_t route_data;
    uint32_t flags;
};

next_route_manager_t * next_route_manager_create( void * context, next_stats_t * stats )
{
    next_route_manager_t * route_manager = (next_route_manager_t*) next_malloc( context, sizeof(next_route_manager_t) );
    if ( !route_manager ) 
        return NULL;
    memset( route_manager, 0, sizeof(next_route_manager_t) );
    route_manager->context = context;
    route_manager->stats = stats;
    route_manager->route_data.direct_route_expire_time = -1.0;
    return route_manager;
}

void next_route_manager_reset( next_route_manager_t * route_manager )
{
    next_assert( route_manager );

    next_stats( route_manager->stats, "reset route manager" );

    route_manager->send_sequence = 0;
    route_manager->fallback_to_direct = false;
    memset( &route_manager->route_data, 0, sizeof(next_route_data_t) );
    route_manager->route_data.direct_route_expire_time = -1.0;
    route_manager->flags = 0;
}

void next_route_manager_fallback_to_direct( next_route_manager_t * route_manager, uint32_t flags )
{
    next_assert( route_manager );

    route_manager->flags |= flags;

    if ( route_manager->fallback_to_direct )
        return;

    route_manager->fallback_to_direct = true;

    next_stats( route_manager->stats, "fallback to direct" );

    next_printf( NEXT_LOG_LEVEL_INFO, "client fallback to direct" );

    route_manager->route_data.previous_route = route_manager->route_data.current_route;
    route_manager->route_data.previous_route_session_id = route_manager->route_data.current_route_session_id;
    route_manager->route_data.previous_route_session_version = route_manager->route_data.current_route_session_version;
    memcpy( route_manager->route_data.previous_route_private_key, route_manager->route_data.current_route_private_key, crypto_box_SECRETKEYBYTES );

    route_manager->route_data.current_route = false;
}

void next_route_manager_direct_route( next_route_manager_t * route_manager, bool quiet )
{
    next_assert( route_manager );

    if ( route_manager->fallback_to_direct )
        return;

    next_stats( route_manager->stats, "direct route" );

    if ( !quiet )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "client direct route" );
    }

    route_manager->route_data.previous_route = route_manager->route_data.current_route;
    route_manager->route_data.previous_route_session_id = route_manager->route_data.current_route_session_id;
    route_manager->route_data.previous_route_session_version = route_manager->route_data.current_route_session_version;
    memcpy( route_manager->route_data.previous_route_private_key, route_manager->route_data.current_route_private_key, crypto_box_SECRETKEYBYTES );

    route_manager->route_data.current_route = false;

    route_manager->route_data.direct_route_expire_time = next_time() + NEXT_DIRECT_ROUTE_EXPIRE_TIME;
}

void next_route_manager_begin_next_route( next_route_manager_t * route_manager, int num_tokens, uint8_t * tokens, const uint8_t * public_key, const uint8_t * private_key )
{
    next_assert( route_manager );
    next_assert( tokens );
    next_assert( num_tokens >= 2 );
    next_assert( num_tokens <= NEXT_MAX_TOKENS );

    if ( route_manager->fallback_to_direct )
        return;

    uint8_t * p = tokens;
    next_route_token_t route_token;
    if ( next_read_encrypted_route_token( &p, &route_token, public_key, private_key ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client received bad route token" );
        next_route_manager_fallback_to_direct( route_manager, NEXT_FLAGS_BAD_ROUTE_TOKEN );
        return;
    }

    next_stats( route_manager->stats, "next route" );

    next_printf( NEXT_LOG_LEVEL_DEBUG, "client next route" );

    route_manager->route_data.pending_route = true;
    route_manager->route_data.pending_route_start_time = next_time();
    route_manager->route_data.pending_route_last_send_time = -1000.0;
    route_manager->route_data.pending_route_next_address = route_token.next_address;
    route_manager->route_data.pending_route_session_id = route_token.session_id;
    route_manager->route_data.pending_route_session_version = route_token.session_version;
    route_manager->route_data.pending_route_session_flags = route_token.session_version;
    route_manager->route_data.pending_route_kbps_up = route_token.kbps_up;
    route_manager->route_data.pending_route_kbps_down = route_token.kbps_down;
    route_manager->route_data.pending_route_request_packet_bytes = 1 + ( num_tokens - 1 ) * NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES;
    route_manager->route_data.pending_route_request_packet_data[0] = NEXT_ROUTE_REQUEST_PACKET;
    memcpy( route_manager->route_data.pending_route_request_packet_data + 1, tokens + NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES, ( num_tokens - 1 ) * NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES );
    memcpy( route_manager->route_data.pending_route_private_key, route_token.private_key, crypto_box_SECRETKEYBYTES );
    next_assert( route_manager->route_data.pending_route_request_packet_bytes <= NEXT_MAX_PACKET_BYTES );
    route_manager->route_data.direct_route_expire_time = -1.0;
}

void next_route_manager_continue_next_route( next_route_manager_t * route_manager, int num_tokens, uint8_t * tokens, const uint8_t * public_key, const uint8_t * private_key )
{
    next_assert( route_manager );
    next_assert( tokens );
    next_assert( num_tokens >= 2 );
    next_assert( num_tokens <= NEXT_MAX_TOKENS );

    if ( route_manager->fallback_to_direct )
        return;

    next_stats( route_manager->stats, "continue route" );

    next_printf( NEXT_LOG_LEVEL_DEBUG, "client continue route" );

    if ( !route_manager->route_data.current_route )
    {
        next_stats( route_manager->stats, "no route to continue" );
        next_printf( NEXT_LOG_LEVEL_ERROR, "client has no route to continue" );
        next_route_manager_fallback_to_direct( route_manager, NEXT_FLAGS_NO_ROUTE_TO_CONTINUE );
        return;
    }

    if ( route_manager->route_data.pending_route || route_manager->route_data.pending_continue )
    {
        next_stats( route_manager->stats, "previous update still pending" );
        next_printf( NEXT_LOG_LEVEL_ERROR, "client previous update still pending" );
        next_route_manager_fallback_to_direct( route_manager, NEXT_FLAGS_PREVIOUS_UPDATE_STILL_PENDING );
        return;
    }

    uint8_t * p = tokens;
    next_continue_token_t continue_token;
    if ( next_read_encrypted_continue_token( &p, &continue_token, public_key, private_key ) != NEXT_OK )
    {
        next_stats( route_manager->stats, "bad continue token" );
        next_printf( NEXT_LOG_LEVEL_ERROR, "client received bad continue token" );
        next_route_manager_fallback_to_direct( route_manager, NEXT_FLAGS_BAD_CONTINUE_TOKEN );
        return;
    }

    route_manager->route_data.pending_continue = true;
    route_manager->route_data.pending_continue_start_time = next_time();
    route_manager->route_data.pending_continue_last_send_time = -1000.0;
    route_manager->route_data.pending_continue_request_packet_bytes = 1 + ( num_tokens - 1 ) * NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES;
    route_manager->route_data.pending_continue_request_packet_data[0] = NEXT_CONTINUE_REQUEST_PACKET;
    memcpy( route_manager->route_data.pending_continue_request_packet_data + 1, tokens + NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES, ( num_tokens - 1 ) * NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES );
    next_assert( route_manager->route_data.pending_continue_request_packet_bytes <= NEXT_MAX_PACKET_BYTES );
    route_manager->route_data.direct_route_expire_time = -1.0;
}

void next_route_manager_update( next_route_manager_t * route_manager, int update_type, int num_tokens, uint8_t * tokens, const uint8_t * public_key, const uint8_t * private_key )
{
    next_assert( route_manager );
    next_assert( public_key );
    next_assert( private_key );

    if ( update_type == NEXT_UPDATE_TYPE_DIRECT )
    {
        next_route_manager_direct_route( route_manager, false );
    }
    else if ( update_type == NEXT_UPDATE_TYPE_ROUTE )
    {
        next_route_manager_begin_next_route( route_manager, num_tokens, tokens, public_key, private_key );
    }
    else if ( update_type == NEXT_UPDATE_TYPE_CONTINUE )
    {
        next_route_manager_continue_next_route( route_manager, num_tokens, tokens, public_key, private_key );
    }
}

bool next_route_manager_has_network_next_route( next_route_manager_t * route_manager )
{
    next_assert( route_manager );
    return route_manager->route_data.current_route;
}

uint64_t next_route_manager_next_send_sequence( next_route_manager_t * route_manager )
{
    next_assert( route_manager );
    return route_manager->send_sequence++;
}

void next_route_manager_prepare_send_packet( next_route_manager_t * route_manager, uint64_t sequence, next_address_t * to, const uint8_t * payload_data, int payload_bytes, uint8_t * packet_data, int * packet_bytes )
{
    next_assert( route_manager );
    next_assert( route_manager->route_data.current_route );
    next_assert( to );
    next_assert( payload_data );
    next_assert( payload_bytes );
    next_assert( packet_data );
    next_assert( packet_bytes );
    next_assert( payload_bytes + NEXT_HEADER_BYTES <= NEXT_MAX_PACKET_BYTES );

    *to = route_manager->route_data.current_route_next_address;

    if ( next_write_header( NEXT_DIRECTION_CLIENT_TO_SERVER, NEXT_CLIENT_TO_SERVER_PACKET, sequence, route_manager->route_data.current_route_session_id, route_manager->route_data.current_route_session_version, 0, route_manager->route_data.current_route_private_key, packet_data, NEXT_MAX_PACKET_BYTES ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client failed to write client to server packet header" );
        return;
    }

    memcpy( packet_data + NEXT_HEADER_BYTES, payload_data, payload_bytes );

    *packet_bytes = NEXT_HEADER_BYTES + payload_bytes;
}

bool next_route_manager_process_server_to_client_packet( next_route_manager_t * route_manager, const next_address_t * from, uint8_t * packet_data, int packet_bytes, uint64_t * payload_sequence, uint8_t * payload_data, int * payload_bytes )
{
    next_assert( route_manager );
    next_assert( from );
    next_assert( packet_data );
    next_assert( payload_sequence );
    next_assert( payload_data );
    next_assert( payload_bytes );

    (void) from;

    if ( packet_bytes <= NEXT_HEADER_BYTES )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored server to client packet. too small" );
        return false;
    }


    if ( !route_manager->route_data.current_route && !route_manager->route_data.previous_route )
    {
        next_stats( route_manager->stats, "ignored next packet from server. no current or previous route" );
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored server to client packet. no current or previous route" );
        return false;
    }

    uint8_t packet_type = 0;
    uint64_t packet_sequence = 0;
    uint64_t packet_session_id = 0;
    uint8_t packet_session_version = 0;
    uint8_t packet_session_flags = 0;

    bool from_current_route = true;

    if ( next_read_header( NEXT_DIRECTION_SERVER_TO_CLIENT, &packet_type, &packet_sequence, &packet_session_id, &packet_session_version, &packet_session_flags, route_manager->route_data.current_route_private_key, packet_data, packet_bytes ) != NEXT_OK )
    {
        from_current_route = false;

        if ( next_read_header( NEXT_DIRECTION_SERVER_TO_CLIENT, &packet_type, &packet_sequence, &packet_session_id, &packet_session_version, &packet_session_flags, route_manager->route_data.previous_route_private_key, packet_data, packet_bytes ) != NEXT_OK )
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "client ignored server to client packet. could not read header" );
            return false;
        }
    }

    if ( from_current_route )
    {
        if ( packet_session_id != route_manager->route_data.current_route_session_id )
        {
            next_stats( route_manager->stats, "ignored next packet from server. session id mismatch (current route)" );
            next_printf( NEXT_LOG_LEVEL_WARN, "client ignored server to client packet. session id mismatch (current route)" );
            return false;
        }

        if ( packet_session_version != route_manager->route_data.current_route_session_version )
        {
            next_stats( route_manager->stats, "ignored next packet from server. session version mismatch (current route)" );
            next_printf( NEXT_LOG_LEVEL_WARN, "client ignored server to client packet. session version mismatch (current route)" );
            return false;
        }
    }
    else
    {
        if ( packet_session_id != route_manager->route_data.previous_route_session_id )
        {
            next_stats( route_manager->stats, "ignored next packet from server. session id mismatch (previous route)" );
            next_printf( NEXT_LOG_LEVEL_WARN, "client ignored server to client packet. session id mismatch (previous route)" );
            return false;
        }

        if ( packet_session_version != route_manager->route_data.previous_route_session_version )
        {
            next_stats( route_manager->stats, "ignored next packet from server. session version mismatch (previous route)" );
            next_printf( NEXT_LOG_LEVEL_WARN, "client ignored server to client packet. session version mismatch (previous route)" );
            return false;
        }
    }

    *payload_sequence = packet_sequence;

    *payload_bytes = packet_bytes - NEXT_HEADER_BYTES;

    if ( *payload_bytes > NEXT_MTU )
    {
        next_stats( route_manager->stats, "ignored next packet from server. too large (%d>%d)", *payload_bytes, NEXT_MTU );
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored server to client packet. too large (%d>%d)", *payload_bytes, NEXT_MTU );
        return false;
    }

    memcpy( payload_data, packet_data + NEXT_HEADER_BYTES, *payload_bytes );

    return true;
}

void next_route_manager_check_for_timeouts( next_route_manager_t * route_manager )
{
    next_assert( route_manager );

    if ( route_manager->fallback_to_direct )
        return;

    const double current_time = next_time();

    if ( route_manager->route_data.current_route && route_manager->route_data.current_route_expire_time <= current_time )
    {
        next_stats( route_manager->stats, "route expired" );
        next_printf( NEXT_LOG_LEVEL_ERROR, "client route expired" );
        next_route_manager_fallback_to_direct( route_manager, NEXT_FLAGS_ROUTE_EXPIRED );
        return;
    }
    
    if ( route_manager->route_data.pending_route && route_manager->route_data.pending_route_start_time + NEXT_ROUTE_REQUEST_TIMEOUT <= current_time )
    {
        next_stats( route_manager->stats, "route request timed out" );
        next_printf( NEXT_LOG_LEVEL_ERROR, "client route request timed out" );
        next_route_manager_fallback_to_direct( route_manager, NEXT_FLAGS_ROUTE_REQUEST_TIMED_OUT );
        return;
    }

    if ( route_manager->route_data.pending_continue && route_manager->route_data.pending_continue_start_time + NEXT_CONTINUE_REQUEST_TIMEOUT <= current_time )
    {
        next_stats( route_manager->stats, "continue request timed out" );
        next_printf( NEXT_LOG_LEVEL_ERROR, "client continue request timed out" );
        next_route_manager_fallback_to_direct( route_manager, NEXT_FLAGS_CONTINUE_REQUEST_TIMED_OUT );
        return;
    }

    if ( route_manager->route_data.direct_route_expire_time >= 0.0 && route_manager->route_data.direct_route_expire_time <= current_time )
    {
        next_stats( route_manager->stats, "direct route expired" );
        next_printf( NEXT_LOG_LEVEL_WARN, "client direct route expired" );
        next_route_manager_fallback_to_direct( route_manager, NEXT_FLAGS_DIRECT_ROUTE_EXPIRED );
        return;
    }
}

bool next_route_manager_send_route_request( next_route_manager_t * route_manager, next_address_t * to, uint8_t * packet_data, int * packet_bytes )
{
    next_assert( route_manager );
    next_assert( to );
    next_assert( packet_data );
    next_assert( packet_bytes );

    if ( route_manager->fallback_to_direct )
        return false;

    if ( !route_manager->route_data.pending_route )
        return false;

    double current_time = next_time();

    if ( route_manager->route_data.pending_route_last_send_time + NEXT_ROUTE_REQUEST_SEND_TIME > current_time )
        return false;

    *to = route_manager->route_data.pending_route_next_address;
    route_manager->route_data.pending_route_last_send_time = current_time;
    *packet_bytes = route_manager->route_data.pending_route_request_packet_bytes;
    memcpy( packet_data, route_manager->route_data.pending_route_request_packet_data, route_manager->route_data.pending_route_request_packet_bytes );

    return true;
}

bool next_route_manager_send_continue_request( next_route_manager_t * route_manager, next_address_t * to, uint8_t * packet_data, int * packet_bytes )
{
    next_assert( route_manager );
    next_assert( to );
    next_assert( packet_data );
    next_assert( packet_bytes );

    if ( route_manager->fallback_to_direct )
        return false;

    if ( !route_manager->route_data.current_route || !route_manager->route_data.pending_continue )
        return false;

    double current_time = next_time();

    if ( route_manager->route_data.pending_continue_last_send_time + NEXT_CONTINUE_REQUEST_SEND_TIME > current_time )
        return false;

    *to = route_manager->route_data.current_route_next_address;
    route_manager->route_data.pending_continue_last_send_time = current_time;
    *packet_bytes = route_manager->route_data.pending_continue_request_packet_bytes;
    memcpy( packet_data, route_manager->route_data.pending_continue_request_packet_data, route_manager->route_data.pending_continue_request_packet_bytes );

    return true;
}

void next_route_manager_process_route_response_packet( next_route_manager_t * route_manager, const next_address_t * from, uint8_t * packet_data, int packet_bytes, next_replay_protection_t * replay_protection )
{
    next_assert( route_manager );
    next_assert( from );
    next_assert( packet_data );
    next_assert( packet_bytes > 1 );
    next_assert( packet_bytes <= NEXT_MAX_PACKET_BYTES );

    (void) from;

    if ( route_manager->fallback_to_direct )
        return;

    if ( !route_manager->route_data.pending_route )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "client ignored route response packet. no pending route" );
        return;
    }

    if ( packet_bytes != NEXT_HEADER_BYTES )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored route response packet. wrong size" );
        return;
    }

    uint8_t packet_type = 0;
    uint64_t packet_sequence = 0;
    uint64_t packet_session_id = 0;
    uint8_t packet_session_version = 0;
    uint8_t packet_session_flags = 0;

    if ( next_read_header( NEXT_DIRECTION_SERVER_TO_CLIENT, &packet_type, &packet_sequence, &packet_session_id, &packet_session_version, &packet_session_flags, route_manager->route_data.pending_route_private_key, packet_data, packet_bytes ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored route response packet. could not read header" );
        return;
    }

    uint64_t clean_sequence = next_clean_sequence( packet_sequence );

    if ( next_replay_protection_already_received( replay_protection, clean_sequence ) )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored route response packet. sequence already received (%" PRIx64 " vs. %" PRIx64 ")", clean_sequence, replay_protection->most_recent_sequence );
        return;
    }

    next_assert( packet_type == NEXT_ROUTE_RESPONSE_PACKET );

    if ( packet_session_id != route_manager->route_data.pending_route_session_id )
    {
        next_stats( route_manager->stats, "ignored route response packet. session id mismatch" );
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored route response packet. session id mismatch" );
        return;
    }

    if ( packet_session_version != route_manager->route_data.pending_route_session_version )
    {
        next_stats( route_manager->stats, "ignored route response packet. session version mismatch" );
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored route response packet. session version mismatch" );
        return;
    }

    next_replay_protection_advance_sequence( replay_protection, clean_sequence );

    next_printf( NEXT_LOG_LEVEL_DEBUG, "client received route response" );

    if ( route_manager->route_data.current_route )
    {
        route_manager->route_data.previous_route = route_manager->route_data.current_route;
        route_manager->route_data.previous_route_session_id = route_manager->route_data.current_route_session_id;
        route_manager->route_data.previous_route_session_version = route_manager->route_data.current_route_session_version;
        memcpy( route_manager->route_data.previous_route_private_key, route_manager->route_data.current_route_private_key, crypto_box_SECRETKEYBYTES );
    }

    route_manager->route_data.current_route_session_id = route_manager->route_data.pending_route_session_id;
    route_manager->route_data.current_route_session_version = route_manager->route_data.pending_route_session_version;
    route_manager->route_data.current_route_kbps_up = route_manager->route_data.pending_route_kbps_up;
    route_manager->route_data.current_route_kbps_down = route_manager->route_data.pending_route_kbps_down;
    route_manager->route_data.current_route_next_address = route_manager->route_data.pending_route_next_address;
    memcpy( route_manager->route_data.current_route_private_key, route_manager->route_data.pending_route_private_key, crypto_box_SECRETKEYBYTES );

    if ( !route_manager->route_data.current_route )
    {
        route_manager->route_data.current_route_expire_time = route_manager->route_data.pending_route_start_time + 2 * NEXT_SLICE_SECONDS;
    }
    else
    {
        route_manager->route_data.current_route_expire_time += 2 * NEXT_SLICE_SECONDS;
    }

    route_manager->route_data.current_route = true;    
    route_manager->route_data.pending_route = false;
}

void next_route_manager_process_continue_response_packet( next_route_manager_t * route_manager, const next_address_t * from, uint8_t * packet_data, int packet_bytes, next_replay_protection_t * replay_protection )
{
    next_assert( route_manager );
    next_assert( from );
    next_assert( packet_data );
    next_assert( packet_bytes > 1 );
    next_assert( packet_bytes <= NEXT_MAX_PACKET_BYTES );

    (void) from;

    if ( route_manager->fallback_to_direct )
        return;

    if ( !route_manager->route_data.current_route )
        return;

    if ( !route_manager->route_data.pending_continue )
        return;

    if ( packet_bytes != NEXT_HEADER_BYTES )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored continue response packet. wrong size" );
        return;
    }

    uint8_t packet_type = 0;
    uint64_t packet_sequence = 0;
    uint64_t packet_session_id = 0;
    uint8_t packet_session_version = 0;
    uint8_t packet_session_flags = 0;

    if ( next_read_header( NEXT_DIRECTION_SERVER_TO_CLIENT, &packet_type, &packet_sequence, &packet_session_id, &packet_session_version, &packet_session_flags, route_manager->route_data.current_route_private_key, packet_data, packet_bytes ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored continue response packet. could not read header" );
        return;
    }

    uint64_t clean_sequence = next_clean_sequence( packet_sequence );

    if ( next_replay_protection_already_received( replay_protection, clean_sequence ) )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored continue response packet. sequence already received (%" PRIx64 " vs. %" PRIx64 ")", clean_sequence, replay_protection->most_recent_sequence );
        return;
    }

    next_assert( packet_type == NEXT_CONTINUE_RESPONSE_PACKET );

    if ( packet_session_id != route_manager->route_data.current_route_session_id )
    {
        next_stats( route_manager->stats, "ignored continue response packet. session id mismatch" );
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored continue response packet. session id mismatch" );
        return;
    }

    if ( packet_session_version != route_manager->route_data.current_route_session_version )
    {
        next_stats( route_manager->stats, "ignored continue response packet. session version mismatch" );
        next_printf( NEXT_LOG_LEVEL_WARN, "client ignored continue response packet. session version mismatch" );
        return;
    }

    next_replay_protection_advance_sequence( replay_protection, clean_sequence );

    next_printf( NEXT_LOG_LEVEL_DEBUG, "client received continue response" );

    route_manager->route_data.current_route_expire_time += NEXT_SLICE_SECONDS;

    route_manager->route_data.pending_continue = false;
}

void next_route_manager_destroy( next_route_manager_t * route_manager )
{
    next_assert( route_manager );
    next_free( route_manager->context, route_manager );
}

// ---------------------------------------------------------------

#define NEXT_CLIENT_COMMAND_OPEN_SESSION            0
#define NEXT_CLIENT_COMMAND_CLOSE_SESSION           1
#define NEXT_CLIENT_COMMAND_DESTROY                 2
#define NEXT_CLIENT_COMMAND_FLAG_SESSION            3

struct next_client_command_t
{
    int type;
};

struct next_client_command_open_session_t : public next_client_command_t
{
    next_address_t server_address;
};

struct next_client_command_close_session_t : public next_client_command_t
{
    // ...
};

struct next_client_command_destroy_t : public next_client_command_t
{
    // ...
};

struct next_client_command_flag_session_t : public next_client_command_t
{
    // ...
};

// ---------------------------------------------------------------

#define NEXT_CLIENT_NOTIFY_PACKET_RECEIVED          0
#define NEXT_CLIENT_NOTIFY_UPGRADED                 1
#define NEXT_CLIENT_NOTIFY_STATS_UPDATED            2

struct next_client_notify_t
{
    int type;
};

struct next_client_notify_packet_received_t : public next_client_notify_t
{
    int packet_bytes;
    uint8_t packet_data[NEXT_MTU];
};

struct next_client_notify_upgraded_t : public next_client_notify_t
{
    uint64_t session_id;
};

struct next_client_notify_downgraded_t : public next_client_notify_t
{
    uint64_t session_id;
};

struct next_client_notify_stats_updated_t : public next_client_notify_t
{
    next_client_stats_t stats;
    bool fallback_to_direct;
};

// ---------------------------------------------------------------

struct next_client_internal_t
{
    void * context;
    next_stats_t * stats;
    next_queue_t * command_queue;
    next_queue_t * notify_queue;
    next_platform_socket_t * socket;
    next_platform_mutex_t * command_mutex;
    next_platform_mutex_t * notify_mutex;
    next_address_t server_address;
    bool session_open;
    bool upgraded;
    bool flagged;
    bool fallback_to_direct;
    bool try_before_you_buy;
    bool multipath;
    uint8_t open_session_sequence;
    uint64_t upgrade_sequence;
    uint64_t session_id;
    uint64_t special_send_sequence;
    uint64_t internal_send_sequence;
    double last_next_ping_time;
    double first_next_ping_time;
    double last_direct_ping_time;
    double last_direct_pong_time;
    double last_stats_update_time;
    double last_stats_report_time;
    uint64_t route_update_sequence;
    next_relay_stats_t near_relay_stats;
    next_relay_manager_t * near_relay_manager;
    next_route_manager_t * route_manager;
    next_platform_mutex_t * route_manager_mutex;
    next_packet_loss_tracker_t packet_loss_tracker;
    uint8_t customer_public_key[crypto_sign_PUBLICKEYBYTES];
    uint8_t client_kx_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t client_kx_private_key[crypto_kx_SECRETKEYBYTES];
    uint8_t client_send_key[crypto_kx_SESSIONKEYBYTES];
    uint8_t client_receive_key[crypto_kx_SESSIONKEYBYTES];
    uint8_t client_route_public_key[crypto_box_PUBLICKEYBYTES];
    uint8_t client_route_private_key[crypto_box_SECRETKEYBYTES];
    next_ping_history_t next_ping_history;
    next_ping_history_t direct_ping_history;
    next_client_stats_t client_stats;
    next_replay_protection_t payload_replay_protection;
    next_replay_protection_t special_replay_protection;
    next_replay_protection_t internal_replay_protection;
    next_platform_mutex_t * bandwidth_mutex;
    bool bandwidth_over_budget;
    float bandwidth_usage_kbps_up;
    float bandwidth_usage_kbps_down;
    float bandwidth_envelope_kbps_up;
    float bandwidth_envelope_kbps_down;
    uint64_t counters[NEXT_CLIENT_COUNTER_MAX];
};

void next_client_internal_destroy( next_client_internal_t * client );

next_client_internal_t * next_client_internal_create( void * context, next_stats_t * stats )
{
    #ifdef NEXT_VERSION_IS_PRESENT
    next_printf( NEXT_LOG_LEVEL_INFO, "client sdk version is %s", NEXT_VERSION_FULL );
    #endif // #ifdef NEXT_VERSION_IS_PRESENT

    next_client_internal_t * client = (next_client_internal_t*) next_malloc( context, sizeof(next_client_internal_t) );
    if ( !client ) 
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "could not create internal client" );
        return NULL;
    }

    memset( client, 0, sizeof( next_client_internal_t) );

    client->context = context;
    client->stats = stats;

    memcpy( client->customer_public_key, next_global_config.customer_public_key, crypto_sign_PUBLICKEYBYTES );

    client->command_queue = next_queue_create( context, NEXT_COMMAND_QUEUE_LENGTH );
    if ( !client->command_queue )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create client command queue" );
        next_client_internal_destroy( client );
        return NULL;
    }
    
    client->notify_queue = next_queue_create( context, NEXT_NOTIFY_QUEUE_LENGTH );
    if ( !client->notify_queue )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create client notify queue" );
        next_client_internal_destroy( client );
        return NULL;
    }

    next_address_t bind_address;
    memset( &bind_address, 0, sizeof(bind_address) );
    bind_address.type = NEXT_ADDRESS_IPV4;
    bind_address.port = 0;

    client->socket = next_platform_socket_create( client->context, &bind_address, NEXT_PLATFORM_SOCKET_BLOCKING, 0.1f, next_global_config.socket_send_buffer_size, next_global_config.socket_receive_buffer_size );
    if ( client->socket == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create socket" );
        next_client_internal_destroy( client );
        return NULL;
    }

    client->command_mutex = next_platform_mutex_create( client->context );
    if ( client->command_mutex == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create command mutex" );
        next_client_internal_destroy( client );
        return NULL;
    }

    client->notify_mutex = next_platform_mutex_create( client->context );
    if ( client->notify_mutex == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create notify mutex" );
        next_client_internal_destroy( client );
        return NULL;
    }

    client->near_relay_manager = next_relay_manager_create( context, client->stats );
    if ( !client->near_relay_manager )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create near relay manager" );
        next_client_internal_destroy( client );
        return NULL;
    }

    client->route_manager = next_route_manager_create( context, client->stats );
    if ( !client->route_manager )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create route manager" );
        next_client_internal_destroy( client );
        return NULL;
    }

    client->route_manager_mutex = next_platform_mutex_create( client->context );
    if ( !client->route_manager_mutex )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create client route manager mutex" );
        next_client_internal_destroy( client );
        return NULL;
    }

    client->bandwidth_mutex = next_platform_mutex_create( client->context );
    if ( client->bandwidth_mutex == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create bandwidth mutex" );
        next_client_internal_destroy( client );
        return NULL;
    }

    next_ping_history_clear( &client->next_ping_history );

    next_ping_history_clear( &client->direct_ping_history );

    next_replay_protection_reset( &client->payload_replay_protection );
    next_replay_protection_reset( &client->special_replay_protection );
    next_replay_protection_reset( &client->internal_replay_protection );

    return client;
}

void next_client_internal_destroy( next_client_internal_t * client )
{
    next_assert( client );
    if ( client->socket )
    {
        next_platform_socket_destroy( client->socket );
    }
    if ( client->command_mutex )
    {
        next_platform_mutex_destroy( client->command_mutex );
    }
    if ( client->notify_mutex )
    {
        next_platform_mutex_destroy( client->notify_mutex );
    }
    if ( client->command_queue )
    {
        next_queue_destroy( client->command_queue );
    }
    if ( client->notify_queue )
    {
        next_queue_destroy( client->notify_queue );
    }
    if ( client->near_relay_manager )
    {
        next_relay_manager_destroy( client->near_relay_manager );
    }
    if ( client->route_manager )
    {
        next_route_manager_destroy( client->route_manager );
    }
    if ( client->route_manager_mutex )
    {
        next_platform_mutex_destroy( client->route_manager_mutex );
    }
    if ( client->bandwidth_mutex )
    {
        next_platform_mutex_destroy( client->bandwidth_mutex );
    }
    clear_and_free( client->context, client, sizeof(next_client_internal_t) );
}

int next_client_internal_send_packet_to_server( next_client_internal_t * client, uint8_t packet_id, void * packet_object )
{
    next_assert( client );
    next_assert( packet_object );
    next_assert( client->session_open );
    if ( !client->session_open )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client can't send internal packet to server because no session is open" );
        return NEXT_ERROR;
    }
    int packet_bytes = 0;
    uint8_t buffer[NEXT_MAX_PACKET_BYTES];
    if ( next_write_packet( packet_id, packet_object, buffer, &packet_bytes, next_encrypted_packets, &client->internal_send_sequence, client->client_send_key ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client failed to write internal packet type %d", packet_id );
        return NEXT_ERROR;
    }
    next_platform_socket_send_packet( client->socket, &client->server_address, buffer, packet_bytes );
    return NEXT_OK;
}

int next_client_internal_process_packet_from_server( next_client_internal_t * client, const next_address_t * from, uint8_t * packet_data, int packet_bytes )
{
    next_assert( client );
    next_assert( from );
    next_assert( packet_data );

    int packet_id = packet_data[0];

    if ( next_encrypted_packets[packet_id] && !client->upgraded )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "client can't decrypt packet. not upgraded yet" );
        return NEXT_ERROR;
    }

    switch ( packet_id )
    {
        case NEXT_UPGRADE_REQUEST_PACKET:
        {
            NextUpgradeRequestPacket packet;

            if ( next_read_packet( packet_data, packet_bytes, &packet, NULL, NULL, NULL, NULL ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade request packet. failed to read" );
                return NEXT_ERROR;
            }

            if ( !next_address_equal( from, &client->server_address ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade request packet. packet does not come from server" );
                return NEXT_ERROR;
            }

            if ( packet.protocol_version != next_protocol_version() )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade request packet. protocol version mismatch" );
                return NEXT_ERROR;
            }

            if ( !next_address_equal( &client->server_address, &packet.server_address ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade request packet. packet server address does not match client server address" );
                return NEXT_ERROR;
            }

            if ( !packet.Verify( client->customer_public_key ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade request packet. did not verify" );
                return NEXT_ERROR;
            }

            if ( next_global_config.disable_network_next )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade request packet. network next is disabled" );
                return NEXT_ERROR;
            }

            if ( client->fallback_to_direct )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade request packet. fallback to direct" );
                return NEXT_ERROR;
            }

            next_post_validate_packet( packet_data, packet_bytes, &packet, NULL, NULL, NULL, NULL, NULL );

            next_printf( NEXT_LOG_LEVEL_DEBUG, "client received upgrade request packet" );

            NextUpgradeResponsePacket response;
            response.client_open_session_sequence = client->open_session_sequence;
            memcpy( response.client_kx_public_key, client->client_kx_public_key, crypto_kx_PUBLICKEYBYTES );
            memcpy( response.client_route_public_key, client->client_route_public_key, crypto_box_PUBLICKEYBYTES );
            memcpy( response.upgrade_token, packet.upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );

            if ( next_client_internal_send_packet_to_server( client, NEXT_UPGRADE_RESPONSE_PACKET, &response ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client failed to send upgrade response packet to server" );
                return NEXT_ERROR;
            }

            next_printf( NEXT_LOG_LEVEL_DEBUG, "client sent upgrade response packet" );

            return NEXT_OK;
        }
        break;

        case NEXT_UPGRADE_CONFIRM_PACKET:
        {
            NextUpgradeConfirmPacket packet;

            if ( next_read_packet( packet_data, packet_bytes, &packet, NULL, NULL, NULL, NULL ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade confirm packet. failed to read" );
                return NEXT_ERROR;
            }

            if ( !next_address_equal( from, &client->server_address ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade confirm packet. packet does not come from server" );
                return NEXT_ERROR;
            }

            if ( !packet.Verify( client->customer_public_key ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade confirm packet. did not verify" );
                return NEXT_ERROR;
            }

            if ( client->fallback_to_direct )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade request packet. fallback to direct" );
                return NEXT_ERROR;
            }

            if ( memcmp( packet.client_kx_public_key, client->client_kx_public_key, crypto_kx_PUBLICKEYBYTES ) != 0 )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade confirm packet. client public key does not match" );
                return NEXT_ERROR;
            }

            if ( client->upgraded && client->upgrade_sequence >= packet.upgrade_sequence )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade confirm packet. client already upgraded" );
                return NEXT_ERROR;
            }

            uint8_t client_send_key[crypto_kx_SESSIONKEYBYTES];
            uint8_t client_receive_key[crypto_kx_SESSIONKEYBYTES];
            if ( crypto_kx_client_session_keys( client_receive_key, client_send_key, client->client_kx_public_key, client->client_kx_private_key, packet.server_kx_public_key ) != 0 )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored upgrade confirm packet. could not generate session keys from server public key" );
                return NEXT_ERROR;
            }

            next_post_validate_packet( packet_data, packet_bytes, &packet, NULL, NULL, NULL, NULL, NULL );

            client->upgraded = true;
            client->upgrade_sequence = packet.upgrade_sequence;
            client->session_id = packet.session_id;
            client->last_direct_pong_time = next_time();
            memcpy( client->client_send_key, client_send_key, crypto_kx_SESSIONKEYBYTES );
            memcpy( client->client_receive_key, client_receive_key, crypto_kx_SESSIONKEYBYTES );

            next_client_notify_upgraded_t * notify = (next_client_notify_upgraded_t*) next_malloc( client->context, sizeof(next_client_notify_upgraded_t) );
            next_assert( notify );
            notify->type = NEXT_CLIENT_NOTIFY_UPGRADED;
            notify->session_id = client->session_id;
            {
                next_mutex_guard( client->notify_mutex );
                next_queue_push( client->notify_queue, notify );
            }

            client->counters[NEXT_CLIENT_COUNTER_UPGRADE_SESSION]++;

            return NEXT_OK;
        }
        break;

        case NEXT_DIRECT_PONG_PACKET:
        {
            NextDirectPongPacket packet;

            uint64_t packet_sequence = 0;

            if ( next_read_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, client->client_receive_key, &client->internal_replay_protection ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored direct pong packet. failed to read" );
                return NEXT_ERROR;
            }

            if ( !next_address_equal( from, &client->server_address ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored direct pong packet. does not come from server" );
                return NEXT_ERROR;
            }

            next_ping_history_pong_received( &client->direct_ping_history, packet.ping_sequence, next_time() );

            next_post_validate_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, client->client_receive_key, &client->internal_replay_protection, &client->packet_loss_tracker );

            client->last_direct_pong_time = next_time();

            next_stats( client->stats, "direct pong %" PRId64, packet_sequence );
        }
        break;        

        case NEXT_ROUTE_UPDATE_PACKET:
        {
            if ( client->fallback_to_direct )
                return NEXT_OK;

            NextRouteUpdatePacket packet;

            uint64_t packet_sequence = 0;

            if ( next_read_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, client->client_receive_key, &client->internal_replay_protection ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored route update packet. failed to read" );
                return NEXT_ERROR;
            }

            if ( !next_address_equal( from, &client->server_address ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored route update packet. does not come from server" );
                return NEXT_ERROR;
            }

            if ( packet.sequence < client->route_update_sequence )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored route update packet. sequence is old" );
                return NEXT_ERROR;
            }

            next_post_validate_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, client->client_receive_key, &client->internal_replay_protection, &client->packet_loss_tracker );

            bool fallback_to_direct = false;

            if ( packet.sequence != client->route_update_sequence )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "client received route update" );

                next_relay_manager_update( client->near_relay_manager, packet.num_near_relays, packet.near_relay_ids, packet.near_relay_addresses );

                next_platform_mutex_acquire( client->route_manager_mutex );
                next_route_manager_update( client->route_manager, packet.update_type, packet.num_tokens, packet.tokens, next_router_public_key, client->client_route_private_key );
                fallback_to_direct = client->route_manager->fallback_to_direct;
                next_platform_mutex_release( client->route_manager_mutex );

                if ( !client->fallback_to_direct && fallback_to_direct )
                {
                    client->counters[NEXT_CLIENT_COUNTER_FALLBACK_TO_DIRECT]++;
                }

                if ( packet.multipath && !client->multipath )
                {
                    next_printf( NEXT_LOG_LEVEL_INFO, "client multipath enabled" );
                    client->multipath = true;
                    client->counters[NEXT_CLIENT_COUNTER_MULTIPATH]++;
                }

                uint64_t packets_lost = packet.packets_lost_client_to_server - client->client_stats.packets_lost_client_to_server;
                if ( packets_lost > 0 )
                {
                    next_stats( client->stats, "%" PRIu64 " client -> server packets lost", packets_lost );
                }

                client->fallback_to_direct = fallback_to_direct;
                client->route_update_sequence = packet.sequence;
                client->client_stats.packets_lost_client_to_server = packet.packets_lost_client_to_server;
                client->counters[NEXT_CLIENT_COUNTER_PACKETS_LOST_CLIENT_TO_SERVER] = packet.packets_lost_client_to_server;
            }

            if ( fallback_to_direct )
                return NEXT_ERROR;

            NextRouteUpdateAckPacket ack;
            ack.sequence = packet.sequence;

            if ( next_client_internal_send_packet_to_server( client, NEXT_ROUTE_UPDATE_ACK_PACKET, &ack ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client failed to send route update ack packet to server" );
                return NEXT_ERROR;
            }

            next_printf( NEXT_LOG_LEVEL_DEBUG, "client acked route update" );
        }
        break;        

        default: break;
    }

    return NEXT_ERROR;
}

int next_client_internal_process_network_next_packet( next_client_internal_t * client, const next_address_t * from, uint8_t * packet_data, int packet_bytes, bool multipath )
{
    next_assert( client );
    next_assert( from );
    next_assert( packet_data );

    int packet_id = packet_data[0];

    switch ( packet_id )
    {
        case NEXT_RELAY_PONG_PACKET:
        {
            if ( !client->upgraded )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored relay pong packet. not upgraded yet" );
                return NEXT_ERROR;
            }            

            NextRelayPongPacket packet;

            if ( next_read_packet( packet_data, packet_bytes, &packet, NULL, NULL, NULL, NULL ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored relay pong packet. failed to read" );
                return NEXT_ERROR;
            }

            if ( packet.session_id != client->session_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignoring relay pong packet. session id does not match" );
                return NEXT_ERROR;
            }

            next_post_validate_packet( packet_data, packet_bytes, &packet, NULL, NULL, NULL, NULL, NULL );

            next_relay_manager_process_pong( client->near_relay_manager, from, packet.ping_sequence );
        }
        break;        

        case NEXT_ROUTE_RESPONSE_PACKET:
        {
            next_platform_mutex_acquire( client->route_manager_mutex );
            next_route_manager_process_route_response_packet( client->route_manager, from, packet_data, packet_bytes, &client->special_replay_protection );
            bool route_established = client->route_manager->route_data.current_route;
            int route_kbps_up = client->route_manager->route_data.current_route_kbps_up;
            int route_kbps_down = client->route_manager->route_data.current_route_kbps_down;
            next_platform_mutex_release( client->route_manager_mutex );

            next_platform_mutex_acquire( client->bandwidth_mutex );
            if ( route_established )
            {
                client->bandwidth_envelope_kbps_up = route_kbps_up;
                client->bandwidth_envelope_kbps_down = route_kbps_down;
            }
            else
            {
                client->bandwidth_envelope_kbps_up = 0;
                client->bandwidth_envelope_kbps_down = 0;
            }
            next_platform_mutex_release( client->bandwidth_mutex );

            return NEXT_OK;
        }
        break;

        case NEXT_CONTINUE_RESPONSE_PACKET:
        {
            next_platform_mutex_acquire( client->route_manager_mutex );
            next_route_manager_process_continue_response_packet( client->route_manager, from, packet_data, packet_bytes, &client->special_replay_protection );
            next_platform_mutex_release( client->route_manager_mutex );
            return NEXT_OK;
        }
        break;

        case NEXT_SERVER_TO_CLIENT_PACKET:
        {
            int payload_bytes = 0;
            uint64_t payload_sequence = 0;
            uint8_t payload_data[NEXT_MTU];

            next_platform_mutex_acquire( client->route_manager_mutex );
            const bool result = next_route_manager_process_server_to_client_packet( client->route_manager, from, packet_data, packet_bytes, &payload_sequence, payload_data, &payload_bytes );
            next_platform_mutex_release( client->route_manager_mutex );

            if ( !result )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored server to client packet. could not verify" );
                return NEXT_ERROR;
            }

            const bool already_received = next_replay_protection_already_received( &client->payload_replay_protection, payload_sequence ) != 0;

            if ( already_received && !multipath )
            {
                next_stats( client->stats, "next duplicate packet %" PRIu64, payload_sequence );
                next_printf( NEXT_LOG_LEVEL_WARN, "client already received server to client packet %" PRIu64, payload_sequence );
                return NEXT_ERROR;
            }

            if ( already_received && multipath )
            {
                client->counters[NEXT_CLIENT_COUNTER_PACKET_RECEIVED_NEXT]++;
                return NEXT_OK;
            }

            uint64_t clean_sequence = next_clean_sequence( payload_sequence );

            next_replay_protection_advance_sequence( &client->payload_replay_protection, clean_sequence );
    
            next_packet_loss_tracker_packet_received( &client->packet_loss_tracker, clean_sequence );

            next_client_notify_packet_received_t * notify = (next_client_notify_packet_received_t*) next_malloc( client->context, sizeof( next_client_notify_packet_received_t ) );
            notify->type = NEXT_CLIENT_NOTIFY_PACKET_RECEIVED;
            notify->packet_bytes = payload_bytes;
            memcpy( notify->packet_data, payload_data, payload_bytes );
            {
                next_mutex_guard( client->notify_mutex );
                next_queue_push( client->notify_queue, notify );            
            }

            client->counters[NEXT_CLIENT_COUNTER_PACKET_RECEIVED_NEXT]++;

            return NEXT_OK;
        }
        break;

        case NEXT_PONG_PACKET:
        {
            int payload_bytes = 0;
            uint64_t payload_sequence = 0;
            uint8_t payload_data[NEXT_MTU];

            next_platform_mutex_acquire( client->route_manager_mutex );
            const bool result = next_route_manager_process_server_to_client_packet( client->route_manager, from, packet_data, packet_bytes, &payload_sequence, payload_data, &payload_bytes );
            next_platform_mutex_release( client->route_manager_mutex );

            if ( !result )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client ignored pong packet. could not verify" );
                return NEXT_ERROR;
            }

            uint64_t clean_sequence = next_clean_sequence( payload_sequence );

            if ( next_replay_protection_already_received( &client->special_replay_protection, clean_sequence ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "client already received pong packet %" PRIu64, clean_sequence );
                return NEXT_ERROR;
            }

            next_replay_protection_advance_sequence( &client->special_replay_protection, clean_sequence );

            const uint8_t * p = packet_data + NEXT_HEADER_BYTES;

            uint64_t ping_sequence = next_read_uint64( &p );

            next_ping_history_pong_received( &client->next_ping_history, ping_sequence, next_time() );

            next_stats( client->stats, "next pong %" PRId64, ping_sequence );

            return NEXT_OK;
        }
        break;
    }

    return NEXT_ERROR;
}

void next_client_internal_block_and_receive_packet( next_client_internal_t * client )
{
    uint8_t packet_data[NEXT_MAX_PACKET_BYTES];

    next_address_t from;
    
    int packet_bytes = next_platform_socket_receive_packet( client->socket, &from, packet_data, sizeof(packet_data) );
    if ( packet_bytes == 0 )
        return;

    const bool from_server_address = client->server_address.type != 0 && next_address_equal( &from, &client->server_address );

    if ( packet_data[0] == 0 && packet_bytes <= NEXT_MTU + 1 && from_server_address )
    {
        next_client_notify_packet_received_t * notify = (next_client_notify_packet_received_t*) next_malloc( client->context, sizeof( next_client_notify_packet_received_t ) );
        notify->type = NEXT_CLIENT_NOTIFY_PACKET_RECEIVED;
        notify->packet_bytes = packet_bytes - 1;
        memcpy( notify->packet_data, packet_data + 1, packet_bytes - 1 );
        {
            next_mutex_guard( client->notify_mutex );
            next_queue_push( client->notify_queue, notify );            
        }
        client->counters[NEXT_CLIENT_COUNTER_PACKET_RECEIVED_DIRECT]++;
    }
    else if ( packet_data[0] == 255 && packet_bytes <= NEXT_MTU + 10 && from_server_address )
    {
        const uint8_t * p = packet_data + 1;
        uint8_t packet_session_sequence = next_read_uint8( &p );
        if ( packet_session_sequence != client->open_session_sequence )
        {
            next_printf( NEXT_LOG_LEVEL_DEBUG, "client ignored direct packet. session mismatch" );
            return;
        }
        uint64_t packet_sequence = next_read_uint64( &p );
        uint64_t clean_sequence = next_clean_sequence( packet_sequence );
        if ( next_replay_protection_already_received( &client->payload_replay_protection, clean_sequence ) )
        {
            if ( !client->multipath )
            {
                next_stats( client->stats, "direct duplicate packet %" PRIu64, clean_sequence );
                next_printf( NEXT_LOG_LEVEL_WARN, "client already received direct packet %" PRIu64 " (%" PRIu64 ")", clean_sequence, client->payload_replay_protection.most_recent_sequence );
            }
            return;
        }
        next_client_notify_packet_received_t * notify = (next_client_notify_packet_received_t*) next_malloc( client->context, sizeof( next_client_notify_packet_received_t ) );
        notify->type = NEXT_CLIENT_NOTIFY_PACKET_RECEIVED;
        notify->packet_bytes = packet_bytes - 10;
        memcpy( notify->packet_data, packet_data + 10, packet_bytes - 10 );
        {
            next_mutex_guard( client->notify_mutex );
            next_queue_push( client->notify_queue, notify );            
        }
        client->counters[NEXT_CLIENT_COUNTER_PACKET_RECEIVED_DIRECT]++;
        next_replay_protection_advance_sequence( &client->payload_replay_protection, clean_sequence );
        next_packet_loss_tracker_packet_received( &client->packet_loss_tracker, clean_sequence );
    }
    else
    {
        if ( from_server_address )
        {
            next_client_internal_process_packet_from_server( client, &from, packet_data, packet_bytes );
        }
        else
        {
            next_client_internal_process_network_next_packet( client, &from, packet_data, packet_bytes, client->multipath );
        }
    }
}

bool next_client_internal_pump_commands( next_client_internal_t * client )
{
    bool quit = false;

    while ( true )
    {
        void * entry = NULL;
        {
            next_mutex_guard( client->command_mutex );
            entry = next_queue_pop( client->command_queue );
        }

        if ( entry == NULL )
            break;

        next_client_command_t * command = (next_client_command_t*) entry;

        switch ( command->type )
        {
            case NEXT_CLIENT_COMMAND_OPEN_SESSION:
            {
                next_client_command_open_session_t * open_session_command = (next_client_command_open_session_t*) entry;
                client->try_before_you_buy = next_global_config.try_before_you_buy;
                client->server_address = open_session_command->server_address;
                client->session_open = true;
                client->open_session_sequence++;
                client->last_direct_ping_time = next_time();
                client->last_stats_update_time = next_time(); 
                client->last_stats_report_time = next_time() + next_random_float();
                crypto_kx_keypair( client->client_kx_public_key, client->client_kx_private_key );
                crypto_box_keypair( client->client_route_public_key, client->client_route_private_key );
                char buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
                next_printf( NEXT_LOG_LEVEL_INFO, "client opened session to %s", next_address_to_string( &open_session_command->server_address, buffer ) );
                client->counters[NEXT_CLIENT_COUNTER_OPEN_SESSION]++;
                next_platform_mutex_acquire( client->route_manager_mutex );
                next_route_manager_reset( client->route_manager );
                next_route_manager_direct_route( client->route_manager, true );
                next_platform_mutex_release( client->route_manager_mutex );
            }
            break;

            case NEXT_CLIENT_COMMAND_CLOSE_SESSION:
            {
                if ( !client->session_open )
                    break;

                char buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
                next_printf( NEXT_LOG_LEVEL_INFO, "client closed session to %s", next_address_to_string( &client->server_address, buffer ) );

                memset( &client->server_address, 0, sizeof(next_address_t) );
                client->session_open = false;
                client->upgraded = false;
                client->flagged = false;
                client->fallback_to_direct = false;
                client->try_before_you_buy = false;
                client->multipath = false;
                client->upgrade_sequence = 0;
                client->session_id = 0;
                client->internal_send_sequence = 0;
                client->last_next_ping_time = 0.0;
                client->first_next_ping_time = 0.0;
                client->last_direct_ping_time = 0.0;
                client->last_direct_pong_time = 0.0;
                client->last_stats_update_time = 0.0;
                client->last_stats_report_time = 0.0;
                client->route_update_sequence = 0;
                memset( &client->near_relay_stats, 0, sizeof(next_relay_stats_t ) );
                next_relay_manager_reset( client->near_relay_manager );
                memset( client->client_kx_public_key, 0, crypto_kx_PUBLICKEYBYTES );
                memset( client->client_kx_private_key, 0, crypto_kx_SECRETKEYBYTES );
                memset( client->client_send_key, 0, crypto_kx_SESSIONKEYBYTES );
                memset( client->client_receive_key, 0, crypto_kx_SESSIONKEYBYTES );
                memset( client->client_route_public_key, 0, crypto_box_PUBLICKEYBYTES );
                memset( client->client_route_private_key, 0, crypto_box_SECRETKEYBYTES );
                next_ping_history_clear( &client->next_ping_history );
                next_ping_history_clear( &client->direct_ping_history );
                memset( &client->client_stats, 0, sizeof(next_client_stats_t) );
                next_replay_protection_reset( &client->payload_replay_protection );
                next_replay_protection_reset( &client->special_replay_protection );
                next_replay_protection_reset( &client->internal_replay_protection );

                next_platform_mutex_acquire( client->bandwidth_mutex );
                client->bandwidth_over_budget = 0;
                client->bandwidth_usage_kbps_up = 0;
                client->bandwidth_usage_kbps_down = 0;
                client->bandwidth_envelope_kbps_up = 0;
                client->bandwidth_envelope_kbps_down = 0;
                next_platform_mutex_release( client->bandwidth_mutex );

                next_platform_mutex_acquire( client->route_manager_mutex );
                next_route_manager_reset( client->route_manager );
                next_platform_mutex_release( client->route_manager_mutex );

                next_packet_loss_tracker_reset( &client->packet_loss_tracker );

                next_stats( client->stats, "closed session" );

                next_stats_session_changed( client->stats, 0 );

                client->counters[NEXT_CLIENT_COUNTER_CLOSE_SESSION]++;
            }
            break;

            case NEXT_CLIENT_COMMAND_DESTROY:
            {
                quit = true;
            }
            break;

            case NEXT_CLIENT_COMMAND_FLAG_SESSION:
            {
                if ( client->session_id != 0 && !client->flagged )
                {
                    next_stats( client->stats, "flagged session" );
                    next_printf( NEXT_LOG_LEVEL_INFO, "client flagged session %" PRIx64, client->session_id );
                    client->flagged = true;
                }
            }
            break;

            default: break;                
        }

        next_free( client->context, command );
    }

    return quit;
}

#if !NEXT_VERSION_IS_PRESENT
float next_fake_direct_packet_loss = 0.0f;
float next_fake_direct_rtt = 0.0f;
float next_fake_next_packet_loss = 0.0f;
float next_fake_next_rtt = 0.0f;
#endif // #if !NEXT_VERSION_IS_PRESENT

void next_client_internal_update_stats( next_client_internal_t * client )
{
    next_assert( client );

    if ( !client->upgraded )
        return;

    double current_time = next_time();

    if ( client->last_stats_update_time + ( 1.0 / NEXT_CLIENT_STATS_UPDATES_PER_SECOND ) < current_time )
    {
        next_platform_mutex_acquire( client->route_manager_mutex );
        const bool network_next = client->route_manager->route_data.current_route;
        const bool fallback_to_direct = client->route_manager->fallback_to_direct;
        const uint64_t flags = client->route_manager->flags;
        next_platform_mutex_release( client->route_manager_mutex );

        client->client_stats.flags = flags;
        client->client_stats.next = network_next;
        client->client_stats.flagged = client->flagged;
        client->client_stats.try_before_you_buy = client->try_before_you_buy;
        client->client_stats.multipath = client->multipath;
        client->client_stats.platform_id = next_platform_id();
        client->client_stats.connection_type = next_platform_connection_type();

        next_route_stats_t next_route_stats;
        next_route_stats_from_ping_history( &client->next_ping_history, current_time - NEXT_CLIENT_STATS_WINDOW, current_time, &next_route_stats, NEXT_PING_SAFETY );

        next_route_stats_t direct_route_stats;
        next_route_stats_from_ping_history( &client->direct_ping_history, current_time - NEXT_CLIENT_STATS_WINDOW, current_time, &direct_route_stats, NEXT_PING_SAFETY );

        if ( network_next )
        {
            client->client_stats.next_min_rtt = next_route_stats.min_rtt;
            client->client_stats.next_max_rtt = next_route_stats.max_rtt;
            client->client_stats.next_mean_rtt = next_route_stats.mean_rtt;
            client->client_stats.next_jitter = next_route_stats.jitter;    
            client->client_stats.next_packet_loss = next_route_stats.packet_loss;
        }
        else
        {
            client->client_stats.next_min_rtt = 0.0f;
            client->client_stats.next_max_rtt = 0.0f;
            client->client_stats.next_mean_rtt = 0.0f;
            client->client_stats.next_jitter = 0.0f;
            client->client_stats.next_packet_loss = 0.0f;
        }

        client->client_stats.direct_min_rtt = direct_route_stats.min_rtt;
        client->client_stats.direct_max_rtt = direct_route_stats.max_rtt;
        client->client_stats.direct_mean_rtt = direct_route_stats.mean_rtt;
        client->client_stats.direct_jitter = direct_route_stats.jitter;    
        client->client_stats.direct_packet_loss = direct_route_stats.packet_loss;

#if !NEXT_VERSION_IS_PRESENT
        client->client_stats.direct_min_rtt += next_fake_direct_rtt;
        client->client_stats.direct_max_rtt += next_fake_direct_rtt;
        client->client_stats.direct_mean_rtt += next_fake_direct_rtt;
        client->client_stats.direct_packet_loss += next_fake_direct_packet_loss;
        client->client_stats.next_min_rtt += next_fake_next_rtt;
        client->client_stats.next_max_rtt += next_fake_next_rtt;
        client->client_stats.next_mean_rtt += next_fake_next_rtt;
        client->client_stats.next_packet_loss += next_fake_next_packet_loss;
#endif // #if !NEXT_VERSION_IS_PRESENT

        if ( !fallback_to_direct )
        {
            const int packets_lost = next_packet_loss_tracker_update( &client->packet_loss_tracker, client->stats );
            client->client_stats.packets_lost_server_to_client += packets_lost;
            client->counters[NEXT_CLIENT_COUNTER_PACKETS_LOST_SERVER_TO_CLIENT] += packets_lost;
        }

        next_relay_manager_get_stats( client->near_relay_manager, &client->near_relay_stats );

        next_client_notify_stats_updated_t * notify = (next_client_notify_stats_updated_t*) next_malloc( client->context, sizeof( next_client_notify_stats_updated_t ) );
        notify->type = NEXT_CLIENT_NOTIFY_STATS_UPDATED;
        notify->stats = client->client_stats;
        notify->fallback_to_direct = fallback_to_direct;
        {
            next_mutex_guard( client->notify_mutex );
            next_queue_push( client->notify_queue, notify );            
        }

        client->last_stats_update_time = current_time;
    }        

    if ( client->last_stats_report_time + 1.0 < current_time && client->client_stats.direct_min_rtt > 0.0f )
    {
        NextClientStatsPacket packet;

        packet.flags = client->client_stats.flags;
        packet.flagged = client->flagged;
        packet.fallback_to_direct = client->fallback_to_direct;
        packet.try_before_you_buy = client->try_before_you_buy;
        packet.multipath = client->multipath;
        packet.platform_id = client->client_stats.platform_id;
        packet.connection_type = client->client_stats.connection_type;

        next_platform_mutex_acquire( client->bandwidth_mutex );
        packet.kbps_up = (int) ceil( client->bandwidth_usage_kbps_up );
        packet.kbps_down = (int) ceil( client->bandwidth_usage_kbps_down );
        next_platform_mutex_release( client->bandwidth_mutex );

        packet.next = client->client_stats.next;
        packet.next_min_rtt = client->client_stats.next_min_rtt;
        packet.next_max_rtt = client->client_stats.next_max_rtt;
        packet.next_mean_rtt = client->client_stats.next_mean_rtt;
        packet.next_jitter = client->client_stats.next_jitter;
        packet.next_packet_loss = client->client_stats.next_packet_loss;

        packet.direct_min_rtt = client->client_stats.direct_min_rtt;
        packet.direct_max_rtt = client->client_stats.direct_max_rtt;
        packet.direct_mean_rtt = client->client_stats.direct_mean_rtt;
        packet.direct_jitter = client->client_stats.direct_jitter;
        packet.direct_packet_loss = client->client_stats.direct_packet_loss;

        if ( !client->fallback_to_direct )
        {
            packet.num_near_relays = client->near_relay_stats.num_relays;
            for ( int i = 0; i < packet.num_near_relays; ++i )
            {
                packet.near_relay_ids[i] = client->near_relay_stats.relay_ids[i];
                packet.near_relay_min_rtt[i] = client->near_relay_stats.relay_min_rtt[i];
                packet.near_relay_max_rtt[i] = client->near_relay_stats.relay_max_rtt[i];
                packet.near_relay_mean_rtt[i] = client->near_relay_stats.relay_mean_rtt[i];
                packet.near_relay_jitter[i] = client->near_relay_stats.relay_jitter[i];
                packet.near_relay_packet_loss[i] = client->near_relay_stats.relay_packet_loss[i];
            }
        }

        packet.packets_lost_server_to_client = client->client_stats.packets_lost_server_to_client;

        if ( next_client_internal_send_packet_to_server( client, NEXT_CLIENT_STATS_PACKET, &packet ) != NEXT_OK )
        {
            next_printf( NEXT_LOG_LEVEL_ERROR, "client failed to send stats packet to server" );
            return;
        }

        client->last_stats_report_time = current_time;
    }    
}

void next_client_internal_update_direct_pings( next_client_internal_t * client )
{
    next_assert( client );

    if ( !client->upgraded )
        return;

    double current_time = next_time();

    if ( client->last_direct_ping_time + ( 1.0 / NEXT_DIRECT_PINGS_PER_SECOND ) <= current_time )
    {
        NextDirectPingPacket packet;
        packet.ping_sequence = next_ping_history_ping_sent( &client->direct_ping_history, current_time );

        if ( next_client_internal_send_packet_to_server( client, NEXT_DIRECT_PING_PACKET, &packet ) != NEXT_OK )
        {
            next_printf( NEXT_LOG_LEVEL_ERROR, "client failed to send direct ping packet to server" );
            return;
        }

        client->last_direct_ping_time = current_time;

        next_stats( client->stats, "direct ping %" PRId64, packet.ping_sequence );
    }

    if ( client->last_direct_pong_time + NEXT_SESSION_TIMEOUT < current_time && !client->fallback_to_direct )
    {
        next_platform_mutex_acquire( client->route_manager_mutex );
        next_route_manager_fallback_to_direct( client->route_manager, NEXT_FLAGS_CLIENT_TIMED_OUT );
        next_platform_mutex_release( client->route_manager_mutex );
    }
}

void next_client_internal_update_next_pings( next_client_internal_t * client )
{
    next_assert( client );

    if ( !client->upgraded )
        return;

    if ( client->fallback_to_direct )
        return;

    double current_time = next_time();

    if ( client->last_next_ping_time + ( 1.0 / NEXT_PINGS_PER_SECOND ) <= current_time )
    {
        next_platform_mutex_acquire( client->route_manager_mutex );
        const bool send_over_network_next = client->route_manager->route_data.current_route;
        next_platform_mutex_release( client->route_manager_mutex );

        if ( !send_over_network_next )
            return;

        next_platform_mutex_acquire( client->route_manager_mutex );
        const uint64_t session_id = client->route_manager->route_data.current_route_session_id;
        const uint8_t session_version = client->route_manager->route_data.current_route_session_version;
        const next_address_t to = client->route_manager->route_data.current_route_next_address;
        uint8_t private_key[crypto_box_SECRETKEYBYTES];
        memcpy( private_key, client->route_manager->route_data.current_route_private_key, crypto_box_SECRETKEYBYTES );
        next_platform_mutex_release( client->route_manager_mutex );

        uint64_t sequence = client->special_send_sequence++;
        sequence |= (1ULL<<62);

        uint8_t packet[NEXT_HEADER_BYTES+8];

        if ( next_write_header( NEXT_DIRECTION_CLIENT_TO_SERVER, NEXT_PING_PACKET, sequence, session_id, session_version, 0, private_key, packet, sizeof(packet) ) != NEXT_OK )
        {
            next_printf( NEXT_LOG_LEVEL_ERROR, "client failed to write next ping packet" );
            return;
        }

        const uint64_t ping_sequence = next_ping_history_ping_sent( &client->next_ping_history, current_time );

        uint8_t * p = packet + NEXT_HEADER_BYTES;
        next_write_uint64( &p, ping_sequence );

        next_platform_socket_send_packet( client->socket, &to, packet, sizeof(packet) );

        client->last_next_ping_time = current_time;

        next_stats( client->stats, "next ping %" PRId64, ping_sequence );

        if ( client->first_next_ping_time == 0.0 )
        {
            client->first_next_ping_time = current_time;
        }
    }
}

void next_client_internal_send_pings_to_near_relays( next_client_internal_t * client )
{
    next_assert( client );

    if ( !client->upgraded )
        return;

    if ( client->fallback_to_direct )
        return;

    next_relay_manager_send_pings( client->near_relay_manager, client->socket, client->session_id );
}

void next_client_internal_update_route_manager( next_client_internal_t * client )
{
    next_assert( client );

    if ( !client->upgraded )
        return;

    if ( client->fallback_to_direct )
        return;

    next_address_t route_request_to;
    next_address_t continue_request_to;

    int route_request_packet_bytes;
    int continue_request_packet_bytes;

    uint8_t route_request_packet_data[NEXT_MAX_PACKET_BYTES];
    uint8_t continue_request_packet_data[NEXT_MAX_PACKET_BYTES];

    next_platform_mutex_acquire( client->route_manager_mutex );
    next_route_manager_check_for_timeouts( client->route_manager );
    const bool fallback_to_direct = client->route_manager->fallback_to_direct;
    const bool send_route_request = next_route_manager_send_route_request( client->route_manager, &route_request_to, route_request_packet_data, &route_request_packet_bytes );
    const bool send_continue_request = next_route_manager_send_continue_request( client->route_manager, &continue_request_to, continue_request_packet_data, &continue_request_packet_bytes );
    next_platform_mutex_release( client->route_manager_mutex );

    if ( !client->fallback_to_direct && fallback_to_direct )
    {
        client->counters[NEXT_CLIENT_COUNTER_FALLBACK_TO_DIRECT]++;
    }

    client->fallback_to_direct = fallback_to_direct;

    if ( send_route_request )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "client sent route request" );
        next_platform_socket_send_packet( client->socket, &route_request_to, route_request_packet_data, route_request_packet_bytes );
    }

    if ( send_continue_request )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "client sent continue request" );
        next_platform_socket_send_packet( client->socket, &continue_request_to, continue_request_packet_data, continue_request_packet_bytes );
    }
}

void next_client_internal_update_try_before_you_buy( next_client_internal_t * client )
{
    if ( client->multipath )
        return;

    if ( client->fallback_to_direct )
        return;
    
    if ( !client->try_before_you_buy )
        return;

    if ( client->first_next_ping_time == 0.0 )
        return;

    double current_time = next_time();

    if ( current_time - client->first_next_ping_time < NEXT_TRY_BEFORE_YOU_BUY_MIN_EVAL_TIME )
        return;

    if ( client->client_stats.next && client->client_stats.next_min_rtt <= client->client_stats.direct_min_rtt && client->client_stats.next_packet_loss <= client->client_stats.direct_packet_loss )
    {
        next_printf( NEXT_LOG_LEVEL_INFO, "client try before you buy completed" );
        client->try_before_you_buy = false;
        client->counters[NEXT_CLIENT_COUNTER_TRY_BEFORE_YOU_BUY_COMPLETED]++;
    }

    if ( current_time - client->first_next_ping_time > NEXT_TRY_BEFORE_YOU_BUY_ABORT_TIME )
    {
        next_printf( NEXT_LOG_LEVEL_INFO, "client try before you buy abort" );
        next_platform_mutex_acquire( client->route_manager_mutex );
        next_route_manager_fallback_to_direct( client->route_manager, NEXT_FLAGS_TRY_BEFORE_YOU_BUY_ABORT );
        next_platform_mutex_release( client->route_manager_mutex );
        client->fallback_to_direct = true;
        client->counters[NEXT_CLIENT_COUNTER_FALLBACK_TO_DIRECT]++;
        client->counters[NEXT_CLIENT_COUNTER_TRY_BEFORE_YOU_BUY_ABORT]++;
        return;
    }
}

static next_platform_thread_return_t NEXT_PLATFORM_THREAD_FUNC next_client_internal_thread_function( void * context )
{
    next_client_internal_t * client = (next_client_internal_t*) context;

    next_assert( client );

    bool quit = false;

    double last_update_time = next_time();

    while ( !quit )
    {
        next_client_internal_block_and_receive_packet( client );

        double current_time = next_time();

        if ( current_time > last_update_time + 0.01 )
        {
            next_client_internal_update_direct_pings( client );

            next_client_internal_update_next_pings( client );

            next_client_internal_send_pings_to_near_relays( client );

            next_client_internal_update_stats( client );

            next_client_internal_update_route_manager( client );

            next_client_internal_update_try_before_you_buy( client );

            quit = next_client_internal_pump_commands( client );

            last_update_time = current_time;
        }
    }

    NEXT_PLATFORM_THREAD_RETURN();
}

// ---------------------------------------------------------------

struct next_client_t
{
    void * context;
    next_stats_t * stats;
    bool session_open;
    bool upgraded;
    bool fallback_to_direct;
    uint8_t open_session_sequence;
    uint64_t session_id;
    next_address_t server_address;
    next_client_internal_t * internal;
    next_platform_thread_t * thread;
    void (*packet_received_callback)( next_client_t * client, void * context, const uint8_t * packet_data, int packet_bytes );
    next_client_stats_t client_stats;
    next_bandwidth_limiter_t send_bandwidth;
    next_bandwidth_limiter_t receive_bandwidth;
    uint64_t counters[NEXT_CLIENT_COUNTER_MAX];
};

void next_client_destroy( next_client_t * client );

next_client_t * next_client_create( void * context, void (*packet_received_callback)( next_client_t * client, void * context, const uint8_t * packet_data, int packet_bytes ) )
{
    next_assert( packet_received_callback );

    next_client_t * client = (next_client_t*) next_malloc( context, sizeof(next_client_t) );
    if ( !client ) 
        return NULL;

    memset( client, 0, sizeof( next_client_t) );

#if NEXT_STATS_ENABLED
    client->stats = next_stats_create( context );
    if ( !client->stats )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create stats" );
        next_client_destroy( client );
        return NULL;
    }
#endif // #if NEXT_STATS_ENABLED

    client->context = context;
    client->packet_received_callback = packet_received_callback;

    client->internal = next_client_internal_create( client->context, client->stats );
    if ( !client->internal )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create internal client" );
        next_client_destroy( client );
        return NULL;
    }

    client->thread = next_platform_thread_create( client->context, next_client_internal_thread_function, client->internal );
    next_assert( client->thread );
    if ( !client->thread )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client could not create thread" );
        next_client_destroy( client );
        return NULL;
    }

    next_bandwidth_limiter_reset( &client->send_bandwidth );
    next_bandwidth_limiter_reset( &client->receive_bandwidth );

    return client;
}

void next_client_destroy( next_client_t * client )
{
    next_assert( client );

    if ( client->thread )
    {
        next_client_command_destroy_t * command = (next_client_command_destroy_t*) next_malloc( client->context, sizeof( next_client_command_destroy_t ) );
        if ( !command )
        {
            next_printf( NEXT_LOG_LEVEL_ERROR, "client destroy failed. could not create destroy command" );
            return;
        }
        command->type = NEXT_CLIENT_COMMAND_DESTROY;
        {
            next_mutex_guard( client->internal->command_mutex );
            next_queue_push( client->internal->command_queue, command );
        }

        next_platform_thread_join( client->thread );
        next_platform_thread_destroy( client->thread );
    }

    if ( client->internal )
    {
        next_client_internal_destroy( client->internal );
    }

#if NEXT_STATS_ENABLED
    if ( client->stats )
    {
        next_stats_destroy( client->stats );
    }
#endif // #if NEXT_STATS_ENABLED

    clear_and_free( client->context, client, sizeof(next_client_t) );
}

void next_client_open_session( next_client_t * client, const char * server_address_string )
{
    next_assert( client );
    next_assert( client->internal );
    next_assert( client->internal->command_mutex );
    next_assert( client->internal->command_queue );

    next_client_close_session( client );

    next_address_t server_address;
    if ( next_address_parse( &server_address, server_address_string ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client open session failed. could not parse server address: %s", server_address_string );
        return;
    }
    
    next_client_command_open_session_t * command = (next_client_command_open_session_t*) next_malloc( client->context, sizeof( next_client_command_open_session_t ) );
    if ( !command )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client open session failed. could not create open session command" );
        return;
    }

    command->type = NEXT_CLIENT_COMMAND_OPEN_SESSION;
    command->server_address = server_address;

    {    
        next_mutex_guard( client->internal->command_mutex );
        next_queue_push( client->internal->command_queue, command );
    }

    client->session_open = true;
    client->server_address = server_address;
    client->open_session_sequence++;
}

void next_client_close_session( next_client_t * client )
{
    next_assert( client );
    next_assert( client->internal );
    next_assert( client->internal->command_mutex );
    next_assert( client->internal->command_queue );

    next_client_command_close_session_t * command = (next_client_command_close_session_t*) next_malloc( client->context, sizeof( next_client_command_close_session_t ) );
    if ( !command )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client close session failed. could not create close session command" );
        return;
    }
    
    command->type = NEXT_CLIENT_COMMAND_CLOSE_SESSION;
    {
        next_mutex_guard( client->internal->command_mutex );    
        next_queue_push( client->internal->command_queue, command );
    }

    client->session_open = false;
    client->upgraded = false;
    client->fallback_to_direct = false;
    client->session_id = 0;    memset( &client->client_stats, 0, sizeof(next_client_stats_t ) );
    memset( &client->server_address, 0, sizeof(next_address_t) );
    next_bandwidth_limiter_reset( &client->send_bandwidth );
    next_bandwidth_limiter_reset( &client->receive_bandwidth );
}

void next_client_update( next_client_t * client )
{
    while ( true )
    {
        void * entry = NULL;
        {
            next_mutex_guard( client->internal->notify_mutex );    
            entry = next_queue_pop( client->internal->notify_queue );
        }

        if ( entry == NULL )
            break;

        next_client_notify_t * notify = (next_client_notify_t*) entry;

        switch ( notify->type )
        {
            case NEXT_CLIENT_NOTIFY_PACKET_RECEIVED:
            {
                next_client_notify_packet_received_t * packet_received = (next_client_notify_packet_received_t*) notify;

                client->packet_received_callback( client, client->context, packet_received->packet_data, packet_received->packet_bytes );

                next_platform_mutex_acquire( client->internal->bandwidth_mutex );
                const int envelope_kbps_down = client->internal->bandwidth_envelope_kbps_down;
                next_platform_mutex_release( client->internal->bandwidth_mutex );

                const int packet_bits = next_wire_packet_bits( NEXT_HEADER_BYTES + packet_received->packet_bytes );

                next_bandwidth_limiter_add_packet( &client->receive_bandwidth, next_time(), envelope_kbps_down, packet_bits );

                double kbps_down = next_bandwidth_limiter_usage_kbps( &client->receive_bandwidth, next_time() );

                next_platform_mutex_acquire( client->internal->bandwidth_mutex );
                client->internal->bandwidth_usage_kbps_down = kbps_down;
                next_platform_mutex_release( client->internal->bandwidth_mutex );
            }
            break;

            case NEXT_CLIENT_NOTIFY_UPGRADED:
            {
                next_client_notify_upgraded_t * upgraded = (next_client_notify_upgraded_t*) notify;
                client->upgraded = true;
                client->session_id = upgraded->session_id;
                next_printf( NEXT_LOG_LEVEL_INFO, "client upgraded to session %" PRIx64, client->session_id );
                next_stats_session_changed( client->stats, client->session_id );
                next_stats( client->stats, "upgraded to session %" PRIx64, client->session_id );
            }
            break;

            case NEXT_CLIENT_NOTIFY_STATS_UPDATED:
            {
                next_client_notify_stats_updated_t * stats_updated = (next_client_notify_stats_updated_t*) notify;
                client->client_stats = stats_updated->stats;
                client->fallback_to_direct = stats_updated->fallback_to_direct;
                if ( client->fallback_to_direct && client->upgraded )
                {
                    client->upgraded = false;
                    client->session_id = 0;
                }
            }
            break;

            default: break;
        }

        next_free( client->context, entry );
    }
}

void next_client_send_packet( next_client_t * client, const uint8_t * packet_data, int packet_bytes )
{
    next_assert( client );
    next_assert( client->internal );
    next_assert( client->internal->socket );
    next_assert( packet_bytes >= 0 );
    next_assert( packet_bytes <= NEXT_MTU );

    if ( !client->session_open )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "client can't send packet because no session is open" );
        return;
    }

    if ( packet_bytes <= 0 )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client can't send packet because packet bytes are less than zero" );
        return;
    }

    if ( packet_bytes > NEXT_MTU )
    {
        next_stats( client->stats, "packet is larger than MTU (%d>%d)", packet_bytes, NEXT_MTU );
        next_printf( NEXT_LOG_LEVEL_ERROR, "client can't send packet because it's larger than MTU (%d)", NEXT_MTU );
        return;
    }

    if ( client->upgraded )
    {
        next_platform_mutex_acquire( client->internal->route_manager_mutex );
        const uint64_t send_sequence = next_route_manager_next_send_sequence( client->internal->route_manager );
        bool send_over_network_next = next_route_manager_has_network_next_route( client->internal->route_manager );
        bool send_direct = !send_over_network_next;
        next_platform_mutex_release( client->internal->route_manager_mutex );

        bool multipath = client->client_stats.multipath;
        bool try_before_you_buy = client->client_stats.try_before_you_buy;

        if ( send_over_network_next && multipath )
        {
            send_direct = true;
        }

        if ( try_before_you_buy && !multipath )
        {
            send_over_network_next = false;
            send_direct = true;
        }

        // don't send over network next if we're over the bandwidth budget

        next_platform_mutex_acquire( client->internal->bandwidth_mutex );
        const int envelope_kbps_up = client->internal->bandwidth_envelope_kbps_up;
        next_platform_mutex_release( client->internal->bandwidth_mutex );

        const int packet_bits = next_wire_packet_bits( NEXT_HEADER_BYTES + packet_bytes );

        bool over_budget = next_bandwidth_limiter_add_packet( &client->send_bandwidth, next_time(), envelope_kbps_up, packet_bits );

        double usage_kbps_up = next_bandwidth_limiter_usage_kbps( &client->send_bandwidth, next_time() );

        next_platform_mutex_acquire( client->internal->bandwidth_mutex );
        client->internal->bandwidth_over_budget = over_budget;
        client->internal->bandwidth_usage_kbps_up = usage_kbps_up;
        next_platform_mutex_release( client->internal->bandwidth_mutex );

        if ( send_over_network_next && over_budget )
        {
            send_over_network_next = false;
            if ( !multipath )
            {
                next_stats( client->stats, "exceeded bandwidth budget" );
                next_printf( NEXT_LOG_LEVEL_WARN, "client exceeded bandwidth budget (%d kbps). sending direct instead", envelope_kbps_up );
                send_direct = true;
            }
        }

        if ( send_over_network_next )
        {
            // send over network next

            int next_packet_bytes = 0;
            next_address_t next_to;
            uint8_t next_packet_data[NEXT_MAX_PACKET_BYTES];

            next_platform_mutex_acquire( client->internal->route_manager_mutex );
            next_route_manager_prepare_send_packet( client->internal->route_manager, send_sequence, &next_to, packet_data, packet_bytes, next_packet_data, &next_packet_bytes );
            next_platform_mutex_release( client->internal->route_manager_mutex );

            next_platform_socket_send_packet( client->internal->socket, &next_to, next_packet_data, next_packet_bytes );

            client->counters[NEXT_CLIENT_COUNTER_PACKET_SENT_NEXT]++;
        }

        if ( send_direct )
        {
            // [255][open session sequence][sequence](payload) style packets direct to server

            uint8_t buffer[10+NEXT_MTU];
            uint8_t * p = buffer;
            next_write_uint8( &p, 255 );
            next_write_uint8( &p, client->open_session_sequence );
            next_write_uint64( &p, send_sequence );
            memcpy( buffer+10, packet_data, packet_bytes );
            next_platform_socket_send_packet( client->internal->socket, &client->server_address, buffer, packet_bytes + 10 );
            client->counters[NEXT_CLIENT_COUNTER_PACKET_SENT_DIRECT]++;
        }
    }
    else
    {
        // send [0](payload) style packet direct to server

        uint8_t buffer[1+NEXT_MTU];
        buffer[0] = 0;
        memcpy( buffer+1, packet_data, packet_bytes );
        next_platform_socket_send_packet( client->internal->socket, &client->server_address, buffer, packet_bytes + 1 );
        client->counters[NEXT_CLIENT_COUNTER_PACKET_SENT_DIRECT]++;
    }
}

void next_client_send_packet_direct( next_client_t * client, const uint8_t * packet_data, int packet_bytes )
{
    next_assert( client );
    next_assert( client->internal );
    next_assert( client->internal->socket );
    next_assert( packet_bytes >= 0 );
    next_assert( packet_bytes <= NEXT_MTU );

    if ( !client->session_open )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "client can't send direct packet because no session is open" );
        return;
    }

    if ( packet_bytes <= 0 )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "client can't send direct packet because packet bytes are less than zero" );
        return;
    }

    if ( packet_bytes > NEXT_MTU )
    {
        next_stats( client->stats, "direct packet is larger than MTU (%d>%d)", packet_bytes, NEXT_MTU );
        next_printf( NEXT_LOG_LEVEL_ERROR, "client can't send direct packet because it's larger than MTU (%d)", NEXT_MTU );
        return;
    }

    if ( client->upgraded )
    {
        next_platform_mutex_acquire( client->internal->route_manager_mutex );
        const uint64_t send_sequence = next_route_manager_next_send_sequence( client->internal->route_manager );
        next_platform_mutex_release( client->internal->route_manager_mutex );

        // [255][open session sequence][sequence](payload) style packets direct to server

        uint8_t buffer[10+NEXT_MTU];
        uint8_t * p = buffer;
        next_write_uint8( &p, 255 );
        next_write_uint8( &p, client->open_session_sequence );
        next_write_uint64( &p, send_sequence );
        memcpy( buffer+10, packet_data, packet_bytes );
        next_platform_socket_send_packet( client->internal->socket, &client->server_address, buffer, packet_bytes + 10 );
        client->counters[NEXT_CLIENT_COUNTER_PACKET_SENT_DIRECT]++;

    }
    else
    {
        // send [0](payload) style packet direct to server

        uint8_t buffer[1+NEXT_MTU];
        buffer[0] = 0;
        memcpy( buffer+1, packet_data, packet_bytes );
        next_platform_socket_send_packet( client->internal->socket, &client->server_address, buffer, packet_bytes + 1 );
        client->counters[NEXT_CLIENT_COUNTER_PACKET_SENT_DIRECT]++;
    }
}

void next_client_flag_session( next_client_t * client )
{
    next_assert( client );

    next_client_command_flag_session_t * command = (next_client_command_flag_session_t*) next_malloc( client->context, sizeof( next_client_command_flag_session_t ) );
    if ( !command )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "flag session failed. could not create flag session command" );
        return;
    }

    command->type = NEXT_CLIENT_COMMAND_FLAG_SESSION;

    {    
        next_mutex_guard( client->internal->command_mutex );
        next_queue_push( client->internal->command_queue, command );
    }
}

uint64_t next_client_session_id( next_client_t * client )
{
    next_assert( client );
    return client->session_id;
}

const next_client_stats_t * next_client_stats( next_client_t * client )
{
    next_assert( client );
    return &client->client_stats;
}

void next_client_counters( next_client_t * client, uint64_t * counters )
{
    uint64_t internal_counters[NEXT_CLIENT_COUNTER_MAX];
    memcpy( counters, client->counters, sizeof(uint64_t) * NEXT_CLIENT_COUNTER_MAX );
    memcpy( internal_counters, client->internal->counters, sizeof(uint64_t) * NEXT_CLIENT_COUNTER_MAX );
    for ( int i = 0; i < NEXT_CLIENT_COUNTER_MAX; ++i )
        counters[i] += internal_counters[i];
}

// ---------------------------------------------------------------

int next_address_parse( next_address_t * address, const char * address_string_in )
{
    next_assert( address );
    next_assert( address_string_in );

    if ( !address )
        return NEXT_ERROR;

    if ( !address_string_in )
        return NEXT_ERROR;

    memset( address, 0, sizeof( next_address_t ) );

    // first try to parse the string as an IPv6 address:
    // 1. if the first character is '[' then it's probably an ipv6 in form "[addr6]:portnum"
    // 2. otherwise try to parse as a raw IPv6 address using inet_pton

    char buffer[NEXT_MAX_ADDRESS_STRING_LENGTH + NEXT_ADDRESS_BUFFER_SAFETY*2];

    char * address_string = buffer + NEXT_ADDRESS_BUFFER_SAFETY;
    strncpy( address_string, address_string_in, NEXT_MAX_ADDRESS_STRING_LENGTH - 1 );
    address_string[NEXT_MAX_ADDRESS_STRING_LENGTH-1] = '\0';

    int address_string_length = (int) strlen( address_string );

    if ( address_string[0] == '[' )
    {
        const int base_index = address_string_length - 1;
        
        // note: no need to search past 6 characters as ":65535" is longest possible port value
        for ( int i = 0; i < 6; ++i )
        {
            const int index = base_index - i;
            if ( index < 0 )
            {
                return NEXT_ERROR;
            }
            if ( address_string[index] == ':' )
            {
                address->port = (uint16_t) ( atoi( &address_string[index + 1] ) );
                address_string[index-1] = '\0';
                break;
            }
            else if ( address_string[index] == ']' )
            {
                // no port number
                address->port = 0;
                address_string[index] = '\0';
                break;
            }
        }
        address_string += 1;
    }

    uint16_t addr6[8];
    if ( next_platform_inet_pton6( address_string, addr6 ) == NEXT_OK )
    {
        address->type = NEXT_ADDRESS_IPV6;
        for ( int i = 0; i < 8; ++i )
        {
            address->data.ipv6[i] = next_ntohs( addr6[i] );
        }
        return NEXT_OK;
    }

    // otherwise it's probably an IPv4 address:
    // 1. look for ":portnum", if found save the portnum and strip it out
    // 2. parse remaining ipv4 address via inet_pton

    address_string_length = (int) strlen( address_string );
    const int base_index = address_string_length - 1;
    for ( int i = 0; i < 6; ++i )
    {
        const int index = base_index - i;
        if ( index < 0 )
            break;
        if ( address_string[index] == ':' )
        {
            address->port = (uint16_t)( atoi( &address_string[index + 1] ) );
            address_string[index] = '\0';
        }
    }

    uint32_t addr4;
    if ( next_platform_inet_pton4( address_string, &addr4 ) == NEXT_OK )
    {
        address->type = NEXT_ADDRESS_IPV4;
        address->data.ipv4[3] = (uint8_t) ( ( addr4 & 0xFF000000 ) >> 24 );
        address->data.ipv4[2] = (uint8_t) ( ( addr4 & 0x00FF0000 ) >> 16 );
        address->data.ipv4[1] = (uint8_t) ( ( addr4 & 0x0000FF00 ) >> 8  );
        address->data.ipv4[0] = (uint8_t) ( ( addr4 & 0x000000FF )     );
        return NEXT_OK;
    }

    return NEXT_ERROR;
}

const char * next_address_to_string( const next_address_t * address, char * buffer )
{
    next_assert( buffer );

    if ( address->type == NEXT_ADDRESS_IPV6 )
    {
#if defined(WINVER) && WINVER <= 0x0502
        // ipv6 not supported
        buffer[0] = '\0';
        return buffer;
#else
        uint16_t ipv6_network_order[8];
        for ( int i = 0; i < 8; ++i )
            ipv6_network_order[i] = next_htons( address->data.ipv6[i] );
        char address_string[NEXT_MAX_ADDRESS_STRING_LENGTH];
        next_platform_inet_ntop6( ipv6_network_order, address_string, sizeof( address_string ) );
        if ( address->port == 0 )
        {
            strncpy( buffer, address_string, NEXT_MAX_ADDRESS_STRING_LENGTH );
            return buffer;
        }
        else
        {
            if ( snprintf( buffer, NEXT_MAX_ADDRESS_STRING_LENGTH, "[%s]:%hu", address_string, address->port ) < 0 )
            {
                next_printf( NEXT_LOG_LEVEL_ERROR, "address string truncated: [%s]:%hu", address_string, address->port );
            }
            return buffer;
        }
#endif
    }
    else if ( address->type == NEXT_ADDRESS_IPV4 )
    {
        if ( address->port != 0 )
        {
            snprintf( buffer, 
                      NEXT_MAX_ADDRESS_STRING_LENGTH, 
                      "%d.%d.%d.%d:%d", 
                      address->data.ipv4[0], 
                      address->data.ipv4[1], 
                      address->data.ipv4[2], 
                      address->data.ipv4[3], 
                      address->port );
        }
        else
        {
            snprintf( buffer, 
                      NEXT_MAX_ADDRESS_STRING_LENGTH, 
                      "%d.%d.%d.%d", 
                      address->data.ipv4[0], 
                      address->data.ipv4[1], 
                      address->data.ipv4[2], 
                      address->data.ipv4[3] );
        }
        return buffer;
    }
    else
    {
        snprintf( buffer, NEXT_MAX_ADDRESS_STRING_LENGTH, "%s", "NONE" );
        return buffer;
    }
}

int next_address_equal( const next_address_t * a, const next_address_t * b )
{
    next_assert( a );
    next_assert( b );

    if ( a->type != b->type )
        return 0;

    if ( a->type == NEXT_ADDRESS_IPV4 )
    {
        if ( a->port != b->port )
            return 0;

        for ( int i = 0; i < 4; ++i )
        {
            if ( a->data.ipv4[i] != b->data.ipv4[i] )
                return 0;
        }
    }
    else if ( a->type == NEXT_ADDRESS_IPV6 )
    {
        if ( a->port != b->port )
            return 0;

        for ( int i = 0; i < 8; ++i )
        {
            if ( a->data.ipv6[i] != b->data.ipv6[i] )
                return 0;
        }
    }

    return 1;
}

// ---------------------------------------------------------------

struct next_pending_session_entry_t
{
    next_address_t address;
    uint64_t session_id;
    uint64_t user_hash;
    uint64_t platform_id;
    uint64_t tag;
    double upgrade_time;
    double last_packet_send_time;
    uint8_t private_key[crypto_secretbox_KEYBYTES];
    uint8_t upgrade_token[NEXT_UPGRADE_TOKEN_BYTES];
};

struct next_pending_session_manager_t
{
    void * context;
    int size;
    int max_entry_index;
    next_address_t * addresses;
    next_pending_session_entry_t * entries;
};

void next_pending_session_manager_destroy( next_pending_session_manager_t * pending_session_manager );

next_pending_session_manager_t * next_pending_session_manager_create( void * context, int initial_size )
{
    next_pending_session_manager_t * pending_session_manager = (next_pending_session_manager_t*) next_malloc( context, sizeof(next_pending_session_manager_t) );
    next_assert( pending_session_manager );
    if ( !pending_session_manager )
        return NULL;
    memset( pending_session_manager, 0, sizeof(next_pending_session_manager_t) );
    pending_session_manager->context = context;
    pending_session_manager->size = initial_size;
    pending_session_manager->addresses = (next_address_t*) next_malloc( context, initial_size * sizeof(next_address_t) );
    pending_session_manager->entries = (next_pending_session_entry_t*) next_malloc( context, initial_size * sizeof(next_pending_session_entry_t) );
    next_assert( pending_session_manager->addresses );
    next_assert( pending_session_manager->entries );
    if ( pending_session_manager->addresses == NULL || pending_session_manager->entries == NULL )
    {
        next_pending_session_manager_destroy( pending_session_manager );
        return NULL;
    }
    memset( pending_session_manager->addresses, 0, initial_size * sizeof(next_address_t) );
    memset( pending_session_manager->entries, 0, initial_size * sizeof(next_pending_session_entry_t) );
    return pending_session_manager;
}

void next_pending_session_manager_destroy( next_pending_session_manager_t * pending_session_manager )
{
    next_assert( pending_session_manager );
    next_free( pending_session_manager->context, pending_session_manager->addresses );
    next_free( pending_session_manager->context, pending_session_manager->entries );
    clear_and_free( pending_session_manager->context, pending_session_manager, sizeof(next_pending_session_manager_t) );
}

bool next_pending_session_manager_expand( next_pending_session_manager_t * pending_session_manager )
{
    next_assert( pending_session_manager );
    int new_size = pending_session_manager->size * 2;
    next_address_t * new_addresses = (next_address_t*) next_malloc( pending_session_manager->context, new_size * sizeof(next_address_t) );
    next_pending_session_entry_t * new_entries = (next_pending_session_entry_t*) next_malloc( pending_session_manager->context, new_size * sizeof(next_pending_session_entry_t) );
    next_assert( pending_session_manager->addresses );
    next_assert( pending_session_manager->entries );
    if ( pending_session_manager->addresses == NULL || pending_session_manager->entries == NULL )
    {
        next_free( pending_session_manager->context, new_addresses );
        next_free( pending_session_manager->context, new_entries );
        return false;
    }
    memset( new_addresses, 0, new_size * sizeof(next_address_t) );
    memset( new_entries, 0, new_size * sizeof(next_pending_session_entry_t) );
    int index = 0;
    const int current_size = pending_session_manager->size;
    for ( int i = 0; i < current_size; ++i )
    {
        if ( pending_session_manager->addresses[i].type != NEXT_ADDRESS_NONE )
        {
            memcpy( &new_addresses[index], &pending_session_manager->addresses[i], sizeof(next_address_t) );
            memcpy( &new_entries[index], &pending_session_manager->entries[i], sizeof(next_pending_session_entry_t) );
            index++;            
        }
    }
    next_free( pending_session_manager->context, pending_session_manager->addresses );
    next_free( pending_session_manager->context, pending_session_manager->entries );
    pending_session_manager->addresses = new_addresses;
    pending_session_manager->entries = new_entries;
    pending_session_manager->size = new_size;
    pending_session_manager->max_entry_index = index - 1;
    return true;
}

next_pending_session_entry_t * next_pending_session_manager_add( next_pending_session_manager_t * pending_session_manager, const next_address_t * address, uint64_t session_id, const uint8_t * private_key, const uint8_t * upgrade_token, double current_time )
{
    next_assert( pending_session_manager );
    next_assert( session_id != 0 );
    next_assert( address );
    next_assert( address->type != NEXT_ADDRESS_NONE );

    // first scan existing entries and see if we can insert there

    const int size = pending_session_manager->size;

    for ( int i = 0; i < size; ++i )
    {
        if ( pending_session_manager->addresses[i].type == NEXT_ADDRESS_NONE )
        {
            pending_session_manager->addresses[i] = *address;
            next_pending_session_entry_t * entry = &pending_session_manager->entries[i];
            entry->address = *address;
            entry->session_id = session_id;
            entry->upgrade_time = current_time;
            entry->last_packet_send_time = -1000.0;
            memcpy( entry->private_key, private_key, crypto_secretbox_KEYBYTES );
            memcpy( entry->upgrade_token, upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );
            if ( i > pending_session_manager->max_entry_index )
            {
                pending_session_manager->max_entry_index = i;
            }
            return entry;
        }        
    }  

    // ok, we need to grow, expand and add at the end (expand compacts existing entries)

    if ( !next_pending_session_manager_expand( pending_session_manager ) )
        return NULL;

    const int i = ++pending_session_manager->max_entry_index;
    pending_session_manager->addresses[i] = *address;
    next_pending_session_entry_t * entry = &pending_session_manager->entries[i];
    entry->address = *address;
    entry->session_id = session_id;
    entry->upgrade_time = current_time;
    entry->last_packet_send_time = -1000.0;
    memcpy( entry->private_key, private_key, crypto_secretbox_KEYBYTES );
    memcpy( entry->upgrade_token, upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );

    return entry;
}

void next_pending_session_manager_remove_at_index( next_pending_session_manager_t * pending_session_manager, int index )
{
    next_assert( pending_session_manager );
    next_assert( index >= 0 );
    next_assert( index <= pending_session_manager->max_entry_index );
    const int max_index = pending_session_manager->max_entry_index;
    pending_session_manager->addresses[index].type = NEXT_ADDRESS_NONE;
    if ( index == max_index )
    {
        while ( index > 0 && pending_session_manager->addresses[index].type == NEXT_ADDRESS_NONE )
        {
            index--;
        }
        pending_session_manager->max_entry_index = index;
    }
}

void next_pending_session_manager_remove_by_address( next_pending_session_manager_t * pending_session_manager, const next_address_t * address )
{
    next_assert( pending_session_manager );
    next_assert( address );
    const int max_index = pending_session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( next_address_equal( address, &pending_session_manager->addresses[i] ) == 1 )
        {
            next_pending_session_manager_remove_at_index( pending_session_manager, i );
            return;
        }        
    }
}

next_pending_session_entry_t * next_pending_session_manager_find( next_pending_session_manager_t * pending_session_manager, const next_address_t * address )
{
    next_assert( pending_session_manager );
    next_assert( address );
    const int max_index = pending_session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( next_address_equal( address, &pending_session_manager->addresses[i] ) == 1 )
        {
            return &pending_session_manager->entries[i];
        }        
    }
    return NULL;
}

int next_pending_session_manager_num_entries( next_pending_session_manager_t * pending_session_manager )
{
    next_assert( pending_session_manager );
    int num_entries = 0;
    const int max_index = pending_session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( pending_session_manager->addresses[i].type != 0 )
        {
            num_entries++;
        }        
    }
    return num_entries;
}

// ---------------------------------------------------------------

struct next_proxy_session_entry_t
{
    next_address_t address;
    uint64_t session_id;
    next_bandwidth_limiter_t send_bandwidth;
};

struct next_proxy_session_manager_t
{
    void * context;
    int size;
    int max_entry_index;
    next_address_t * addresses;
    next_proxy_session_entry_t * entries;
};

void next_proxy_session_manager_destroy( next_proxy_session_manager_t * session_manager );

next_proxy_session_manager_t * next_proxy_session_manager_create( void * context, int initial_size )
{
    next_proxy_session_manager_t * session_manager = (next_proxy_session_manager_t*) next_malloc( context, sizeof(next_proxy_session_manager_t) );
    next_assert( session_manager );
    if ( !session_manager )
        return NULL;
    memset( session_manager, 0, sizeof(next_proxy_session_manager_t) );
    session_manager->context = context;
    session_manager->size = initial_size;
    session_manager->addresses = (next_address_t*) next_malloc( context, initial_size * sizeof(next_address_t) );
    session_manager->entries = (next_proxy_session_entry_t*) next_malloc( context, initial_size * sizeof(next_proxy_session_entry_t) );
    next_assert( session_manager->addresses );
    next_assert( session_manager->entries );
    if ( session_manager->addresses == NULL || session_manager->entries == NULL )
    {
        next_proxy_session_manager_destroy( session_manager );
        return NULL;
    }
    memset( session_manager->addresses, 0, initial_size * sizeof(next_address_t) );
    memset( session_manager->entries, 0, initial_size * sizeof(next_proxy_session_entry_t) );
    return session_manager;
}

void next_proxy_session_manager_destroy( next_proxy_session_manager_t * session_manager )
{
    next_assert( session_manager );
    next_free( session_manager->context, session_manager->addresses );
    next_free( session_manager->context, session_manager->entries );
    clear_and_free( session_manager->context, session_manager, sizeof(next_proxy_session_manager_t) );
}

bool next_proxy_session_manager_expand( next_proxy_session_manager_t * session_manager )
{
    next_assert( session_manager );
    int new_size = session_manager->size * 2;
    next_address_t * new_addresses = (next_address_t*) next_malloc( session_manager->context, new_size * sizeof(next_address_t) );
    next_proxy_session_entry_t * new_entries = (next_proxy_session_entry_t*) next_malloc( session_manager->context, new_size * sizeof(next_proxy_session_entry_t) );
    next_assert( session_manager->addresses );
    next_assert( session_manager->entries );
    if ( session_manager->addresses == NULL || session_manager->entries == NULL )
    {
        next_free( session_manager->context, new_addresses );
        next_free( session_manager->context, new_entries );
        return false;
    }
    memset( new_addresses, 0, new_size * sizeof(next_address_t) );
    memset( new_entries, 0, new_size * sizeof(next_proxy_session_entry_t) );
    int index = 0;
    const int current_size = session_manager->size;
    for ( int i = 0; i < current_size; ++i )
    {
        if ( session_manager->addresses[i].type != NEXT_ADDRESS_NONE )
        {
            memcpy( &new_addresses[index], &session_manager->addresses[i], sizeof(next_address_t) );
            memcpy( &new_entries[index], &session_manager->entries[i], sizeof(next_proxy_session_entry_t) );
            index++;            
        }
    }
    next_free( session_manager->context, session_manager->addresses );
    next_free( session_manager->context, session_manager->entries );
    session_manager->addresses = new_addresses;
    session_manager->entries = new_entries;
    session_manager->size = new_size;
    session_manager->max_entry_index = index - 1;
    return true;
}

next_proxy_session_entry_t * next_proxy_session_manager_add( next_proxy_session_manager_t * session_manager, const next_address_t * address, uint64_t session_id )
{
    next_assert( session_manager );
    next_assert( session_id != 0 );
    next_assert( address );
    next_assert( address->type != NEXT_ADDRESS_NONE );

    // first scan existing entries and see if we can insert there

    const int size = session_manager->size;

    for ( int i = 0; i < size; ++i )
    {
        if ( session_manager->addresses[i].type == NEXT_ADDRESS_NONE )
        {
            session_manager->addresses[i] = *address;
            next_proxy_session_entry_t * entry = &session_manager->entries[i];
            entry->address = *address;
            entry->session_id = session_id;
            next_bandwidth_limiter_reset( &entry->send_bandwidth );
            if ( i > session_manager->max_entry_index )
            {
                session_manager->max_entry_index = i;
            }
            return entry;
        }        
    }  

    // ok, we need to grow, expand and add at the end (expand compacts existing entries)

    if ( !next_proxy_session_manager_expand( session_manager ) )
        return NULL;

    const int i = ++session_manager->max_entry_index;
    session_manager->addresses[i] = *address;
    next_proxy_session_entry_t * entry = &session_manager->entries[i];
    entry->address = *address;
    entry->session_id = session_id;
    next_bandwidth_limiter_reset( &entry->send_bandwidth );

    return entry;
}

void next_proxy_session_manager_remove_at_index( next_proxy_session_manager_t * session_manager, int index )
{
    next_assert( session_manager );
    next_assert( index >= 0 );
    next_assert( index <= session_manager->max_entry_index );
    const int max_index = session_manager->max_entry_index;
    session_manager->addresses[index].type = NEXT_ADDRESS_NONE;
    if ( index == max_index )
    {
        while ( index > 0 && session_manager->addresses[index].type == NEXT_ADDRESS_NONE )
        {
            index--;
        }
        session_manager->max_entry_index = index;
    }
}

void next_proxy_session_manager_remove_by_address( next_proxy_session_manager_t * session_manager, const next_address_t * address )
{
    next_assert( session_manager );
    next_assert( address );
    const int max_index = session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( next_address_equal( address, &session_manager->addresses[i] ) == 1 )
        {
            next_proxy_session_manager_remove_at_index( session_manager, i );
            return;
        }        
    }
}

next_proxy_session_entry_t * next_proxy_session_manager_find( next_proxy_session_manager_t * session_manager, const next_address_t * address )
{
    next_assert( session_manager );
    next_assert( address );
    const int max_index = session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( next_address_equal( address, &session_manager->addresses[i] ) == 1 )
        {
            return &session_manager->entries[i];
        }        
    }
    return NULL;
}

int next_proxy_session_manager_num_entries( next_proxy_session_manager_t * proxy_session_manager )
{
    next_assert( proxy_session_manager );
    int num_entries = 0;
    const int max_index = proxy_session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( proxy_session_manager->addresses[i].type != 0 )
        {
            num_entries++;
        }
    }
    return num_entries;
}

// ---------------------------------------------------------------

struct next_session_entry_t
{
    next_address_t address;
    uint64_t session_id;
    uint8_t most_recent_session_version;
    uint64_t special_send_sequence;
    uint64_t internal_send_sequence;
    uint64_t stats_sequence;
    uint64_t user_hash;
    uint64_t platform_id_override;
    uint64_t tag;
    uint8_t client_open_session_sequence;

    uint64_t stats_flags;
    bool stats_flagged;
    bool stats_multipath;
    bool stats_fallback_to_direct;
    bool stats_try_before_you_buy;
    uint64_t stats_platform_id;
    int stats_connection_type;
    float stats_kbps_up;
    float stats_kbps_down;
    float stats_direct_min_rtt;
    float stats_direct_max_rtt;
    float stats_direct_mean_rtt;
    float stats_direct_jitter;
    float stats_direct_packet_loss;
    bool stats_next;
    float stats_next_min_rtt;
    float stats_next_max_rtt;
    float stats_next_mean_rtt;
    float stats_next_jitter;
    float stats_next_packet_loss;
    int stats_num_near_relays;
    uint64_t stats_near_relay_ids[NEXT_MAX_NEAR_RELAYS];
    float stats_near_relay_min_rtt[NEXT_MAX_NEAR_RELAYS];
    float stats_near_relay_max_rtt[NEXT_MAX_NEAR_RELAYS];
    float stats_near_relay_mean_rtt[NEXT_MAX_NEAR_RELAYS];
    float stats_near_relay_jitter[NEXT_MAX_NEAR_RELAYS];
    float stats_near_relay_packet_loss[NEXT_MAX_NEAR_RELAYS];
    uint64_t stats_packets_lost_client_to_server;
    uint64_t stats_packets_lost_server_to_client;
    
    double next_packet_loss_update_time;
    double next_session_update_time;
    double next_session_resend_time;
    double last_client_stats_update;

    uint64_t update_sequence;
    bool update_dirty;
    bool waiting_for_update_response;
    bool multipath;
    double update_last_send_time;
    uint8_t update_type;
    int update_num_tokens;
    uint8_t update_tokens[NEXT_MAX_TOKENS*NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES];
    int update_num_near_relays;
    uint64_t update_near_relay_ids[NEXT_MAX_NEAR_RELAYS];
    next_address_t update_near_relay_addresses[NEXT_MAX_NEAR_RELAYS];
    uint8_t update_packet_data[NEXT_MAX_PACKET_BYTES];
    int update_packet_bytes;

    bool has_pending_route;
    uint8_t pending_route_session_version;
    uint64_t pending_route_expire_timestamp;
    double pending_route_expire_time;
    int pending_route_kbps_up;
    int pending_route_kbps_down;
    next_address_t pending_route_send_address;
    uint8_t pending_route_private_key[crypto_box_SECRETKEYBYTES];

    bool has_current_route;
    uint8_t current_route_session_version;
    uint64_t current_route_expire_timestamp;
    double current_route_expire_time;
    int current_route_kbps_up;
    int current_route_kbps_down;
    next_address_t current_route_send_address;
    uint8_t current_route_private_key[crypto_box_SECRETKEYBYTES];

    bool has_previous_route;
    next_address_t previous_route_send_address;
    uint8_t previous_route_private_key[crypto_box_SECRETKEYBYTES];

    uint8_t ephemeral_private_key[crypto_secretbox_KEYBYTES];
    uint8_t send_key[crypto_kx_SESSIONKEYBYTES];
    uint8_t receive_key[crypto_kx_SESSIONKEYBYTES];
    uint8_t client_route_public_key[crypto_box_PUBLICKEYBYTES];

    uint8_t upgrade_token[NEXT_UPGRADE_TOKEN_BYTES];
    
    next_replay_protection_t payload_replay_protection;
    next_replay_protection_t special_replay_protection;
    next_replay_protection_t internal_replay_protection;
    next_packet_loss_tracker_t packet_loss_tracker;

    bool mutex_multipath;
    int mutex_envelope_kbps_up;
    int mutex_envelope_kbps_down;
    uint64_t mutex_payload_send_sequence;
    uint64_t mutex_session_id;
    uint8_t mutex_session_version;
    bool mutex_send_over_network_next;
    next_address_t mutex_send_address;
    uint8_t mutex_private_key[crypto_box_SECRETKEYBYTES];
};

struct next_session_manager_t
{
    void * context;
    int size;
    int max_entry_index;
    uint64_t * session_ids;
    next_address_t * addresses;
    next_session_entry_t * entries;
};

void next_session_manager_destroy( next_session_manager_t * session_manager );

next_session_manager_t * next_session_manager_create( void * context, int initial_size )
{
    next_session_manager_t * session_manager = (next_session_manager_t*) next_malloc( context, sizeof(next_session_manager_t) );
    next_assert( session_manager );
    if ( !session_manager )
        return NULL;
    memset( session_manager, 0, sizeof(next_session_manager_t) );
    session_manager->context = context;
    session_manager->size = initial_size;
    session_manager->session_ids = (uint64_t*) next_malloc( context, initial_size * 8 );
    session_manager->addresses = (next_address_t*) next_malloc( context, initial_size * sizeof(next_address_t) );
    session_manager->entries = (next_session_entry_t*) next_malloc( context, initial_size * sizeof(next_session_entry_t) );
    next_assert( session_manager->session_ids );
    next_assert( session_manager->addresses );
    next_assert( session_manager->entries );
    if ( session_manager->session_ids == NULL || session_manager->addresses == NULL || session_manager->entries == NULL )
    {
        next_session_manager_destroy( session_manager );
        return NULL;
    }
    memset( session_manager->session_ids, 0, initial_size * 8 );
    memset( session_manager->addresses, 0, initial_size * sizeof(next_address_t) );
    memset( session_manager->entries, 0, initial_size * sizeof(next_session_entry_t) );
    return session_manager;
}

void next_session_manager_destroy( next_session_manager_t * session_manager )
{
    next_assert( session_manager );
    next_free( session_manager->context, session_manager->session_ids );
    next_free( session_manager->context, session_manager->addresses );
    next_free( session_manager->context, session_manager->entries );
    clear_and_free( session_manager->context, session_manager, sizeof(next_session_manager_t) );
}

bool next_session_manager_expand( next_session_manager_t * session_manager )
{
    next_assert( session_manager );

    int new_size = session_manager->size * 2;

    uint64_t * new_session_ids = (uint64_t*) next_malloc( session_manager->context, new_size * 8 );
    next_address_t * new_addresses = (next_address_t*) next_malloc( session_manager->context, new_size * sizeof(next_address_t) );
    next_session_entry_t * new_entries = (next_session_entry_t*) next_malloc( session_manager->context, new_size * sizeof(next_session_entry_t) );

    next_assert( session_manager->session_ids );
    next_assert( session_manager->addresses );
    next_assert( session_manager->entries );

    if ( session_manager->session_ids == NULL || session_manager->addresses == NULL || session_manager->entries == NULL )
    {
        next_free( session_manager->context, new_session_ids );
        next_free( session_manager->context, new_addresses );
        next_free( session_manager->context, new_entries );
        return false;
    }

    memset( new_session_ids, 0, new_size * 8 );
    memset( new_addresses, 0, new_size * sizeof(next_address_t) );
    memset( new_entries, 0, new_size * sizeof(next_session_entry_t) );

    int index = 0;
    const int current_size = session_manager->size;
    for ( int i = 0; i < current_size; ++i )
    {
        if ( session_manager->session_ids[i] != 0 )
        {
            memcpy( &new_session_ids[index], &session_manager->session_ids[i], 8 );
            memcpy( &new_addresses[index], &session_manager->addresses[i], sizeof(next_address_t) );
            memcpy( &new_entries[index], &session_manager->entries[i], sizeof(next_session_entry_t) );
            index++;            
        }
    }

    next_free( session_manager->context, session_manager->session_ids );
    next_free( session_manager->context, session_manager->addresses );
    next_free( session_manager->context, session_manager->entries );

    session_manager->session_ids = new_session_ids;
    session_manager->addresses = new_addresses;
    session_manager->entries = new_entries;
    session_manager->size = new_size;
    session_manager->max_entry_index = index - 1;

    return true;
}

void next_clear_session_entry( next_session_entry_t * entry, const next_address_t * address, uint64_t session_id )
{
    memset( entry, 0, sizeof(next_session_entry_t) );
    entry->address = *address;
    entry->session_id = session_id;
    next_replay_protection_reset( &entry->payload_replay_protection );
    next_replay_protection_reset( &entry->special_replay_protection );
    next_replay_protection_reset( &entry->internal_replay_protection );
    next_packet_loss_tracker_reset( &entry->packet_loss_tracker );
    entry->stats_try_before_you_buy = true;
}

next_session_entry_t * next_session_manager_add( next_session_manager_t * session_manager, const next_address_t * address, uint64_t session_id, const uint8_t * ephemeral_private_key, const uint8_t * upgrade_token )
{
    next_assert( session_manager );
    next_assert( session_id != 0 );
    next_assert( address );
    next_assert( address->type != NEXT_ADDRESS_NONE );

    // first scan existing entries and see if we can insert there

    const int size = session_manager->size;

    for ( int i = 0; i < size; ++i )
    {
        if ( session_manager->session_ids[i] == 0 )
        {
            session_manager->session_ids[i] = session_id;
            session_manager->addresses[i] = *address;
            next_session_entry_t * entry = &session_manager->entries[i];
            next_clear_session_entry( entry, address, session_id );
            memcpy( entry->ephemeral_private_key, ephemeral_private_key, crypto_secretbox_KEYBYTES );
            memcpy( entry->upgrade_token, upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );
            if ( i > session_manager->max_entry_index )
            {
                session_manager->max_entry_index = i;
            }
            return entry;
        }
    }  

    // ok, we need to grow, expand and add at the end (expand compacts existing entries)

    if ( !next_session_manager_expand( session_manager ) )
        return NULL;

    const int i = ++session_manager->max_entry_index;

    session_manager->session_ids[i] = session_id;
    session_manager->addresses[i] = *address;
    next_session_entry_t * entry = &session_manager->entries[i];
    next_clear_session_entry( entry, address, session_id );
    memcpy( entry->ephemeral_private_key, ephemeral_private_key, crypto_secretbox_KEYBYTES );
    memcpy( entry->upgrade_token, upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );

    return entry;
}

void next_session_manager_remove_at_index( next_session_manager_t * session_manager, int index )
{
    next_assert( session_manager );
    next_assert( index >= 0 );
    next_assert( index <= session_manager->max_entry_index );
    const int max_index = session_manager->max_entry_index;
    session_manager->session_ids[index] = 0;
    session_manager->addresses[index].type = NEXT_ADDRESS_NONE;
    if ( index == max_index )
    {
        while ( index > 0 && session_manager->session_ids[index] == 0 )
        {
            index--;
        }
        session_manager->max_entry_index = index;
    }
}

void next_session_manager_remove_by_address( next_session_manager_t * session_manager, const next_address_t * address )
{
    next_assert( session_manager );
    next_assert( address );
    const int max_index = session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( next_address_equal( address, &session_manager->addresses[i] ) == 1 )
        {
            next_session_manager_remove_at_index( session_manager, i );
            return;
        }        
    }
}

next_session_entry_t * next_session_manager_find_by_address( next_session_manager_t * session_manager, const next_address_t * address )
{
    next_assert( session_manager );
    next_assert( address );
    const int max_index = session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( next_address_equal( address, &session_manager->addresses[i] ) == 1 )
        {
            return &session_manager->entries[i];
        }        
    }
    return NULL;
}

next_session_entry_t * next_session_manager_find_by_session_id( next_session_manager_t * session_manager, uint64_t session_id )
{
    next_assert( session_manager );
    next_assert( session_id );
    const int max_index = session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( session_id == session_manager->session_ids[i] )
        {
            return &session_manager->entries[i];
        }        
    }
    return NULL;
}

int next_session_manager_num_entries( next_session_manager_t * session_manager )
{
    next_assert( session_manager );
    int num_entries = 0;
    const int max_index = session_manager->max_entry_index;
    for ( int i = 0; i <= max_index; ++i )
    {
        if ( session_manager->session_ids[i] != 0 )
        {
            num_entries++;
        }        
    }
    return num_entries;
}

// ---------------------------------------------------------------

#define NEXT_BACKEND_PACKET_BASE                        200
#define NEXT_BACKEND_SERVER_UPDATE_PACKET               NEXT_BACKEND_PACKET_BASE
#define NEXT_BACKEND_SESSION_UPDATE_PACKET              NEXT_BACKEND_PACKET_BASE + 1
#define NEXT_BACKEND_SESSION_RESPONSE_PACKET            NEXT_BACKEND_PACKET_BASE + 2

struct NextBackendServerUpdatePacket
{
    uint64_t sequence;
    int version_major;
    int version_minor;
    int version_patch;
    uint64_t customer_id;
    uint64_t datacenter_id;
    uint32_t num_sessions_pending;
    uint32_t num_sessions_upgraded;
    next_address_t server_address;
    next_address_t server_address_internal;
    uint8_t server_route_public_key[crypto_box_PUBLICKEYBYTES];
    uint8_t signature[crypto_sign_BYTES];

    NextBackendServerUpdatePacket()
    {
        sequence = 0;
        version_major = NEXT_VERSION_MAJOR_INT;
        version_minor = NEXT_VERSION_MINOR_INT;
        version_patch = NEXT_VERSION_PATCH_INT;
        customer_id = 0;
        datacenter_id = 0;
        num_sessions_pending = 0;
        num_sessions_upgraded = 0;
        memset( &server_address, 0, sizeof(next_address_t) );
        memset( &server_address_internal, 0, sizeof(next_address_t) );
        memset( server_route_public_key, 0, sizeof(server_route_public_key) );
        memset( signature, 0, crypto_sign_BYTES );
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, sequence );
        serialize_int( stream, version_major, 0, NEXT_VERSION_MAJOR_MAX );
        serialize_int( stream, version_minor, 0, NEXT_VERSION_MINOR_MAX );
        serialize_int( stream, version_patch, 0, NEXT_VERSION_PATCH_MAX );
        serialize_uint64( stream, customer_id );
        serialize_uint64( stream, datacenter_id );
        serialize_uint32( stream, num_sessions_pending );
        serialize_uint32( stream, num_sessions_upgraded );
        serialize_address( stream, server_address );
        serialize_address( stream, server_address_internal );
        serialize_bytes( stream, server_route_public_key, crypto_box_PUBLICKEYBYTES );
        serialize_bytes( stream, signature, crypto_sign_BYTES );
        return true;
    }

    int GetSignData( uint8_t * buffer, int buffer_size )
    {
        uint8_t * p = buffer;
        next_write_uint64( &p, sequence );
        next_write_uint64( &p, version_major );
        next_write_uint64( &p, version_minor );
        next_write_uint64( &p, version_patch );
        next_write_uint64( &p, customer_id );
        next_write_uint64( &p, datacenter_id );
        next_write_uint32( &p, num_sessions_pending );
        next_write_uint32( &p, num_sessions_upgraded );
        next_write_address( &p, &server_address );
        next_write_address( &p, &server_address_internal );
        next_write_bytes( &p, server_route_public_key, crypto_box_PUBLICKEYBYTES );
        next_assert( p - buffer <= buffer_size );
        (void) buffer_size;
        return p - buffer;
    }

    void Sign( const uint8_t * private_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[1024];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        crypto_sign_final_create( &state, signature, NULL, private_key );
    }

    bool Verify( const uint8_t * public_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[1024];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        return crypto_sign_final_verify( &state, signature, public_key ) == 0;
    }
};

struct NextBackendSessionUpdatePacket
{
    uint64_t sequence;
    uint64_t customer_id;
    next_address_t server_address;
    uint64_t session_id;
    uint64_t user_hash;
    uint64_t platform_id;
    uint64_t tag;
    uint64_t flags;
    bool flagged;
    bool fallback_to_direct;
    bool try_before_you_buy;
    int connection_type;
    float direct_min_rtt;
    float direct_max_rtt;
    float direct_mean_rtt;
    float direct_jitter;
    float direct_packet_loss;
    bool next;
    float next_min_rtt;
    float next_max_rtt;
    float next_mean_rtt;
    float next_jitter;
    float next_packet_loss;
    int num_near_relays;
    uint64_t near_relay_ids[NEXT_MAX_NEAR_RELAYS];
    float near_relay_min_rtt[NEXT_MAX_NEAR_RELAYS];
    float near_relay_max_rtt[NEXT_MAX_NEAR_RELAYS];
    float near_relay_mean_rtt[NEXT_MAX_NEAR_RELAYS];
    float near_relay_jitter[NEXT_MAX_NEAR_RELAYS];
    float near_relay_packet_loss[NEXT_MAX_NEAR_RELAYS];
    next_address_t client_address;
    uint8_t client_route_public_key[crypto_box_PUBLICKEYBYTES];
    uint32_t kbps_up;
    uint32_t kbps_down;
    uint64_t packets_lost_client_to_server;
    uint64_t packets_lost_server_to_client;
    uint8_t signature[crypto_sign_BYTES];

    NextBackendSessionUpdatePacket()
    {
        memset( this, 0, sizeof(NextBackendSessionUpdatePacket) );
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, sequence );
        serialize_uint64( stream, customer_id );
        serialize_address( stream, server_address );
        serialize_uint64( stream, session_id );
        serialize_uint64( stream, user_hash );
        serialize_uint64( stream, platform_id );
        serialize_uint64( stream, tag );
        serialize_bits( stream, flags, NEXT_FLAGS_COUNT );
        serialize_bool( stream, flagged );
        serialize_bool( stream, fallback_to_direct );
        serialize_bool( stream, try_before_you_buy );
        serialize_int( stream, connection_type, NEXT_CONNECTION_TYPE_UNKNOWN, NEXT_CONNECTION_TYPE_CELLULAR );
        serialize_float( stream, direct_min_rtt );
        serialize_float( stream, direct_max_rtt );
        serialize_float( stream, direct_mean_rtt );
        serialize_float( stream, direct_jitter );
        serialize_float( stream, direct_packet_loss );
        serialize_bool( stream, next );
        if ( next )
        {
            serialize_float( stream, next_min_rtt );
            serialize_float( stream, next_max_rtt );
            serialize_float( stream, next_mean_rtt );
            serialize_float( stream, next_jitter );
            serialize_float( stream, next_packet_loss );
        }
        serialize_int( stream, num_near_relays, 0, NEXT_MAX_NEAR_RELAYS );
        for ( int i = 0; i < num_near_relays; ++i )
        {
            serialize_uint64( stream, near_relay_ids[i] );
            serialize_float( stream, near_relay_min_rtt[i] );
            serialize_float( stream, near_relay_max_rtt[i] );
            serialize_float( stream, near_relay_mean_rtt[i] );
            serialize_float( stream, near_relay_jitter[i] );
            serialize_float( stream, near_relay_packet_loss[i] );
        }
        serialize_address( stream, client_address );
        serialize_bytes( stream, client_route_public_key, crypto_box_PUBLICKEYBYTES );
        serialize_uint32( stream, kbps_up );
        serialize_uint32( stream, kbps_down );
        serialize_uint64( stream, packets_lost_client_to_server );
        serialize_uint64( stream, packets_lost_server_to_client );
        serialize_bytes( stream, signature, crypto_sign_BYTES );
        return true;
    }

    int GetSignData( uint8_t * buffer, int buffer_size )
    {
        uint8_t * p = buffer;
        next_write_uint64( &p, sequence );
        next_write_uint64( &p, customer_id );
        next_write_uint64( &p, session_id );
        next_write_uint64( &p, user_hash );
        next_write_uint64( &p, platform_id );
        next_write_uint64( &p, tag );
        next_write_uint32( &p, flags );
        next_write_uint8( &p, (uint8_t) flagged );
        next_write_uint8( &p, (uint8_t) fallback_to_direct );
        next_write_uint8( &p, (uint8_t) try_before_you_buy );
        next_write_uint8( &p, connection_type );
        next_write_uint8( &p, (uint8_t) next );
        next_write_float32( &p, direct_min_rtt );
        next_write_float32( &p, direct_max_rtt );
        next_write_float32( &p, direct_mean_rtt );
        next_write_float32( &p, direct_jitter );
        next_write_float32( &p, direct_packet_loss );
        next_write_float32( &p, next_min_rtt );
        next_write_float32( &p, next_max_rtt );
        next_write_float32( &p, next_mean_rtt );
        next_write_float32( &p, next_jitter );
        next_write_float32( &p, next_packet_loss );
        next_write_uint32( &p, num_near_relays );
        for ( int i = 0; i < num_near_relays; ++i )
        {
            next_write_uint64( &p, near_relay_ids[i] );
            next_write_float32( &p, near_relay_min_rtt[i] );
            next_write_float32( &p, near_relay_max_rtt[i] );
            next_write_float32( &p, near_relay_mean_rtt[i] );
            next_write_float32( &p, near_relay_jitter[i] );
            next_write_float32( &p, near_relay_packet_loss[i] );
        }
        next_write_address( &p, &client_address );
        next_write_address( &p, &server_address );
        next_write_uint32( &p, kbps_up );
        next_write_uint32( &p, kbps_down );
        next_write_uint64( &p, packets_lost_client_to_server );
        next_write_uint64( &p, packets_lost_server_to_client );
        next_write_bytes( &p, client_route_public_key, crypto_box_PUBLICKEYBYTES );
        next_assert( p - buffer <= buffer_size );
        (void) buffer_size;
        return p - buffer;
    }

    void Sign( const uint8_t * private_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[1500];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        crypto_sign_final_create( &state, signature, NULL, private_key );
    }

    bool Verify( const uint8_t * public_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[1500];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        return crypto_sign_final_verify( &state, signature, public_key ) == 0;
    }
};

struct NextBackendSessionResponsePacket
{
    uint64_t sequence;
    uint64_t session_id;
    int num_near_relays;
    uint64_t near_relay_ids[NEXT_MAX_NEAR_RELAYS];
    next_address_t near_relay_addresses[NEXT_MAX_NEAR_RELAYS];
    uint8_t response_type;
    bool multipath;
    int num_tokens;
    uint8_t tokens[NEXT_MAX_TOKENS*NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES];
    uint8_t server_route_public_key[crypto_box_PUBLICKEYBYTES];
    uint8_t signature[crypto_sign_BYTES];

    NextBackendSessionResponsePacket()
    {
        memset( this, 0, sizeof(NextBackendSessionResponsePacket) );
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_uint64( stream, sequence );
        serialize_uint64( stream, session_id );
        serialize_int( stream, num_near_relays, 0, NEXT_MAX_NEAR_RELAYS );
        for ( int i = 0; i < num_near_relays; ++i )
        {
            serialize_uint64( stream, near_relay_ids[i] );
            serialize_address( stream, near_relay_addresses[i] );
        }
        serialize_int( stream, response_type, 0, NEXT_UPDATE_TYPE_CONTINUE );
        if ( response_type != NEXT_UPDATE_TYPE_DIRECT )
        {
            serialize_bool( stream, multipath );
            serialize_int( stream, num_tokens, 0, NEXT_MAX_TOKENS );
        }
        if ( response_type == NEXT_UPDATE_TYPE_ROUTE )
        {
            serialize_bytes( stream, tokens, num_tokens * NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES );
        }
        else if ( response_type == NEXT_UPDATE_TYPE_CONTINUE )
        {
            serialize_bytes( stream, tokens, num_tokens * NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES );
        }
        serialize_bytes( stream, server_route_public_key, crypto_box_PUBLICKEYBYTES );
        serialize_bytes( stream, signature, crypto_sign_BYTES );
        return true;
    }

    int GetSignData( uint8_t * buffer, int buffer_size )
    {
        uint8_t * p = buffer;
        next_write_uint64( &p, sequence );
        next_write_uint64( &p, session_id );
        next_write_uint8( &p, num_near_relays );
        for ( int i = 0; i < num_near_relays; ++i )
        {
            next_write_uint64( &p, near_relay_ids[i] );
            next_write_address( &p, &near_relay_addresses[i] );
        }
        next_write_uint8( &p, response_type );
        if ( response_type != NEXT_UPDATE_TYPE_DIRECT )
        {
            next_write_uint8( &p, multipath );
            next_write_uint8( &p, num_tokens );
        }
        if ( response_type == NEXT_UPDATE_TYPE_ROUTE )
        {
            next_write_bytes( &p, tokens, num_tokens * NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES );
        }
        else if ( response_type == NEXT_UPDATE_TYPE_CONTINUE )
        {
            next_write_bytes( &p, tokens, num_tokens * NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES );
        }
        next_write_bytes( &p, server_route_public_key, crypto_box_PUBLICKEYBYTES );
        next_assert( p - buffer <= buffer_size );
        (void) buffer_size;
        return p - buffer;
    }

    void Sign( const uint8_t * private_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[2048];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        crypto_sign_final_create( &state, signature, NULL, private_key );
    }

    bool Verify( const uint8_t * public_key )
    {
        crypto_sign_state state;
        crypto_sign_init( &state );
        uint8_t sign_data[2048];
        const int sign_bytes = GetSignData( sign_data, sizeof(sign_data) );
        crypto_sign_update( &state, sign_data, sign_bytes );
        return crypto_sign_final_verify( &state, signature, public_key ) == 0;
    }
};

// ---------------------------------------------------------------

int next_write_backend_packet( uint8_t packet_id, void * packet_object, uint8_t * packet_data, int * packet_bytes )
{
    next_assert( packet_object );
    next_assert( packet_data );
    next_assert( packet_bytes );

    next::WriteStream stream( packet_data, NEXT_MAX_PACKET_BYTES );

    typedef next::WriteStream Stream;

    serialize_bits( stream, packet_id, 8 );

    switch ( packet_id )
    {
        case NEXT_BACKEND_SERVER_UPDATE_PACKET:
        {
            NextBackendServerUpdatePacket * packet = (NextBackendServerUpdatePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;
        
        case NEXT_BACKEND_SESSION_UPDATE_PACKET:
        {
            NextBackendSessionUpdatePacket * packet = (NextBackendSessionUpdatePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_BACKEND_SESSION_RESPONSE_PACKET:
        {
            NextBackendSessionResponsePacket * packet = (NextBackendSessionResponsePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        default:
            return NEXT_ERROR;
    }

    stream.Flush();

    *packet_bytes = stream.GetBytesProcessed();

    return NEXT_OK;
}

int next_read_backend_packet( uint8_t * packet_data, int packet_bytes, void * packet_object )
{
    next_assert( packet_data );
    next_assert( packet_object );

    if ( packet_bytes < 1 )
        return NEXT_ERROR;

    uint8_t packet_id = packet_data[0];

    next::ReadStream stream( packet_data + 1, packet_bytes - 1 );

    switch ( packet_id )
    {
        case NEXT_BACKEND_SERVER_UPDATE_PACKET:
        {
            NextBackendServerUpdatePacket * packet = (NextBackendServerUpdatePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;
        
        case NEXT_BACKEND_SESSION_UPDATE_PACKET:
        {
            NextBackendSessionUpdatePacket * packet = (NextBackendSessionUpdatePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        case NEXT_BACKEND_SESSION_RESPONSE_PACKET:
        {
            NextBackendSessionResponsePacket * packet = (NextBackendSessionResponsePacket*) packet_object;
            if ( !packet->Serialize( stream ) )
                return NEXT_ERROR;
        }
        break;

        default:
            return NEXT_ERROR;
    }

    return (int) packet_id;
}

// ---------------------------------------------------------------

#define NEXT_SERVER_COMMAND_UPGRADE_SESSION             0
#define NEXT_SERVER_COMMAND_TAG_SESSION                 1
#define NEXT_SERVER_COMMAND_DESTROY                     2

struct next_server_command_t
{
    int type;
};

struct next_server_command_upgrade_session_t : public next_server_command_t
{
    next_address_t address;
    uint64_t session_id;
    uint64_t user_hash;
    uint64_t platform_id;
    uint64_t tag;
};

struct next_server_command_tag_session_t : public next_server_command_t
{
    next_address_t address;
    uint64_t tag;
};

struct next_server_command_destroy_t : public next_server_command_t
{
    // ...
};

// ---------------------------------------------------------------

#define NEXT_SERVER_NOTIFY_PACKET_RECEIVED                      0
#define NEXT_SERVER_NOTIFY_PENDING_SESSION_TIMED_OUT            1
#define NEXT_SERVER_NOTIFY_SESSION_UPGRADED                     2
#define NEXT_SERVER_NOTIFY_SESSION_TIMED_OUT                    3
#define NEXT_SERVER_NOTIFY_FAILED_TO_RESOLVE_HOSTNAME           4

struct next_server_notify_t
{
    int type;
};

struct next_server_notify_packet_received_t : public next_server_notify_t
{
    next_address_t from;
    int packet_bytes;
    uint8_t packet_data[NEXT_MTU];
};

struct next_server_notify_pending_session_cancelled_t : public next_server_notify_t
{
    next_address_t address;
    uint64_t session_id;
};

struct next_server_notify_pending_session_timed_out_t : public next_server_notify_t
{
    next_address_t address;
    uint64_t session_id;
};

struct next_server_notify_session_upgraded_t : public next_server_notify_t
{
    next_address_t address;
    uint64_t session_id;
    uint64_t tag;
};

struct next_server_notify_session_timed_out_t : public next_server_notify_t
{
    next_address_t address;
    uint64_t session_id;
};

struct next_server_notify_failed_to_resolve_hostname_t : public next_server_notify_t
{
    // ...
};

// ---------------------------------------------------------------

struct next_server_internal_t
{
    void * context;
    uint64_t datacenter_id;
    uint64_t customer_id;
    uint8_t customer_private_key[crypto_sign_SECRETKEYBYTES];
    bool invalid_customer_private_key;
    bool no_datacenter_specified;
    bool failed_to_resolve_hostname;
    bool resolve_hostname_finished;
    bool first_server_update;
    uint64_t upgrade_sequence;
    uint64_t server_update_sequence;
    double last_backend_server_update;
    next_address_t resolve_hostname_result;
    next_address_t backend_address;
    next_address_t server_address;
    next_address_t server_address_internal;
    next_address_t bind_address;
    next_queue_t * command_queue;
    next_queue_t * notify_queue;
    next_platform_mutex_t * session_mutex;
    next_platform_mutex_t * command_mutex;
    next_platform_mutex_t * notify_mutex;
    next_platform_socket_t * socket;
    next_platform_mutex_t * resolve_hostname_mutex;
    next_platform_thread_t * resolve_hostname_thread;
    next_pending_session_manager_t * pending_session_manager;
    next_session_manager_t * session_manager;
    uint8_t server_kx_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t server_kx_private_key[crypto_kx_SECRETKEYBYTES];
    uint8_t server_route_public_key[crypto_box_PUBLICKEYBYTES];
    uint8_t server_route_private_key[crypto_box_SECRETKEYBYTES];
};

void next_server_internal_destroy( next_server_internal_t * server );

static next_platform_thread_return_t NEXT_PLATFORM_THREAD_FUNC next_server_internal_resolve_hostname_thread_function( void * context );

static next_platform_thread_return_t NEXT_PLATFORM_THREAD_FUNC next_server_internal_thread_function( void * context );

next_server_internal_t * next_server_internal_create( void * context, const char * server_address_string, const char * bind_address_string, const char * datacenter_string )
{
    #ifdef NEXT_VERSION_IS_PRESENT
    next_printf( NEXT_LOG_LEVEL_INFO, "server sdk version is %s", NEXT_VERSION_FULL );
    #endif // #ifdef NEXT_VERSION_IS_PRESENT

    next_assert( server_address_string );
    next_assert( bind_address_string );

    const char * server_address_override = next_platform_getenv( "NEXT_SERVER_ADDRESS" );
    if ( server_address_override )
    {
        server_address_string = server_address_override;
    }

    next_address_t server_address;
    if ( next_address_parse( &server_address, server_address_string ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server failed to parse server address: %s", server_address_string );
        return NULL;
    }

    const char * bind_address_override = next_platform_getenv( "NEXT_BIND_ADDRESS" );
    if ( bind_address_override )
    {
        bind_address_string = bind_address_override;
    }

    next_address_t bind_address;
    if ( next_address_parse( &bind_address, bind_address_string ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server failed to parse bind address: %s", bind_address_string );
        return NULL;
    }

    next_server_internal_t * server = (next_server_internal_t*) next_malloc( context, sizeof(next_server_internal_t) );
    if ( !server ) 
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create internal server" );
        return NULL;
    }

    memset( server, 0, sizeof( next_server_internal_t) );

    server->context = context;
    server->customer_id = next_global_config.customer_id;
    memcpy( server->customer_private_key, next_global_config.customer_private_key, crypto_sign_SECRETKEYBYTES );
    server->invalid_customer_private_key = next_global_config.invalid_customer_private_key;

    if ( !server->invalid_customer_private_key )
    {
        const char * datacenter = datacenter_string;
        const char * datacenter_env = next_platform_getenv( "NEXT_DATACENTER" );
        if ( datacenter_env )
        {
            datacenter = datacenter_env;
        }
        if ( datacenter != NULL && datacenter[0] != 0 )
        {
            next_printf( NEXT_LOG_LEVEL_INFO, "server datacenter is '%s'", datacenter );
            server->datacenter_id = next_datacenter_id( datacenter );
        }
        else
        {
            next_printf( NEXT_LOG_LEVEL_ERROR, "server could not determine datacenter it is running in" );
            server->no_datacenter_specified = true;
        }
    }

    server->command_queue = next_queue_create( context, NEXT_COMMAND_QUEUE_LENGTH );
    if ( !server->command_queue )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create command queue" );
        next_server_internal_destroy( server );
        return NULL;
    }
    
    server->notify_queue = next_queue_create( context, NEXT_NOTIFY_QUEUE_LENGTH );
    if ( !server->notify_queue )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create notify queue" );
        next_server_internal_destroy( server );
        return NULL;
    }

    server->socket = next_platform_socket_create( server->context, &bind_address, NEXT_PLATFORM_SOCKET_BLOCKING, 0.1f, next_global_config.socket_send_buffer_size, next_global_config.socket_receive_buffer_size );
    if ( server->socket == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create server socket" );
        next_server_internal_destroy( server );
        return NULL;
    }

    if ( server_address.port == 0 )
    {
        server_address.port = bind_address.port;
    }

    server->bind_address = bind_address;
    server->server_address = server_address;
    server->server_address_internal = server_address;

    const char * internal_address_string = next_platform_getenv( "NEXT_SERVER_ADDRESS_INTERNAL" );
    if ( internal_address_string )
        next_address_parse( &server->server_address_internal, internal_address_string );

    server->session_mutex = next_platform_mutex_create( server->context );
    if ( server->session_mutex == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create session mutex" );
        next_server_internal_destroy( server );
        return NULL;
    }

    server->command_mutex = next_platform_mutex_create( server->context );
    if ( server->command_mutex == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create command mutex" );
        next_server_internal_destroy( server );
        return NULL;
    }

    server->notify_mutex = next_platform_mutex_create( server->context );
    if ( server->notify_mutex == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create notify mutex" );
        next_server_internal_destroy( server );
        return NULL;
    }

    server->resolve_hostname_mutex = next_platform_mutex_create( server->context );
    if ( server->resolve_hostname_mutex == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create resolve hostname mutex" );
        next_server_internal_destroy( server );
        return NULL;
    }

    server->pending_session_manager = next_pending_session_manager_create( context, NEXT_INITIAL_PENDING_SESSION_SIZE );
    if ( server->pending_session_manager == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create pending session manager" );
        next_server_internal_destroy( server );
        return NULL;
    }

    server->session_manager = next_session_manager_create( context, NEXT_INITIAL_SESSION_SIZE );
    if ( server->session_manager == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create session manager" );
        next_server_internal_destroy( server );
        return NULL;
    }

    server->resolve_hostname_thread = next_platform_thread_create( server->context, next_server_internal_resolve_hostname_thread_function, server );
    if ( !server->resolve_hostname_thread )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create resolve hostname thread" );
        next_server_internal_destroy( server );
        return NULL;
    }

    char address_string[NEXT_MAX_ADDRESS_STRING_LENGTH];
    next_printf( NEXT_LOG_LEVEL_INFO, "server started on %s", next_address_to_string( &server_address, address_string ) );

    crypto_kx_keypair( server->server_kx_public_key, server->server_kx_private_key );

    crypto_box_keypair( server->server_route_public_key, server->server_route_private_key );

    server->last_backend_server_update = next_time() - NEXT_SECONDS_BETWEEN_SERVER_UPDATES * next_random_float();

    server->first_server_update = true;

    return server;
}

void next_server_internal_destroy( next_server_internal_t * server )
{
    next_assert( server );
    if ( server->socket )
    {
        next_platform_socket_destroy( server->socket );
    }
    if ( server->session_mutex )
    {
        next_platform_mutex_destroy( server->session_mutex );
    }
    if ( server->command_mutex )
    {
        next_platform_mutex_destroy( server->command_mutex );
    }
    if ( server->notify_mutex )
    {
        next_platform_mutex_destroy( server->notify_mutex );
    }
    if ( server->resolve_hostname_thread )
    {
        next_platform_thread_destroy( server->resolve_hostname_thread );
    }
    if ( server->resolve_hostname_mutex )
    {
        next_platform_mutex_destroy( server->resolve_hostname_mutex );
    }
    if ( server->command_queue )
    {
        next_queue_destroy( server->command_queue );
    }
    if ( server->notify_queue )
    {
        next_queue_destroy( server->notify_queue );
    }
    if ( server->session_manager )
    {
        next_session_manager_destroy( server->session_manager );
    }
    if ( server->pending_session_manager )
    {
        next_pending_session_manager_destroy( server->pending_session_manager );
    }
    clear_and_free( server->context, server, sizeof(next_server_internal_t) );
}

int next_server_internal_send_packet( next_server_internal_t * server, const next_address_t * to_address, uint8_t packet_id, void * packet_object )
{
    next_assert( server );
    next_assert( server->socket );
    next_assert( packet_object );
    
    int packet_bytes = 0;
    
    uint8_t buffer[NEXT_MAX_PACKET_BYTES];

    uint64_t * sequence = NULL;
    uint8_t * send_key = NULL;

    if ( next_encrypted_packets[packet_id] )
    {
        next_session_entry_t * session = next_session_manager_find_by_address( server->session_manager, to_address );
        
        if ( !session )
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "server can't send encrypted packet to address. no session found" );
            return NEXT_ERROR;
        }

        sequence = &session->internal_send_sequence;
        send_key = session->send_key;
    }

    if ( next_write_packet( packet_id, packet_object, buffer, &packet_bytes, next_encrypted_packets, sequence, send_key ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server failed to write internal packet with id %d", packet_id );
        return NEXT_ERROR;
    }
    
    next_platform_socket_send_packet( server->socket, to_address, buffer, packet_bytes );
    
    return NEXT_OK;
}

inline int next_sequence_greater_than( uint8_t s1, uint8_t s2 )
{
    return ( ( s1 > s2 ) && ( s1 - s2 <= 128 ) ) || 
           ( ( s1 < s2 ) && ( s2 - s1  > 128 ) );
}

next_session_entry_t * next_server_internal_check_client_to_server_packet( next_server_internal_t * server, uint8_t * packet_data, int packet_bytes )
{
    next_assert( server );
    next_assert( packet_data );
 
    if ( packet_bytes <= NEXT_HEADER_BYTES )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "server ignored client to server packet. too small to be valid" );
        return NULL;
    }

    uint8_t packet_type = 0;
    uint64_t packet_sequence = 0;
    uint64_t packet_session_id = 0;
    uint8_t packet_session_version = 0;
    uint8_t packet_session_flags = 0;

    if ( next_peek_header( NEXT_DIRECTION_CLIENT_TO_SERVER, &packet_type, &packet_sequence, &packet_session_id, &packet_session_version, &packet_session_flags, packet_data, packet_bytes ) != NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "server ignored client to server packet. could not peek header" );
        return NULL;
    }

    next_session_entry_t * entry = next_session_manager_find_by_session_id( server->session_manager, packet_session_id );
    if ( !entry )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "server ignored client to server packet. could not find session %" PRIx64 );
        return NULL;
    }

    if ( !entry->has_pending_route && !entry->has_current_route && !entry->has_previous_route )
    {
        next_printf( NEXT_LOG_LEVEL_WARN, "server ignored client to server packet. session has no route" );
        return NULL;
    }

    next_assert( packet_type == NEXT_CLIENT_TO_SERVER_PACKET || packet_type == NEXT_PING_PACKET );

    next_replay_protection_t * replay_protection = ( packet_type == NEXT_CLIENT_TO_SERVER_PACKET ) ? &entry->payload_replay_protection : &entry->special_replay_protection;

    uint64_t clean_sequence = next_clean_sequence( packet_sequence );

    if ( next_replay_protection_already_received( replay_protection, clean_sequence ) )
    {
        if ( !entry->multipath )
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "server ignored client to server packet. already received (%" PRId64 ",%" PRId64 ")", clean_sequence, replay_protection->most_recent_sequence );
        }
        return NULL;
    }

    if ( entry->has_pending_route && next_read_header( NEXT_DIRECTION_CLIENT_TO_SERVER, &packet_type, &packet_sequence, &packet_session_id, &packet_session_version, &packet_session_flags, entry->pending_route_private_key, packet_data, packet_bytes ) == NEXT_OK )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "server promotes pending route for %" PRIx64, entry->session_id );

        if ( entry->has_current_route )
        {
            entry->has_previous_route = true;
            entry->previous_route_send_address = entry->current_route_send_address;
            memcpy( entry->previous_route_private_key, entry->current_route_private_key, crypto_box_SECRETKEYBYTES );
        }

        entry->has_pending_route = false;
        entry->has_current_route = true;
        entry->current_route_session_version = entry->pending_route_session_version;
        entry->current_route_expire_timestamp = entry->pending_route_expire_timestamp;
        entry->current_route_expire_time = entry->pending_route_expire_time;
        entry->current_route_kbps_up = entry->pending_route_kbps_up;
        entry->current_route_kbps_down = entry->pending_route_kbps_down;
        entry->current_route_send_address = entry->pending_route_send_address;
        memcpy( entry->current_route_private_key, entry->pending_route_private_key, crypto_box_SECRETKEYBYTES );

        next_platform_mutex_acquire( server->session_mutex );
        entry->mutex_envelope_kbps_up = entry->current_route_kbps_up;
        entry->mutex_envelope_kbps_down = entry->current_route_kbps_down;
        entry->mutex_send_over_network_next = true;
        entry->mutex_session_id = entry->session_id;
        entry->mutex_session_version = entry->current_route_session_version;
        entry->mutex_send_address = entry->current_route_send_address;
        memcpy( entry->mutex_private_key, entry->current_route_private_key, crypto_box_SECRETKEYBYTES );
        next_platform_mutex_release( server->session_mutex );
    }
    else
    {
        if ( next_read_header( NEXT_DIRECTION_CLIENT_TO_SERVER, &packet_type, &packet_sequence, &packet_session_id, &packet_session_version, &packet_session_flags, entry->current_route_private_key, packet_data, packet_bytes ) != NEXT_OK )
        {
            if ( next_read_header( NEXT_DIRECTION_CLIENT_TO_SERVER, &packet_type, &packet_sequence, &packet_session_id, &packet_session_version, &packet_session_flags, entry->previous_route_private_key, packet_data, packet_bytes ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_ERROR, "server ignored client to server packet. could not read header" );
                return NULL;
            }
        }
    }

    next_replay_protection_advance_sequence( replay_protection, clean_sequence );

    if ( packet_type == NEXT_CLIENT_TO_SERVER_PACKET )
    {
        next_packet_loss_tracker_packet_received( &entry->packet_loss_tracker, clean_sequence );
    }

    return entry;
}

int next_server_internal_process_packet( next_server_internal_t * server, const next_address_t * from, uint8_t * packet_data, int packet_bytes )
{
    next_assert( server );
    next_assert( from );
    next_assert( packet_data );

    int packet_id = packet_data[0];

    next_session_entry_t * session = NULL;

    if ( next_encrypted_packets[packet_id] )
    {
        session = next_session_manager_find_by_address( server->session_manager, from );
        if ( !session )
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "server ignored encrypted packet. no session for address" );
            return NEXT_ERROR;
        }
    }

    switch ( packet_id )
    {
        case NEXT_UPGRADE_RESPONSE_PACKET:
        {
            NextUpgradeResponsePacket packet;

            if ( next_read_packet( packet_data, packet_bytes, &packet, NULL, NULL, NULL, NULL ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server failed to read upgrade response packet" );
                return NEXT_ERROR;
            }

            NextUpgradeToken upgrade_token;

            // does the session already exist? if so we still need to reply with upgrade commit in case of server -> client packet loss

            next_session_entry_t * existing_entry = next_session_manager_find_by_address( server->session_manager, from );
            
            if ( existing_entry )
            {
                if ( !upgrade_token.Read( packet.upgrade_token, existing_entry->ephemeral_private_key ) )
                {
                    next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. could not decrypt upgrade token (existing entry)" );
                    return NEXT_ERROR;
                }

                if ( upgrade_token.session_id != existing_entry->session_id )
                {
                    next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. session id does not match existing entry" );
                    return NEXT_ERROR;
                }

                if ( !next_address_equal( &upgrade_token.client_address, &existing_entry->address ) )
                {
                    next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. client address does not match existing entry" );
                    return NEXT_ERROR;
                }
            }
            else
            {
                // session does not exist yet. look up pending upgrade entry...

                next_pending_session_entry_t * pending_entry = next_pending_session_manager_find( server->pending_session_manager, from );
                if ( pending_entry == NULL )
                {
                    next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. does not match any pending upgrade" );
                    return NEXT_ERROR;
                }

                if ( !upgrade_token.Read( packet.upgrade_token, pending_entry->private_key ) )
                {
                    next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. could not decrypt upgrade token" );
                    return NEXT_ERROR;
                }

                if ( upgrade_token.session_id != pending_entry->session_id )
                {
                    next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. session id does not match pending upgrade entry" );
                    return NEXT_ERROR;
                }

                if ( !next_address_equal( &upgrade_token.client_address, &pending_entry->address ) )
                {
                    next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. client address does not match pending upgrade entry" );
                    return NEXT_ERROR;
                }

                uint8_t server_send_key[crypto_kx_SESSIONKEYBYTES];
                uint8_t server_receive_key[crypto_kx_SESSIONKEYBYTES];
                if ( crypto_kx_server_session_keys( server_receive_key, server_send_key, server->server_kx_public_key, server->server_kx_private_key, packet.client_kx_public_key ) != 0 )
                {
                    next_printf( NEXT_LOG_LEVEL_WARN, "server could not generate session keys from client public key" );
                    return NEXT_ERROR;
                }

                // remove from pending upgrade

                next_pending_session_manager_remove_by_address( server->pending_session_manager, from );

                // add to established sessions

                next_platform_mutex_acquire( server->session_mutex );
                next_session_entry_t * entry = next_session_manager_add( server->session_manager, &pending_entry->address, pending_entry->session_id, pending_entry->private_key, pending_entry->upgrade_token );
                next_platform_mutex_release( server->session_mutex );
                if ( entry == NULL )
                {
                    next_printf( NEXT_LOG_LEVEL_ERROR, "server ignored upgrade response. failed to add session" );
                    return NEXT_ERROR;
                }

                memcpy( entry->send_key, server_send_key, crypto_kx_SESSIONKEYBYTES );
                memcpy( entry->receive_key, server_receive_key, crypto_kx_SESSIONKEYBYTES );
                memcpy( entry->client_route_public_key, packet.client_route_public_key, crypto_box_PUBLICKEYBYTES );
                entry->last_client_stats_update = next_time();
                entry->user_hash = pending_entry->user_hash;
                entry->platform_id_override = pending_entry->platform_id;
                entry->tag = pending_entry->tag;
                entry->client_open_session_sequence = packet.client_open_session_sequence;

                // log that we upgraded

                char address_buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
                next_printf( NEXT_LOG_LEVEL_DEBUG, "server upgraded %s to session %" PRIx64 " (internal)", next_address_to_string( &entry->address, address_buffer ), entry->session_id );

                // notify session upgraded

                next_server_notify_session_upgraded_t * notify = (next_server_notify_session_upgraded_t*) next_malloc( server->context, sizeof( next_server_notify_session_upgraded_t ) );
                notify->type = NEXT_SERVER_NOTIFY_SESSION_UPGRADED;
                notify->address = entry->address;
                notify->session_id = entry->session_id;
                notify->tag = entry->tag;
                {
                    next_mutex_guard( server->notify_mutex );
                    next_queue_push( server->notify_queue, notify );            
                }
            }

            if ( !next_address_equal( &upgrade_token.client_address, from ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. client address does not match from address" );
                return NEXT_ERROR;
            }

            if ( upgrade_token.expire_timestamp < uint64_t(next_time()) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. upgrade token expired" );
                return NEXT_ERROR;
            }

            if ( !next_address_equal( &upgrade_token.client_address, from ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. client address does not match from address" );
                return NEXT_ERROR;
            }

            if ( !next_address_equal( &upgrade_token.server_address, &server->server_address ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored upgrade response. server address does not match" );
                return NEXT_ERROR;
            }

            next_post_validate_packet( packet_data, packet_bytes, &packet, NULL, NULL, NULL, NULL, NULL );

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server received upgrade response packet" );

            // reply with upgrade confirm

            NextUpgradeConfirmPacket response;
            response.upgrade_sequence = server->upgrade_sequence++;
            response.session_id = upgrade_token.session_id;
            response.server_address = server->server_address;
            memcpy( response.client_kx_public_key, packet.client_kx_public_key, crypto_kx_PUBLICKEYBYTES );
            memcpy( response.server_kx_public_key, server->server_kx_public_key, crypto_kx_PUBLICKEYBYTES );
            response.Sign( server->customer_private_key );

            if ( next_server_internal_send_packet( server, from, NEXT_UPGRADE_CONFIRM_PACKET, &response ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_ERROR, "server could not send upgrade confirm packet" );
                return NEXT_ERROR;
            }

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server sent upgrade confirm packet" );

            return NEXT_OK;
        }
        break;

        case NEXT_DIRECT_PING_PACKET:
        {
			if ( !session )
			{
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored direct ping packet. can't find session for address" );
                return NEXT_ERROR;
			}

            NextDirectPingPacket packet;

            uint64_t packet_sequence = 0;

            if ( next_read_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, session->receive_key, &session->internal_replay_protection ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored direct ping packet. failed to read" );
                return NEXT_ERROR;
            }

            next_post_validate_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, session->receive_key, &session->internal_replay_protection, NULL );

            NextDirectPongPacket response;
            response.ping_sequence = packet.ping_sequence;

            if ( next_server_internal_send_packet( server, from, NEXT_DIRECT_PONG_PACKET, &response ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server could not send upgrade confirm packet" );
                return NEXT_ERROR;
            }

            return NEXT_OK;
        }
        break;        

        case NEXT_CLIENT_STATS_PACKET:
        {
			if ( !session )
			{
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored direct ping packet. can't find session for address" );
                return NEXT_ERROR;
			}

            NextClientStatsPacket packet;

            uint64_t packet_sequence = 0;

            if ( next_read_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, session->receive_key, &session->internal_replay_protection ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored client stats packet. failed to read" );
                return NEXT_ERROR;
            }

            next_post_validate_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, session->receive_key, &session->internal_replay_protection, NULL );

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server received client stats packet for %" PRIx64, session->session_id );

            if ( packet_sequence > session->stats_sequence )
            {
                session->stats_sequence = packet_sequence;

                if ( !packet.try_before_you_buy && session->stats_try_before_you_buy )
                {
                    next_printf( NEXT_LOG_LEVEL_INFO, "server try before you buy completed for %" PRIx64, session->session_id );
                    session->stats_try_before_you_buy = false;
                }

                session->stats_flags |= packet.flags;
                session->stats_flagged = packet.flagged;
                session->stats_multipath = packet.multipath;
                session->stats_fallback_to_direct = packet.fallback_to_direct;

                session->stats_platform_id = packet.platform_id;
                session->stats_connection_type = packet.connection_type;
                session->stats_kbps_up = packet.kbps_up;
                session->stats_kbps_down = packet.kbps_down;
                session->stats_direct_min_rtt = packet.direct_min_rtt;
                session->stats_direct_max_rtt = packet.direct_max_rtt;
                session->stats_direct_mean_rtt = packet.direct_mean_rtt;
                session->stats_direct_jitter = packet.direct_jitter;
                session->stats_direct_packet_loss = packet.direct_packet_loss;
                session->stats_next = packet.next;
                session->stats_next_min_rtt = packet.next_min_rtt;
                session->stats_next_max_rtt = packet.next_max_rtt;
                session->stats_next_mean_rtt = packet.next_mean_rtt;
                session->stats_next_jitter = packet.next_jitter;
                session->stats_next_packet_loss = packet.next_packet_loss;
                session->stats_num_near_relays = packet.num_near_relays;
                for ( int i = 0; i < packet.num_near_relays; ++i )
                {
                    session->stats_near_relay_ids[i] = packet.near_relay_ids[i];
                    session->stats_near_relay_min_rtt[i] = packet.near_relay_min_rtt[i];
                    session->stats_near_relay_max_rtt[i] = packet.near_relay_max_rtt[i];
                    session->stats_near_relay_mean_rtt[i] = packet.near_relay_mean_rtt[i];
                    session->stats_near_relay_jitter[i] = packet.near_relay_jitter[i];
                    session->stats_near_relay_packet_loss[i] = packet.near_relay_packet_loss[i];
                }
                session->stats_packets_lost_server_to_client = packet.packets_lost_server_to_client;
                session->last_client_stats_update = next_time();
            }

            return NEXT_OK;
        }
        break;

        case NEXT_ROUTE_UPDATE_ACK_PACKET:
        {
			if ( !session )
			{
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored direct ping packet. can't find session for address" );
                return NEXT_ERROR;
			}

            NextRouteUpdateAckPacket packet;

            uint64_t packet_sequence = 0;

            if ( next_read_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, session->receive_key, &session->internal_replay_protection ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored route update ack packet. failed to read" );
                return NEXT_ERROR;
            }

            if ( packet.sequence != session->update_sequence )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored route update ack packet. wrong update sequence number" );
                return NEXT_ERROR;
            }

            next_post_validate_packet( packet_data, packet_bytes, &packet, next_encrypted_packets, &packet_sequence, session->receive_key, &session->internal_replay_protection, NULL );

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server received route update ack for %" PRIx64, session->session_id );

            session->update_dirty = false;

            return NEXT_OK;
        }
        break;

        case NEXT_BACKEND_SESSION_RESPONSE_PACKET:
        {
            NextBackendSessionResponsePacket packet;

            if ( next_read_backend_packet( packet_data, packet_bytes, &packet ) != packet_id )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored session response packet. packet failed to read" );
                return NEXT_ERROR;
            }

            if ( !packet.Verify( next_backend_public_key ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored session response packet. did not verify" );
                return NEXT_ERROR;
            }

            if ( memcmp( packet.server_route_public_key, server->server_route_public_key, sizeof(packet.server_route_public_key) ) != 0 )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored session response packet. server public key mismatch" );
                return NEXT_ERROR;
            }

            next_session_entry_t * entry = next_session_manager_find_by_session_id( server->session_manager, packet.session_id );
            if ( !entry )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored session response packet. could not find session %" PRIx64, packet.session_id );
                return NEXT_ERROR;
            }

            if ( !entry->waiting_for_update_response )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "server ignored session response packet. not waiting for session response" );
                return NEXT_ERROR;
            }

            if ( packet.sequence != entry->update_sequence )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored session response packet. wrong sequence number" );
                return NEXT_ERROR;
            }

            const char * update_type = "???";

            switch ( packet.response_type )
            {   
                case NEXT_UPDATE_TYPE_DIRECT:    update_type = "direct route";     break;
                case NEXT_UPDATE_TYPE_ROUTE:     update_type = "next route";       break;
                case NEXT_UPDATE_TYPE_CONTINUE:  update_type = "continue route";   break;
            }

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server received session response for %" PRIx64 " (%s)", entry->session_id, update_type );

            bool multipath = packet.multipath;

            if ( multipath && !entry->multipath )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "server multipath enabled for session %" PRIx64, entry->session_id );
                entry->multipath = true;
                next_platform_mutex_acquire( server->session_mutex );
                entry->mutex_multipath = true;
                next_platform_mutex_release( server->session_mutex );
            }

            entry->update_dirty = true;
            entry->update_type = (uint8_t) packet.response_type;
            entry->update_num_tokens = packet.num_tokens;
            if ( packet.response_type == NEXT_UPDATE_TYPE_ROUTE )
            {
                memcpy( entry->update_tokens, packet.tokens, NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES * packet.num_tokens );
            }
            else if ( packet.response_type == NEXT_UPDATE_TYPE_CONTINUE )
            {
                memcpy( entry->update_tokens, packet.tokens, NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES * packet.num_tokens );
            }
            entry->update_num_near_relays = packet.num_near_relays;
            memcpy( entry->update_near_relay_ids, packet.near_relay_ids, 8 * packet.num_near_relays );
            memcpy( entry->update_near_relay_addresses, packet.near_relay_addresses, sizeof(next_address_t) * packet.num_near_relays );
            entry->update_last_send_time = -1000.0;

            entry->waiting_for_update_response = false;

            if ( packet.response_type == NEXT_UPDATE_TYPE_DIRECT )
            {
                bool session_transitions_to_direct = false;

                next_platform_mutex_acquire( server->session_mutex );
                if ( entry->mutex_send_over_network_next )
                {
                    entry->mutex_send_over_network_next = false;
                    session_transitions_to_direct = true;
                }
                next_platform_mutex_release( server->session_mutex );

                if ( session_transitions_to_direct )
                {
                    entry->has_previous_route = entry->has_current_route;
                    entry->has_current_route = false;
                    entry->previous_route_send_address = entry->current_route_send_address;
                    memcpy( entry->previous_route_private_key, entry->current_route_private_key, crypto_box_SECRETKEYBYTES );
                }
            }

            return NEXT_OK;
        }   
        break;

        case NEXT_ROUTE_REQUEST_PACKET:
        {
            if ( packet_bytes != 1 + NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored route request packet. incorrect packet size. expected %d bytes, got %d", 1 + NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES, packet_bytes );
                return NEXT_ERROR;
            }

            uint8_t * buffer = &packet_data[1];

            next_route_token_t route_token;

            if ( next_read_encrypted_route_token( &buffer, &route_token, next_router_public_key, server->server_route_private_key ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored route request packet. could not read route token" );
                return NEXT_ERROR;
            }

            next_session_entry_t * entry = next_session_manager_find_by_session_id( server->session_manager, route_token.session_id );
            if ( !entry )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored route request packet. could not find session %" PRIx64 );
                return NEXT_ERROR;
            }

            if ( entry->has_current_route && route_token.expire_timestamp < entry->current_route_expire_timestamp )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored route request packet. expire timestamp is older than current route" );
                return NEXT_ERROR;
            }

            if ( entry->has_current_route && next_sequence_greater_than( entry->most_recent_session_version, route_token.session_version ) )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored route request packet. route is older than most recent session (%d vs. %d)", route_token.session_version, entry->most_recent_session_version );
                return NEXT_ERROR;
            }

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server received route request packet for %" PRIx64, route_token.session_id );

            if ( next_sequence_greater_than( route_token.session_version, entry->pending_route_session_version ) )
            {
                next_printf( NEXT_LOG_LEVEL_DEBUG, "server added pending route for %" PRIx64, route_token.session_id );
                entry->has_pending_route = true;
                entry->pending_route_session_version = route_token.session_version;
                entry->pending_route_expire_timestamp = route_token.expire_timestamp;
                entry->pending_route_expire_time = entry->has_current_route ? ( entry->current_route_expire_time + NEXT_SLICE_SECONDS * 2 ) : ( next_time() + NEXT_SLICE_SECONDS * 2 );
                entry->pending_route_kbps_up = route_token.kbps_up;
                entry->pending_route_kbps_down = route_token.kbps_down;
                entry->pending_route_send_address = *from;
                memcpy( entry->pending_route_private_key, route_token.private_key, crypto_box_SECRETKEYBYTES );
                entry->most_recent_session_version = route_token.session_version;
            }

            uint8_t response[NEXT_HEADER_BYTES];

            uint64_t session_send_sequence = entry->special_send_sequence++;
            session_send_sequence |= uint64_t(1) << 63;
            session_send_sequence |= uint64_t(1) << 62;

            if ( next_write_header( NEXT_DIRECTION_SERVER_TO_CLIENT, NEXT_ROUTE_RESPONSE_PACKET, session_send_sequence, entry->session_id, entry->pending_route_session_version, 0, entry->pending_route_private_key, response, NEXT_MTU ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server failed to write next route response packet" );
                return NEXT_ERROR;
            }

            next_platform_socket_send_packet( server->socket, from, response, NEXT_HEADER_BYTES );

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server sent route response packet to %" PRIx64, entry->session_id );

            return NEXT_OK;
        }
        break;

        case NEXT_CONTINUE_REQUEST_PACKET:
        {
            if ( packet_bytes != 1 + NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored continue request packet. incorrect packet size. expected %d bytes, got %d", 1 + NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES, packet_bytes );
                return NEXT_ERROR;
            }

            uint8_t * buffer = &packet_data[1];

            next_continue_token_t continue_token;

            if ( next_read_encrypted_continue_token( &buffer, &continue_token, next_router_public_key, server->server_route_private_key ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored continue request packet. could not read continue token" );
                return NEXT_ERROR;
            }

            next_session_entry_t * entry = next_session_manager_find_by_session_id( server->session_manager, continue_token.session_id );
            if ( !entry )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored continue request packet. could not find session %" PRIx64 );
                return NEXT_ERROR;
            }

            if ( !entry->has_current_route )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored continue request packet. session has no route to continue" );
                return NEXT_ERROR;
            }

            if ( continue_token.session_version != entry->current_route_session_version )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored continue request packet. session version does not match" );
                return NEXT_ERROR;
            }

            if ( continue_token.expire_timestamp < entry->current_route_expire_timestamp )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored continue request packet. expire timestamp is older than current route" );
                return NEXT_ERROR;
            }

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server received continue request packet for %" PRIx64, continue_token.session_id );

            entry->current_route_expire_timestamp = continue_token.expire_timestamp;
            entry->current_route_expire_time += NEXT_SLICE_SECONDS;
            entry->has_previous_route = false;

            uint8_t response[NEXT_HEADER_BYTES];

            uint64_t session_send_sequence = entry->special_send_sequence++;
            session_send_sequence |= uint64_t(1) << 63;
            session_send_sequence |= uint64_t(1) << 62;

            if ( next_write_header( NEXT_DIRECTION_SERVER_TO_CLIENT, NEXT_CONTINUE_RESPONSE_PACKET, session_send_sequence, entry->session_id, entry->current_route_session_version, 0, entry->current_route_private_key, response, NEXT_MTU ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_ERROR, "server failed to write next continue response packet" );
                return NEXT_ERROR;
            }

            next_platform_socket_send_packet( server->socket, from, response, NEXT_HEADER_BYTES );

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server sent continue response packet to %" PRIx64, entry->session_id );

            return NEXT_OK;
        }
        break;

        case NEXT_CLIENT_TO_SERVER_PACKET:
        {
            next_session_entry_t * entry = next_server_internal_check_client_to_server_packet( server, packet_data, packet_bytes );
            if ( !entry )
                return NEXT_ERROR;

            next_server_notify_packet_received_t * notify = (next_server_notify_packet_received_t*) next_malloc( server->context, sizeof( next_server_notify_packet_received_t ) );
            notify->type = NEXT_SERVER_NOTIFY_PACKET_RECEIVED;
            notify->from = entry->address;
            notify->packet_bytes = packet_bytes - NEXT_HEADER_BYTES;
            next_assert( packet_bytes > NEXT_HEADER_BYTES );
            memcpy( notify->packet_data, packet_data + NEXT_HEADER_BYTES, packet_bytes - NEXT_HEADER_BYTES );
            {
                next_mutex_guard( server->notify_mutex );
                next_queue_push( server->notify_queue, notify );
            }

            return NEXT_OK;
        }
        break;

        case NEXT_PING_PACKET:
        {
            if ( packet_bytes != NEXT_HEADER_BYTES + 8 )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server ignored client ping. wrong size" );
                return NEXT_ERROR;
            }

            next_session_entry_t * entry = next_server_internal_check_client_to_server_packet( server, packet_data, packet_bytes );
            if ( !entry )
                return NEXT_ERROR;

            uint64_t send_sequence = entry->special_send_sequence++;
            send_sequence |= uint64_t(1) << 63;
            send_sequence |= uint64_t(1) << 62;

            if ( next_write_header( NEXT_DIRECTION_SERVER_TO_CLIENT, NEXT_PONG_PACKET, send_sequence, entry->session_id, entry->current_route_session_version, 0, entry->current_route_private_key, packet_data, NEXT_HEADER_BYTES + 8 ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_WARN, "server failed to write pong packet header" );
                return NEXT_ERROR;
            }

            next_platform_socket_send_packet( server->socket, from, packet_data, NEXT_HEADER_BYTES + 8 );

            return NEXT_OK;
        }
        break;

        default: break;
    }

    return NEXT_ERROR;
}

void next_server_internal_update_route( next_server_internal_t * server )
{
    next_assert( server );

    const double current_time = next_time();
    
    const int max_index = server->session_manager->max_entry_index;

    for ( int i = 0; i <= max_index; ++i )
    {
        if ( server->session_manager->session_ids[i] == 0 )
            continue;

        next_session_entry_t * entry = &server->session_manager->entries[i];

        if ( entry->update_dirty && entry->update_last_send_time + NEXT_UPDATE_SEND_TIME <= current_time )
        {
            NextRouteUpdatePacket packet;
            packet.sequence = entry->update_sequence;
            packet.num_near_relays = entry->update_num_near_relays;
            memcpy( packet.near_relay_ids, entry->update_near_relay_ids, 8 * entry->update_num_near_relays );
            memcpy( packet.near_relay_addresses, entry->update_near_relay_addresses, sizeof(next_address_t) * entry->update_num_near_relays );
            packet.update_type = entry->update_type;
            packet.multipath = entry->multipath;
            packet.num_tokens = entry->update_num_tokens;
            if ( entry->update_type == NEXT_UPDATE_TYPE_ROUTE )
            {
                memcpy( packet.tokens, entry->update_tokens, NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES * entry->update_num_tokens );
            }
            else if ( entry->update_type == NEXT_UPDATE_TYPE_CONTINUE )
            {
                memcpy( packet.tokens, entry->update_tokens, NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES * entry->update_num_tokens );
            }
            packet.packets_lost_client_to_server = entry->stats_packets_lost_client_to_server;

            next_server_internal_send_packet( server, &entry->address, NEXT_ROUTE_UPDATE_PACKET, &packet );            

            entry->update_last_send_time = current_time;

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server sent route update to %" PRIx64, entry->session_id );
        }
    }
}

void next_server_internal_update_pending_upgrades( next_server_internal_t * server )
{
    next_assert( server );

    const double current_time = next_time();
    
    const double packet_resend_time = 0.25;

    const int max_index = server->pending_session_manager->max_entry_index;

    for ( int i = 0; i <= max_index; ++i )
    {
        if ( server->pending_session_manager->addresses[i].type == NEXT_ADDRESS_NONE )
            continue;
     
        next_pending_session_entry_t * entry = &server->pending_session_manager->entries[i];

        if ( entry->upgrade_time + NEXT_UPGRADE_TIMEOUT <= current_time )
        {
            char address_buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
            next_printf( NEXT_LOG_LEVEL_ERROR, "server upgrade request timed out for client %s", next_address_to_string( &entry->address, address_buffer ) );
            next_pending_session_manager_remove_at_index( server->pending_session_manager, i );
            next_server_notify_pending_session_timed_out_t * notify = (next_server_notify_pending_session_timed_out_t*) next_malloc( server->context, sizeof( next_server_notify_pending_session_timed_out_t ) );
            notify->type = NEXT_SERVER_NOTIFY_PENDING_SESSION_TIMED_OUT;
            notify->address = entry->address;
            notify->session_id = entry->session_id;
            {
                next_mutex_guard( server->notify_mutex );
                next_queue_push( server->notify_queue, notify );            
            }
            continue;
        }

        if ( entry->last_packet_send_time + packet_resend_time <= current_time )
        {
            next_printf( NEXT_LOG_LEVEL_DEBUG, "server sent upgrade request packet" );
            
            entry->last_packet_send_time = current_time;

            NextUpgradeRequestPacket packet;
            packet.protocol_version = next_protocol_version();
            packet.session_id = entry->session_id;
            packet.server_address = server->server_address;
            memcpy( packet.server_kx_public_key, server->server_kx_public_key, crypto_kx_PUBLICKEYBYTES );
            memcpy( packet.upgrade_token, entry->upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );        
            packet.Sign( server->customer_private_key );

            next_server_internal_send_packet( server, &entry->address, NEXT_UPGRADE_REQUEST_PACKET, &packet );
        }
    }
}

void next_server_internal_update_sessions( next_server_internal_t * server )
{
    next_assert( server );

    const double current_time = next_time();
    
    int index = 0;

    while ( index <= server->session_manager->max_entry_index )
    {
        if ( server->session_manager->session_ids[index] == 0 )
        {
            ++index;
            continue;
        }
     
        next_session_entry_t * entry = &server->session_manager->entries[index];

        if ( entry->last_client_stats_update + NEXT_SESSION_TIMEOUT <= current_time )
        {
            char address_buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
            next_printf( NEXT_LOG_LEVEL_INFO, "server session timed out for client %s", next_address_to_string( &entry->address, address_buffer ) );

            next_server_notify_session_timed_out_t * notify = (next_server_notify_session_timed_out_t*) next_malloc( server->context, sizeof( next_server_notify_session_timed_out_t ) );
            notify->type = NEXT_SERVER_NOTIFY_SESSION_TIMED_OUT;
            notify->address = entry->address;
            notify->session_id = entry->session_id;
            {
                next_mutex_guard( server->notify_mutex );
                next_queue_push( server->notify_queue, notify );
            }

            next_platform_mutex_acquire( server->session_mutex );
            next_session_manager_remove_at_index( server->session_manager, index );
            next_platform_mutex_release( server->session_mutex );

            continue;
        }

        if ( entry->has_current_route && entry->current_route_expire_time <= current_time )
        {
            next_printf( NEXT_LOG_LEVEL_ERROR, "server network next route expired for %" PRIx64, entry->session_id );
            
            entry->has_current_route = false;
            entry->has_previous_route = false;
            entry->update_dirty = false;
            entry->waiting_for_update_response = false;

            next_platform_mutex_acquire( server->session_mutex );
            entry->mutex_send_over_network_next = false;
            next_platform_mutex_release( server->session_mutex );
        }

        index++;
    }
}

void next_server_internal_block_and_receive_packet( next_server_internal_t * server )
{
    uint8_t packet_data[NEXT_MAX_PACKET_BYTES];

    next_address_t from;
    
    const int packet_bytes = next_platform_socket_receive_packet( server->socket, &from, packet_data, sizeof(packet_data) );
    
    if ( packet_bytes == 0 )
        return;

    if ( packet_data[0] == 0 && packet_bytes >= 2 && packet_bytes <= NEXT_MTU + 1 )
    {
        next_server_notify_packet_received_t * notify = (next_server_notify_packet_received_t*) next_malloc( server->context, sizeof( next_server_notify_packet_received_t ) );
        notify->type = NEXT_SERVER_NOTIFY_PACKET_RECEIVED;
        notify->from = from;
        notify->packet_bytes = packet_bytes - 1;
        memcpy( notify->packet_data, packet_data + 1, packet_bytes - 1 );
        {
            next_mutex_guard( server->notify_mutex );
            next_queue_push( server->notify_queue, notify );            
        }
    }
    else if ( packet_data[0] == 255 && packet_bytes >= 11 && packet_bytes <= NEXT_MTU + 10 )
    {
        const uint8_t * p = packet_data + 1;
        uint8_t packet_session_sequence = next_read_uint8( &p );
        uint64_t packet_sequence = next_read_uint64( &p );
        next_session_entry_t * entry = next_session_manager_find_by_address( server->session_manager, &from );
        if ( !entry )
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "server ignored direct packet. could not find session for address" );
            return;
        }
        if ( packet_session_sequence != entry->client_open_session_sequence )
        {
            next_printf( NEXT_LOG_LEVEL_DEBUG, "server ignored direct packet. session mismatch" );
            return;
        }
        uint64_t clean_sequence = next_clean_sequence( packet_sequence );
        if ( next_replay_protection_already_received( &entry->payload_replay_protection, clean_sequence ) )
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "server ignored direct packet. already received (%" PRIx64 " vs. %" PRIx64 ")", clean_sequence, entry->payload_replay_protection.most_recent_sequence );
            return;
        }
        next_replay_protection_advance_sequence( &entry->payload_replay_protection, clean_sequence );
        next_packet_loss_tracker_packet_received( &entry->packet_loss_tracker, clean_sequence );
        next_server_notify_packet_received_t * notify = (next_server_notify_packet_received_t*) next_malloc( server->context, sizeof( next_server_notify_packet_received_t ) );
        notify->type = NEXT_SERVER_NOTIFY_PACKET_RECEIVED;
        notify->from = from;
        notify->packet_bytes = packet_bytes - 10;
        memcpy( notify->packet_data, packet_data + 10, packet_bytes - 10 );
        {
            next_mutex_guard( server->notify_mutex );
            next_queue_push( server->notify_queue, notify );            
        }
    }
    else
    {
        next_server_internal_process_packet( server, &from, packet_data, packet_bytes );
    }
}

void next_server_internal_upgrade_session( next_server_internal_t * server, const next_address_t * address, uint64_t session_id, uint64_t user_hash, uint32_t platform_id, uint64_t tag )
{
    next_assert( server );
    next_assert( address );

    char buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];

    next_printf( NEXT_LOG_LEVEL_DEBUG, "server upgrading client %s to session %" PRIx64, next_address_to_string( address, buffer ), session_id );

    NextUpgradeToken upgrade_token;

    upgrade_token.session_id = session_id;
    upgrade_token.expire_timestamp = uint64_t( next_time() ) + 10;
    upgrade_token.client_address = *address;
    upgrade_token.server_address = server->server_address;

    unsigned char session_private_key[crypto_secretbox_KEYBYTES];
    crypto_secretbox_keygen( session_private_key );

    uint8_t upgrade_token_data[NEXT_UPGRADE_TOKEN_BYTES];

    upgrade_token.Write( upgrade_token_data, session_private_key );

    next_pending_session_manager_remove_by_address( server->pending_session_manager, address );

    next_session_manager_remove_by_address( server->session_manager, address );

    next_pending_session_entry_t * entry = next_pending_session_manager_add( server->pending_session_manager, address, upgrade_token.session_id, session_private_key, upgrade_token_data, next_time() );

    if ( entry == NULL )
    {
        next_assert( !"could not add pending session entry. this should never happen!" );
    }

    entry->user_hash = user_hash;
    entry->platform_id = platform_id;
    entry->tag = tag;
}

void next_server_internal_tag_session( next_server_internal_t * server, const next_address_t * address, uint64_t tag )
{
    next_assert( server );
    next_assert( address );

    next_pending_session_entry_t * pending_entry = next_pending_session_manager_find( server->pending_session_manager, address );
    if ( pending_entry )
    {
        pending_entry->tag = tag;
        return;
    }

    next_session_entry_t * entry = next_session_manager_find_by_address( server->session_manager, address );    
    if ( entry )
    {
        entry->tag = tag;
        return;
    }
}

bool next_server_internal_pump_commands( next_server_internal_t * server, bool quit )
{
    while ( true )
    {
        void * entry = NULL;
        {
            next_mutex_guard( server->command_mutex );
            entry = next_queue_pop( server->command_queue );
        }

        if ( entry == NULL )
            break;

        next_server_command_t * command = (next_server_command_t*) entry;

        switch ( command->type )
        {
            case NEXT_SERVER_COMMAND_UPGRADE_SESSION:
            {
                next_server_command_upgrade_session_t * upgrade_session = (next_server_command_upgrade_session_t*) command;
                next_server_internal_upgrade_session( server, &upgrade_session->address, upgrade_session->session_id, upgrade_session->user_hash, upgrade_session->platform_id, upgrade_session->tag );
            }
            break;

            case NEXT_SERVER_COMMAND_TAG_SESSION:
            {
                next_server_command_tag_session_t * tag_session = (next_server_command_tag_session_t*) command;
                next_server_internal_tag_session( server, &tag_session->address, tag_session->tag );
            }
            break;

            case NEXT_SERVER_COMMAND_DESTROY:
            {
                quit = true;
            }
            break;

            default: break;                
        }

        next_free( server->context, command );
    }

    return quit;
}

static next_platform_thread_return_t NEXT_PLATFORM_THREAD_FUNC next_server_internal_resolve_hostname_thread_function( void * context )
{
    next_assert( context );

    next_server_internal_t * server = (next_server_internal_t*) context;

    const char * hostname = NEXT_HOSTNAME;
    const char * port = NEXT_PORT;
    const char * override_hostname = next_platform_getenv( "NEXT_HOSTNAME" );
    const char * override_port = next_platform_getenv( "NEXT_PORT" );

    if ( override_hostname )
        hostname = override_hostname;

    if ( override_port )
        port = override_port;

    next_printf( NEXT_LOG_LEVEL_INFO, "server resolving hostname %s", hostname );

    next_address_t address;

    for ( int i = 0; i < 10; ++i )
    {
        if ( next_platform_hostname_resolve( hostname, port, &address ) == NEXT_OK )
        {
            {
                next_mutex_guard( server->resolve_hostname_mutex );
                server->resolve_hostname_finished = true;
                server->resolve_hostname_result = address;
            }
            NEXT_PLATFORM_THREAD_RETURN();

        }
        else
        {
            next_printf( NEXT_LOG_LEVEL_WARN, "server failed to resolve hostname (%d)", i );
        }
    }

    next_printf( NEXT_LOG_LEVEL_ERROR, "server failed to resolve %s", hostname );

    {
        next_mutex_guard( server->resolve_hostname_mutex );
        server->resolve_hostname_finished = true;
        memset( &server->resolve_hostname_result, 0, sizeof(next_address_t) );
    }

    NEXT_PLATFORM_THREAD_RETURN();
}

static bool next_server_internal_update_resolve_hostname( next_server_internal_t * server )
{
    if ( !server->resolve_hostname_thread )
        return true;

    bool finished = false;
    next_address_t result;
    memset( &result, 0, sizeof(next_address_t) );
    {
        next_mutex_guard( server->resolve_hostname_mutex );
        finished = server->resolve_hostname_finished;
        result = server->resolve_hostname_result;
    }

    if ( !finished )
        return false;

    next_platform_thread_join( server->resolve_hostname_thread );

    next_platform_thread_destroy( server->resolve_hostname_thread );

    server->resolve_hostname_thread = NULL;

    if ( result.type == NEXT_ADDRESS_NONE )
    {
        server->failed_to_resolve_hostname = true;
        next_server_notify_failed_to_resolve_hostname_t * notify = (next_server_notify_failed_to_resolve_hostname_t*) next_malloc( server->context, sizeof( next_server_notify_failed_to_resolve_hostname_t ) );
        notify->type = NEXT_SERVER_NOTIFY_FAILED_TO_RESOLVE_HOSTNAME;
        {
            next_mutex_guard( server->notify_mutex );
            next_queue_push( server->notify_queue, notify );
        }
        return true;
    }

    char address_buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];

    next_printf( NEXT_LOG_LEVEL_INFO, "server resolved hostname to %s", next_address_to_string( &result, address_buffer ) );

    server->backend_address = result;

    return true;
}

void next_server_internal_backend_update( next_server_internal_t * server )
{
    next_assert( server );

    if ( server->resolve_hostname_thread )
        return;

    if ( server->failed_to_resolve_hostname )
        return;

    double current_time = next_time();

    // server update

    bool first_server_update = server->first_server_update;

    if ( server->last_backend_server_update + NEXT_SECONDS_BETWEEN_SERVER_UPDATES <= current_time )
    {
        NextBackendServerUpdatePacket packet;
        packet.sequence = ++server->server_update_sequence;
        packet.customer_id = server->customer_id;
        packet.datacenter_id = server->datacenter_id;
        packet.num_sessions_pending = next_pending_session_manager_num_entries( server->pending_session_manager );
        packet.num_sessions_upgraded = next_session_manager_num_entries( server->session_manager );
        packet.server_address = server->server_address;
        packet.server_address_internal = server->server_address_internal;
        memcpy( packet.server_route_public_key, server->server_route_public_key, crypto_box_PUBLICKEYBYTES );
        packet.Sign( server->customer_private_key );

        uint8_t packet_data[NEXT_MAX_PACKET_BYTES];

        int packet_bytes = 0;

        if ( next_write_backend_packet( NEXT_BACKEND_SERVER_UPDATE_PACKET, &packet, packet_data, &packet_bytes ) != NEXT_OK )
        {
            next_printf( NEXT_LOG_LEVEL_ERROR, "server failed to write backend update packet" );
            return;
        }

        next_platform_socket_send_packet( server->socket, &server->backend_address, packet_data, packet_bytes );

        server->last_backend_server_update = current_time;

        next_printf( NEXT_LOG_LEVEL_DEBUG, "server sent update to backend" );

        server->first_server_update = false;
    }

    if ( first_server_update )
        return;

    // session updates

    const int max_entry_index = server->session_manager->max_entry_index;

    for ( int i = 0; i <= max_entry_index; ++i )
    {
        if ( server->session_manager->session_ids[i] == 0 )
            continue;

        next_session_entry_t * session = &server->session_manager->entries[i];

        if ( session->next_session_update_time >= 0.0 && session->next_session_update_time <= current_time )
        {
            NextBackendSessionUpdatePacket packet;

            packet.sequence = ++session->update_sequence;
            packet.customer_id = server->customer_id;
            packet.session_id = session->session_id;
            packet.platform_id = session->stats_platform_id;
            if ( ( session->platform_id_override & 0xFF ) != 0 )
            {
                packet.platform_id &= ~0xFF;
                packet.platform_id |= session->platform_id_override & 0xFF;
            }
            packet.platform_id |= session->platform_id_override & ~0xFF;
            packet.user_hash = session->user_hash;
            packet.tag = session->tag;
            packet.flags = session->stats_flags;
            packet.flagged = session->stats_flagged;
            packet.fallback_to_direct = session->stats_fallback_to_direct;
            packet.try_before_you_buy = session->stats_try_before_you_buy;
            packet.connection_type = session->stats_connection_type;
            packet.kbps_up = session->stats_kbps_up;
            packet.kbps_down = session->stats_kbps_down;
            packet.packets_lost_client_to_server = session->stats_packets_lost_client_to_server;
            packet.packets_lost_server_to_client = session->stats_packets_lost_server_to_client;
            packet.next = session->stats_next;
            packet.next_min_rtt = session->stats_next_min_rtt;
            packet.next_max_rtt = session->stats_next_max_rtt;
            packet.next_mean_rtt = session->stats_next_mean_rtt;
            packet.next_jitter = session->stats_next_jitter;
            packet.next_packet_loss = session->stats_next_packet_loss;
            packet.direct_min_rtt = session->stats_direct_min_rtt;
            packet.direct_max_rtt = session->stats_direct_max_rtt;
            packet.direct_mean_rtt = session->stats_direct_mean_rtt;
            packet.direct_jitter = session->stats_direct_jitter;
            packet.direct_packet_loss = session->stats_direct_packet_loss;
            packet.num_near_relays = session->stats_num_near_relays;
            for ( int j = 0; j < packet.num_near_relays; ++j )
            {
                packet.near_relay_ids[j] = session->stats_near_relay_ids[j];
                packet.near_relay_min_rtt[j] = session->stats_near_relay_min_rtt[j];
                packet.near_relay_max_rtt[j] = session->stats_near_relay_max_rtt[j];
                packet.near_relay_mean_rtt[j] = session->stats_near_relay_mean_rtt[j];
                packet.near_relay_jitter[j] = session->stats_near_relay_jitter[j];
                packet.near_relay_packet_loss[j] = session->stats_near_relay_packet_loss[j];
            }
            packet.client_address = session->address;
            packet.server_address = server->server_address;
            memcpy( packet.client_route_public_key, session->client_route_public_key, crypto_box_PUBLICKEYBYTES );
            packet.Sign( server->customer_private_key );

            if ( next_write_backend_packet( NEXT_BACKEND_SESSION_UPDATE_PACKET, &packet, session->update_packet_data, &session->update_packet_bytes ) != NEXT_OK )
            {
                next_printf( NEXT_LOG_LEVEL_ERROR, "server failed to write backend session update packet" );
                return;
            }

            next_platform_socket_send_packet( server->socket, &server->backend_address, session->update_packet_data, session->update_packet_bytes );

            next_printf( NEXT_LOG_LEVEL_DEBUG, "server sent session update for %" PRIx64, session->session_id );

            if ( session->next_session_update_time == 0.0 )
            {
                session->next_session_update_time = current_time + NEXT_SECONDS_BETWEEN_SESSION_UPDATES;
            }
            else
            {
                session->next_session_update_time += NEXT_SECONDS_BETWEEN_SESSION_UPDATES;
            }

            session->next_session_resend_time = current_time + NEXT_SESSION_UPDATE_RESEND_TIME;

            session->waiting_for_update_response = true;
        }

        if ( session->waiting_for_update_response && session->next_session_resend_time <= current_time )
        {
            next_printf( NEXT_LOG_LEVEL_DEBUG, "server resent session update for %" PRIx64, session->session_id );

            next_platform_socket_send_packet( server->socket, &server->backend_address, session->update_packet_data, session->update_packet_bytes );

            session->next_session_resend_time += NEXT_SESSION_UPDATE_RESEND_TIME;
        }

        if ( session->waiting_for_update_response && session->next_session_update_time - NEXT_SECONDS_BETWEEN_SESSION_UPDATES + NEXT_SESSION_UPDATE_TIMEOUT <= current_time )
        {
            next_printf( NEXT_LOG_LEVEL_ERROR, "server timed out waiting for backend session response for %" PRIx64, session->session_id );
            session->waiting_for_update_response = false;
            session->next_session_update_time = -1.0;
        }
    }

    // packet loss updates

    for ( int i = 0; i <= max_entry_index; ++i )
    {
        if ( server->session_manager->session_ids[i] == 0 )
            continue;

        next_session_entry_t * session = &server->session_manager->entries[i];

        if ( session->stats_fallback_to_direct )
            continue;

        if ( session->next_packet_loss_update_time <= current_time )
        {
            const int packets_lost = next_packet_loss_tracker_update( &session->packet_loss_tracker, NULL );
            session->stats_packets_lost_client_to_server += packets_lost;
            session->next_packet_loss_update_time = current_time + NEXT_SECONDS_BETWEEN_PACKET_LOSS_UPDATES;
        }
    }
}

static next_platform_thread_return_t NEXT_PLATFORM_THREAD_FUNC next_server_internal_thread_function( void * context )
{
    next_assert( context );

    next_server_internal_t * server = (next_server_internal_t*) context;

    bool quit = false;

    bool finished_hostname_resolve = false;

    double last_update_time = next_time();

    while ( !quit || !finished_hostname_resolve )
    {
        next_server_internal_block_and_receive_packet( server );

        double current_time = next_time();

        if ( current_time >= last_update_time + 0.1 )
        {
            next_server_internal_update_pending_upgrades( server );

            next_server_internal_update_route( server );

            next_server_internal_update_sessions( server );

            next_server_internal_backend_update( server );

            quit = next_server_internal_pump_commands( server, quit );

            finished_hostname_resolve = next_server_internal_update_resolve_hostname( server );

            last_update_time = current_time;
        }
    }

    NEXT_PLATFORM_THREAD_RETURN();
}

// ---------------------------------------------------------------

struct next_server_t
{
    void * context;
    next_server_internal_t * internal;
    next_platform_thread_t * thread;
    bool failed_to_resolve_hostname;
    void (*packet_received_callback)( next_server_t * server, void * context, const next_address_t * from, const uint8_t * packet_data, int packet_bytes );
    next_proxy_session_manager_t * pending_session_manager;
    next_proxy_session_manager_t * session_manager;
    uint16_t bound_port;
};

void next_server_destroy( next_server_t * server );

next_server_t * next_server_create( void * context, const char * server_address, const char * bind_address, const char * datacenter, void (*packet_received_callback)( next_server_t * server, void * context, const next_address_t * from, const uint8_t * packet_data, int packet_bytes ) )
{
    next_assert( server_address );
    next_assert( bind_address );
    next_assert( packet_received_callback );

    next_server_t * server = (next_server_t*) next_malloc( context, sizeof(next_server_t) );
    if ( !server ) 
        return NULL;

    memset( server, 0, sizeof( next_server_t) );

    server->context = context;

    server->internal = next_server_internal_create( context, server_address, bind_address, datacenter );
    if ( !server->internal )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create internal server" );
        next_server_destroy( server );
        return NULL;
    }

    server->bound_port = server->internal->server_address.port;

    server->thread = next_platform_thread_create( server->context, next_server_internal_thread_function, server->internal );
    if ( !server->thread )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create server thread" );
        next_server_destroy( server );
        return NULL;
    }

    if ( next_global_config.high_priority_server_thread )
    {
        next_printf( NEXT_LOG_LEVEL_INFO, "server thread set to high priority" );
        next_platform_thread_set_sched_max( server->thread );
    }

    server->pending_session_manager = next_proxy_session_manager_create( context, NEXT_INITIAL_PENDING_SESSION_SIZE );
    if ( server->pending_session_manager == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create pending session manager (proxy)" );
        next_server_destroy( server );
        return NULL;
    }

    server->session_manager = next_proxy_session_manager_create( context, NEXT_INITIAL_SESSION_SIZE );
    if ( server->session_manager == NULL )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server could not create session manager (proxy)" );
        next_server_destroy( server );
        return NULL;
    }

    server->context = context;
    server->packet_received_callback = packet_received_callback;

    return server;
}

uint16_t next_server_port( next_server_t * server )
{
    return server->bound_port;
}

void next_server_destroy( next_server_t * server )
{
    next_assert( server );

    if ( server->pending_session_manager )
    {
        next_proxy_session_manager_destroy( server->pending_session_manager );
    }

    if ( server->session_manager )
    {
        next_proxy_session_manager_destroy( server->session_manager );
    }

    if ( server->thread )
    {
        next_server_command_destroy_t * command = (next_server_command_destroy_t*) next_malloc( server->context, sizeof( next_server_command_destroy_t ) );
        if ( !command )
        {
            next_printf( NEXT_LOG_LEVEL_ERROR, "server destroy failed. could not create destroy command" );
            return;
        }
        command->type = NEXT_SERVER_COMMAND_DESTROY;
        {
            next_mutex_guard( server->internal->command_mutex );
            next_queue_push( server->internal->command_queue, command );
        }

        next_platform_thread_join( server->thread );
        next_platform_thread_destroy( server->thread );
    }

    if ( server->internal )
    {
        next_server_internal_destroy( server->internal );
    }

    clear_and_free( server->context, server, sizeof(next_server_t) );
}

void next_server_update( next_server_t * server )
{
    next_assert( server );

    while ( true )
    {
        void * queue_entry = NULL;
        {
            next_mutex_guard( server->internal->notify_mutex );    
            queue_entry = next_queue_pop( server->internal->notify_queue );
        }

        if ( queue_entry == NULL )
            break;

        next_server_notify_t * notify = (next_server_notify_t*) queue_entry;

        switch ( notify->type )
        {
            case NEXT_SERVER_NOTIFY_PACKET_RECEIVED:
            {
                next_server_notify_packet_received_t * packet_received = (next_server_notify_packet_received_t*) notify;
                next_assert( packet_received->packet_data );
                next_assert( packet_received->packet_bytes > 0 && packet_received->packet_bytes <= NEXT_MTU );
                server->packet_received_callback( server, server->context, &packet_received->from, packet_received->packet_data, packet_received->packet_bytes );
            }
            break;

            case NEXT_SERVER_NOTIFY_SESSION_UPGRADED:
            {
                next_server_notify_session_upgraded_t * session_upgraded = (next_server_notify_session_upgraded_t*) notify;
                char address_buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
                next_printf( NEXT_LOG_LEVEL_INFO, "server upgraded client %s to session %" PRIx64, next_address_to_string( &session_upgraded->address, address_buffer ), session_upgraded->session_id );
                next_proxy_session_entry_t * pending_entry = next_proxy_session_manager_find( server->pending_session_manager, &session_upgraded->address );
                if ( pending_entry && pending_entry->session_id == session_upgraded->session_id )
                {
                    next_proxy_session_manager_remove_by_address( server->session_manager, &session_upgraded->address );
                    next_proxy_session_manager_remove_by_address( server->pending_session_manager, &session_upgraded->address );
                    next_proxy_session_manager_add( server->session_manager, &session_upgraded->address, session_upgraded->session_id );
                }
            }
            break;

            case NEXT_SERVER_NOTIFY_PENDING_SESSION_TIMED_OUT:
            {
                next_server_notify_pending_session_timed_out_t * pending_session_timed_out = (next_server_notify_pending_session_timed_out_t*) notify;
                char address_buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
                next_printf( NEXT_LOG_LEVEL_WARN, "server timed out pending upgrade of client %s to session %" PRIx64, next_address_to_string( &pending_session_timed_out->address, address_buffer ), pending_session_timed_out->session_id );
                next_proxy_session_entry_t * pending_entry = next_proxy_session_manager_find( server->pending_session_manager, &pending_session_timed_out->address );
                if ( pending_entry && pending_entry->session_id == pending_session_timed_out->session_id )
                {
                    next_proxy_session_manager_remove_by_address( server->pending_session_manager, &pending_session_timed_out->address );
                    next_proxy_session_manager_remove_by_address( server->session_manager, &pending_session_timed_out->address );
                }
            }
            break;

            case NEXT_SERVER_NOTIFY_SESSION_TIMED_OUT:
            {
                next_server_notify_session_timed_out_t * session_timed_out = (next_server_notify_session_timed_out_t*) notify;
                char address_buffer[NEXT_MAX_ADDRESS_STRING_LENGTH];
                next_printf( NEXT_LOG_LEVEL_INFO, "server timed out client %s from session %" PRIx64, next_address_to_string( &session_timed_out->address, address_buffer ), session_timed_out->session_id );
                next_proxy_session_entry_t * proxy_session_entry = next_proxy_session_manager_find( server->session_manager, &session_timed_out->address );
                if ( proxy_session_entry && proxy_session_entry->session_id == session_timed_out->session_id )
                {
                    next_proxy_session_manager_remove_by_address( server->session_manager, &session_timed_out->address );
                }
            }
            break;

            case NEXT_SERVER_NOTIFY_FAILED_TO_RESOLVE_HOSTNAME:
            {
                server->failed_to_resolve_hostname = true;
            }
            break;

            default: break;
        }

        next_free( server->context, queue_entry );
    }
}

uint64_t next_generate_session_id()
{
    uint64_t session_id = 0;
    while ( session_id == 0 )
    {
        next_random_bytes( (uint8_t*) &session_id, 8 );
    }
    return session_id;
}

uint64_t next_server_upgrade_session( next_server_t * server, const next_address_t * address, const char * user_id, uint32_t platform_id, const char * tag )
{
    next_assert( server );
    next_assert( server->internal );
    next_assert( server->internal->command_mutex );
    next_assert( server->internal->command_queue );

    // can't upgrade sessions if network next is disabled

    if ( next_global_config.disable_network_next )
    {
        return 0;
    }

    // if we failed to resolve the network next hostname, we can't upgrade sessions

    if ( server->internal->failed_to_resolve_hostname )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "server can't upgrade session. failed to resolve hostname" );
        return 0;
    }

    // if we don't have a valid customer private key, we can't upgrade sessions

    if ( server->internal->invalid_customer_private_key )
    {
        next_printf( NEXT_LOG_LEVEL_DEBUG, "server can't upgrade session. invalid customer private key" );
        return 0;
    }

    // send upgrade session command to internal server

    next_server_command_upgrade_session_t * command = (next_server_command_upgrade_session_t*) next_malloc( server->context, sizeof( next_server_command_upgrade_session_t ) );
    if ( !command )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server upgrade session failed. could not create upgrade session command" );
        return 0;
    }

    uint64_t session_id = next_generate_session_id();

    uint64_t tag_id = next_tag_id( tag );
    
    if ( tag_id != 0 )
    {
        next_printf( NEXT_LOG_LEVEL_INFO, "server tagged session %" PRIx64 " as '%s' [%" PRIx64 "]", session_id, tag, tag_id );
    }

    uint64_t user_hash = ( user_id != NULL ) ? next_hash_string( user_id ) : 0;

    command->type = NEXT_SERVER_COMMAND_UPGRADE_SESSION;
    command->address = *address;
    command->user_hash = user_hash;
    command->platform_id = platform_id;
    command->session_id = session_id;
    command->tag = tag_id;

    {    
        next_mutex_guard( server->internal->command_mutex );
        next_queue_push( server->internal->command_queue, command );
    }

    // remove any existing entry for this address. latest upgrade takes precedence

    next_proxy_session_manager_remove_by_address( server->session_manager, address );
    next_proxy_session_manager_remove_by_address( server->pending_session_manager, address );

    // add a new pending session entry for this address

    next_proxy_session_entry_t * entry = next_proxy_session_manager_add( server->pending_session_manager, address, session_id );

    if ( entry == NULL )
    {
        next_assert( !"could not add pending session entry. this should never happen!" );
        return 0;
    }

    return session_id;
}

void next_server_tag_session( next_server_t * server, const next_address_t * address, const char * tag )
{
    next_assert( server );
    next_assert( server->internal );
    next_assert( server->internal->command_mutex );
    next_assert( server->internal->command_queue );

    // send tag session command to internal server

    next_server_command_tag_session_t * command = (next_server_command_tag_session_t*) next_malloc( server->context, sizeof( next_server_command_tag_session_t ) );
    if ( !command )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server tag session failed. could not create tag session command" );
        return;
    }

    uint64_t session_id = next_generate_session_id();

    uint64_t tag_id = next_tag_id( tag );
    
    if ( tag_id != 0 )
    {
        next_printf( NEXT_LOG_LEVEL_INFO, "server tagged session %" PRIx64 " as '%s' [%" PRIx64 "]", session_id, tag, tag_id );
    }

    command->type = NEXT_SERVER_COMMAND_UPGRADE_SESSION;
    command->address = *address;
    command->tag = tag_id;

    {    
        next_mutex_guard( server->internal->command_mutex );
        next_queue_push( server->internal->command_queue, command );
    }
}

int next_server_session_upgraded( next_server_t * server, const next_address_t * address )
{
    next_assert( server );
    next_assert( server->internal );
    next_assert( server->internal->command_mutex );
    next_assert( server->internal->command_queue );
    next_proxy_session_entry_t * pending_entry = next_proxy_session_manager_find( server->pending_session_manager, address );
    if ( pending_entry != NULL )
        return 1;
    next_proxy_session_entry_t * entry = next_proxy_session_manager_find( server->session_manager, address );
    if ( entry != NULL )
        return 1;
    return 0;
}

void next_server_send_packet( next_server_t * server, const next_address_t * to_address, const uint8_t * packet_data, int packet_bytes )
{
    next_assert( server );
    next_assert( to_address );
    next_assert( packet_data );
    next_assert( packet_bytes >= 0 );
    next_assert( packet_bytes <= NEXT_MTU );

    if ( packet_bytes <= 0 )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server can't send packet because packet size is <= 0 bytes" );
        return;
    }

    if ( packet_bytes > NEXT_MTU )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server can't send packet because packet size is greater than MTU (%d)", NEXT_MTU );
        return;
    }

    next_proxy_session_entry_t * entry = next_proxy_session_manager_find( server->session_manager, to_address );

    bool send_zero_byte_direct = true;
    bool send_over_network_next = false;
    bool send_upgraded_direct = false;

    if ( entry )
    {
        bool multipath = false;
        int envelope_kbps_down = 0;
        uint8_t open_session_sequence = 0;
        uint64_t send_sequence = 0;
        uint64_t session_id = 0;
        uint8_t session_version = 0;
        next_address_t session_address;
        uint8_t session_private_key[crypto_box_SECRETKEYBYTES];

        next_platform_mutex_acquire( server->internal->session_mutex );
        next_session_entry_t * internal_entry = next_session_manager_find_by_address( server->internal->session_manager, to_address );
        if ( internal_entry )
        {
            multipath = internal_entry->mutex_multipath;
            envelope_kbps_down = internal_entry->mutex_envelope_kbps_down;
            send_zero_byte_direct = false;
            send_over_network_next = internal_entry->mutex_send_over_network_next && ( !internal_entry->stats_try_before_you_buy || multipath );
            send_upgraded_direct = !send_over_network_next || multipath;
            send_sequence = internal_entry->mutex_payload_send_sequence++;
            send_sequence |= uint64_t(1) << 63;
            open_session_sequence = internal_entry->client_open_session_sequence;
            session_id = internal_entry->mutex_session_id;
            session_version = internal_entry->mutex_session_version;
            session_address = internal_entry->mutex_send_address;
            memcpy( session_private_key, internal_entry->mutex_private_key, crypto_box_SECRETKEYBYTES );
        }
        next_platform_mutex_release( server->internal->session_mutex );

        if ( !send_zero_byte_direct )
        {
            if ( send_over_network_next )
            {
                const int packet_bits = next_wire_packet_bits( NEXT_HEADER_BYTES + packet_bytes );

                bool over_budget = next_bandwidth_limiter_add_packet( &entry->send_bandwidth, next_time(), envelope_kbps_down, packet_bits );

                if ( over_budget )
                {
                    send_over_network_next = false;
                    if ( !multipath )
                    {
                        next_printf( NEXT_LOG_LEVEL_WARN, "server exceeded bandwidth budget for %" PRIx64 " (%d kbps). sending direct instead", session_id, envelope_kbps_down );
                        send_upgraded_direct = true;
                    }
                }
            }

            if ( send_over_network_next )
            {
                // send over network next

                uint8_t next_packet_data[NEXT_MAX_PACKET_BYTES];
                
                if ( next_write_header( NEXT_DIRECTION_SERVER_TO_CLIENT, NEXT_SERVER_TO_CLIENT_PACKET, send_sequence, session_id, session_version, 0, session_private_key, next_packet_data, NEXT_MAX_PACKET_BYTES ) != NEXT_OK )
                {
                    next_printf( NEXT_LOG_LEVEL_ERROR, "server failed to write server to client packet header" );
                    return;
                }

                memcpy( next_packet_data + NEXT_HEADER_BYTES, packet_data, packet_bytes );

                int next_packet_bytes = NEXT_HEADER_BYTES + packet_bytes;

                next_platform_socket_send_packet( server->internal->socket, &session_address, next_packet_data, next_packet_bytes );
            }

            if ( send_upgraded_direct )
            {
                // [255][open session sequence][packet sequence](payload) style packet direct to client

                uint8_t buffer[10+NEXT_MTU];
                uint8_t * p = buffer;
                next_write_uint8( &p, 255 );
                next_write_uint8( &p, open_session_sequence );
                next_write_uint64( &p, send_sequence );
                memcpy( buffer+10, packet_data, packet_bytes );
                next_platform_socket_send_packet( server->internal->socket, to_address, buffer, packet_bytes + 10 );
            }
        }
    }

    if ( send_zero_byte_direct )
    {
        // [0](payload) style packet direct to client

        uint8_t buffer[1+NEXT_MTU];
        buffer[0] = 0;
        memcpy( buffer+1, packet_data, packet_bytes );
        next_platform_socket_send_packet( server->internal->socket, to_address, buffer, packet_bytes + 1 );
    }
}

void next_server_send_packet_direct( next_server_t * server, const next_address_t * to_address, const uint8_t * packet_data, int packet_bytes )
{
    next_assert( server );
    next_assert( to_address );
    next_assert( packet_data );
    next_assert( packet_bytes >= 0 );
    next_assert( packet_bytes <= NEXT_MTU );

    if ( packet_bytes <= 0 )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server can't send packet because packet size is <= 0 bytes" );
        return;
    }

    if ( packet_bytes > NEXT_MTU )
    {
        next_printf( NEXT_LOG_LEVEL_ERROR, "server can't send packet because packet size is greater than MTU (%d)", NEXT_MTU );
        return;
    }

    // [0](payload) style packet direct to client

    uint8_t buffer[1+NEXT_MTU];
    buffer[0] = 0;
    memcpy( buffer+1, packet_data, packet_bytes );
    next_platform_socket_send_packet( server->internal->socket, to_address, buffer, packet_bytes + 1 );
}

// ---------------------------------------------------------------

static void next_check_handler( const char * condition, 
                                const char * function,
                                const char * file,
                                int line )
{
    printf( "check failed: ( %s ), function %s, file %s, line %d\n", condition, function, file, line );
    fflush( stdout );
#ifndef NDEBUG
    #if defined( __GNUC__ )
        __builtin_trap();
    #elif defined( _MSC_VER )
        __debugbreak();
    #endif
#endif
    exit( 1 );
}

#define check( condition )                                                                                  \
do                                                                                                          \
{                                                                                                           \
    if ( !(condition) )                                                                                     \
    {                                                                                                       \
        next_check_handler( #condition, (const char*) __FUNCTION__, (const char*) __FILE__, __LINE__ );     \
    }                                                                                                       \
} while(0)


static void test_time()
{
    double start = next_time();
    next_sleep( 0.1 );
    double finish = next_time();
    check( finish > start );
}

static void test_endian()
{
    uint32_t value = 0x11223344;
    char bytes[4];
    memcpy( bytes, &value, 4 );

#if NEXT_LITTLE_ENDIAN

    check( bytes[0] == 0x44 );
    check( bytes[1] == 0x33 );
    check( bytes[2] == 0x22 );
    check( bytes[3] == 0x11 );

#else // #if NEXT_LITTLE_ENDIAN

    check( bytes[3] == 0x44 );
    check( bytes[2] == 0x33 );
    check( bytes[1] == 0x22 );
    check( bytes[0] == 0x11 );

#endif // #if NEXT_LITTLE_ENDIAN
}

static void test_base64()
{
    const char * input = "a test string. let's see if it works properly";
    char encoded[1024];
    char decoded[1024];
    check( next_base64_encode_string( input, encoded, sizeof(encoded) ) > 0 );
    check( next_base64_decode_string( encoded, decoded, sizeof(decoded) ) > 0 );
    check( strcmp( decoded, input ) == 0 );
    check( next_base64_decode_string( encoded, decoded, 10 ) == 0 );
}

static void test_fnv1a()
{
    uint64_t hash = next_datacenter_id( "local" );
    check( hash == 0x249f1fb6f3a680e8ULL );
}

static void test_queue()
{
    const int QueueSize = 64;
    const int EntrySize = 1024;

    next_queue_t * queue = next_queue_create( NULL, QueueSize );

    check( queue->num_entries == 0 );
    check( queue->start_index == 0 );

    // attempting to pop a packet off an empty queue should return NULL

    check( next_queue_pop( queue ) == NULL );

    // add some entries to the queue and make sure they pop off in the correct order
    {
        const int NumEntries = 50;

        void * entries[NumEntries];

        int i;
        for ( i = 0; i < NumEntries; ++i )
        {
            entries[i] = next_malloc( NULL, EntrySize );
            memset( entries[i], 0, EntrySize );
            check( next_queue_push( queue, entries[i] ) == NEXT_OK );
        }

        check( queue->num_entries == NumEntries );

        for ( i = 0; i < NumEntries; ++i )
        {
            void * entry = next_queue_pop( queue );
            check( entry == entries[i] );
            next_free( NULL, entry );
        }
    }

    // after all entries are popped off, the queue is empty, so calls to pop should return NULL

    check( queue->num_entries == 0 );

    check( next_queue_pop( queue ) == NULL );

    // test that the queue can be filled to max capacity

    void * entries[QueueSize];

    int i;
    for ( i = 0; i < QueueSize; ++i )
    {
        entries[i] = next_malloc( NULL, EntrySize );
        check( next_queue_push( queue, entries[i] ) == NEXT_OK );
    }

    check( queue->num_entries == QueueSize );

    // when the queue is full, attempting to push an entry should fail

    check( next_queue_push( queue, next_malloc( NULL, 100 ) ) == NEXT_ERROR );

    // make sure all packets pop off in the correct order

    for ( i = 0; i < QueueSize; ++i )
    {
        void * entry = next_queue_pop( queue );
        check( entry == entries[i] );
        next_free( NULL, entry );
    }

    // add some entries again

    for ( i = 0; i < QueueSize; ++i )
    {
        entries[i] = next_malloc( NULL, EntrySize );
        check( next_queue_push( queue, entries[i] ) == NEXT_OK );
    }

    // clear the queue and make sure that all entries are freed

    next_queue_clear( queue );

    check( queue->start_index == 0 );
    check( queue->num_entries == 0 );
    for ( i = 0; i < QueueSize; ++i )
        check( queue->entries[i] == NULL );

    // destroy the queue

    next_queue_destroy( queue );
}

using namespace next;

static void test_bitpacker()
{
    const int BufferSize = 256;

    uint8_t buffer[BufferSize];

    BitWriter writer( buffer, BufferSize );

    check( writer.GetData() == buffer );
    check( writer.GetBitsWritten() == 0 );
    check( writer.GetBytesWritten() == 0 );
    check( writer.GetBitsAvailable() == BufferSize * 8 );

    writer.WriteBits( 0, 1 );
    writer.WriteBits( 1, 1 );
    writer.WriteBits( 10, 8 );
    writer.WriteBits( 255, 8 );
    writer.WriteBits( 1000, 10 );
    writer.WriteBits( 50000, 16 );
    writer.WriteBits( 9999999, 32 );
    writer.FlushBits();

    const int bitsWritten = 1 + 1 + 8 + 8 + 10 + 16 + 32;

    check( writer.GetBytesWritten() == 10 );
    check( writer.GetBitsWritten() == bitsWritten );
    check( writer.GetBitsAvailable() == BufferSize * 8 - bitsWritten );

    const int bytesWritten = writer.GetBytesWritten();

    check( bytesWritten == 10 );

    memset( buffer + bytesWritten, 0, BufferSize - bytesWritten );

    BitReader reader( buffer, bytesWritten );

    check( reader.GetBitsRead() == 0 );
    check( reader.GetBitsRemaining() == bytesWritten * 8 );

    uint32_t a = reader.ReadBits( 1 );
    uint32_t b = reader.ReadBits( 1 );
    uint32_t c = reader.ReadBits( 8 );
    uint32_t d = reader.ReadBits( 8 );
    uint32_t e = reader.ReadBits( 10 );
    uint32_t f = reader.ReadBits( 16 );
    uint32_t g = reader.ReadBits( 32 );

    check( a == 0 );
    check( b == 1 );
    check( c == 10 );
    check( d == 255 );
    check( e == 1000 );
    check( f == 50000 );
    check( g == 9999999 );

    check( reader.GetBitsRead() == bitsWritten );
    check( reader.GetBitsRemaining() == bytesWritten * 8 - bitsWritten );
}

const int MaxItems = 11;

struct TestData
{
    TestData()
    {
        memset( this, 0, sizeof( TestData ) );
    }

    int a,b,c;
    uint32_t d : 8;
    uint32_t e : 8;
    uint32_t f : 8;
    bool g;
    int numItems;
    int items[MaxItems];
    float float_value;
    double double_value;
    uint64_t uint64_value;
    uint8_t bytes[17];
    char string[256];
    next_address_t address_a, address_b, address_c;
};

struct TestContext
{
    int min;
    int max;
};

struct TestObject
{
    TestData data;

    void Init()
    {
        data.a = 1;
        data.b = -2;
        data.c = 150;
        data.d = 55;
        data.e = 255;
        data.f = 127;
        data.g = true;

        data.numItems = MaxItems / 2;
        for ( int i = 0; i < data.numItems; ++i )
            data.items[i] = i + 10;     

        data.float_value = 3.1415926f;
        data.double_value = 1 / 3.0;   
        data.uint64_value = 0x1234567898765432L;

        for ( int i = 0; i < (int) sizeof( data.bytes ); ++i )
            data.bytes[i] = ( i * 37 ) % 255;

        strcpy( data.string, "hello world!" );

        memset( &data.address_a, 0, sizeof(next_address_t) );

        next_address_parse( &data.address_b, "127.0.0.1:50000" );

        next_address_parse( &data.address_c, "[::1]:50000" );
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        const TestContext & context = *(const TestContext*) stream.GetContext();

        serialize_int( stream, data.a, context.min, context.max );
        serialize_int( stream, data.b, context.min, context.max );

        serialize_int( stream, data.c, -100, 10000 );

        serialize_bits( stream, data.d, 6 );
        serialize_bits( stream, data.e, 8 );
        serialize_bits( stream, data.f, 7 );

        serialize_align( stream );

        serialize_bool( stream, data.g );

        serialize_int( stream, data.numItems, 0, MaxItems - 1 );
        for ( int i = 0; i < data.numItems; ++i )
            serialize_bits( stream, data.items[i], 8 );

        serialize_float( stream, data.float_value );

        serialize_double( stream, data.double_value );

        serialize_uint64( stream, data.uint64_value );

        serialize_bytes( stream, data.bytes, sizeof( data.bytes ) );

        serialize_string( stream, data.string, sizeof( data.string ) );

        serialize_address( stream, data.address_a );
        serialize_address( stream, data.address_b );
        serialize_address( stream, data.address_c );

        return true;
    }

    bool operator == ( const TestObject & other ) const
    {
        return memcmp( &data, &other.data, sizeof( TestData ) ) == 0;
    }

    bool operator != ( const TestObject & other ) const
    {
        return ! ( *this == other );
    }
};

static void test_stream()
{
    const int BufferSize = 1024;

    uint8_t buffer[BufferSize];

    TestContext context;
    context.min = -10;
    context.max = +10;

    WriteStream writeStream( buffer, BufferSize );

    TestObject writeObject;
    writeObject.Init();
    writeStream.SetContext( &context );
    writeObject.Serialize( writeStream );
    writeStream.Flush();

    const int bytesWritten = writeStream.GetBytesProcessed();

    memset( buffer + bytesWritten, 0, BufferSize - bytesWritten );

    TestObject readObject;
    ReadStream readStream( buffer, bytesWritten );
    readStream.SetContext( &context );
    readObject.Serialize( readStream );

    check( readObject == writeObject );
}

static bool equal_within_tolerance( float a, float b, float tolerance = 0.0001f )
{
    return fabs(a-b) <= tolerance;
}

static void test_bits_required()
{
    check( bits_required( 0, 0 ) == 0 );
    check( bits_required( 0, 1 ) == 1 );
    check( bits_required( 0, 2 ) == 2 );
    check( bits_required( 0, 3 ) == 2 );
    check( bits_required( 0, 4 ) == 3 );
    check( bits_required( 0, 5 ) == 3 );
    check( bits_required( 0, 6 ) == 3 );
    check( bits_required( 0, 7 ) == 3 );
    check( bits_required( 0, 8 ) == 4 );
    check( bits_required( 0, 255 ) == 8 );
    check( bits_required( 0, 65535 ) == 16 );
    check( bits_required( 0, 4294967295U ) == 32 );
}

static void test_address()
{
    {
        struct next_address_t address;
        check( next_address_parse( &address, "" ) == NEXT_ERROR );
        check( next_address_parse( &address, "[" ) == NEXT_ERROR );
        check( next_address_parse( &address, "[]" ) == NEXT_ERROR );
        check( next_address_parse( &address, "[]:" ) == NEXT_ERROR );
        check( next_address_parse( &address, ":" ) == NEXT_ERROR );
#if !defined(WINVER) || WINVER > 0x502 // windows xp sucks
        check( next_address_parse( &address, "1" ) == NEXT_ERROR );
        check( next_address_parse( &address, "12" ) == NEXT_ERROR );
        check( next_address_parse( &address, "123" ) == NEXT_ERROR );
        check( next_address_parse( &address, "1234" ) == NEXT_ERROR );
#endif
        check( next_address_parse( &address, "1234.0.12313.0000" ) == NEXT_ERROR );
        check( next_address_parse( &address, "1234.0.12313.0000.0.0.0.0.0" ) == NEXT_ERROR );
        check( next_address_parse( &address, "1312313:123131:1312313:123131:1312313:123131:1312313:123131:1312313:123131:1312313:123131" ) == NEXT_ERROR );
        check( next_address_parse( &address, "." ) == NEXT_ERROR );
        check( next_address_parse( &address, ".." ) == NEXT_ERROR );
        check( next_address_parse( &address, "..." ) == NEXT_ERROR );
        check( next_address_parse( &address, "...." ) == NEXT_ERROR );
        check( next_address_parse( &address, "....." ) == NEXT_ERROR );
    }

    {
        struct next_address_t address;
        check( next_address_parse( &address, "107.77.207.77" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV4 );
        check( address.port == 0 );
        check( address.data.ipv4[0] == 107 );
        check( address.data.ipv4[1] == 77 );
        check( address.data.ipv4[2] == 207 );
        check( address.data.ipv4[3] == 77 );
    }

    {
        struct next_address_t address;
        check( next_address_parse( &address, "127.0.0.1" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV4 );
        check( address.port == 0 );
        check( address.data.ipv4[0] == 127 );
        check( address.data.ipv4[1] == 0 );
        check( address.data.ipv4[2] == 0 );
        check( address.data.ipv4[3] == 1 );
    }

    {
        struct next_address_t address;
        check( next_address_parse( &address, "107.77.207.77:40000" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV4 );
        check( address.port == 40000 );
        check( address.data.ipv4[0] == 107 );
        check( address.data.ipv4[1] == 77 );
        check( address.data.ipv4[2] == 207 );
        check( address.data.ipv4[3] == 77 );
    }

    {
        struct next_address_t address;
        check( next_address_parse( &address, "127.0.0.1:40000" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV4 );
        check( address.port == 40000 );
        check( address.data.ipv4[0] == 127 );
        check( address.data.ipv4[1] == 0 );
        check( address.data.ipv4[2] == 0 );
        check( address.data.ipv4[3] == 1 );
    }

#if NEXT_PLATFORM_HAS_IPV6
    {
        struct next_address_t address;
        check( next_address_parse( &address, "fe80::202:b3ff:fe1e:8329" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV6 );
        check( address.port == 0 );
        check( address.data.ipv6[0] == 0xfe80 );
        check( address.data.ipv6[1] == 0x0000 );
        check( address.data.ipv6[2] == 0x0000 );
        check( address.data.ipv6[3] == 0x0000 );
        check( address.data.ipv6[4] == 0x0202 );
        check( address.data.ipv6[5] == 0xb3ff );
        check( address.data.ipv6[6] == 0xfe1e );
        check( address.data.ipv6[7] == 0x8329 );
    }

    {
        struct next_address_t address;
        check( next_address_parse( &address, "::" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV6 );
        check( address.port == 0 );
        check( address.data.ipv6[0] == 0x0000 );
        check( address.data.ipv6[1] == 0x0000 );
        check( address.data.ipv6[2] == 0x0000 );
        check( address.data.ipv6[3] == 0x0000 );
        check( address.data.ipv6[4] == 0x0000 );
        check( address.data.ipv6[5] == 0x0000 );
        check( address.data.ipv6[6] == 0x0000 );
        check( address.data.ipv6[7] == 0x0000 );
    }

    {
        struct next_address_t address;
        check( next_address_parse( &address, "::1" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV6 );
        check( address.port == 0 );
        check( address.data.ipv6[0] == 0x0000 );
        check( address.data.ipv6[1] == 0x0000 );
        check( address.data.ipv6[2] == 0x0000 );
        check( address.data.ipv6[3] == 0x0000 );
        check( address.data.ipv6[4] == 0x0000 );
        check( address.data.ipv6[5] == 0x0000 );
        check( address.data.ipv6[6] == 0x0000 );
        check( address.data.ipv6[7] == 0x0001 );
    }

    {
        struct next_address_t address;
        check( next_address_parse( &address, "[fe80::202:b3ff:fe1e:8329]:40000" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV6 );
        check( address.port == 40000 );
        check( address.data.ipv6[0] == 0xfe80 );
        check( address.data.ipv6[1] == 0x0000 );
        check( address.data.ipv6[2] == 0x0000 );
        check( address.data.ipv6[3] == 0x0000 );
        check( address.data.ipv6[4] == 0x0202 );
        check( address.data.ipv6[5] == 0xb3ff );
        check( address.data.ipv6[6] == 0xfe1e );
        check( address.data.ipv6[7] == 0x8329 );
    }

    {
        struct next_address_t address;
        check( next_address_parse( &address, "[::]:40000" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV6 );
        check( address.port == 40000 );
        check( address.data.ipv6[0] == 0x0000 );
        check( address.data.ipv6[1] == 0x0000 );
        check( address.data.ipv6[2] == 0x0000 );
        check( address.data.ipv6[3] == 0x0000 );
        check( address.data.ipv6[4] == 0x0000 );
        check( address.data.ipv6[5] == 0x0000 );
        check( address.data.ipv6[6] == 0x0000 );
        check( address.data.ipv6[7] == 0x0000 );
    }

    {
        struct next_address_t address;
        check( next_address_parse( &address, "[::1]:40000" ) == NEXT_OK );
        check( address.type == NEXT_ADDRESS_IPV6 );
        check( address.port == 40000 );
        check( address.data.ipv6[0] == 0x0000 );
        check( address.data.ipv6[1] == 0x0000 );
        check( address.data.ipv6[2] == 0x0000 );
        check( address.data.ipv6[3] == 0x0000 );
        check( address.data.ipv6[4] == 0x0000 );
        check( address.data.ipv6[5] == 0x0000 );
        check( address.data.ipv6[6] == 0x0000 );
        check( address.data.ipv6[7] == 0x0001 );
    }
#endif // #if NEXT_PLATFORM_HAS_IPV6
}

static void test_replay_protection()
{
    next_replay_protection_t replay_protection;

    int i;
    for ( i = 0; i < 2; ++i )
    {
        next_replay_protection_reset( &replay_protection );

        check( replay_protection.most_recent_sequence == 0 );

        // the first time we receive packets, they should not be already received

        #define MAX_SEQUENCE ( NEXT_REPLAY_PROTECTION_BUFFER_SIZE * 4 )

        uint64_t sequence;
        for ( sequence = 0; sequence < MAX_SEQUENCE; ++sequence )
        {
            check( next_replay_protection_already_received( &replay_protection, sequence ) == 0 );
            next_replay_protection_advance_sequence( &replay_protection, sequence );
        }

        // old packets outside buffer should be considered already received

        check( next_replay_protection_already_received( &replay_protection, 0 ) == 1 );

        // packets received a second time should be flagged already received

        for ( sequence = MAX_SEQUENCE - 10; sequence < MAX_SEQUENCE; ++sequence )
        {
            check( next_replay_protection_already_received( &replay_protection, sequence ) == 1 );
        }

        // jumping ahead to a much higher sequence should be considered not already received

        check( next_replay_protection_already_received( &replay_protection, MAX_SEQUENCE + NEXT_REPLAY_PROTECTION_BUFFER_SIZE ) == 0 );

        // old packets should be considered already received

        for ( sequence = 0; sequence < MAX_SEQUENCE; ++sequence )
        {
            check( next_replay_protection_already_received( &replay_protection, sequence ) == 1 );
        }
    }
}

static void test_ping_stats()
{
    // default ping history should have -1 rtt, indicating "no data"
    {
        const double ping_safety = 1.0;

        next_ping_history_t history;
        next_ping_history_clear( &history );
        
        next_route_stats_t route_stats;
        next_route_stats_from_ping_history( &history, 0.0, 10.0, &route_stats, ping_safety );
        
        check( route_stats.min_rtt == 0.0f );
        check( route_stats.max_rtt == 0.0f );
        check( route_stats.mean_rtt == 0.0f );
        check( route_stats.jitter == 0.0f );
        check( route_stats.packet_loss == 0.0f );
    }

    // add some pings without pong response, packet loss should be 100%
    {
        const double ping_safety = 1.0;

        next_ping_history_t history;
        next_ping_history_clear( &history );

        for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; ++i )
        {
            next_ping_history_ping_sent( &history, i * 0.01 );
        }

        next_route_stats_t route_stats;
        next_route_stats_from_ping_history( &history, 0.0, 10.0, &route_stats, ping_safety );

        check( route_stats.min_rtt == 0.0f );
        check( route_stats.max_rtt == 0.0f );
        check( route_stats.mean_rtt == 0.0f );
        check( route_stats.jitter == 0.0f );
        check( route_stats.packet_loss == 100.0f );
    }

    // add some pings and set them to have a pong response, packet loss should be 0%
    {
        const double ping_safety = 1.0;

        next_ping_history_t history;
        next_ping_history_clear( &history );

        const double expected_rtt = 0.1;

        for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; ++i )
        {
            uint64_t sequence = next_ping_history_ping_sent( &history, i * 0.01 );
            next_ping_history_pong_received( &history, sequence, i * 0.01 + expected_rtt );
        }

        next_route_stats_t route_stats;
        next_route_stats_from_ping_history( &history, 0.0, 100.0, &route_stats, ping_safety );

        check( equal_within_tolerance( route_stats.min_rtt, expected_rtt * 1000.0 ) );
        check( equal_within_tolerance( route_stats.max_rtt, expected_rtt * 1000.0 ) );
        check( equal_within_tolerance( route_stats.mean_rtt, expected_rtt * 1000.0 ) );
        check( equal_within_tolerance( route_stats.jitter, 0.0 ) );
        check( route_stats.packet_loss == 0.0 );
    }

    // add some pings and set them to have a pong response, but leave the last second of pings without response. packet loss should be zero
    {
        const double ping_safety = 1.0;

        next_ping_history_t history;
        next_ping_history_clear( &history );

        const double expected_rtt = 0.1;

        const double delta_time = 10.0 / NEXT_PING_HISTORY_ENTRY_COUNT;

        for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; ++i )
        {
            const double ping_send_time = i * delta_time;
            const double pong_recv_time = ping_send_time + expected_rtt;

            if ( ping_send_time > 9.0 )
            {
                uint64_t sequence = next_ping_history_ping_sent( &history, ping_send_time );
                next_ping_history_pong_received( &history, sequence, pong_recv_time );
            }
        }

        next_route_stats_t route_stats;
        next_route_stats_from_ping_history( &history, 0.0, 10.0, &route_stats, ping_safety );

        check( equal_within_tolerance( route_stats.min_rtt, expected_rtt * 1000.0 ) );
        check( equal_within_tolerance( route_stats.max_rtt, expected_rtt * 1000.0 ) );
        check( equal_within_tolerance( route_stats.mean_rtt, expected_rtt * 1000.0 ) );
        check( equal_within_tolerance( route_stats.jitter, 0.0 ) );
        check( route_stats.packet_loss == 0.0 );
    }

    // drop 1 in every 2 packets. packet loss should be 50%
    {
        const double ping_safety = 1.0;

        next_ping_history_t history;
        next_ping_history_clear( &history );

        const double expected_rtt = 0.1;

        for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; ++i )
        {
            uint64_t sequence = next_ping_history_ping_sent( &history, i * 0.01 );
            if ( i & 1 )
                next_ping_history_pong_received( &history, sequence, i * 0.01 + expected_rtt );
        }

        next_route_stats_t route_stats;
        next_route_stats_from_ping_history( &history, 0.0, 100.0, &route_stats, ping_safety );

        check( equal_within_tolerance( route_stats.min_rtt, expected_rtt * 1000.0 ) );
        check( equal_within_tolerance( route_stats.max_rtt, expected_rtt * 1000.0 ) );
        check( equal_within_tolerance( route_stats.mean_rtt, expected_rtt * 1000.0 ) );
        check( equal_within_tolerance( route_stats.jitter, 0.0 ) );
        check( equal_within_tolerance( route_stats.packet_loss, 50.0 ) );
    }

    // drop 1 in every 10 packets. packet loss should be ~10%
    {
        const double ping_safety = 1.0;

        next_ping_history_t history;
        next_ping_history_clear( &history );

        const double expected_rtt = 0.1;

        for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; ++i )
        {
            uint64_t sequence = next_ping_history_ping_sent( &history, i * 0.01 );
            if ( ( i % 10 ) )
                next_ping_history_pong_received( &history, sequence, i * 0.01 + expected_rtt );
        }

        next_route_stats_t route_stats;
        next_route_stats_from_ping_history( &history, 0.0, 100.0, &route_stats, ping_safety );

        check( equal_within_tolerance( route_stats.min_rtt, expected_rtt * 1000.0f ) );
        check( equal_within_tolerance( route_stats.max_rtt, expected_rtt * 1000.0f ) );
        check( equal_within_tolerance( route_stats.mean_rtt, expected_rtt * 1000.0f ) );
        check( equal_within_tolerance( route_stats.jitter, 0.0 ) );
        check( equal_within_tolerance( route_stats.packet_loss, 10.0f, 0.25f ) );
    }

    // drop 9 in every 10 packets. packet loss should be ~90%
    {
        const double ping_safety = 1.0;

        next_ping_history_t history;
        next_ping_history_clear( &history );

        const double expected_rtt = 0.1;

        for ( int i = 0; i < NEXT_PING_HISTORY_ENTRY_COUNT; ++i )
        {
            uint64_t sequence = next_ping_history_ping_sent( &history, i * 0.01 );
            if ( ( i % 10 ) == 0 )
                next_ping_history_pong_received( &history, sequence, i * 0.01 + expected_rtt );
        }

        next_route_stats_t route_stats;
        next_route_stats_from_ping_history( &history, 0.0, 100.0, &route_stats, ping_safety );

        check( equal_within_tolerance( route_stats.min_rtt, expected_rtt * 1000.0f ) );
        check( equal_within_tolerance( route_stats.max_rtt, expected_rtt * 1000.0f ) );
        check( equal_within_tolerance( route_stats.mean_rtt, expected_rtt * 1000.0f ) );
        check( equal_within_tolerance( route_stats.jitter, 0.0f ) );
        check( equal_within_tolerance( route_stats.packet_loss, 90.0f, 0.25f ) );
    }
}

static void test_random_bytes()
{
    const int BufferSize = 64;
    uint8_t buffer[BufferSize];
    next_random_bytes( buffer, BufferSize );
    for ( int i = 0; i < 100; ++i )
    {
        uint8_t next_buffer[BufferSize];
        next_random_bytes( next_buffer, BufferSize );
        check( memcmp( buffer, next_buffer, BufferSize ) != 0 );
        memcpy( buffer, next_buffer, BufferSize );
    }
}

static void test_random_float()
{
    for ( int i = 0; i < 1000; ++i )
    {
        float value = next_random_float();
        check( value >= 0.0f );
        check( value <= 1.0f );
    }
}

static void test_crypto_box()
{
    #define CRYPTO_BOX_MESSAGE (const unsigned char *) "test"
    #define CRYPTO_BOX_MESSAGE_LEN 4
    #define CRYPTO_BOX_CIPHERTEXT_LEN ( crypto_box_MACBYTES + CRYPTO_BOX_MESSAGE_LEN )

    unsigned char sender_publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char sender_secretkey[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair( sender_publickey, sender_secretkey );

    unsigned char receiver_publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char receiver_secretkey[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair( receiver_publickey, receiver_secretkey );

    unsigned char nonce[crypto_box_NONCEBYTES];
    unsigned char ciphertext[CRYPTO_BOX_CIPHERTEXT_LEN];
    next_random_bytes( nonce, sizeof nonce );
    check( crypto_box_easy( ciphertext, CRYPTO_BOX_MESSAGE, CRYPTO_BOX_MESSAGE_LEN, nonce, receiver_publickey, sender_secretkey ) == 0 );

    unsigned char decrypted[CRYPTO_BOX_MESSAGE_LEN];
    check( crypto_box_open_easy( decrypted, ciphertext, CRYPTO_BOX_CIPHERTEXT_LEN, nonce, sender_publickey, receiver_secretkey ) == 0 );

    check( memcmp( decrypted, CRYPTO_BOX_MESSAGE, CRYPTO_BOX_MESSAGE_LEN ) == 0 );
}

static void test_crypto_secret_box()
{
    #define CRYPTO_SECRET_BOX_MESSAGE ((const unsigned char *) "test")
    #define CRYPTO_SECRET_BOX_MESSAGE_LEN 4
    #define CRYPTO_SECRET_BOX_CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + CRYPTO_SECRET_BOX_MESSAGE_LEN)

    unsigned char key[crypto_secretbox_KEYBYTES];
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char ciphertext[CRYPTO_SECRET_BOX_CIPHERTEXT_LEN];

    crypto_secretbox_keygen( key );
    randombytes_buf( nonce, crypto_secretbox_NONCEBYTES );
    crypto_secretbox_easy( ciphertext, CRYPTO_SECRET_BOX_MESSAGE, CRYPTO_SECRET_BOX_MESSAGE_LEN, nonce, key );

    unsigned char decrypted[CRYPTO_SECRET_BOX_MESSAGE_LEN];
    check( crypto_secretbox_open_easy( decrypted, ciphertext, CRYPTO_SECRET_BOX_CIPHERTEXT_LEN, nonce, key ) == 0 );
}

static void test_crypto_aead()
{
    #define CRYPTO_AEAD_MESSAGE (const unsigned char *) "test"
    #define CRYPTO_AEAD_MESSAGE_LEN 4
    #define CRYPTO_AEAD_ADDITIONAL_DATA (const unsigned char *) "123456"
    #define CRYPTO_AEAD_ADDITIONAL_DATA_LEN 6

    unsigned char nonce[crypto_aead_chacha20poly1305_NPUBBYTES];
    unsigned char key[crypto_aead_chacha20poly1305_KEYBYTES];
    unsigned char ciphertext[CRYPTO_AEAD_MESSAGE_LEN + crypto_aead_chacha20poly1305_ABYTES];
    unsigned long long ciphertext_len;

    crypto_aead_chacha20poly1305_keygen( key );
    randombytes_buf( nonce, sizeof(nonce) );

    crypto_aead_chacha20poly1305_encrypt( ciphertext, &ciphertext_len,
                                          CRYPTO_AEAD_MESSAGE, CRYPTO_AEAD_MESSAGE_LEN,
                                          CRYPTO_AEAD_ADDITIONAL_DATA, CRYPTO_AEAD_ADDITIONAL_DATA_LEN,
                                          NULL, nonce, key );

    unsigned char decrypted[CRYPTO_AEAD_MESSAGE_LEN];
    unsigned long long decrypted_len;
    check( crypto_aead_chacha20poly1305_decrypt( decrypted, &decrypted_len,
                                                 NULL,
                                                 ciphertext, ciphertext_len,
                                                 CRYPTO_AEAD_ADDITIONAL_DATA,
                                                 CRYPTO_AEAD_ADDITIONAL_DATA_LEN,
                                                 nonce, key) == 0 );
}

static void test_crypto_aead_ietf()
{
    #define CRYPTO_AEAD_IETF_MESSAGE (const unsigned char *) "test"
    #define CRYPTO_AEAD_IETF_MESSAGE_LEN 4
    #define CRYPTO_AEAD_IETF_ADDITIONAL_DATA (const unsigned char *) "123456"
    #define CRYPTO_AEAD_IETF_ADDITIONAL_DATA_LEN 6

    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
    unsigned char ciphertext[CRYPTO_AEAD_IETF_MESSAGE_LEN + crypto_aead_xchacha20poly1305_ietf_ABYTES];
    unsigned long long ciphertext_len;

    crypto_aead_xchacha20poly1305_ietf_keygen( key );
    randombytes_buf( nonce, sizeof(nonce) );

    crypto_aead_xchacha20poly1305_ietf_encrypt( ciphertext, &ciphertext_len, CRYPTO_AEAD_IETF_MESSAGE, CRYPTO_AEAD_IETF_MESSAGE_LEN, CRYPTO_AEAD_IETF_ADDITIONAL_DATA, CRYPTO_AEAD_IETF_ADDITIONAL_DATA_LEN, NULL, nonce, key);

    unsigned char decrypted[CRYPTO_AEAD_IETF_MESSAGE_LEN];
    unsigned long long decrypted_len;
    check(crypto_aead_xchacha20poly1305_ietf_decrypt( decrypted, &decrypted_len, NULL, ciphertext, ciphertext_len, CRYPTO_AEAD_IETF_ADDITIONAL_DATA, CRYPTO_AEAD_IETF_ADDITIONAL_DATA_LEN, nonce, key ) == 0 );
}

static void test_crypto_sign()
{
    #define CRYPTO_SIGN_MESSAGE (const unsigned char *) "test"
    #define CRYPTO_SIGN_MESSAGE_LEN 4

    unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
    unsigned char private_key[crypto_sign_SECRETKEYBYTES];
    crypto_sign_keypair( public_key, private_key );

    unsigned char signed_message[crypto_sign_BYTES + CRYPTO_SIGN_MESSAGE_LEN];
    unsigned long long signed_message_len;

    crypto_sign( signed_message, &signed_message_len, CRYPTO_SIGN_MESSAGE, CRYPTO_SIGN_MESSAGE_LEN, private_key );

    unsigned char unsigned_message[CRYPTO_SIGN_MESSAGE_LEN];
    unsigned long long unsigned_message_len;
    check( crypto_sign_open( unsigned_message, &unsigned_message_len, signed_message, signed_message_len, public_key ) == 0 );
}

static void test_crypto_sign_detached()
{
    #define MESSAGE_PART1 ((const unsigned char *) "Arbitrary data to hash")
    #define MESSAGE_PART1_LEN 22

    #define MESSAGE_PART2 ((const unsigned char *) "is longer than expected")
    #define MESSAGE_PART2_LEN 23

    unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
    unsigned char private_key[crypto_sign_SECRETKEYBYTES];
    crypto_sign_keypair( public_key, private_key );

    crypto_sign_state state;

    unsigned char signature[crypto_sign_BYTES];

    crypto_sign_init( &state );
    crypto_sign_update( &state, MESSAGE_PART1, MESSAGE_PART1_LEN );
    crypto_sign_update( &state, MESSAGE_PART2, MESSAGE_PART2_LEN );
    crypto_sign_final_create( &state, signature, NULL, private_key );

    crypto_sign_init( &state );
    crypto_sign_update( &state, MESSAGE_PART1, MESSAGE_PART1_LEN );
    crypto_sign_update( &state, MESSAGE_PART2, MESSAGE_PART2_LEN );
    check( crypto_sign_final_verify( &state, signature, public_key ) == 0 );
}

static void test_crypto_key_exchange()
{
    uint8_t client_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t client_private_key[crypto_kx_SECRETKEYBYTES];
    crypto_kx_keypair( client_public_key, client_private_key );

    uint8_t server_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t server_private_key[crypto_kx_SECRETKEYBYTES];
    crypto_kx_keypair( server_public_key, server_private_key );

    uint8_t client_send_key[crypto_kx_SESSIONKEYBYTES];
    uint8_t client_receive_key[crypto_kx_SESSIONKEYBYTES];
    check( crypto_kx_client_session_keys( client_receive_key, client_send_key, client_public_key, client_private_key, server_public_key ) == 0 );

    uint8_t server_send_key[crypto_kx_SESSIONKEYBYTES];
    uint8_t server_receive_key[crypto_kx_SESSIONKEYBYTES];
    check( crypto_kx_server_session_keys( server_receive_key, server_send_key, server_public_key, server_private_key, client_public_key ) == 0 );

    check( memcmp( client_send_key, server_receive_key, crypto_kx_SESSIONKEYBYTES ) == 0 );
    check( memcmp( server_send_key, client_receive_key, crypto_kx_SESSIONKEYBYTES ) == 0 );
}

static void test_basic_read_and_write()
{
    uint8_t buffer[1024];

    uint8_t * p = buffer;
    next_write_uint8( &p, 105 );
    next_write_uint16( &p, 10512 );
    next_write_uint32( &p, 105120000 );
    next_write_uint64( &p, 105120000000000000LL );
    next_write_float32( &p, 100.0f );
    next_write_float64( &p, 100000000000000.0 );
    next_write_bytes( &p, (uint8_t*)"hello", 6 );

    const uint8_t * q = buffer;

    uint8_t a = next_read_uint8( &q );
    uint16_t b = next_read_uint16( &q );
    uint32_t c = next_read_uint32( &q );
    uint64_t d = next_read_uint64( &q );
    float e = next_read_float32( &q );
    double f = next_read_float64( &q );
    uint8_t g[6];
    next_read_bytes( &q, g, 6 );

    check( a == 105 );
    check( b == 10512 );
    check( c == 105120000 );
    check( d == 105120000000000000LL );
    check( e == 100.0f );
    check( f == 100000000000000.0 );
    check( memcmp( g, "hello", 6 ) == 0 );
}

static void test_address_read_and_write()
{
    struct next_address_t a, b, c;

    memset( &a, 0, sizeof(a) );

    next_address_parse( &b, "127.0.0.1:50000" );

    next_address_parse( &c, "[::1]:50000" );

    uint8_t buffer[1024];

    uint8_t * p = buffer;

    next_write_address( &p, &a );
    next_write_address( &p, &b );
    next_write_address( &p, &c );

    struct next_address_t read_a, read_b, read_c;

    const uint8_t * q = buffer;

    next_read_address( &q, &read_a );
    next_read_address( &q, &read_b );
    next_read_address( &q, &read_c );

    check( next_address_equal( &a, &read_a ) );
    check( next_address_equal( &b, &read_b ) );
    check( next_address_equal( &c, &read_c ) );
}

static void test_platform_socket()
{
    // non-blocking socket (ipv4)
    {
        next_address_t bind_address;
        next_address_t local_address;
        next_address_parse( &bind_address, "0.0.0.0" );
        next_address_parse( &local_address, "127.0.0.1" );
        next_platform_socket_t * socket = next_platform_socket_create( NULL, &bind_address, NEXT_PLATFORM_SOCKET_NON_BLOCKING, 0, 64*1024, 64*1024 );
        local_address.port = bind_address.port;
        check( socket );
        uint8_t packet[256];
        memset( packet, 0, sizeof(packet) );
        next_platform_socket_send_packet( socket, &local_address, packet, sizeof(packet) );
        next_address_t from;
        while ( next_platform_socket_receive_packet( socket, &from, packet, sizeof(packet) ) )
        {
            check( next_address_equal( &from, &local_address ) );
        }
        next_platform_socket_destroy( socket );
    }

    // blocking socket with timeout (ipv4)
    {
        next_address_t bind_address;
        next_address_t local_address;
        next_address_parse( &bind_address, "0.0.0.0" );
        next_address_parse( &local_address, "127.0.0.1" );
        next_platform_socket_t * socket = next_platform_socket_create( NULL, &bind_address, NEXT_PLATFORM_SOCKET_BLOCKING, 0.01f, 64*1024, 64*1024 );
        local_address.port = bind_address.port;
        check( socket );
        uint8_t packet[256];
        memset( packet, 0, sizeof(packet) );
        next_platform_socket_send_packet( socket, &local_address, packet, sizeof(packet) );
        next_address_t from;
        while ( next_platform_socket_receive_packet( socket, &from, packet, sizeof(packet) ) )
        {
            check( next_address_equal( &from, &local_address ) );
        }
        next_platform_socket_destroy( socket );
    }

    // blocking socket with no timeout (ipv4)
    {
        next_address_t bind_address;
        next_address_t local_address;
        next_address_parse( &bind_address, "0.0.0.0" );
        next_address_parse( &local_address, "127.0.0.1" );
        next_platform_socket_t * socket = next_platform_socket_create( NULL, &bind_address, NEXT_PLATFORM_SOCKET_BLOCKING, -1.0f, 64*1024, 64*1024 );
        local_address.port = bind_address.port;
        check( socket );
        uint8_t packet[256];
        memset( packet, 0, sizeof(packet) );
        next_platform_socket_send_packet( socket, &local_address, packet, sizeof(packet) );
        next_address_t from;
        next_platform_socket_receive_packet( socket, &from, packet, sizeof(packet) );
        check( next_address_equal( &from, &local_address ) );
        next_platform_socket_destroy( socket );
    }

    // non-blocking socket (ipv6)
#if NEXT_PLATFORM_HAS_IPV6
    {
        next_address_t bind_address;
        next_address_t local_address;
        next_address_parse( &bind_address, "[::]" );
        next_address_parse( &local_address, "[::1]" );
        next_platform_socket_t * socket = next_platform_socket_create( NULL, &bind_address, NEXT_PLATFORM_SOCKET_NON_BLOCKING, 0, 64*1024, 64*1024 );
        local_address.port = bind_address.port;
        check( socket );
        uint8_t packet[256];
        memset( packet, 0, sizeof(packet) );
        next_platform_socket_send_packet( socket, &local_address, packet, sizeof(packet) );
        next_address_t from;
        while ( next_platform_socket_receive_packet( socket, &from, packet, sizeof(packet) ) )
        {
            check( next_address_equal( &from, &local_address ) );
        }
        next_platform_socket_destroy( socket );
    }

    // blocking socket with timeout (ipv6)
    {
        next_address_t bind_address;
        next_address_t local_address;
        next_address_parse( &bind_address, "[::]" );
        next_address_parse( &local_address, "[::1]" );
        next_platform_socket_t * socket = next_platform_socket_create( NULL, &bind_address, NEXT_PLATFORM_SOCKET_BLOCKING, 0.01f, 64*1024, 64*1024 );
        local_address.port = bind_address.port;
        check( socket );
        uint8_t packet[256];
        memset( packet, 0, sizeof(packet) );
        next_platform_socket_send_packet( socket, &local_address, packet, sizeof(packet) );
        next_address_t from;
        while ( next_platform_socket_receive_packet( socket, &from, packet, sizeof(packet) ) )
        {
            check( next_address_equal( &from, &local_address ) );
        }
        next_platform_socket_destroy( socket );
    }

    // blocking socket with no timeout (ipv6)
    {
        next_address_t bind_address;
        next_address_t local_address;
        next_address_parse( &bind_address, "[::]" );
        next_address_parse( &local_address, "[::1]" );
        next_platform_socket_t * socket = next_platform_socket_create( NULL, &bind_address, NEXT_PLATFORM_SOCKET_BLOCKING, -1.0f, 64*1024, 64*1024 );
        local_address.port = bind_address.port;
        check( socket );
        uint8_t packet[256];
        memset( packet, 0, sizeof(packet) );
        next_platform_socket_send_packet( socket, &local_address, packet, sizeof(packet) );
        next_address_t from;
        next_platform_socket_receive_packet( socket, &from, packet, sizeof(packet) );
        check( next_address_equal( &from, &local_address ) );
        next_platform_socket_destroy( socket );
    }
#endif
}

static bool threads_work = false;

static next_platform_thread_return_t NEXT_PLATFORM_THREAD_FUNC test_thread_function(void*)
{
    threads_work = true;
    NEXT_PLATFORM_THREAD_RETURN();
}

static void test_platform_thread()
{
    next_platform_thread_t * thread = next_platform_thread_create( NULL, test_thread_function, NULL );
    check( thread );
    next_platform_thread_join( thread );
    next_platform_thread_destroy( thread );
    check( threads_work );
}

static void test_platform_mutex()
{
    next_platform_mutex_t * mutex = next_platform_mutex_create( NULL );
    check( mutex );
    next_platform_mutex_acquire( mutex );
    next_platform_mutex_release( mutex );
    {
        next_mutex_guard( mutex );
        // ...
    }
    next_platform_mutex_destroy( mutex );
}

static int num_client_packets_received = 0;

static void test_client_packet_received_callback( next_client_t * client, void * context, const uint8_t * packet_data, int packet_bytes )
{
    (void) client;
    (void) context;
    (void) packet_data;
    (void) packet_bytes;
    num_client_packets_received++;
}

static void test_client()
{
    next_client_t * client = next_client_create( NULL, test_client_packet_received_callback );
    check( client );
    next_client_open_session( client, "127.0.0.1:12345" );
    uint8_t packet[256];
    memset( packet, 0, sizeof(packet) );
    next_client_send_packet( client, packet, sizeof(packet) );
    next_client_update( client );
    next_client_close_session( client );
    next_client_destroy( client );
}

static int num_server_packets_received = 0;

static void test_server_packet_received_callback( next_server_t * server, void * context, const next_address_t * from, const uint8_t * packet_data, int packet_bytes )
{
    (void) server; (void) context;
    next_server_send_packet( server, from, packet_data, packet_bytes );
    num_server_packets_received++;
}

static void test_server()
{
    next_server_t * server = next_server_create( NULL, "127.0.0.1:0", "0.0.0.0:0", "local", test_server_packet_received_callback );
    check( server );
    next_address_t address;
    next_address_parse( &address, "127.0.0.1" );
    address.port = server->bound_port;
    uint8_t packet[256];
    memset( packet, 0, sizeof(packet) );
    next_server_send_packet( server, &address, packet, sizeof(packet) );
    check( next_server_port( server ) != 0 );
    next_server_update( server );
    next_server_destroy( server );
}

static void test_direct()
{
    next_server_t * server = next_server_create( NULL, "127.0.0.1", "0.0.0.0", "local", test_server_packet_received_callback );
    check( server );
    next_client_t * client = next_client_create( NULL, test_client_packet_received_callback );
    check( client );
    char server_address[NEXT_MAX_ADDRESS_STRING_LENGTH];
    snprintf( server_address, sizeof( server_address ), "127.0.0.1:%hu", server->bound_port );
    next_client_open_session( client, server_address );
    uint8_t packet[256];
    memset( packet, 0, sizeof(packet) );
    num_client_packets_received = 0;
    num_server_packets_received = 0;
    for ( int i = 0; i < 10000; ++i )
    {
        next_client_send_packet( client, packet, sizeof(packet) );
        next_server_update( server );
        next_client_update( client );
        if ( num_client_packets_received > 0 && num_server_packets_received > 0 )
            break;
    }
    check( num_client_packets_received > 0 );
    check( num_server_packets_received > 0 );
    next_client_close_session( client );
    next_client_destroy( client );
    next_server_destroy( server );
}

static void test_upgrade_token()
{
    NextUpgradeToken in, out;

    next_random_bytes( (uint8_t*) &in.session_id, 8 );
    next_random_bytes( (uint8_t*) &in.expire_timestamp, 8 );
    next_address_parse( &in.client_address, "127.0.0.1:40000" );
    next_address_parse( &in.server_address, "127.0.0.1:50000" );

    unsigned char private_key[crypto_secretbox_KEYBYTES];
    crypto_secretbox_keygen( private_key );

    uint8_t buffer[NEXT_UPGRADE_TOKEN_BYTES];

    in.Write( buffer, private_key );
    
    check( out.Read( buffer, private_key ) == true );

    check( memcmp( &in, &out, sizeof(NextUpgradeToken) ) == 0 );
}

static void test_packets()
{
    uint8_t buffer[NEXT_MAX_PACKET_BYTES];

    // upgrade request
    {
        unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
        unsigned char private_key[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair( public_key, private_key );

        NextUpgradeRequestPacket in, out;
        in.protocol_version = next_protocol_version();
        in.session_id = 1231234127431LL;
        next_address_parse( &in.server_address, "127.0.0.1:12345" );
        next_random_bytes( in.server_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        next_random_bytes( in.upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );
        in.Sign( private_key );

        int packet_bytes = 0;
        check( next_write_packet( NEXT_UPGRADE_REQUEST_PACKET, &in, buffer, &packet_bytes, NULL, NULL, NULL ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, NULL, NULL, NULL, NULL ) == NEXT_UPGRADE_REQUEST_PACKET );

        check( in.protocol_version == out.protocol_version );
        check( in.session_id == out.session_id );
        check( next_address_equal( &in.server_address, &out.server_address ) );
        check( memcmp( in.server_kx_public_key, out.server_kx_public_key, crypto_kx_PUBLICKEYBYTES ) == 0 );
        check( memcmp( in.upgrade_token, out.upgrade_token, NEXT_UPGRADE_TOKEN_BYTES ) == 0 );
        check( out.Verify( public_key ) );
    }

    // upgrade response
    {
        NextUpgradeResponsePacket in, out;
        next_random_bytes( in.client_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        next_random_bytes( in.client_route_public_key, crypto_box_PUBLICKEYBYTES );
        next_random_bytes( in.upgrade_token, NEXT_UPGRADE_TOKEN_BYTES );

        int packet_bytes = 0;
        check( next_write_packet( NEXT_UPGRADE_RESPONSE_PACKET, &in, buffer, &packet_bytes, NULL, NULL, NULL ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, NULL, NULL, NULL, NULL ) == NEXT_UPGRADE_RESPONSE_PACKET );

        check( memcmp( in.client_kx_public_key, out.client_kx_public_key, crypto_kx_PUBLICKEYBYTES ) == 0 );
        check( memcmp( in.client_route_public_key, out.client_route_public_key, crypto_box_PUBLICKEYBYTES ) == 0 );
        check( memcmp( in.upgrade_token, out.upgrade_token, NEXT_UPGRADE_TOKEN_BYTES ) == 0 );
    }

    // upgrade confirm
    {
        unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
        unsigned char private_key[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair( public_key, private_key );

        NextUpgradeConfirmPacket in, out;
        in.upgrade_sequence = 1000;
        in.session_id = 1231234127431LL;
        next_address_parse( &in.server_address, "127.0.0.1:12345" );
        next_random_bytes( in.client_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        next_random_bytes( in.server_kx_public_key, crypto_kx_PUBLICKEYBYTES );
        in.Sign( private_key );

        int packet_bytes = 0;
        check( next_write_packet( NEXT_UPGRADE_CONFIRM_PACKET, &in, buffer, &packet_bytes, NULL, NULL, NULL ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, NULL, NULL, NULL, NULL ) == NEXT_UPGRADE_CONFIRM_PACKET );

        check( in.upgrade_sequence == out.upgrade_sequence );
        check( in.session_id == out.session_id );
        check( next_address_equal( &in.server_address, &out.server_address ) );
        check( memcmp( in.client_kx_public_key, out.client_kx_public_key, crypto_kx_PUBLICKEYBYTES ) == 0 );
        check( memcmp( in.server_kx_public_key, out.server_kx_public_key, crypto_kx_PUBLICKEYBYTES ) == 0 );
        check( out.Verify( public_key ) );
    }

    uint8_t private_key[crypto_aead_chacha20poly1305_KEYBYTES];
    next_random_bytes( private_key, sizeof(private_key) );

    // direct ping packet
    {
        NextDirectPingPacket in, out;
        in.ping_sequence = 0xFFFFFFFFFFFFFFFFULL;
        uint64_t in_sequence = 1000;
        uint64_t out_sequence = 0;
        int packet_bytes = 0;
        next_replay_protection_t replay_protection;
        next_replay_protection_reset( &replay_protection );
        check( next_write_packet( NEXT_DIRECT_PING_PACKET, &in, buffer, &packet_bytes, next_encrypted_packets, &in_sequence, private_key ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, next_encrypted_packets, &out_sequence, private_key, &replay_protection ) == NEXT_DIRECT_PING_PACKET );
        check( in_sequence == out_sequence + 1 );
        check( in.ping_sequence == out.ping_sequence );
    }

    // direct pong packet
    {
        NextDirectPongPacket in, out;
        in.ping_sequence = 10000000;
        uint64_t in_sequence = 1000;
        uint64_t out_sequence = 0;
        int packet_bytes = 0;
        next_replay_protection_t replay_protection;
        next_replay_protection_reset( &replay_protection );
        check( next_write_packet( NEXT_DIRECT_PONG_PACKET, &in, buffer, &packet_bytes, next_encrypted_packets, &in_sequence, private_key ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, next_encrypted_packets, &out_sequence, private_key, &replay_protection ) == NEXT_DIRECT_PONG_PACKET );
        check( in_sequence == out_sequence + 1 );
        check( in.ping_sequence == out.ping_sequence );
    }

    // client stats packet
    {
        NextClientStatsPacket in, out;
        in.try_before_you_buy = true;
        in.flags = NEXT_FLAGS_BAD_ROUTE_TOKEN | NEXT_FLAGS_DIRECT_ROUTE_EXPIRED;
        in.flagged = true;
        in.fallback_to_direct = true;
        in.platform_id = NEXT_PLATFORM_WINDOWS | NEXT_PLATFORM_EGS;
        in.connection_type = NEXT_CONNECTION_TYPE_CELLULAR;
        in.direct_min_rtt = 50.0f;
        in.direct_max_rtt = 110.0f;
        in.direct_mean_rtt = 60.0f;
        in.direct_jitter = 10.0f;
        in.direct_packet_loss = 0.1f;
        in.next = true;
        in.next_min_rtt = 50.0f;
        in.next_max_rtt = 100.0f;
        in.next_mean_rtt = 55.0f;
        in.next_jitter = 5.0f;
        in.next_packet_loss = 0.01f;
        in.num_near_relays = NEXT_MAX_NEAR_RELAYS;
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            in.near_relay_ids[i] = 10000000 + i;
            in.near_relay_min_rtt[i] = 5 * i;
            in.near_relay_max_rtt[i] = 5 * i + 50;
            in.near_relay_mean_rtt[i] = 5 * i + 10;
            in.near_relay_jitter[i] = 0.01f * i;
            in.near_relay_packet_loss[i] = i;
        }
        in.packets_lost_server_to_client = 1000;
        uint64_t in_sequence = 1000;
        uint64_t out_sequence = 0;
        int packet_bytes = 0;
        next_replay_protection_t replay_protection;
        next_replay_protection_reset( &replay_protection );
        check( next_write_packet( NEXT_CLIENT_STATS_PACKET, &in, buffer, &packet_bytes, next_encrypted_packets, &in_sequence, private_key ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, next_encrypted_packets, &out_sequence, private_key, &replay_protection ) == NEXT_CLIENT_STATS_PACKET );
        check( in_sequence == out_sequence + 1 );
        check( in.try_before_you_buy == out.try_before_you_buy );
        check( in.flags == out.flags );
        check( in.flagged == out.flagged );
        check( in.fallback_to_direct == out.fallback_to_direct );
        check( in.platform_id == out.platform_id );
        check( in.connection_type == out.connection_type );
        check( in.direct_min_rtt == out.direct_min_rtt );
        check( in.direct_max_rtt == out.direct_max_rtt );
        check( in.direct_mean_rtt == out.direct_mean_rtt );
        check( in.direct_jitter == out.direct_jitter );
        check( in.direct_packet_loss == out.direct_packet_loss );
        check( in.next == out.next );
        check( in.next_min_rtt == out.next_min_rtt );
        check( in.next_max_rtt == out.next_max_rtt );
        check( in.next_mean_rtt == out.next_mean_rtt );
        check( in.next_jitter == out.next_jitter );
        check( in.next_packet_loss == out.next_packet_loss );
        check( in.num_near_relays == out.num_near_relays );
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            check( in.near_relay_ids[i] == out.near_relay_ids[i] );
            check( in.near_relay_min_rtt[i] == out.near_relay_min_rtt[i] );
            check( in.near_relay_max_rtt[i] == out.near_relay_max_rtt[i] );
            check( in.near_relay_mean_rtt[i] == out.near_relay_mean_rtt[i] );
            check( in.near_relay_jitter[i] == out.near_relay_jitter[i] );
            check( in.near_relay_packet_loss[i] == out.near_relay_packet_loss[i] );
        }
        check( in.packets_lost_server_to_client == out.packets_lost_server_to_client );
    }

    // route update packet (direct)
    {
        NextRouteUpdatePacket in, out;
        in.sequence = 100000;
        in.num_near_relays = NEXT_MAX_NEAR_RELAYS;
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            char relay_name[32];
            char relay_address[256];
            sprintf( relay_name, "relay%d", i );
            sprintf( relay_address, "127.0.0.1:%d", 40000 + i );
            in.near_relay_ids[i] = next_relay_id( relay_name );
            next_address_parse( &in.near_relay_addresses[i], relay_address );
        }
        in.update_type = NEXT_UPDATE_TYPE_DIRECT;
        in.packets_lost_client_to_server = 10000;
        uint64_t in_sequence = 1000;
        uint64_t out_sequence = 0;
        int packet_bytes = 0;
        next_replay_protection_t replay_protection;
        next_replay_protection_reset( &replay_protection );
        check( next_write_packet( NEXT_ROUTE_UPDATE_PACKET, &in, buffer, &packet_bytes, next_encrypted_packets, &in_sequence, private_key ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, next_encrypted_packets, &out_sequence, private_key, &replay_protection ) == NEXT_ROUTE_UPDATE_PACKET );
        check( in_sequence == out_sequence + 1 );
        check( in.sequence == out.sequence );
        check( in.num_near_relays == out.num_near_relays );
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            check( in.near_relay_ids[i] == out.near_relay_ids[i] );
            check( next_address_equal( &in.near_relay_addresses[i], &out.near_relay_addresses[i] ) );
        }
        check( in.update_type == out.update_type );
        check( in.packets_lost_client_to_server == out.packets_lost_client_to_server );
    }

    // route update packet (route)
    {
        NextRouteUpdatePacket in, out;
        in.sequence = 100000;
        in.num_near_relays = NEXT_MAX_NEAR_RELAYS;
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            char relay_name[32];
            char relay_address[256];
            sprintf( relay_name, "relay%d", i );
            sprintf( relay_address, "127.0.0.1:%d", 40000 + i );
            in.near_relay_ids[i] = next_relay_id( relay_name );
            next_address_parse( &in.near_relay_addresses[i], relay_address );
        }
        in.update_type = NEXT_UPDATE_TYPE_ROUTE;
        in.num_tokens = NEXT_MAX_TOKENS;
        next_random_bytes( in.tokens, NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES * NEXT_MAX_TOKENS );
        in.packets_lost_client_to_server = 10000;
        uint64_t in_sequence = 1000;
        uint64_t out_sequence = 0;
        int packet_bytes = 0;
        next_replay_protection_t replay_protection;
        next_replay_protection_reset( &replay_protection );
        check( next_write_packet( NEXT_ROUTE_UPDATE_PACKET, &in, buffer, &packet_bytes, next_encrypted_packets, &in_sequence, private_key ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, next_encrypted_packets, &out_sequence, private_key, &replay_protection ) == NEXT_ROUTE_UPDATE_PACKET );
        check( in_sequence == out_sequence + 1 );
        check( in.sequence == out.sequence );
        check( in.num_near_relays == out.num_near_relays );
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            check( in.near_relay_ids[i] == out.near_relay_ids[i] );
            check( next_address_equal( &in.near_relay_addresses[i], &out.near_relay_addresses[i] ) );
        }
        check( in.update_type == out.update_type );
        check( in.num_tokens == out.num_tokens );
        check( memcmp( in.tokens, out.tokens, NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES * NEXT_MAX_TOKENS ) == 0 );
        check( in.packets_lost_client_to_server == out.packets_lost_client_to_server );
    }

    // route update packet (update)
    {
        NextRouteUpdatePacket in, out;
        in.sequence = 100000;
        in.num_near_relays = NEXT_MAX_NEAR_RELAYS;
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            char relay_name[32];
            char relay_address[256];
            sprintf( relay_name, "relay%d", i );
            sprintf( relay_address, "127.0.0.1:%d", 40000 + i );
            in.near_relay_ids[i] = next_relay_id( relay_name );
            next_address_parse( &in.near_relay_addresses[i], relay_address );
        }
        in.update_type = NEXT_UPDATE_TYPE_CONTINUE;
        in.num_tokens = NEXT_MAX_TOKENS;
        next_random_bytes( in.tokens, NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES * NEXT_MAX_TOKENS );
        in.packets_lost_client_to_server = 10000;
        uint64_t in_sequence = 1000;
        uint64_t out_sequence = 0;
        int packet_bytes = 0;
        next_replay_protection_t replay_protection;
        next_replay_protection_reset( &replay_protection );
        check( next_write_packet( NEXT_ROUTE_UPDATE_PACKET, &in, buffer, &packet_bytes, next_encrypted_packets, &in_sequence, private_key ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, next_encrypted_packets, &out_sequence, private_key, &replay_protection ) == NEXT_ROUTE_UPDATE_PACKET );
        check( in_sequence == out_sequence + 1 );
        check( in.sequence == out.sequence );
        check( in.num_near_relays == out.num_near_relays );
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            check( in.near_relay_ids[i] == out.near_relay_ids[i] );
            check( next_address_equal( &in.near_relay_addresses[i], &out.near_relay_addresses[i] ) );
        }
        check( in.update_type == out.update_type );
        check( in.num_tokens == out.num_tokens );
        check( memcmp( in.tokens, out.tokens, NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES * NEXT_MAX_TOKENS ) == 0 );
        check( in.packets_lost_client_to_server == out.packets_lost_client_to_server );
    }

    // route update ack packet
    {
        NextRouteUpdateAckPacket in, out;
        in.sequence = 100000;
        uint64_t in_sequence = 1000;
        uint64_t out_sequence = 0;
        int packet_bytes = 0;
        next_replay_protection_t replay_protection;
        next_replay_protection_reset( &replay_protection );
        check( next_write_packet( NEXT_ROUTE_UPDATE_ACK_PACKET, &in, buffer, &packet_bytes, next_encrypted_packets, &in_sequence, private_key ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, next_encrypted_packets, &out_sequence, private_key, &replay_protection ) == NEXT_ROUTE_UPDATE_ACK_PACKET );
        check( in_sequence == out_sequence + 1 );
        check( in.sequence == out.sequence );
    }

    // relay ping packet
    {
        NextRelayPingPacket in, out;
        in.ping_sequence = 0xFFFFFFFFFFFFFFFFULL;
        in.session_id = 1000000;
        int packet_bytes = 0;
        check( next_write_packet( NEXT_RELAY_PING_PACKET, &in, buffer, &packet_bytes, NULL, NULL, NULL ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, NULL, NULL, NULL, NULL ) == NEXT_RELAY_PING_PACKET );
        check( in.ping_sequence == out.ping_sequence );
        check( in.session_id == out.session_id );
        check( in.padding1 == out.padding1 );
        check( in.padding2 == out.padding2 );
    }

    // relay pong packet
    {
        NextRelayPongPacket in, out;
        in.ping_sequence = 0xFFFFFFFFFFFFFFFFULL;
        in.session_id = 1000000;
        int packet_bytes = 0;
        check( next_write_packet( NEXT_RELAY_PONG_PACKET, &in, buffer, &packet_bytes, NULL, NULL, NULL ) == NEXT_OK );
        check( next_read_packet( buffer, packet_bytes, &out, NULL, NULL, NULL, NULL ) == NEXT_RELAY_PONG_PACKET );
        check( in.ping_sequence == out.ping_sequence );
        check( in.session_id == out.session_id );
    }
}

static void test_pending_session_manager()
{
    const int InitialSize = 32;

    next_pending_session_manager_t * pending_session_manager = next_pending_session_manager_create( NULL, InitialSize );

    check( pending_session_manager );

    next_address_t address;
    next_address_parse( &address, "127.0.0.1:12345" );

    double time = 10.0;

    // test private keys

    uint8_t private_keys[InitialSize*3*crypto_secretbox_KEYBYTES];
    next_random_bytes( private_keys, sizeof(private_keys) );

    // test upgrade tokens 

    uint8_t upgrade_tokens[InitialSize*3*NEXT_UPGRADE_TOKEN_BYTES];
    next_random_bytes( upgrade_tokens, sizeof(upgrade_tokens) );

    // add enough entries to make sure we have to expand

    for ( int i = 0; i < InitialSize*3; ++i )
    {
        next_pending_session_entry_t * entry = next_pending_session_manager_add( pending_session_manager, &address, i+1000, &private_keys[i*crypto_secretbox_KEYBYTES], &upgrade_tokens[i*NEXT_UPGRADE_TOKEN_BYTES], time );
        check( entry );
        check( entry->session_id == uint64_t(i) + 1000 );
        check( entry->upgrade_time == time );
        check( entry->last_packet_send_time < 0.0 );
        check( next_address_equal( &address, &entry->address ) == 1 );        
        check( memcmp( entry->private_key, &private_keys[i*crypto_secretbox_KEYBYTES], crypto_secretbox_KEYBYTES ) == 0 );
        check( memcmp( entry->upgrade_token, &upgrade_tokens[i*NEXT_UPGRADE_TOKEN_BYTES], NEXT_UPGRADE_TOKEN_BYTES ) == 0 );
        address.port++;
    }

    // verify that all entries are there

    address.port = 12345;
    for ( int i = 0; i < InitialSize*3; ++i )
    {
        next_pending_session_entry_t * entry = next_pending_session_manager_find( pending_session_manager, &address );
        check( entry );
        check( entry->session_id == uint64_t(i) + 1000 );
        check( entry->upgrade_time == time );
        check( entry->last_packet_send_time < 0.0 );
        check( next_address_equal( &address, &entry->address ) == 1 );        
        address.port++;
    }

    check( next_pending_session_manager_num_entries( pending_session_manager ) == InitialSize*3 );

    // remove every second entry

    for ( int i = 0; i < InitialSize*3; ++i )
    {
        if ( (i%2) == 0 )
        {
            next_pending_session_manager_remove_by_address( pending_session_manager, &pending_session_manager->addresses[i] );
        }
    }

    // verify only the entries that remain can be found

    address.port = 12345;
    for ( int i = 0; i < InitialSize*3; ++i )
    {
        next_pending_session_entry_t * entry = next_pending_session_manager_find( pending_session_manager, &address );
        if ( (i%2) != 0 )
        {
            check( entry );
            check( entry->session_id == uint64_t(i) + 1000 );
            check( entry->upgrade_time == time );
            check( entry->last_packet_send_time < 0.0 );
            check( next_address_equal( &address, &entry->address ) == 1 );
        }
        else
        {
            check( entry == NULL );
        }
        address.port++;
    }

    // expand, and verify that all entries get collapsed

    next_pending_session_manager_expand( pending_session_manager );

    address.port = 12346;
    for ( int i = 0; i < pending_session_manager->size; ++i )
    {
        if ( pending_session_manager->addresses[i].type != NEXT_ADDRESS_NONE )
        {
            check( next_address_equal( &address, &pending_session_manager->addresses[i] ) == 1 );
            next_pending_session_entry_t * entry = &pending_session_manager->entries[i];
            check( entry->session_id == uint64_t(i*2)+1001 );
            check( entry->upgrade_time == time );
            check( entry->last_packet_send_time < 0.0 );
            check( next_address_equal( &address, &entry->address ) == 1 );
        }
        address.port += 2;
    }

    // remove all remaining entries manually

    for ( int i = 0; i < pending_session_manager->size; ++i )
    {
        if ( pending_session_manager->addresses[i].type != NEXT_ADDRESS_NONE )
        {
            next_pending_session_manager_remove_by_address( pending_session_manager, &pending_session_manager->addresses[i] );
        }
    }

    check( pending_session_manager->max_entry_index == 0 );

    check( next_pending_session_manager_num_entries( pending_session_manager ) == 0 );

    next_pending_session_manager_destroy( pending_session_manager );
}

static void test_proxy_session_manager()
{
    const int InitialSize = 32;

    next_proxy_session_manager_t * proxy_session_manager = next_proxy_session_manager_create( NULL, InitialSize );

    check( proxy_session_manager );

    next_address_t address;
    next_address_parse( &address, "127.0.0.1:12345" );

    // test private keys

    uint8_t private_keys[InitialSize*3*crypto_secretbox_KEYBYTES];
    next_random_bytes( private_keys, sizeof(private_keys) );

    // test upgrade tokens 

    uint8_t upgrade_tokens[InitialSize*3*NEXT_UPGRADE_TOKEN_BYTES];
    next_random_bytes( upgrade_tokens, sizeof(upgrade_tokens) );

    // add enough entries to make sure we have to expand

    for ( int i = 0; i < InitialSize*3; ++i )
    {
        next_proxy_session_entry_t * entry = next_proxy_session_manager_add( proxy_session_manager, &address, i+1000 );
        check( entry );
        check( entry->session_id == uint64_t(i) + 1000 );
        check( next_address_equal( &address, &entry->address ) == 1 );        
        address.port++;
    }

    // verify that all entries are there

    address.port = 12345;
    for ( int i = 0; i < InitialSize*3; ++i )
    {
        next_proxy_session_entry_t * entry = next_proxy_session_manager_find( proxy_session_manager, &address );
        check( entry );
        check( entry->session_id == uint64_t(i) + 1000 );
        check( next_address_equal( &address, &entry->address ) == 1 );        
        address.port++;
    }

    check( next_proxy_session_manager_num_entries( proxy_session_manager ) == InitialSize*3 );

    // remove every second entry

    for ( int i = 0; i < InitialSize*3; ++i )
    {
        if ( (i%2) == 0 )
        {
            next_proxy_session_manager_remove_by_address( proxy_session_manager, &proxy_session_manager->addresses[i] );
        }
    }

    // verify only the entries that remain can be found

    address.port = 12345;
    for ( int i = 0; i < InitialSize*3; ++i )
    {
        next_proxy_session_entry_t * entry = next_proxy_session_manager_find( proxy_session_manager, &address );
        if ( (i%2) != 0 )
        {
            check( entry );
            check( entry->session_id == uint64_t(i) + 1000 );
            check( next_address_equal( &address, &entry->address ) == 1 );
        }
        else
        {
            check( entry == NULL );
        }
        address.port++;
    }

    // expand, and verify that all entries get collapsed

    next_proxy_session_manager_expand( proxy_session_manager );

    address.port = 12346;
    for ( int i = 0; i < proxy_session_manager->size; ++i )
    {
        if ( proxy_session_manager->addresses[i].type != NEXT_ADDRESS_NONE )
        {
            check( next_address_equal( &address, &proxy_session_manager->addresses[i] ) == 1 );
            next_proxy_session_entry_t * entry = &proxy_session_manager->entries[i];
            check( entry->session_id == uint64_t(i*2)+1001 );
            check( next_address_equal( &address, &entry->address ) == 1 );
        }
        address.port += 2;
    }

    // remove all remaining entries manually

    for ( int i = 0; i < proxy_session_manager->size; ++i )
    {
        if ( proxy_session_manager->addresses[i].type != NEXT_ADDRESS_NONE )
        {
            next_proxy_session_manager_remove_by_address( proxy_session_manager, &proxy_session_manager->addresses[i] );
        }
    }

    check( proxy_session_manager->max_entry_index == 0 );

    check( next_proxy_session_manager_num_entries( proxy_session_manager ) == 0 );

    next_proxy_session_manager_destroy( proxy_session_manager );
}

static void test_session_manager()
{
    const int InitialSize = 32;

    next_session_manager_t * session_manager = next_session_manager_create( NULL, InitialSize );

    check( session_manager );

    next_address_t address;
    next_address_parse( &address, "127.0.0.1:12345" );

    // test private keys

    uint8_t private_keys[InitialSize*3*crypto_secretbox_KEYBYTES];
    next_random_bytes( private_keys, sizeof(private_keys) );

    // test upgrade tokens 

    uint8_t upgrade_tokens[InitialSize*3*NEXT_UPGRADE_TOKEN_BYTES];
    next_random_bytes( upgrade_tokens, sizeof(upgrade_tokens) );

    // add enough entries to make sure we have to expand

    for ( int i = 0; i < InitialSize*3; ++i )
    {
        next_session_entry_t * entry = next_session_manager_add( session_manager, &address, i+1000, &private_keys[i*crypto_secretbox_KEYBYTES], &upgrade_tokens[i*NEXT_UPGRADE_TOKEN_BYTES] );
        check( entry );
        check( entry->session_id == uint64_t(i) + 1000 );
        check( next_address_equal( &address, &entry->address ) == 1 );
        check( memcmp( entry->ephemeral_private_key, &private_keys[i*crypto_secretbox_KEYBYTES], crypto_secretbox_KEYBYTES ) == 0 );
        check( memcmp( entry->upgrade_token, &upgrade_tokens[i*NEXT_UPGRADE_TOKEN_BYTES], NEXT_UPGRADE_TOKEN_BYTES ) == 0 );
        address.port++;
    }

    // verify that all entries are there

    address.port = 12345;
    for ( int i = 0; i < InitialSize*3; ++i )
    {
        next_session_entry_t * entry = next_session_manager_find_by_address( session_manager, &address );
        check( entry );
        check( entry->session_id == uint64_t(i)+1000 );
        check( next_address_equal( &address, &entry->address ) == 1 );        
        address.port++;
    }

    check( next_session_manager_num_entries( session_manager ) == InitialSize*3 );

    // remove every second entry

    for ( int i = 0; i < InitialSize*3; ++i )
    {
        if ( (i%2) == 0 )
        {
            next_session_manager_remove_by_address( session_manager, &session_manager->addresses[i] );
        }
    }

    // verify only the entries that remain can be found

    address.port = 12345;
    for ( int i = 0; i < InitialSize*3; ++i )
    {
        next_session_entry_t * entry = next_session_manager_find_by_address( session_manager, &address );
        if ( (i%2) != 0 )
        {
            check( entry );
            check( entry->session_id == uint64_t(i)+1000 );
            check( next_address_equal( &address, &entry->address ) == 1 );
        }
        else
        {
            check( entry == NULL );
        }
        address.port++;
    }

    // expand, and verify that all entries get collapsed

    next_session_manager_expand( session_manager );

    address.port = 12346;
    for ( int i = 0; i < session_manager->size; ++i )
    {
        if ( session_manager->addresses[i].type != NEXT_ADDRESS_NONE )
        {
            check( next_address_equal( &address, &session_manager->addresses[i] ) == 1 );
            next_session_entry_t * entry = &session_manager->entries[i];
            check( entry->session_id == uint64_t(i*2)+1001 );
            check( next_address_equal( &address, &entry->address ) == 1 );
        }
        address.port += 2;
    }

    // remove all remaining entries manually

    for ( int i = 0; i < session_manager->size; ++i )
    {
        if ( session_manager->addresses[i].type != NEXT_ADDRESS_NONE )
        {
            next_session_manager_remove_by_address( session_manager, &session_manager->addresses[i] );
        }
    }

    check( session_manager->max_entry_index == 0 );

    check( next_session_manager_num_entries( session_manager ) == 0 );

    next_session_manager_destroy( session_manager );
}

static void test_backend_packets()
{
    uint8_t buffer[NEXT_MAX_PACKET_BYTES];

    // server update
    {
        unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
        unsigned char private_key[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair( public_key, private_key );

        NextBackendServerUpdatePacket in, out;
        in.sequence = 10000;
        in.customer_id = 1231234127431LL;
        in.datacenter_id = next_datacenter_id( "local" );
        in.num_sessions_pending = 10;
        in.num_sessions_upgraded = 20;
        next_address_parse( &in.server_address, "127.0.0.1:12345" );
        next_address_parse( &in.server_address_internal, "127.0.0.1:20000" );
        next_random_bytes( in.server_route_public_key, crypto_box_PUBLICKEYBYTES );
        in.Sign( private_key );

        int packet_bytes = 0;
        check( next_write_backend_packet( NEXT_BACKEND_SERVER_UPDATE_PACKET, &in, buffer, &packet_bytes ) == NEXT_OK );
        check( next_read_backend_packet( buffer, packet_bytes, &out ) == NEXT_BACKEND_SERVER_UPDATE_PACKET );

        check( in.sequence == out.sequence );
        check( in.customer_id == out.customer_id );
        check( in.datacenter_id == out.datacenter_id );
        check( in.num_sessions_pending == out.num_sessions_pending );
        check( in.num_sessions_upgraded == out.num_sessions_upgraded );
        check( next_address_equal( &in.server_address, &out.server_address ) );
        check( next_address_equal( &in.server_address_internal, &out.server_address_internal ) );
        check( memcmp( in.server_route_public_key, out.server_route_public_key, crypto_box_PUBLICKEYBYTES ) == 0 );
        check( out.Verify( public_key ) );
    }

    // session update
    {
        unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
        unsigned char private_key[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair( public_key, private_key );

        NextBackendSessionUpdatePacket in, out;
        in.sequence = 10000;
        in.customer_id = 1231234127431LL;
        in.session_id = 1234342431431LL;
        in.user_hash = 11111111;
        in.platform_id = 3;
        in.tag = 0x1231314141;
        in.flags = NEXT_FLAGS_BAD_ROUTE_TOKEN | NEXT_FLAGS_ROUTE_REQUEST_TIMED_OUT | NEXT_FLAGS_DIRECT_ROUTE_EXPIRED;
        in.flagged = true;
        in.fallback_to_direct = true;
        in.try_before_you_buy = true;
        in.connection_type = NEXT_CONNECTION_TYPE_WIRED;
        in.direct_min_rtt = 10.1f;
        in.direct_max_rtt = 100.1f;
        in.direct_mean_rtt = 20.1f;
        in.direct_jitter = 5.2f;
        in.direct_packet_loss = 0.1f;
        in.next = true;
        in.next_min_rtt = 5.0f;
        in.next_max_rtt = 20.0f;
        in.next_mean_rtt = 10.0f;
        in.next_jitter = 1.5f;
        in.next_packet_loss = 0.0f;
        in.num_near_relays = NEXT_MAX_NEAR_RELAYS;
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            in.near_relay_ids[i] = i;
            in.near_relay_min_rtt[i] = i + 10.0f;
            in.near_relay_max_rtt[i] = i + 100.0f;
            in.near_relay_mean_rtt[i] = i + 20.0f;
            in.near_relay_jitter[i] = i + 11.0f;
            in.near_relay_packet_loss[i] = i + 12.0f;
        }
        next_address_parse( &in.client_address, "127.0.0.1:40000" );
        next_address_parse( &in.server_address, "127.0.0.1:12345" );
        next_random_bytes( in.client_route_public_key, crypto_box_PUBLICKEYBYTES );
        in.Sign( private_key );

        int packet_bytes = 0;
        check( next_write_backend_packet( NEXT_BACKEND_SESSION_UPDATE_PACKET, &in, buffer, &packet_bytes ) == NEXT_OK );
        check( next_read_backend_packet( buffer, packet_bytes, &out ) == NEXT_BACKEND_SESSION_UPDATE_PACKET );

        check( in.sequence == out.sequence );
        check( in.customer_id == out.customer_id );
        check( in.session_id == out.session_id );
        check( in.user_hash == out.user_hash );
        check( in.platform_id == out.platform_id );
        check( in.tag == out.tag );
        check( in.flags == out.flags );
        check( in.flagged == out.flagged );
        check( in.fallback_to_direct == out.fallback_to_direct );
        check( in.try_before_you_buy == out.try_before_you_buy );
        check( in.connection_type == out.connection_type );
        check( in.direct_min_rtt == out.direct_min_rtt );
        check( in.direct_max_rtt == out.direct_max_rtt );
        check( in.direct_mean_rtt == out.direct_mean_rtt );
        check( in.direct_jitter == out.direct_jitter );
        check( in.direct_packet_loss == out.direct_packet_loss );
        check( in.next == out.next );
        check( in.next_min_rtt == out.next_min_rtt );
        check( in.next_max_rtt == out.next_max_rtt );
        check( in.next_mean_rtt == out.next_mean_rtt );
        check( in.next_jitter == out.next_jitter );
        check( in.next_packet_loss == out.next_packet_loss );
        check( in.num_near_relays == out.num_near_relays );
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            check( in.near_relay_ids[i] == out.near_relay_ids[i] );
            check( in.near_relay_min_rtt[i] == out.near_relay_min_rtt[i] );
            check( in.near_relay_max_rtt[i] == out.near_relay_max_rtt[i] );
            check( in.near_relay_mean_rtt[i] == out.near_relay_mean_rtt[i] );
            check( in.near_relay_jitter[i] == out.near_relay_jitter[i] );
            check( in.near_relay_packet_loss[i] == out.near_relay_packet_loss[i] );
        }
        check( next_address_equal( &in.client_address, &out.client_address ) );
        check( next_address_equal( &in.server_address, &out.server_address ) );
        check( memcmp( in.client_route_public_key, out.client_route_public_key, crypto_box_PUBLICKEYBYTES ) == 0 );
        check( out.Verify( public_key ) );
    }

    // session response packet (direct)
    {
        unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
        unsigned char private_key[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair( public_key, private_key );

        NextBackendSessionResponsePacket in, out;
        in.sequence = 10000;
        in.session_id = 1234342431431LL;
        in.num_near_relays = NEXT_MAX_NEAR_RELAYS;
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            char relay_name[32];
            char relay_address[256];
            sprintf( relay_name, "relay%d", i );
            sprintf( relay_address, "127.0.0.1:%d", 40000 + i );
            in.near_relay_ids[i] = next_relay_id( relay_name );
            next_address_parse( &in.near_relay_addresses[i], relay_address );
        }
        in.response_type = NEXT_UPDATE_TYPE_DIRECT;
        next_random_bytes( in.server_route_public_key, sizeof(in.server_route_public_key) );
        in.Sign( private_key );

        int packet_bytes = 0;
        check( next_write_backend_packet( NEXT_BACKEND_SESSION_RESPONSE_PACKET, &in, buffer, &packet_bytes ) == NEXT_OK );
        check( next_read_backend_packet( buffer, packet_bytes, &out ) == NEXT_BACKEND_SESSION_RESPONSE_PACKET );

        check( in.sequence == out.sequence );
        check( in.session_id == out.session_id );
        check( in.num_near_relays == out.num_near_relays );
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            check( in.near_relay_ids[i] == out.near_relay_ids[i] );
            check( next_address_equal( &in.near_relay_addresses[i], &out.near_relay_addresses[i] ) );
        }
        check( in.response_type == out.response_type );
        check( memcmp( in.server_route_public_key, out.server_route_public_key, sizeof(in.server_route_public_key) ) == 0 );
        check( out.Verify( public_key ) );
    }

    // session response packet (route)
    {
        unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
        unsigned char private_key[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair( public_key, private_key );

        NextBackendSessionResponsePacket in, out;
        in.sequence = 10000;
        in.session_id = 1234342431431LL;
        in.num_near_relays = NEXT_MAX_NEAR_RELAYS;
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            char relay_name[32];
            char relay_address[256];
            sprintf( relay_name, "relay%d", i );
            sprintf( relay_address, "127.0.0.1:%d", 40000 + i );
            in.near_relay_ids[i] = next_relay_id( relay_name );
            next_address_parse( &in.near_relay_addresses[i], relay_address );
        }
        in.response_type = NEXT_UPDATE_TYPE_ROUTE;
        in.multipath = true;
        in.num_tokens = NEXT_MAX_TOKENS;
        next_random_bytes( in.tokens, NEXT_MAX_TOKENS * NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES );
        next_random_bytes( in.server_route_public_key, sizeof(in.server_route_public_key) );
        in.Sign( private_key );

        int packet_bytes = 0;
        check( next_write_backend_packet( NEXT_BACKEND_SESSION_RESPONSE_PACKET, &in, buffer, &packet_bytes ) == NEXT_OK );
        check( next_read_backend_packet( buffer, packet_bytes, &out ) == NEXT_BACKEND_SESSION_RESPONSE_PACKET );

        check( in.sequence == out.sequence );
        check( in.session_id == out.session_id );
        check( in.num_near_relays == out.num_near_relays );
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            check( in.near_relay_ids[i] == out.near_relay_ids[i] );
            check( next_address_equal( &in.near_relay_addresses[i], &out.near_relay_addresses[i] ) );
        }
        check( in.response_type == out.response_type );
        check( in.multipath == out.multipath );
        check( in.num_tokens == out.num_tokens );
        check( memcmp( in.tokens, out.tokens, NEXT_MAX_TOKENS * NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES ) == 0 );
        check( memcmp( in.server_route_public_key, out.server_route_public_key, sizeof(in.server_route_public_key) ) == 0 );
        check( out.Verify( public_key ) );
    }

    // session response packet (continue)
    {
        unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
        unsigned char private_key[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair( public_key, private_key );

        NextBackendSessionResponsePacket in, out;
        in.sequence = 10000;
        in.session_id = 1234342431431LL;
        in.num_near_relays = NEXT_MAX_NEAR_RELAYS;
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            char relay_name[32];
            char relay_address[256];
            sprintf( relay_name, "relay%d", i );
            sprintf( relay_address, "127.0.0.1:%d", 40000 + i );
            in.near_relay_ids[i] = next_relay_id( relay_name );
            next_address_parse( &in.near_relay_addresses[i], relay_address );
        }
        in.response_type = NEXT_UPDATE_TYPE_CONTINUE;
        in.multipath = true;
        in.num_tokens = NEXT_MAX_TOKENS;
        next_random_bytes( in.tokens, NEXT_MAX_TOKENS * NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES );
        next_random_bytes( in.server_route_public_key, sizeof(in.server_route_public_key) );
        in.Sign( private_key );

        int packet_bytes = 0;
        check( next_write_backend_packet( NEXT_BACKEND_SESSION_RESPONSE_PACKET, &in, buffer, &packet_bytes ) == NEXT_OK );
        check( next_read_backend_packet( buffer, packet_bytes, &out ) == NEXT_BACKEND_SESSION_RESPONSE_PACKET );

        check( in.sequence == out.sequence );
        check( in.session_id == out.session_id );
        check( in.multipath == out.multipath );
        check( in.num_near_relays == out.num_near_relays );
        for ( int i = 0; i < NEXT_MAX_NEAR_RELAYS; ++i )
        {
            check( in.near_relay_ids[i] == out.near_relay_ids[i] );
            check( next_address_equal( &in.near_relay_addresses[i], &out.near_relay_addresses[i] ) );
        }
        check( in.response_type == out.response_type );
        check( in.num_tokens == out.num_tokens );
        check( memcmp( in.tokens, out.tokens, NEXT_MAX_TOKENS * NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES ) == 0 );
        check( memcmp( in.server_route_public_key, out.server_route_public_key, sizeof(in.server_route_public_key) ) == 0 );
        check( out.Verify( public_key ) );
    }
}

static void test_relay_manager()
{
    const int MaxRelays = 64;
    const int NumRelays = 32;

    uint64_t relay_ids[MaxRelays];
    next_address_t relay_addresses[MaxRelays];

    for ( int i = 0; i < MaxRelays; ++i )
    {
        relay_ids[i] = i;
        char address_string[256];
        sprintf( address_string, "127.0.0.1:%d", 40000 + i );
        next_address_parse( &relay_addresses[i], address_string );
    }

    next_relay_manager_t * manager = next_relay_manager_create( NULL, NULL );

    // should be no relays when manager is first created
    {
        next_relay_stats_t stats;
        next_relay_manager_get_stats( manager, &stats );
        check( stats.num_relays == 0 );
    }

    // add max relays
    
    next_relay_manager_update( manager, NumRelays, relay_ids, relay_addresses );
    {
        next_relay_stats_t stats;
        next_relay_manager_get_stats( manager, &stats );
        check( stats.num_relays == NumRelays );
        for ( int i = 0; i < NumRelays; ++i )
        {
            check( relay_ids[i] == stats.relay_ids[i] );
        }
    }

    // remove all relays

    next_relay_manager_update( manager, 0, relay_ids, relay_addresses );
    {
        next_relay_stats_t stats;
        next_relay_manager_get_stats( manager, &stats );
        check( stats.num_relays == 0 );
    }

    // add same relay set repeatedly

    for ( int j = 0; j < 2; ++j )
    {
        next_relay_manager_update( manager, NumRelays, relay_ids, relay_addresses );
        {
            next_relay_stats_t stats;
            next_relay_manager_get_stats( manager, &stats );
            check( stats.num_relays == NumRelays );
            for ( int i = 0; i < NumRelays; ++i )
            {
                check( relay_ids[i] == stats.relay_ids[i] );
            }
        }
    }
    
    // now add a few new relays, while some relays remain the same

    next_relay_manager_update( manager, NumRelays, relay_ids + 4, relay_addresses + 4 );
    {
        next_relay_stats_t stats;
        next_relay_manager_get_stats( manager, &stats );
        check( stats.num_relays == NumRelays );
        for ( int i = 0; i < NumRelays - 4; ++i )
        {
            check( relay_ids[i+4] == stats.relay_ids[i] );
        }
    }

    next_relay_manager_destroy( manager );
}

static void test_route_token()
{
    uint8_t buffer[NEXT_ENCRYPTED_ROUTE_TOKEN_BYTES];

    next_route_token_t input_token;
    memset( &input_token, 0, sizeof( input_token ) );

    input_token.expire_timestamp = 1234241431241LL;
    input_token.session_id = 1234241431241LL;
    input_token.session_version = 5;
    input_token.session_flags = 1;
    input_token.next_address.type = NEXT_ADDRESS_IPV4;
    input_token.next_address.data.ipv4[0] = 127;
    input_token.next_address.data.ipv4[1] = 0;
    input_token.next_address.data.ipv4[2] = 0;
    input_token.next_address.data.ipv4[3] = 1;
    input_token.next_address.port = 40000;

    next_write_route_token( &input_token, buffer, NEXT_ROUTE_TOKEN_BYTES );

    unsigned char sender_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char sender_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair( sender_public_key, sender_private_key );

    unsigned char receiver_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char receiver_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair( receiver_public_key, receiver_private_key );

    unsigned char nonce[crypto_box_NONCEBYTES];
    next_random_bytes( nonce, crypto_box_NONCEBYTES );

    check( next_encrypt_route_token( sender_private_key, receiver_public_key, nonce, buffer, sizeof( buffer ) ) == NEXT_OK );

    check( next_decrypt_route_token( sender_public_key, receiver_private_key, nonce, buffer ) == NEXT_OK );

    next_route_token_t output_token;

    next_read_route_token( &output_token, buffer );

    check( input_token.expire_timestamp == output_token.expire_timestamp );
    check( input_token.session_id == output_token.session_id );
    check( input_token.session_version == output_token.session_version );
    check( input_token.session_flags == output_token.session_flags );
    check( input_token.kbps_up == output_token.kbps_up );
    check( input_token.kbps_down == output_token.kbps_down );
    check( memcmp( input_token.private_key, output_token.private_key, crypto_box_SECRETKEYBYTES ) == 0 );
    check( next_address_equal( &input_token.next_address, &output_token.next_address ) == 1 );

    uint8_t * p = buffer;

    check( next_write_encrypted_route_token( &p, &input_token, sender_private_key, receiver_public_key ) == NEXT_OK );

    p = buffer;

    check( next_read_encrypted_route_token( &p, &output_token, sender_public_key, receiver_private_key ) == NEXT_OK );

    check( input_token.expire_timestamp == output_token.expire_timestamp );
    check( input_token.session_id == output_token.session_id );
    check( input_token.session_version == output_token.session_version );
    check( input_token.session_flags == output_token.session_flags );
    check( input_token.kbps_up == output_token.kbps_up );
    check( input_token.kbps_down == output_token.kbps_down );
    check( memcmp( input_token.private_key, output_token.private_key, crypto_box_SECRETKEYBYTES ) == 0 );
    check( next_address_equal( &input_token.next_address, &output_token.next_address ) == 1 );
}

static void test_continue_token()
{
    uint8_t buffer[NEXT_ENCRYPTED_CONTINUE_TOKEN_BYTES];

    next_continue_token_t input_token;
    memset( &input_token, 0, sizeof( input_token ) );

    input_token.expire_timestamp = 1234241431241LL;
    input_token.session_id = 1234241431241LL;
    input_token.session_version = 5;
    input_token.session_flags = 1;

    next_write_continue_token( &input_token, buffer, NEXT_CONTINUE_TOKEN_BYTES );

    unsigned char sender_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char sender_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair( sender_public_key, sender_private_key );

    unsigned char receiver_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char receiver_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair( receiver_public_key, receiver_private_key );

    unsigned char nonce[crypto_box_NONCEBYTES];
    next_random_bytes( nonce, crypto_box_NONCEBYTES );

    check( next_encrypt_continue_token( sender_private_key, receiver_public_key, nonce, buffer, sizeof( buffer ) ) == NEXT_OK );

    check( next_decrypt_continue_token( sender_public_key, receiver_private_key, nonce, buffer ) == NEXT_OK );

    next_continue_token_t output_token;

    next_read_continue_token( &output_token, buffer );

    check( input_token.expire_timestamp == output_token.expire_timestamp );
    check( input_token.session_id == output_token.session_id );
    check( input_token.session_version == output_token.session_version );
    check( input_token.session_flags == output_token.session_flags );

    uint8_t * p = buffer;

    check( next_write_encrypted_continue_token( &p, &input_token, sender_private_key, receiver_public_key ) == NEXT_OK );

    p = buffer;

    memset( &output_token, 0, sizeof(output_token) );

    check( next_read_encrypted_continue_token( &p, &output_token, sender_public_key, receiver_private_key ) == NEXT_OK );

    check( input_token.expire_timestamp == output_token.expire_timestamp );
    check( input_token.session_id == output_token.session_id );
    check( input_token.session_flags == output_token.session_flags );
}

static void test_header()
{
    uint8_t private_key[crypto_box_SECRETKEYBYTES];

    next_random_bytes( private_key, crypto_box_SECRETKEYBYTES );

    uint8_t buffer[NEXT_MTU];

    // client -> server
    {
        uint64_t sequence = 123123130131LL;
        uint64_t session_id = 0x12313131;
        uint8_t session_version = 0x12;
        uint8_t session_flags = 0x1;

        check( next_write_header( NEXT_DIRECTION_CLIENT_TO_SERVER, NEXT_CLIENT_TO_SERVER_PACKET, sequence, session_id, session_version, session_flags, private_key, buffer, sizeof( buffer ) ) == NEXT_OK );

        uint8_t read_type = 0;
        uint64_t read_sequence = 0;
        uint64_t read_session_id = 0;
        uint8_t read_session_version = 0;
        uint8_t read_session_flags = 0;

        check( next_peek_header( NEXT_DIRECTION_CLIENT_TO_SERVER, &read_type, &read_sequence, &read_session_id, &read_session_version, &read_session_flags, buffer, sizeof( buffer ) ) == NEXT_OK );

        check( read_type == NEXT_CLIENT_TO_SERVER_PACKET );
        check( read_sequence == sequence );
        check( read_session_id == session_id );
        check( read_session_version == session_version );
        check( read_session_flags == session_flags );

        read_type = 0;
        read_sequence = 0;
        read_session_id = 0;
        read_session_version = 0;
        read_session_flags = 0;

        check( next_read_header( NEXT_DIRECTION_CLIENT_TO_SERVER, &read_type, &read_sequence, &read_session_id, &read_session_version, &read_session_flags, private_key, buffer, sizeof( buffer ) ) == NEXT_OK );

        check( read_type == NEXT_CLIENT_TO_SERVER_PACKET );
        check( read_sequence == sequence );
        check( read_session_id == session_id );
        check( read_session_version == session_version );
        check( read_session_flags == session_flags );
    }

    // server -> client
    {
        uint64_t sequence = 123123130131LL | ( 1ULL << 63 );
        uint64_t session_id = 0x12313131;
        uint8_t session_version = 0x12;
        uint8_t session_flags = 0x1;

        check( next_write_header( NEXT_DIRECTION_SERVER_TO_CLIENT, NEXT_SERVER_TO_CLIENT_PACKET, sequence, session_id, session_version, session_flags, private_key, buffer, sizeof( buffer ) ) == NEXT_OK );

        uint8_t read_type = 0;
        uint64_t read_sequence = 0;
        uint64_t read_session_id = 0;
        uint8_t read_session_version = 0;
        uint8_t read_session_flags = 0;

        check( next_peek_header( NEXT_DIRECTION_SERVER_TO_CLIENT, &read_type, &read_sequence, &read_session_id, &read_session_version, &read_session_flags, buffer, sizeof( buffer ) ) == NEXT_OK );

        check( read_type == NEXT_SERVER_TO_CLIENT_PACKET );
        check( read_sequence == sequence );
        check( read_session_id == session_id );
        check( read_session_version == session_version );
        check( read_session_flags == session_flags );

        read_type = 0;
        read_sequence = 0;
        read_session_id = 0;
        read_session_version = 0;
        read_session_flags = 0;

        check( next_read_header( NEXT_DIRECTION_SERVER_TO_CLIENT, &read_type, &read_sequence, &read_session_id, &read_session_version, &read_session_flags, private_key, buffer, sizeof( buffer ) ) == NEXT_OK );

        check( read_type == NEXT_SERVER_TO_CLIENT_PACKET );
        check( read_sequence == sequence );
        check( read_session_id == session_id );
        check( read_session_version == session_version );
        check( read_session_flags == session_flags );
    }
}

static void test_tag()
{
    check( next_tag_id( NULL ) == 0 );
    check( next_tag_id( "" ) == 0 );
    check( next_tag_id( "none" ) == 0 );
    check( next_tag_id( "default" ) == 0 );
}

static void test_bandwidth_limiter()
{
    next_bandwidth_limiter_t bandwidth_limiter;

    next_bandwidth_limiter_reset( &bandwidth_limiter );

    check( next_bandwidth_limiter_usage_kbps( &bandwidth_limiter, 0.0 ) == 0.0 );

    // come in way under
    {
        const int kbps_allowed = 1000;
        const int packet_bits = 50;

        for ( int i = 0; i < 10; ++i )
        {
            check( !next_bandwidth_limiter_add_packet( &bandwidth_limiter, i * ( NEXT_BANDWIDTH_LIMITER_INTERVAL / 10.0 ), kbps_allowed, packet_bits ) );
        }
    }

    // get really close
    {
        next_bandwidth_limiter_reset( &bandwidth_limiter );        

        const int kbps_allowed = 1000;
        const int packet_bits = kbps_allowed / 10 * 1000;

        for ( int i = 0; i < 10; ++i )
        {
            check( !next_bandwidth_limiter_add_packet( &bandwidth_limiter, i * ( NEXT_BANDWIDTH_LIMITER_INTERVAL / 10.0 ), kbps_allowed, packet_bits ) );
        }
    }    

    // really close for several intervals
    {
        next_bandwidth_limiter_reset( &bandwidth_limiter );        

        const int kbps_allowed = 1000;
        const int packet_bits = kbps_allowed / 10 * 1000;

        for ( int i = 0; i < 30; ++i )
        {
            check( !next_bandwidth_limiter_add_packet( &bandwidth_limiter, i * ( NEXT_BANDWIDTH_LIMITER_INTERVAL / 10.0 ), kbps_allowed, packet_bits ) );
        }
    } 

    // go over budget
    {
        next_bandwidth_limiter_reset( &bandwidth_limiter );        

        const int kbps_allowed = 1000;
        const int packet_bits = kbps_allowed / 10 * 1000 * 1.01f;

        bool over_budget = false;

        for ( int i = 0; i < 30; ++i )
        {
            over_budget |= next_bandwidth_limiter_add_packet( &bandwidth_limiter, i * ( NEXT_BANDWIDTH_LIMITER_INTERVAL / 10.0 ), kbps_allowed, packet_bits );
        }

        check( over_budget );
    }

    // make sure we recover
    {
        const int kbps_allowed = 1000;
        const int packet_bits = 50;

        bool recovered = false;

        for ( int i = 0; i < 100; ++i )
        {
            if ( next_bandwidth_limiter_add_packet( &bandwidth_limiter, i * ( NEXT_BANDWIDTH_LIMITER_INTERVAL / 10.0 ), kbps_allowed, packet_bits ) )
            {
                recovered = true;
                break;
            }
        }

        check( recovered );
    }
}

static void context_check_free( void * context, void * p )
{
    (void)p;

    // the context should not be cleared
    check( context );
    check( *((int *)context) == 23 );
}

static void test_free_retains_context()
{
    void * (*current_malloc)( void * context, size_t bytes ) = next_malloc_function;
    void (*current_free)( void * context, void * p ) = next_free_function;

    next_allocator( next_default_malloc_function, context_check_free );

    int canary = 23;
    void * context = (void *)&canary;
    next_queue_t *q = next_queue_create( context, 1 );
    next_queue_destroy( q );

    check( context );
    check( *((int *)context) == 23 );
    check( canary == 23 );

    next_allocator( current_malloc, current_free );
}

static void test_packet_loss_tracker()
{
    next_packet_loss_tracker_t tracker;
    memset( &tracker, 0, sizeof(tracker) );
    next_packet_loss_tracker_reset( &tracker );

    check( next_packet_loss_tracker_update( &tracker, NULL ) == 0 );

    uint64_t sequence = 0;

    for ( int i = 0; i < NEXT_PACKET_LOSS_TRACKER_SAFETY; ++i )
    {
        next_packet_loss_tracker_packet_received( &tracker, sequence );
        sequence++;
    }

    check( next_packet_loss_tracker_update( &tracker, NULL ) == 0 );

    for ( int i = 0; i < 200; ++i )
    {
        next_packet_loss_tracker_packet_received( &tracker, sequence );
        sequence++;
    }

    check( next_packet_loss_tracker_update( &tracker, NULL ) == 0 );

    for ( int i = 0; i < 200; ++i )
    {
        if ( sequence & 1 )
        {
            next_packet_loss_tracker_packet_received( &tracker, sequence );
        }
        sequence++;
    }

    check( next_packet_loss_tracker_update( &tracker, NULL ) == ( 200 - NEXT_PACKET_LOSS_TRACKER_SAFETY ) / 2 );

    check( next_packet_loss_tracker_update( &tracker, NULL ) == 0 );

    next_packet_loss_tracker_reset( &tracker );
    
    sequence = 0;

    next_packet_loss_tracker_packet_received( &tracker, 200 + NEXT_PACKET_LOSS_TRACKER_SAFETY - 1 );

    check( next_packet_loss_tracker_update( &tracker, NULL ) == 200 );

    next_packet_loss_tracker_packet_received( &tracker, 1000 );

    check( next_packet_loss_tracker_update( &tracker, NULL ) > 500 );

    next_packet_loss_tracker_packet_received( &tracker, 0xFFFFFFFFFFFFFFFULL );

    check( next_packet_loss_tracker_update( &tracker, NULL ) == 0 );
}

#define RUN_TEST( test_function )                                           \
    do                                                                      \
    {                                                                       \
        printf( "    " #test_function "\n" );                               \
        fflush( stdout );                                                   \
        test_function();                                                    \
    }                                                                       \
    while (0)

void next_test()
{
    RUN_TEST( test_time );
    RUN_TEST( test_endian );
    RUN_TEST( test_base64 );
    RUN_TEST( test_fnv1a );
    RUN_TEST( test_queue );
    RUN_TEST( test_bitpacker );
    RUN_TEST( test_bits_required );
    RUN_TEST( test_stream );
    RUN_TEST( test_address );
    RUN_TEST( test_replay_protection );
    RUN_TEST( test_ping_stats );
    RUN_TEST( test_random_bytes );
    RUN_TEST( test_random_float );
    RUN_TEST( test_crypto_box );
    RUN_TEST( test_crypto_secret_box );
    RUN_TEST( test_crypto_aead );
    RUN_TEST( test_crypto_aead_ietf );
    RUN_TEST( test_crypto_sign );
    RUN_TEST( test_crypto_sign_detached );
    RUN_TEST( test_crypto_key_exchange );
    RUN_TEST( test_basic_read_and_write );
    RUN_TEST( test_address_read_and_write );
    RUN_TEST( test_platform_socket );
    RUN_TEST( test_platform_thread );
    RUN_TEST( test_platform_mutex );
    RUN_TEST( test_client );
    RUN_TEST( test_server );
    RUN_TEST( test_direct );
    RUN_TEST( test_upgrade_token );
    RUN_TEST( test_packets );
    RUN_TEST( test_pending_session_manager );
    RUN_TEST( test_proxy_session_manager );
    RUN_TEST( test_session_manager );
    RUN_TEST( test_backend_packets );
    RUN_TEST( test_relay_manager );
    RUN_TEST( test_route_token );
    RUN_TEST( test_continue_token );
    RUN_TEST( test_header );
    RUN_TEST( test_tag );
    RUN_TEST( test_bandwidth_limiter );
    RUN_TEST( test_free_retains_context );
    RUN_TEST( test_packet_loss_tracker );
}

NEXT_PACK_POP()
