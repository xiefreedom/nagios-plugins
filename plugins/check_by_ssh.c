/******************************************************************************
 *
 * This file is part of the Nagios Plugins.
 *
 * Copyright (c) 1999, 2000, 2001 Karl DeBisschop <karl@debisschop.net>
 *
 * The Nagios Plugins are free software; you can redistribute them
 * and/or modify them under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id$
 *
 *****************************************************************************/
 
#define PROGRAM check_by_ssh
#define DESCRIPTION "Run checks on a remote system using ssh, wrapping the proper timeout around the ssh invocation."
#define AUTHOR "Karl DeBisschop"
#define EMAIL "karl@debisschop.net"
#define COPYRIGHTDATE "1999, 2000, 2001"

#include "config.h"
#include "common.h"
#include "popen.h"
#include "utils.h"
#include <time.h>

#define PROGNAME "check_by_ssh"

int process_arguments (int, char **);
int call_getopt (int, char **);
int validate_arguments (void);
void print_help (char *command_name);
void print_usage (void);


int commands;
char *remotecmd = NULL;
char *comm = NULL;
char *hostname = NULL;
char *outputfile = NULL;
char *host_shortname = NULL;
char *servicelist = NULL;
int passive = FALSE;
int verbose = FALSE;


int
main (int argc, char **argv)
{

	char input_buffer[MAX_INPUT_BUFFER] = "";
	char *result_text = NULL;
	char *status_text;
	char *output = NULL;
	char *eol = NULL;
	char *srvc_desc = NULL;
	int cresult;
	int result = STATE_UNKNOWN;
	time_t local_time;
	FILE *fp = NULL;


	/* process arguments */
	if (process_arguments (argc, argv) == ERROR)
		usage ("Could not parse arguments\n");


	/* Set signal handling and alarm timeout */
	if (signal (SIGALRM, popen_timeout_alarm_handler) == SIG_ERR) {
		printf ("Cannot catch SIGALRM");
		return STATE_UNKNOWN;
	}
	alarm (timeout_interval);


	/* run the command */

	if (verbose)
		printf ("%s\n", comm);

	child_process = spopen (comm);

	if (child_process == NULL) {
		printf ("Unable to open pipe: %s", comm);
		return STATE_UNKNOWN;
	}


	/* open STDERR  for spopen */
	child_stderr = fdopen (child_stderr_array[fileno (child_process)], "r");
	if (child_stderr == NULL) {
		printf ("Could not open stderr for %s\n", SSH_COMMAND);
	}


	/* get results from remote command */
	result_text = realloc (result_text, 1);
	result_text[0] = 0;
	while (fgets (input_buffer, MAX_INPUT_BUFFER - 1, child_process))
		result_text = strscat (result_text, input_buffer);


	/* WARNING if output found on stderr */
	if (fgets (input_buffer, MAX_INPUT_BUFFER - 1, child_stderr)) {
		printf ("%s\n", input_buffer);
		return STATE_WARNING;
	}
	(void) fclose (child_stderr);


	/* close the pipe */
	result = spclose (child_process);


	/* process output */
	if (passive) {

		if (!(fp = fopen (outputfile, "a"))) {
			printf ("SSH WARNING: could not open %s\n", outputfile);
			exit (STATE_UNKNOWN);
		}

		time (&local_time);
		srvc_desc = strtok (servicelist, ":");
		while (result_text != NULL) {
			status_text = (strstr (result_text, "STATUS CODE: "));
			if (status_text == NULL) {
				printf ("%s", result_text);
				return result;
			}
			output = result_text;
			result_text = strnl (status_text);
			eol = strpbrk (output, "\r\n");
			if (eol != NULL)
				eol[0] = 0;
			if (srvc_desc && status_text
					&& sscanf (status_text, "STATUS CODE: %d", &cresult) == 1) {
				fprintf (fp, "%d PROCESS_SERVICE_CHECK_RESULT;%s;%s;%d;%s\n",
								 (int) local_time, host_shortname, srvc_desc, cresult,
								 output);
				srvc_desc = strtok (NULL, ":");
			}
		}

	}

	/* print the first line from the remote command */
	else {
		eol = strpbrk (result_text, "\r\n");
		if (eol)
			eol[0] = 0;
		printf ("%s\n", result_text);

	}


	/* return error status from remote command */
	return result;
}





/* process command-line arguments */
int
process_arguments (int argc, char **argv)
{
	int c;

	if (argc < 2)
		return ERROR;

	remotecmd = realloc (remotecmd, 1);
	remotecmd[0] = 0;

	for (c = 1; c < argc; c++)
		if (strcmp ("-to", argv[c]) == 0)
			strcpy (argv[c], "-t");

	comm = strscpy (comm, SSH_COMMAND);

	c = 0;
	while (c += (call_getopt (argc - c, &argv[c]))) {

		if (argc <= c)
			break;

		if (hostname == NULL) {
			if (!is_host (argv[c]))
				terminate (STATE_UNKNOWN, "%s: Invalid host name %s\n", PROGNAME,
									 argv[c]);
			hostname = argv[c];
		}
		else if (remotecmd == NULL) {
			remotecmd = strscpy (remotecmd, argv[c++]);
			for (; c < argc; c++)
				remotecmd = ssprintf (remotecmd, "%s %s", remotecmd, argv[c]);
		}

	}

	if (commands > 1)
		remotecmd = strscat (remotecmd, ";echo STATUS CODE: $?;");

	if (remotecmd == NULL || strlen (remotecmd) <= 1)
		usage ("No remotecmd\n");

	comm = ssprintf (comm, "%s %s '%s'", comm, hostname, remotecmd);

	return validate_arguments ();
}





