#include <pack.h>
#include <json_pack.h>

JSON_OBJECT jerr(int jerrno,const char *errmsg)
{
JSON_OBJECT ejson,ijson,sjson;

        ejson=json_object_new_object();
        ijson=json_object_new_int(jerrno);
        json_object_object_add(ejson,"errno",ijson);
        sjson=json_object_new_string(errmsg);
        json_object_object_add(ejson,"msg",sjson);
        return ejson;
}

