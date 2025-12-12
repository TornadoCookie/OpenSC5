#include "hash.h"
#include <string.h>
#include<ctype.h>

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}

// Adapted from https://simswiki.info/wiki.php?title=Spore:HashVal
unsigned long TheHash(const char *strptr) {
	char oname[1024];
	unsigned int hash;
	char *p;

	strcpy(&oname[0], strptr);
	strlwr(&oname[0]);
	hash = 0x811C9DC5;
	p=&oname[0];
	while(*p) {
		hash *= 0x1000193;
		hash ^= (*p);
		p++;
		}
	return(hash);
}
