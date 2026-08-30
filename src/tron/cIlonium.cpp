#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef DEDICATED

#include "cIlonium.h"
#include "cBackend.h"
#include "tDirectories.h"
#include "tConsole.h"
#include "tString.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <mutex>

#ifndef WIN32
#include <sys/stat.h>
#else
#include <windows.h>
#endif

// The library where the build has it, the program where it does not. Both
// paths send the same bytes; the second one exists because a machine without
// curl's headers is still a machine somebody builds this on.
#ifdef LIBCURL_PROTOCOL_HTTP
#include <curl/curl.h>
#endif

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

namespace
{

// The private key never leaves this file and never leaves the machine. It is
// written once, read once per launch, and the only thing derived from it that
// travels is a signature.
std::string s_secret;      //!< raw 64 bytes, ed25519 private key
std::string s_public;      //!< base64 of the 32 byte public half
std::mutex  s_lock;
IloniumSelf s_me;

std::string Base64( unsigned char const * data, size_t len )
{
    static char const * alphabet =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string out;
    out.reserve( ( ( len + 2 ) / 3 ) * 4 );

    for ( size_t i = 0; i < len; i += 3 )
    {
        unsigned int block = data[i] << 16;
        if ( i + 1 < len ) block |= data[i+1] << 8;
        if ( i + 2 < len ) block |= data[i+2];

        out += alphabet[ ( block >> 18 ) & 63 ];
        out += alphabet[ ( block >> 12 ) & 63 ];
        out += ( i + 1 < len ) ? alphabet[ ( block >> 6 ) & 63 ] : '=';
        out += ( i + 2 < len ) ? alphabet[ block & 63 ] : '=';
    }

    return out;
}

std::string Hex( unsigned char const * data, size_t len )
{
    static char const * digits = "0123456789abcdef";
    std::string out;
    out.reserve( len * 2 );
    for ( size_t i = 0; i < len; ++i )
    {
        out += digits[ data[i] >> 4 ];
        out += digits[ data[i] & 15 ];
    }
    return out;
}

tString KeyPath()
{
    return rc_InternalPath( "ilonium_key" );
}

//! Why the last request did not happen. One message used to cover a missing
//! key, a build with no way to speak http at all, a network that never
//! answered and an answer that made no sense - and telling them apart is the
//! whole of finding out what is wrong on a machine that is not this one.
std::mutex  s_whyLock;
std::string s_why = "not tried yet";

void Why( std::string const & said )
{
    std::lock_guard< std::mutex > hold( s_whyLock );
    s_why = said;
}

//! Writes the private key with the tightest permissions the platform offers.
void Restrict( char const * path )
{
#ifndef WIN32
    chmod( path, 0600 );
#else
    (void)path;
#endif
}

bool LoadKey()
{
    std::ifstream in( (char const *)KeyPath() );
    if ( !in )
        return false;

    std::string stored;
    std::getline( in, stored );
    if ( stored.size() != 128 )     // 64 bytes as hex
        return false;

    unsigned char raw[ 64 ];
    for ( int i = 0; i < 64; ++i )
    {
        unsigned int byte = 0;
        if ( sscanf( stored.c_str() + i * 2, "%2x", &byte ) != 1 )
            return false;
        raw[ i ] = (unsigned char)byte;
    }

    s_secret.assign( (char const *)raw, 64 );
    // the public half is the tail of an ed25519 private key
    s_public = Base64( raw + 32, 32 );
    return true;
}

bool MakeKey()
{
    EVP_PKEY * key = NULL;
    EVP_PKEY_CTX * ctx = EVP_PKEY_CTX_new_id( EVP_PKEY_ED25519, NULL );
    if ( !ctx )
        return false;

    bool made = EVP_PKEY_keygen_init( ctx ) == 1 && EVP_PKEY_keygen( ctx, &key ) == 1;
    EVP_PKEY_CTX_free( ctx );
    if ( !made || !key )
        return false;

    unsigned char priv[ 32 ], pub[ 32 ];
    size_t privLen = sizeof( priv ), pubLen = sizeof( pub );
    bool got = EVP_PKEY_get_raw_private_key( key, priv, &privLen ) == 1 &&
               EVP_PKEY_get_raw_public_key( key, pub, &pubLen ) == 1;
    EVP_PKEY_free( key );

    if ( !got || privLen != 32 || pubLen != 32 )
        return false;

    unsigned char both[ 64 ];
    memcpy( both, priv, 32 );
    memcpy( both + 32, pub, 32 );

    tString path = KeyPath();
    {
        std::ofstream out( (char const *)path, std::ios::trunc );
        if ( !out )
            return false;
        out << Hex( both, 64 ) << "\n";
    }
    Restrict( (char const *)path );

    s_secret.assign( (char const *)both, 64 );
    s_public = Base64( pub, 32 );

    memset( priv, 0, sizeof( priv ) );
    memset( both, 0, sizeof( both ) );
    return true;
}

//! Signs the line the server will rebuild from the request it receives.
bool Sign( std::string const & text, std::string & out )
{
    if ( s_secret.size() != 64 )
        return false;

    EVP_PKEY * key = EVP_PKEY_new_raw_private_key(
        EVP_PKEY_ED25519, NULL, (unsigned char const *)s_secret.data(), 32 );
    if ( !key )
        return false;

    EVP_MD_CTX * md = EVP_MD_CTX_new();
    unsigned char sig[ 64 ];
    size_t sigLen = sizeof( sig );

    bool ok = md &&
        EVP_DigestSignInit( md, NULL, NULL, NULL, key ) == 1 &&
        EVP_DigestSign( md, sig, &sigLen, (unsigned char const *)text.data(), text.size() ) == 1 &&
        sigLen == 64;

    if ( md ) EVP_MD_CTX_free( md );
    EVP_PKEY_free( key );

    if ( !ok )
        return false;

    out = Base64( sig, 64 );
    return true;
}

#ifdef LIBCURL_PROTOCOL_HTTP
size_t Collect( void * data, size_t size, size_t count, void * target )
{
    ( (std::string *)target )->append( (char const *)data, size * count );
    return size * count;
}
#else

//! The same request without the library: curl the program, with the body in a
//! file rather than on the command line. A json body is full of quotes and
//! braces, and handing that to a shell is how a message about a bug becomes a
//! bug of its own.
//!
//! Written in double quotes and with the right name for the bin. It used to be
//! written for a unix shell - single quotes, and errors sent to /dev/null - and
//! on Windows a single quote is not a quote at all: every request went out with
//! the quotes still in it, reached nobody, and the chat said the server was not
//! answering. The values here are base64, hex and a fixed address, so there is
//! nothing in them that double quotes cannot carry.
bool Fetch( char const * method, std::string const & url, std::string const & body,
            char const * stamp, std::string const & nonce, std::string const & signature,
            long & code, std::string & out )
{
    tString const bodyPath = rc_InternalPath( "ilonium_post.json" );
    if ( !body.empty() )
    {
        std::ofstream write( (char const *)bodyPath, std::ios::trunc | std::ios::binary );
        if ( !write )
            return false;
        write << body;
    }

    std::string cmd = "curl -s -S -w \"\\n%{http_code}\" --max-time 20 --connect-timeout 8";
    cmd += " -A \"IloniumClient/1.3\"";
    cmd += " -H \"Content-Type: application/json\"";
    cmd += " -H \"X-Ilonium-Key: " + s_public + "\"";
    cmd += std::string( " -H \"X-Ilonium-Time: " ) + stamp + "\"";
    cmd += " -H \"X-Ilonium-Nonce: " + nonce + "\"";
    cmd += " -H \"X-Ilonium-Sign: " + signature + "\"";

    if ( strcmp( method, "POST" ) == 0 )
        cmd += std::string( " -X POST --data-binary @\"" ) + (char const *)bodyPath + "\"";

    cmd += " \"" + url + "\"";

#if defined( _WIN32 ) || defined( WIN32 )
    cmd += " 2>nul";
#else
    cmd += " 2>/dev/null";
#endif

    // Named differently by the same library on Windows, and this file only ever
    // reaches here on a build without libcurl.
#if defined( _WIN32 ) || defined( WIN32 )
    FILE * pipe = _popen( cmd.c_str(), "r" );
#else
    FILE * pipe = popen( cmd.c_str(), "r" );
#endif
    if ( !pipe )
        return false;

    std::string answer;
    char buffer[ 4096 ];
    while ( fgets( buffer, sizeof( buffer ), pipe ) )
        answer += buffer;
#if defined( _WIN32 ) || defined( WIN32 )
    _pclose( pipe );
#else
    pclose( pipe );
#endif

    if ( !body.empty() )
        remove( (char const *)bodyPath );

    // the status code is the last line, the body is everything above it
    size_t const split = answer.find_last_of( '\n' );
    if ( split == std::string::npos )
    {
        Why( "curl the program said nothing - this build has no libcurl in it" );
        return false;
    }

    code = atol( answer.c_str() + split + 1 );
    out = answer.substr( 0, split );

    if ( code <= 0 )
        Why( "curl the program gave no status" );
    else
        Why( "ok" );

    return code > 0;
}
#endif

//! Pulls a value out of a flat json object. The answers here are small and
//! shallow, and a parser for them is a dependency this file does not need.
std::string Field( std::string const & json, char const * name )
{
    std::string key = std::string( "\"" ) + name + "\"";
    size_t at = json.find( key );
    if ( at == std::string::npos )
        return "";

    at = json.find( ':', at + key.size() );
    if ( at == std::string::npos )
        return "";
    ++at;

    while ( at < json.size() && ( json[at] == ' ' || json[at] == '\t' ) )
        ++at;
    if ( at >= json.size() )
        return "";

    if ( json[at] == '"' )
    {
        std::string out;
        for ( size_t i = at + 1; i < json.size(); ++i )
        {
            if ( json[i] == '\\' && i + 1 < json.size() )
            {
                char const c = json[++i];
                switch ( c )
                {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'u':
                    // the server sends utf-8 as itself; an escape here is rare
                    // enough that dropping it beats half decoding it
                    i += 4;
                    break;
                default:  out += c;    break;
                }
                continue;
            }
            if ( json[i] == '"' )
                break;
            out += json[i];
        }
        return out;
    }

    size_t end = json.find_first_of( ",}", at );
    if ( end == std::string::npos )
        end = json.size();
    std::string out = json.substr( at, end - at );
    while ( !out.empty() && ( out.back() == ' ' || out.back() == '\n' ) )
        out.erase( out.size() - 1 );
    return out;
}

long long Number( std::string const & json, char const * name )
{
    std::string const raw = Field( json, name );
    return raw.empty() ? 0 : atoll( raw.c_str() );
}

std::string Escape( std::string const & text )
{
    std::string out;
    out.reserve( text.size() + 8 );
    for ( size_t i = 0; i < text.size(); ++i )
    {
        unsigned char const c = (unsigned char)text[i];
        switch ( c )
        {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if ( c < 0x20 )
            {
                char buf[ 8 ];
                snprintf( buf, sizeof( buf ), "\\u%04x", c );
                out += buf;
            }
            else
            {
                out += (char)c;
            }
        }
    }
    return out;
}

//! Keeps what the server last said about us, so screens can draw it without
//! asking again.
void Remember( std::string const & json )
{
    std::string const tag = Field( json, "role_tag" );
    std::string const color = Field( json, "role_color" );

    s_me.roleTag = tag;
    s_me.roleColor = color;
    s_me.muted = Field( json, "muted" ) == "true";
    s_me.chatBanned = Field( json, "chat_banned" ) == "true";
    s_me.tickBanned = Field( json, "tick_banned" ) == "true";
    if ( s_me.chatBanned )
        s_me.chatBanReason = Field( json, "chat_ban_reason" );
    if ( s_me.tickBanned )
        s_me.tickBanReason = Field( json, "tick_ban_reason" );
    s_me.muteUntil = Number( json, "mute_until" );
    s_me.muteReason = Field( json, "mute_reason" );
    if ( json.find( "\"ticket_wait\"" ) != std::string::npos )
        s_me.ticketWait = (int)Number( json, "ticket_wait" );
    s_me.known = true;
}

}

