#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define MC_MAINTAINER_MODE
#include "json.h"

/******************************************************
 * 测试非法操作 
 ******************************************************/

char str[]="/* more difficult test case */ { \"glossary\": { \"title\": \"example glossary\", \"GlossDiv\": { \"title\": \"S\", \"GlossList\": [ { \"ID\": \"SGML\", \"SortA";
int main(int argc, char **argv)
{
  json_object *new_obj;
  int i;

  MC_SET_DEBUG(1);

  new_obj = json_tokener_parse("/* more difficult test case */ { \"glossary\": { \"title\": \"example glossary\", \"GlossDiv\": { \"title\": \"S\", \"GlossList\": [ { \"ID\": \"SGML\", \"SortAs\": \"SGML\", \"GlossTerm\": \"Standard Generalized Markup Language\", \"Acronym\": \"SGML\", \"Abbrev\": \"ISO 8879:1986\", \"GlossDef\": \"A meta-markup language, used to create markup languages such as DocBook.\", \"GlossSeeAlso\": [\"GML\", \"XML\", \"markup\"] } ] } } }");
   printf("new_obj.to_string()=%s\n", json_object_to_json_string(new_obj));

	i=json_object_array_length(new_obj);
	printf("array_length=%d\n",i);
  json_object_put(new_obj);

  new_obj = json_tokener_parse(str);
printf("new_obj=%lld\n",(long long)new_obj);
  if(new_obj == 0) printf("非法的JSON串:%s:\n",str);
  else {
	printf("new_obj.to_string()=%s\n", json_object_to_json_string(new_obj));
	json_object_put(new_obj);
  }
  return 0;
}
