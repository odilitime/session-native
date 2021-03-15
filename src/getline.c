//#include <stdio.h>
//#include <stdlib.h>

// for getline (10.6)
#include <errno.h>   // errno

ssize_t getline(char **linep, size_t *np, FILE *stream) {
  char *p = NULL;
  size_t i = 0;
  
  if (!linep || !np) {
    errno = EINVAL;
    return -1;
  }
  
  if (!(*linep) || !(*np)) {
    *np = 120;
    *linep = (char *)malloc(*np);
    if (!(*linep)) {
      return -1;
    }
  }
  
  flockfile(stream);
  
  p = *linep;
  int ch;
  for (; (ch = getc_unlocked(stream)) != EOF;) {
    if (i > *np) {
      /* Grow *linep. */
      size_t m = *np * 2;
      char *s = (char *)realloc(*linep, m);
      
      if (!s) {
        int error = errno;
        funlockfile(stream);
        errno = error;
        return -1;
      }
      
      *linep = s;
      *np = m;
    }
    
    p[i] = ch;
    if ('\n' == ch) break;
    i += 1;
  }
  funlockfile(stream);
  
  /* Null-terminate the string. */
  if (i > *np) {
    /* Grow *linep. */
    size_t m = *np * 2;
    char *s = (char *)realloc(*linep, m);
    
    if (!s) {
      return -1;
    }
    
    *linep = s;
    *np = m;
  }
  
  p[i + 1] = '\0';
  return ((i > 0)? i : -1);
}