// ---------------------------------------------------------------------------

std::string rc_IloniumWhy()
{
    std::lock_guard< std::mutex > hold( s_whyLock );
    return s_why;
}

bool rc_IloniumReady()
{
    std::lock_guard< std::mutex > hold( s_lock );
    if ( s_secret.size() == 64 )
        return true;
    if ( LoadKey() )
        return true;
    return MakeKey();
}

std::string rc_IloniumPublicKey()
{
    rc_IloniumReady();
    return s_public;
}

bool rc_IloniumCall( char const * method, std::string const & path,
                     std::string const & body, long & code, std::string & out )
{
    if ( !rc_IloniumReady() )
    {
        Why( "no key - could not write " + std::string( (char const *)KeyPath() ) );
        return false;
    }

    // A random nonce and the clock: together they mean a request that is
    // overheard cannot be sent again, either now or tomorrow.
    unsigned char noise[ 12 ];
    if ( RAND_bytes( noise, sizeof( noise ) ) != 1 )
    {
        Why( "no randomness from the crypto library" );
        return false;
    }
    std::string const nonce = Hex( noise, sizeof( noise ) );

    char stamp[ 32 ];
    snprintf( stamp, sizeof( stamp ), "%lld", (long long)time( NULL ) );

    unsigned char digest[ SHA256_DIGEST_LENGTH ];
    SHA256( (unsigned char const *)body.data(), body.size(), digest );

    std::string const line = std::string( method ) + "\n" + path + "\n" +
                             stamp + "\n" + nonce + "\n" + Hex( digest, sizeof( digest ) );

    std::string signature;
    {
        std::lock_guard< std::mutex > hold( s_lock );
        if ( !Sign( line, signature ) )
        {
            Why( "could not sign the request" );
            return false;
        }
    }

    std::string const url = std::string( RC_ILONIUM_API ) + path;

#ifndef LIBCURL_PROTOCOL_HTTP
    return Fetch( method, url, body, stamp, nonce, signature, code, out );
#else
    CURL * curl = curl_easy_init();
    if ( !curl )
        return false;

    struct curl_slist * headers = NULL;
    headers = curl_slist_append( headers, "Content-Type: application/json" );
    headers = curl_slist_append( headers, ( "X-Ilonium-Key: " + s_public ).c_str() );
    headers = curl_slist_append( headers, ( std::string( "X-Ilonium-Time: " ) + stamp ).c_str() );
    headers = curl_slist_append( headers, ( "X-Ilonium-Nonce: " + nonce ).c_str() );
    headers = curl_slist_append( headers, ( "X-Ilonium-Sign: " + signature ).c_str() );

    out.clear();
    curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
    curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, Collect );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, &out );
    curl_easy_setopt( curl, CURLOPT_TIMEOUT, 20L );
    curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 8L );
    curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L );
    // A name of our own, so the service in front can tell our traffic from a
    // scraper's and never mistakes one for the other.
    curl_easy_setopt( curl, CURLOPT_USERAGENT, "IloniumClient/1.3" );

