/*
 *  LibNoPoll: A websocket library
 *  Copyright (C) 2011 Advanced Software Production Line, S.L.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 *  
 *  You may find a copy of the license under this software is released
 *  at COPYING file. This is LGPL software: you are welcome to develop
 *  proprietary applications using this library without any royalty or
 *  fee but returning back any change, improvement or addition in the
 *  form of source code, project image, documentation patches, etc.
 *
 *  For commercial support on build Websocket enabled solutions
 *  contact us:
 *          
 *      Postal address:
 *         Advanced Software Production Line, S.L.
 *         Edificio Alius A, Oficina 102,
 *         C/ Antonio Suarez Nº 10,
 *         Alcalá de Henares 28802 Madrid
 *         Spain
 *
 *      Email address:
 *         info@aspl.es - http://www.aspl.es/nopoll
 */
#include <nopoll.h>

nopoll_bool test_01 (void) {
	return nopoll_true;
}

int main (int argc, char ** argv)
{
	printf ("** NoPoll: Websocket toolkit (regression test).\n");
	printf ("** Copyright (C) 2011 Advanced Software Production Line, S.L.\n**\n");
	printf ("** NoPoll regression tests: version=%s\n**\n",
		VERSION);
	printf ("** To gather information about time performance you can use:\n**\n");
	printf ("**     >> time ./test_01\n**\n");
	printf ("** To gather information about memory consumed (and leaks) use:\n**\n");
	printf ("**     >> libtool --mode=execute valgrind --leak-check=yes --error-limit=no ./test_01\n**\n");
	printf ("**\n");
	printf ("** Report bugs to:\n**\n");
	printf ("**     <info@aspl.es> NoPoll mailing list\n**\n");


	if (test_01 ()) {	
		printf ("Test 01: Library initialization and finalization [   OK   ]\n");
	}else {
		printf ("Test 01: Library initialization and finalization [ FAILED ]\n");
		return -1;
	}

	return 0;
}
