#include "core/list.h"
#include "core/log.h"

static void test_types()
{
  const char *hello = "hello";
  int ok = 1;
  List l = NULL;

  message("Testing type checking...");

  l = list_push(l, int, 154321);
  l = list_push(l, List, l);
  l = list_push(l, const char *, hello);

  ok = ok && hello == list_head(l, const char *);
  l = list_pop(l);
  ok = ok && l->next == list_head(l, List);
  l = list_pop(l);
  ok = ok && 154321 == list_head(l, int);
  
  message("-> Tests %s.", ok?"passed":"failed");
  list_delete(l);
}

static void test_push_back()
{
  const int base = 100;
  const int diam = 50;
  List l = NULL;
  int i;
  int ok;

  message("Testing push_back...");
  
  for (i=0; i<diam; i++)
  {
    l = list_push_back(l, int, base+i);
    if (i>0)
      l = list_push(l, int, base-i);
  }

  ok = 1;
  for (i=base-diam+1; i<base+diam; i++)
  {
    ok = ok && (list_head(l, int) == i);
    l = list_pop(l);
  }

  message("-> Tests %s.", ok?"passed":"failed");
}

static int is_odd(ListItem item)
{
  return ((int) item)%2 == 1;
}
static void test_filter()
{
  const int n = 100;
  List list = NULL;
  int i;
  int ok;

  message("Testing FILTER...");
  
  for (i=1; i<n; i++)
    list = list_push_back(list, int, i);

  list = list_filter(list, int, is_odd, NULL);

  ok = 1;
  for (i=2; i<n; i+=2)
  {
    ok = ok && (list_head(list, int) == i);
    list = list_pop(list);
  }
  
  message("-> Tests %s.", ok?"passed":"failed");
}

static void test_foreach()
{
  const int n = 100;
  List l = NULL;
  int i;
  int sum;
  int ok;

  message("Testing FOREACH...");
  
  for (i=1; i<n; i++)
    l = list_push(l, int, i);

  sum = 0;
  FOREACH(int, number, l)
  {
    sum += number;
  } FOREACH_END; 

  ok = (n*(n-1))/2 == sum;
  
  message("-> Tests %s.", ok?"passed":"failed");
}

int main()
{
  test_types();
  test_push_back();
  test_foreach();
  test_filter();

  message("Should now fail with type mismatch:");
  /* should fail here */
  (void)list_head(list_push(NULL, int, 15), char *);
  
  return 0;
}
