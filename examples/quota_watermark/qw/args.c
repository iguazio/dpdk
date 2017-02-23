/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <rte_common.h>
#include <rte_lcore.h>

#include "args.h"


unsigned int portmask = 0;


static void
usage(const char *prgname)
{
	fprintf(stderr, "Usage: %s [EAL args] -- -p <portmask>\n"
			"-p PORTMASK: hexadecimal bitmask of NIC ports to configure\n",
			prgname);
}

static unsigned long
parse_portmask(const char *portmask_str)
{
	return strtoul(portmask_str, NULL, 16);
}

static void
check_core_count(void)
{
	if (rte_lcore_count() < 3)
		rte_exit(EXIT_FAILURE,
				"At least 3 cores need to be passed in the coremask\n");
}

static void
check_portmask_value(unsigned int portmask)
{
	unsigned int port_nb = 0;

	port_nb = __builtin_popcount(portmask);

	if (port_nb == 0)
		rte_exit(EXIT_FAILURE,
				"At least 2 ports need to be passed in the portmask\n");

	if (port_nb % 2 != 0)
		rte_exit(EXIT_FAILURE,
				"An even number of ports is required in the portmask\n");
}

int
parse_qw_args(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "h:p:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			break;
		case 'p':
			portmask = parse_portmask(optarg);
			break;
		default:
			usage(argv[0]);
		}
	}

	check_core_count();
	check_portmask_value(portmask);

	return 0;
}