#if defined( _WIN32 ) || defined( WIN32 )
    // Which certificates to trust.
    //
    // The library is built against OpenSSL, and OpenSSL on Windows does not
    // read the certificate store the rest of the system uses - it looks for a
    // file, and there was no file. Every request failed before it was sent,
    // with "problem with the SSL CA cert", which reads like a fault at the far
    // end and is not one.
    //
    // Ask for the system's own store where the library is new enough to reach
    // it, and fall back to the list packaged beside the game.
#ifdef CURLSSLOPT_NATIVE_CA
    curl_easy_setopt( curl, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_NATIVE_CA );
#endif

    {
        static std::string bundle;
        static bool looked = false;

        if ( !looked )
        {
            looked = true;

            char self[ MAX_PATH ] = { 0 };
            if ( GetModuleFileNameA( NULL, self, sizeof( self ) - 1 ) )
            {
                std::string beside( self );
                size_t const cut = beside.find_last_of( "\\/" );
                if ( cut != std::string::npos )
                {
                    std::string const candidate = beside.substr( 0, cut ) + "/ca-bundle.crt";
                    FILE * there = fopen( candidate.c_str(), "r" );
                    if ( there )
                    {
                        fclose( there );
                        bundle = candidate;
                    }
                }
            }
        }

        if ( !bundle.empty() )
            curl_easy_setopt( curl, CURLOPT_CAINFO, bundle.c_str() );
    }
