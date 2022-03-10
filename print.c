#include <stdio.h>

/* testing program for shell file redirection 
 * Author: Ben Stenberg
 */

int main(int argc, char *argv[]) {
  char str[1024];
  
  printf("Enter something to print. I promise I will only print it once.\n");
  fgets(str, 1024, stdin);
  printf("I lied.\n");
  for(int i = 0; i < 1000; i++) {
    printf("%s\n", str);
  }
  return 0;
}