/* Call getopt */
int
call_getopt (int argc, char **argv)
{
	int c, i = 1;

#ifdef HAVE_GETOPT_H
	int option_index = 0;
	static struct option long_options[] = {
		{"version", no_argument, 0, 'V'},
		{"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
		{"fork", no_argument, 0, 'f'},
		{"timeout", required_argument, 0, 't'},
		{"host", required_argument, 0, 'H'},
		{"port", required_argument,0,'P'},
		{"output", required_argument, 0, 'O'},
		{"name", required_argument, 0, 'n'},
		{"services", required_argument, 0, 's'},
		{"identity", required_argument, 0, 'i'},
		{"user", required_argument, 0, 'u'},
		{"logname", required_argument, 0, 'l'},
		{"command", required_argument, 0, 'C'},
		{0, 0, 0, 0}
	};
#endif

	while (1) {
#ifdef HAVE_GETOPT_H
		c =
			getopt_long (argc, argv, "+?Vvhft:H:O:P:p:i:u:l:C:n:s:", long_options,
									 &option_index);
#else
		c = getopt (argc, argv, "+?Vvhft:H:O:P:p:i:u:l:C:n:s:");
#endif

		if (c == -1 || c == EOF)
			break;

		i++;
		switch (c) {
		case 't':
		case 'H':
		case 'O':
		case 'p':
		case 'i':
		case 'u':
		case 'l':
		case 'n':
		case 's':
			i++;
		}

		switch (c) {
		case '?':									/* help */
			print_usage ();
			exit (STATE_UNKNOWN);
		case 'V':									/* version */
			print_revision (PROGNAME, "$Revision$");
			exit (STATE_OK);
		case 'h':									/* help */
			print_help (PROGNAME);
			exit (STATE_OK);
		case 'v':									/* help */
			verbose = TRUE;
			break;
		case 'f':									/* fork to background */
			comm = ssprintf (comm, "%s -f", comm);
			break;
		case 't':									/* timeout period */
			if (!is_integer (optarg))
				usage2 ("timeout interval must be an integer", optarg);
			timeout_interval = atoi (optarg);
			break;
		case 'H':									/* host */
			if (!is_host (optarg))
				usage2 ("invalid host name", optarg);
			hostname = optarg;
			break;
		case 'P': /* port number */
		case 'p': /* port number */
			if (!is_integer (optarg))
				usage2 ("port must be an integer", optarg);
			comm = ssprintf (comm,"%s -p %s", comm, optarg);
			break;
		case 'O':									/* output file */
			outputfile = optarg;
			passive = TRUE;
			break;
		case 's':									/* description of service to check */
			servicelist = optarg;
			break;
		case 'n':									/* short name of host in nagios configuration */
			host_shortname = optarg;
			break;
		case 'u':
			c = 'l';
		case 'l':									/* login name */
		case 'i':									/* identity */
			comm = ssprintf (comm, "%s -%c %s", comm, c, optarg);
			break;
		case 'C':									/* Command for remote machine */
			commands++;
			if (commands > 1)
				remotecmd = strscat (remotecmd, ";echo STATUS CODE: $?;");
			remotecmd = strscat (remotecmd, optarg);
		}
	}
	return i;
}





int
validate_arguments (void)
{
	if (remotecmd == NULL || hostname == NULL)
		return ERROR;
	return OK;
}





void
print_help (char *cmd)
{
	print_revision (cmd, "$Revision$");

	printf
		("Copyright (c) 1999	Karl DeBisschop (kdebisschop@alum.mit.edu)\n\n"
		 "This plugin will execute a command on a remote host using SSH\n\n");

	print_usage ();

	printf
		("\nOptions:\n"
		 "-H, --hostname=HOST\n"
		 "   name or IP address of remote host\n"
		 "-C, --command='COMMAND STRING'\n"
		 "   command to execute on the remote machine\n"
		 "-f tells ssh to fork rather than create a tty\n"
		 "-t, --timeout=INTEGER\n"
		 "   specify timeout (default: %d seconds) [optional]\n"
		 "-l, --logname=USERNAME\n"
		 "   SSH user name on remote host [optional]\n"
		 "-i, --identity=KEYFILE\n"
		 "   identity of an authorized key [optional]\n"
		 "-O, --output=FILE\n"
		 "   external command file for nagios [optional]\n"
		 "-s, --services=LIST\n"
		 "   list of nagios service names, separated by ':' [optional]\n"
		 "-n, --name=NAME\n"
		 "   short name of host in nagios configuration [optional]\n"
		 "\n"
		 "The most common mode of use is to refer to a local identity file with\n"
		 "the '-i' option. In this mode, the identity pair should have a null\n"
		 "passphrase and the public key should be listed in the authorized_keys\n"
		 "file of the remote host. Usually the key will be restricted to running\n"
		 "only one command on the remote server. If the remote SSH server tracks\n"
		 "invocation agruments, the one remote program may be an agent that can\n"
		 "execute additional commands as proxy\n"
		 "\n"
		 "To use passive mode, provide multiple '-C' options, and provide\n"
		 "all of -O, -s, and -n options (servicelist order must match '-C'\n"
		 "options)\n", DEFAULT_SOCKET_TIMEOUT);
}





void
print_usage (void)
{
	printf
		("Usage:\n"
		 "check_by_ssh [-f] [-t timeout] [-i identity] [-l user] -H <host> <command>\n"
		 "             [-n name] [-s servicelist] [-O outputfile] [-P port]\n"
		 "check_by_ssh  -V prints version info\n"
		 "check_by_ssh  -h prints more detailed help\n");
}
