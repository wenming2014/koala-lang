
#include "koala.h"

void test_stringobject(void)
{
  Object *s1 = String_New("hello");
  Object *s2 = String_New("hello");
  assert(s1 == s2);
  assert(OB_REFCNT(s1) == 2);
  OB_DECREF(s1);
  assert(OB_REFCNT(s1) == 1);
  OB_DECREF(s1);
  assert(s1 == NULL);
  s1 = String_New("hello");
  s2 = String_New(" world");
  Object *s3 = OB_KLASS(s1)->num_ops->add(s1, s2);
  assert(!strcmp("hello world", String_Raw(s3)));
  OB_DECREF(s1);
  OB_DECREF(s2);
  OB_DECREF(s3);
  assert(s1 == NULL);
  assert(s2 == NULL);
  assert(s3 == NULL);
}

int main(int argc, char *argv[])
{
  Koala_Initialize();
  test_stringobject();
  Koala_Finalize();
  return 0;
}
