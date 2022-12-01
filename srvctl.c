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

#define SCTL "systemctl"

void usage (int argc, char **argv)
{
    fprintf( stderr, "Usage: %s <on|off>\n", program_invocation_short_name );
    fprintf( stderr, "  called as:  %s ", argv[0] );

    for( int x = 1; x < argc; x++ )
        fprintf( stderr, "%s ", argv[x] );
    fprintf( stderr, "\n" );
}

int main (int argc, char **argv)
{
    char *args[] = { SCTL, NULL, "steamos-reset" , NULL };

    if( argc == 2 )
    {
        if( strcmp( argv[1], "on" ) == 0 )
        {
            args[1] = "start";
            execv( "/sbin/" SCTL, args );
        }
        else if ( strcmp( argv[1], "off" ) == 0 )
        {
            args[1] = "stop";
            execv( "/sbin/" SCTL, args );
        }
        else
        {
            usage( argc, argv );
        }
    }
    else
    {
        usage( argc, argv );
    }

    return 1;
}
