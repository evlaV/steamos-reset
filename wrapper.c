// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>

bool needs_privilege ()
{
    return
      ( strcmp( program_invocation_short_name, "os-status"     ) == 0 ||
        strcmp( program_invocation_short_name, "boot-status"   ) == 0 ||
        strcmp( program_invocation_short_name, "clear"         ) == 0 ||
        strcmp( program_invocation_short_name, "undo-reset"    ) == 0 ||
        strcmp( program_invocation_short_name, "factory-reset" ) == 0 );
}

bool raise_privs (void)
{
    return setuid( 0 ) == 0;
}

bool drop_privs (void)
{
    uid_t euid = geteuid();
    gid_t egid = getegid();
    uid_t uid_new = euid;
    gid_t gid_new = egid;

    if( euid == 0 || egid == 0 )
    {
        struct passwd *user = getpwnam( "nobody" );

        if( user )
        {
            if( egid == 0 )
            {
                setegid( user->pw_gid );
                gid_new = getegid();
            }

            if( euid == 0 )
            {
                seteuid( user->pw_uid );
                uid_new = geteuid();
            }
        }
    }

    return ( uid_new != 0 ) && ( gid_new != 0 );
}

bool get_wrapped_script( char *path, size_t maxpath )
{
    struct stat script = { 0 };

    *path = '\0';

    if( strcmp( program_invocation_short_name, WRAPPER ) == 0 )
        return true;

    snprintf( path, maxpath - 1, CGI_LIBDIR "/%s",
              program_invocation_short_name );
    *(path + maxpath - 1 ) = '\0';

    if( stat( path, &script ) != 0 )
        return false;

    return true;
}

void sanitise(char *raw)
{
    char *c = raw;

    while( c && *c )
    {
        if( !isprint(*c) && !isspace(*c) )
            *c = '.';
        else if( *c == '\"' )
            *c = '\'';
        c++;
    }

    // not allowing JSON encoded string to end in a \ for reasons
    // of convenience and sanity:
    if( c > raw && *(c - 1) == '\\' )
        *(c - 1) = '\0';
}

void emit_error (uint status, char *path, char *message)
{
    sanitise( path );
    sanitise( message );

    printf( "Content-Type: application/json\r\n\r\n" );
    printf(
"{\"service\":\"%s\",\n\
\"version\":\"%s\",\n\
\"status\":%03d,\n\
\"message\":\"Service %s, path \\\"%s\\\": %s\"}",
           program_invocation_short_name,
           VERSION,
           status,
           program_invocation_short_name, path, message );
}

void emit_null_response (void)
{
    printf( "Content-Type: application/json\r\n\r\n" );
    printf("{"
           "\"service\":\"%s\",\n"
           "\"version\":\"%s\",\n"
           "\"status\":%03d,\n"
           "\"uid\": %d,\n"
           "\"gid\": %d,\n"
           "\"euid\": %d,\n"
           "\"egid\": %d,\n"
           "\"message\":\"Service %s called directly "
           "(should be invoked by wrapped service name)\"}\n",
           program_invocation_short_name,
           VERSION,
           200,
           getuid(),
           getgid(),
           geteuid(),
           getegid(),
           program_invocation_short_name );
}

char ** make_argv_from_query (char *query)
{
    char *c;
    int args = 1;
    char **argv = NULL;
    char char_ok[128] = { 0 };

    char_ok['.'] = 1;
    char_ok['='] = 1;
    char_ok['-'] = 1;

    if( !query )
        query = getenv( "QUERY_STRING" );

    if( query && *query )
        args++;

    for( c = query; c && *c; c++ )
        if( *c == '&' || *c == ';' )
            args++;

    argv = calloc( args + 1, sizeof(char *) );

    argv[0] = program_invocation_short_name;

    if( args >= 2 )
        argv[1] = query;

    int j = 2;

    for( c = query; c && *c && (j < args); c++ )
    {
        if( *c == '&' || *c == ';' )
        {
            *c = '\0';
            argv[ j++ ] = (c + 1);
        }
        // must be printable and [ alphanumeric . = - ]
        else if( !isprint(*c) ||
                 (!isalnum(*c) && !char_ok[(uint)*c]) )
        {
            *c = '.';
        }
    }

    argv[ args ] = NULL;

    return argv;
}

int wrap_script (char *path)
{
    char **argv = make_argv_from_query( NULL );

    int rc = execv( path, argv );

    free( argv );

    return rc;
}

int main (int argc, char **argv)
{
    char wrapped_script[256] = "";

    sanitise(program_invocation_short_name);

    if( !get_wrapped_script( &wrapped_script[0], sizeof(wrapped_script) ) )
    {
        emit_error( 404, &wrapped_script[0], "backend not found" );
        return 0;
    }

    if( wrapped_script[0] == '\0' )
    {
        emit_null_response();
        return 0;
    }

    if( needs_privilege() )
    {
        raise_privs();
    }
    else
    {
        if( ! drop_privs() )
        {
            emit_error( 502, &wrapped_script[0], "unable to drop privileges" );
            return 0;
        }
    }

    int rc = wrap_script( &wrapped_script[0] );
    char *wrap_error = strdup( strerror( rc ) );

    emit_error( 502, &wrapped_script[0], wrap_error );

    free( wrap_error );

    return 0;
}
