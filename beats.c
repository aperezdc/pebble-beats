/*
 * beats.c
 * Copyright (C) 2015 Adrian Perez <aperez@igalia.com>
 *
 * This is used to validate that the modifications done over the time_t
 * UTC value, and calculation of the .beats value are valid. To check the
 * output for different timezones, the following can be done in a Linux
 * box (change "PDT" to the desired timezone):
 *      TZ=PDT ./beats
 *
 * Build with:
 *      gcc -std=c99 -o beats beats.c
 *
 * Distributed under terms of the MIT license.
 */

#include <time.h>
#include <stdio.h>


int
main (int argc, const char *argv[])
{
	time_t local = time (NULL);
	struct tm *localtm = localtime (&local);
	time_t utctime = mktime (localtm);

	utctime += (60 * 60);  /* TZ=UTC+1 */
	struct tm *T = gmtime (&utctime);

	printf ("@%03u\n",
	        ((T->tm_sec) + (T->tm_min * 60) + (T->tm_hour * 3600)) * 10 / 864);
	return 0;
}
