#include "list.h"
#include "debug.h"

int main()
{
  const char *hello = "hello";
  int ok = 1;
  List l = NULL;

  l = list_push(l, int, 154321);
  l = list_push(l, List, l);
  l = list_push(l, const char *, hello);

  ok = ok && hello == list_head(l, const char *);
  l = list_pop(l);
  ok = ok && l->next == list_head(l, List);
  l = list_pop(l);
  ok = ok && 154321 == list_head(l, int);
  message("Positive tests %s.", ok?"passed":"failed");

  /* should fail here */
  (void)list_head(l, char *);
  
  return !ok;
}
