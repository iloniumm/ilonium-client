/*

*************************************************************************

RetroCycles

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

***************************************************************************

*/

#include "cShell.h"
#include "tOwnership.h"

RC_OWNERSHIP( shell )

#include <cstdio>
#include <vector>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>

bool rc_RunHidden( const std::string & command, std::string & output )
{
    output.clear();

    SECURITY_ATTRIBUTES security;
    security.nLength = sizeof( security );
    security.bInheritHandle = TRUE;
    security.lpSecurityDescriptor = NULL;

    HANDLE readEnd = NULL, writeEnd = NULL;
    if ( !CreatePipe( &readEnd, &writeEnd, &security, 0 ) )
        return false;

    // only the child may inherit the writing end, or the read below never ends
    SetHandleInformation( readEnd, HANDLE_FLAG_INHERIT, 0 );

    STARTUPINFOA startup;
    ZeroMemory( &startup, sizeof( startup ) );
    startup.cb = sizeof( startup );
    startup.hStdOutput = writeEnd;
    startup.hStdError = writeEnd;
    startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION process;
    ZeroMemory( &process, sizeof( process ) );

    std::vector< char > line( command.begin(), command.end() );
    line.push_back( '\0' );

    const BOOL started = CreateProcessA(
        NULL, line.data(), NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, NULL, &startup, &process );

    CloseHandle( writeEnd );

    if ( !started )
    {
        CloseHandle( readEnd );
        return false;
    }

    char buffer[ 4096 ];
    DWORD got = 0;
    while ( ReadFile( readEnd, buffer, sizeof( buffer ), &got, NULL ) && got > 0 )
        output.append( buffer, got );

    CloseHandle( readEnd );
    WaitForSingleObject( process.hProcess, INFINITE );
    CloseHandle( process.hProcess );
    CloseHandle( process.hThread );
    return true;
}

#else

bool rc_RunHidden( const std::string & command, std::string & output )
{
    output.clear();

    FILE * pipe = popen( command.c_str(), "r" );
    if ( !pipe )
        return false;

    char buffer[ 4096 ];
    while ( fgets( buffer, sizeof( buffer ), pipe ) != NULL )
        output += buffer;

    pclose( pipe );
    return true;
}

#endif
