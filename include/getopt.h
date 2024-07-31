#ifndef _GETOPT_
#define _GETOPT_

int getopt(int argc, char * const argv[],
                  const char *optstring);

extern char *optarg;
extern int optind, opterr, optopt;

#endif