#endif

    if ( strcmp( method, "POST" ) == 0 )
    {
        curl_easy_setopt( curl, CURLOPT_POST, 1L );
        curl_easy_setopt( curl, CURLOPT_POSTFIELDS, body.c_str() );
        curl_easy_setopt( curl, CURLOPT_POSTFIELDSIZE, (long)body.size() );
    }

    CURLcode const result = curl_easy_perform( curl );
    curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &code );

    if ( result != CURLE_OK )
        Why( std::string( "network: " ) + curl_easy_strerror( result ) );
    else
        Why( "ok" );

    curl_slist_free_all( headers );
    curl_easy_cleanup( curl );

    return result == CURLE_OK;
#endif
}

bool rc_IloniumFetch( std::vector< IloniumLine > & out, std::string & error )
{
    long code = 0;
    std::string body;
    if ( !rc_IloniumCall( "GET", "/v1/chat?limit=60", "", code, body ) )
    {
        error = "no answer - " + rc_IloniumWhy();
        return false;
    }
    if ( code == 403 && Field( body, "error" ) == "banned" )
    {
        s_me.chatBanned = true;
        s_me.chatBanReason = Field( body, "reason" );
        s_me.known = true;
        error = s_me.chatBanReason.empty() ? "banned from the chat"
                                           : ( "banned from the chat: " + s_me.chatBanReason );
        return false;
    }

    if ( code != 200 )
    {
        error = Field( body, "error" );
        if ( error.empty() )
            error = "the server refused";
        return false;
    }

    s_me.chatBanned = false;

    size_t const you = body.find( "\"you\"" );
    if ( you != std::string::npos )
        Remember( body.substr( you ) );

    out.clear();

    // The array arrives oldest first, one flat object per line.
    size_t at = body.find( "\"messages\"" );
    if ( at == std::string::npos )
        return true;

    while ( ( at = body.find( '{', at ) ) != std::string::npos )
    {
        size_t const end = body.find( '}', at );
        if ( end == std::string::npos )
            break;

        std::string const one = body.substr( at, end - at + 1 );
        if ( one.find( "\"message\"" ) == std::string::npos )
        {
            at = end + 1;
            continue;
        }

        IloniumLine line;
        line.id = Number( one, "id" );
        line.name = Field( one, "name" );
        line.message = Field( one, "message" );
        line.stamp = Number( one, "timestamp" );
        line.roleTag = Field( one, "role_tag" );
        line.roleColor = Field( one, "role_color" );
        out.push_back( line );

        at = end + 1;
    }

    return true;
}

bool rc_IloniumSay( std::string const & name, std::string const & text, std::string & error )
{
    std::string const body =
        "{\"name\":\"" + Escape( name ) + "\",\"message\":\"" + Escape( text ) + "\"}";

    long code = 0;
    std::string answer;
    if ( !rc_IloniumCall( "POST", "/v1/chat", body, code, answer ) )
    {
        error = "no answer - " + rc_IloniumWhy();
        return false;
    }

    if ( code == 200 )
        return true;

    // The server says why, and how long, and the client just repeats it. There
    // is nothing here to negotiate with: the wait it names is the wait.
    std::string const what = Field( answer, "error" );
    if ( what == "muted" )
    {
        long long const left = Number( answer, "left" );
        std::string const why = Field( answer, "reason" );
        char buf[ 160 ];
        snprintf( buf, sizeof( buf ), "muted for another %lld min%s%s",
                  ( left + 59 ) / 60, why.empty() ? "" : ": ", why.c_str() );
        error = buf;
    }
    else if ( what == "banned" )
    {
        s_me.chatBanned = true;
        s_me.chatBanReason = Field( answer, "reason" );
        std::string const why = s_me.chatBanReason;
        error = why.empty() ? "banned from the chat" : ( "banned from the chat: " + why );
    }
    else if ( code == 429 )
    {
        error = "not so fast";
    }
    else
    {
        error = what.empty() ? "the message did not go through" : what;
    }

    return false;
}

bool rc_IloniumTicket( std::string const & name, std::string const & title,
                       std::string const & body, IloniumMachine const & box,
                       std::string & error )
{
    std::string const payload =
        "{\"name\":\"" + Escape( name ) +
        "\",\"title\":\"" + Escape( title ) +
        "\",\"message\":\"" + Escape( body ) +
        "\",\"os\":\"" + Escape( box.os ) +
        "\",\"cpu\":\"" + Escape( box.cpu ) +
        "\",\"gpu\":\"" + Escape( box.gpu ) +
        "\",\"ram\":\"" + Escape( box.ram ) +
        "\",\"disk\":\"" + Escape( box.disk ) + "\"}";

    long code = 0;
    std::string answer;
    if ( !rc_IloniumCall( "POST", "/v1/ticket", payload, code, answer ) )
    {
        error = "no answer - " + rc_IloniumWhy();
        return false;
    }

    if ( code == 200 )
    {
        s_me.ticketWait = 3600;
        return true;
    }

    if ( code == 403 )
    {
        s_me.tickBanned = true;
        s_me.tickBanReason = Field( answer, "reason" );
        error = s_me.tickBanReason.empty() ? "tickets are closed for you"
                                           : ( "tickets are closed: " + s_me.tickBanReason );
        return false;
    }

    if ( code == 429 )
    {
        int const wait = (int)Number( answer, "wait" );
        s_me.ticketWait = wait;
        char buf[ 96 ];
        snprintf( buf, sizeof( buf ), "one ticket an hour - %d min left", ( wait + 59 ) / 60 );
        error = buf;
        return false;
    }

    std::string const what = Field( answer, "error" );
    error = what.empty() ? "the ticket did not go through" : what;
    return false;
}

IloniumSelf const & rc_IloniumMe()
{
    return s_me;
}

bool rc_IloniumRefreshMe( std::string & error )
{
    long code = 0;
    std::string body;
    if ( !rc_IloniumCall( "GET", "/v1/me", "", code, body ) )
    {
        error = "no answer - " + rc_IloniumWhy();
        return false;
    }

    if ( code != 200 )
    {
        error = Field( body, "error" );
        return false;
    }

    Remember( body );
    return true;
}

#endif // !DEDICATED
