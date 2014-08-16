#include "json_module.hpp" 
#include <jansson.h>
#include <functional>

/*
    based on https://github.com/thraaawn/SMJansson
*/

typedef void* JsonIterT;

class JsonTypeHandler : public IHandleDispatch
{
public:

    virtual void free(HandleData object)
    {
        json_decref(reinterpret_cast<json_t*>(object));
    }
};

class JsonIterTypeHandler : public IHandleDispatch
{
public:

    virtual void free(HandleData object)
    {
        //nothing
    }
};

HandleTable* g_HandleTable;
JsonTypeHandler g_JsonTypeHandler;
JsonIterTypeHandler g_JsonIterTypeHandler;

/*
v 1.0 - 1.2 - was bad idea to use smart pointers

Go to hell mr. smart pointer. I hate you

v 1.3 - no stl in amx_handle.cpp (see https://forums.alliedmods.net/showthread.php?t=244605&page=2)

i so don't want to remake this module. i so like std::bind

*/


namespace JsonFunc
{

typedef std::function<json_t* (void)> JsonType;
typedef std::function<cell (json_t*)> JsonValueFunc;
typedef std::function<cell (json_t*, cell)> JsonSimpleSetFunc;
typedef std::function<cell (json_t*, cell, cell)> JsonSetFunc;
typedef std::function<json_t* (AMX*, json_t*, cell*)> JsonGetFunc;

typedef std::function<void* (void)> JsonIterType;

static cell create(AMX* amx, JsonType jtype)
{
    auto object = jtype();

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    HandleKey handle = g_HandleTable->create(object, &g_JsonTypeHandler);
    return handle;
}

static cell iter(AMX* amx, JsonIterType jiter)
{
    auto object = jiter();

    if (!object)
    {
        // MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json iter handle");
        return INVALID_HANDLE;
    }

    HandleKey handle = g_HandleTable->create(object, &g_JsonIterTypeHandler);
    return handle;
}

static cell simple(AMX* amx, cell* params, JsonValueFunc jfunc)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    auto object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return jfunc(object);
}

static cell set(AMX* amx, cell* params, JsonSimpleSetFunc jfunc)
{
    return JsonFunc::simple(amx, params, 
        [&jfunc, &params](json_t* json)
        {
            return jfunc(json, params[2]) == JSON_SUCCESS;
        }
    );
}

static cell set(AMX* amx, cell* params, JsonSetFunc jfunc)
{
    return JsonFunc::simple(amx, params, 
        [&jfunc, &params](json_t* json)
        {
            return jfunc(json, params[2], params[3]) == JSON_SUCCESS;
        }
    );
}

#define value simple

static cell get(AMX* amx, cell* params, JsonGetFunc jfunc)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    auto object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return INVALID_HANDLE;
    }

    /*it's a borrowed reference, 
    but have to create a new json wrapper with this reference and user have to close it
    so we have to increase reference on object  */
    json_t* result = jfunc(amx, object, params);

    if (!result)
    {
        return INVALID_HANDLE;
    }

    json_incref(result);
    HandleKey resHandle = g_HandleTable->create(result, &g_JsonTypeHandler);

    return resHandle;
}

} //JsonFunc


//native destroy_handle(any:handle);
static cell AMX_NATIVE_CALL AMX_DestroyHandle(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);

    if (!g_HandleTable->destroy(handle))
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", params[1]);
        return ERROR;
    }

    return SUCCESS;
}

//native JsonType:json_typeof(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonTypeof(AMX* amx, cell* params)
{ 
    return JsonFunc::value(amx, params, 
        [](json_t* json)
        {
            return json_typeof(json);
        }
    );
}

//native JsonHandle:json_true();
static cell AMX_NATIVE_CALL AMX_JsonTrue(AMX* amx, cell* params)
{
    return JsonFunc::create(amx, json_true);
}

//native JsonHandle:json_false();
static cell AMX_NATIVE_CALL AMX_JsonFalse(AMX* amx, cell* params)
{
    return JsonFunc::create(amx, json_false);
}

//native JsonHandle:json_null();
static cell AMX_NATIVE_CALL AMX_JsonNull(AMX* amx, cell* params)
{
    return JsonFunc::create(amx, json_null);
}

//native JsonHandle:json_integer();
static cell AMX_NATIVE_CALL AMX_JsonInteger(AMX* amx, cell* params)
{
    return JsonFunc::create(amx, std::bind(json_integer, params[1]));
}

//native json_integer_value(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonIntegerValue(AMX* amx, cell* params)
{
    return JsonFunc::value(amx, params, json_integer_value);
}

//native json_integer_set(JsonHandle:handle, value);
static cell AMX_NATIVE_CALL AMX_JsonIntegerSet(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, json_integer_set);
}

//native JsonHandle:json_real(Float:value);
static cell AMX_NATIVE_CALL AMX_JsonReal(AMX* amx, cell* params)
{
    return JsonFunc::create(amx, std::bind(json_real, amx_ctof(params[1])));
}

//native Float:json_real_value(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonRealValue(AMX* amx, cell* params)
{
    return JsonFunc::value(amx, params,  
        [](json_t* json)
        {
            return amx_ftoc(json_real_value(json));
        }
    );
}

//native Float:json_real_set(JsonHandle:handle, Float:value);
static cell AMX_NATIVE_CALL AMX_JsonRealSet(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, 
        [](json_t* json, cell value)
        {
            return json_real_set(json, amx_ctof(value));
        }
    );
}

//native JsonHandle:json_array();
static cell AMX_NATIVE_CALL AMX_JsonArray(AMX* amx, cell* params)
{
    return JsonFunc::create(amx, json_array);
}

//native json_array_size(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonArraySize(AMX* amx, cell* params)
{
    return JsonFunc::simple(amx, params, json_array_size);
}

//native JsonHandle:json_array_get(JsonHandle:handle, index);
static cell AMX_NATIVE_CALL AMX_JsonArrayGet(AMX* amx, cell* params)
{
    return JsonFunc::get(amx, params, 
        [](AMX* amx, json_t* json, cell* params)
        {
            return json_array_get(json, params[2]);
        }
    );
}

//native json_array_set(JsonHandle:handle, index, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArraySet(AMX* amx, cell* params)
{  
    return JsonFunc::set(amx, params, 
        [](json_t* json, cell index, cell value)
        {
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(value));

            if (!source)
            {
                return ERROR;
            }

            return json_array_set(json, index, source);
        }
    );
}

//native json_array_set_new(JsonHandle:handle, index, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArraySetNew(AMX* amx, cell* params)
{  
    return JsonFunc::set(amx, params, 
        [](json_t* json, cell index, HandleKey value)
        {
            HandleKey handle = static_cast<HandleKey>(value);
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

            if (!source)
            {
                return ERROR;
            }

            cell result = json_array_set(json, index, source);
            g_HandleTable->destroy(handle);
            return result;
        }
    );
}

//native json_array_append(JsonHandle:handle, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArrayAppend(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, 
        [](json_t* json, cell value)
        {
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(value));

            if (!source)
            {
                return ERROR;
            }

            return json_array_append(json, source);
        }
    );
}

//native json_array_append_new(JsonHandle:handle, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArrayAppendNew(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, 
        [](json_t* json, cell value)
        {
            HandleKey handle = static_cast<HandleKey>(value);
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

            if (!source)
            {
                return ERROR;
            }

            cell result = json_array_append(json, source);
            g_HandleTable->destroy(handle);
            return result;
        }
    );
}

//native json_array_insert(JsonHandle:handle, index, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArrayInsert(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, 
        [](json_t* json, cell index, cell value)
        {
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(value));

            if (!source)
            {
                return ERROR;
            }

            return json_array_insert(json, index, source);
        }
    );
}

//native json_array_insert_new(JsonHandle:handle, index, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArrayInsertNew(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, 
        [](json_t* json, cell index, cell value)
        {
            HandleKey handle = static_cast<HandleKey>(value);
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

            if (!source)
            {
                return ERROR;
            }

            cell result = json_array_insert(json, index, source);
            g_HandleTable->destroy(handle);
            return result; 
        }
    );
}

//native json_array_extend(JsonHandle:handle, JsonHandle:other);
static cell AMX_NATIVE_CALL AMX_JsonArrayExtend(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, 
        [](json_t* json, cell value)
        {
            HandleKey handle = static_cast<HandleKey>(value);
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

            if (!source)
            {
                return ERROR;
            }

            return json_array_extend(json, source); 
        }
    );
}

//native json_array_remove(JsonHandle:handle, index);
static cell AMX_NATIVE_CALL AMX_JsonArrayRemove(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, json_array_remove);
}

//native json_array_clear(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonArrayClear(AMX* amx, cell* params)
{
    return JsonFunc::simple(amx, params, json_array_clear) == JSON_SUCCESS;
}

//native JsonHandle:json_string(const value[]);
static cell AMX_NATIVE_CALL AMX_JsonString(AMX* amx, cell* params)
{
    return JsonFunc::create(amx, 
        std::bind(json_string, MF_GetAmxString(amx, params[1], 0, NULL)));
}

//native JsonHandle:json_stringn(const value[], len);
static cell AMX_NATIVE_CALL AMX_JsonStringn(AMX* amx, cell* params)
{
    return JsonFunc::create(amx, 
        std::bind(json_stringn, MF_GetAmxString(amx, params[1], 0, NULL), 
            params[2]));
}

//native json_string_value(JsonHandle:handle, buffer[], len);
static cell AMX_NATIVE_CALL AMX_JsonStringValue(AMX* amx, cell* params)
{
    return JsonFunc::value(amx, params,  
        [&](json_t* json)
        {
            const char* value = json_string_value(json);
            return MF_SetAmxString(amx, params[2], value, params[3]);
        }
    ) > 0;
}

//native json_string_length(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonStringLength(AMX* amx, cell* params)
{
    return JsonFunc::value(amx, params, json_string_length);
}

//native json_string_set(JsonHandle:handle, const value[]);
static cell AMX_NATIVE_CALL AMX_JsonStringSet(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, 
        [&](json_t* json, cell value)
        {
            char* data = MF_GetAmxString(amx, value, 0, NULL);
            return json_string_set(json, data);
        }
    );
}

//native JsonHandle:json_object();
static cell AMX_NATIVE_CALL AMX_JsonObject(AMX* amx, cell* params)
{
    return JsonFunc::create(amx, json_object);
}

//native json_object_size(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonObjectSize(AMX* amx, cell* params)
{
    return JsonFunc::simple(amx, params, json_object_size);
}

//native json_object_del(JsonHandle:handle, const key[]);
static cell AMX_NATIVE_CALL AMX_JsonObjectDel(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params, 
        [&amx](json_t* json, cell value)
        {
            char* key = MF_GetAmxString(amx, value, 0, NULL);
            return json_object_del(json, key);
        }
    );
}

//native JsonHandle:json_object_get(JsonHandle:handle, const key[]);
static cell AMX_NATIVE_CALL AMX_JsonObjectGet(AMX* amx, cell* params)
{
    return JsonFunc::get(amx, params, 
        [](AMX* amx, json_t* json, cell* params)
        {
            char* key = MF_GetAmxString(amx, params[2], 0, NULL);
            return json_object_get(json, key);
        }
    );
}

//native json_object_set(JsonHandle:handle, const key[], JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonObjectSet(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params,
        [&amx](json_t* json, cell amxkey, cell value)
        {
            char* key = MF_GetAmxString(amx, amxkey, 0, NULL);
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(value));

            if (!source)
            {
                return ERROR;
            }

            return json_object_set(json, key, source);
        }
    );
}

//native json_object_set_new(JsonHandle:handle, const key[], JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonObjectSetNew(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params,
        [&amx](json_t* json, cell amxkey, cell value)
        {
            HandleKey handle = static_cast<HandleKey>(value);
            char* key = MF_GetAmxString(amx, amxkey, 0, NULL);
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

            if (!source)
            {
                return ERROR;
            }

            cell result = json_object_set(json, key, source);
            g_HandleTable->destroy(handle);
            return result;
        }
    );
}

//native json_object_clear(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonObjectClear(AMX* amx, cell* params)
{
    return JsonFunc::simple(amx, params, json_object_clear) == JSON_SUCCESS;
}

//native JsonIter:json_object_iter(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonObjectIter(AMX* amx, cell* params)
{
    return JsonFunc::value(amx, params, 
        [&amx](json_t* json)
        {
            return JsonFunc::iter(amx, std::bind(json_object_iter, json));
        }
    );
}

//native JsonIter:json_object_iter_at(JsonHandle:handle, const key[])
static cell AMX_NATIVE_CALL AMX_JsonObjectIterAt(AMX* amx, cell* params)
{
    return JsonFunc::value(amx, params, 
        [&amx, &params](json_t* json)
        {
            char* key = MF_GetAmxString(amx, params[2], 0, NULL);
            return JsonFunc::iter(amx, std::bind(json_object_iter_at, json, key));
        }
    );
}

//native JsonIter:json_object_iter_next(JsonHandle:handle, JsonIter:iter);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterNext(AMX* amx, cell* params)
{
    return JsonFunc::value(amx, params, 
        [&amx, &params](json_t* json)
        {
            HandleKey iter = static_cast<HandleKey>(params[2]);
            auto object = reinterpret_cast<JsonIterT>(g_HandleTable->read(iter));

            if (!object)
            {
                MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", params[2]);
                return ERROR;
            }

            return JsonFunc::iter(amx, 
                std::bind(json_object_iter_next, json, object));
        }
    );
}

//native json_object_iter_key(JsonIter:iter, buf[], len);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterKey(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    auto object = reinterpret_cast<JsonIterT>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid iter handle: %d", handle);
        return ERROR;
    }

    return MF_SetAmxString(amx, params[2], 
        json_object_iter_key(object), params[3]) != ERROR;
}


//native JsonHandle:json_object_iter_value(JsonIter:iter);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterValue(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    auto object = reinterpret_cast<JsonIterT>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", params[1]);
        return ERROR;
    }

    return JsonFunc::create(amx, std::bind(json_object_iter_value, object));
}

//native json_object_iter_set(JsonHandle:handle, JsonIter:iter, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterSet(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params,
        [&amx](json_t* json, cell ihandle, cell value)
        {
            auto it = reinterpret_cast<JsonIterT>(g_HandleTable->read(ihandle));
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(value));

            if (!source || !it)
            {
                return ERROR;
            }

            return json_object_iter_set(json, it, source);
        }
    );
}


//native json_object_iter_set_new(JsonHandle:handle, JsonIter:iter, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterSetNew(AMX* amx, cell* params)
{
    return JsonFunc::set(amx, params,
        [&amx](json_t* json, cell ihandle, cell value)
        {
            HandleKey jhandle = static_cast<HandleKey>(value);
            auto it = reinterpret_cast<JsonIterT>(g_HandleTable->read(ihandle));
            auto source = reinterpret_cast<json_t*>(g_HandleTable->read(jhandle));

            if (!source || !it)
            {
                return ERROR;
            }

            cell result = json_object_iter_set(json, it, source);
            g_HandleTable->destroy(jhandle);
            return result;
        }
    );
}

//native JsonIter:json_object_key_to_iter(const key[]);
static cell AMX_NATIVE_CALL AMX_JsonObjectKeyToIter(AMX* amx, cell* params)
{
    return JsonFunc::iter(amx, std::bind(json_object_key_to_iter, 
        MF_GetAmxString(amx, params[1], 0, NULL)));
}

//native json_dumps(JsonHandle:handle, flags, buffer[], len);
static cell AMX_NATIVE_CALL AMX_JsonDumps(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    auto object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }
    //have to free it (see http://jansson.readthedocs.org/en/latest/apiref.html#encoding)
    char* result = json_dumps(object, params[2]);

    if (!result)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Json dumps null string");
        return ERROR;
    }

    MF_SetAmxString(amx, params[3], result, params[4]);
    free(result);

    return SUCCESS;
}

//native json_dump_file(JsonHandle:handle, const path[], flags);
static cell AMX_NATIVE_CALL AMX_JsonDumpFile(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    auto object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    char* path = MF_GetAmxString(amx, params[2], 0, NULL);
    return json_dump_file(object, path, params[3]) == JSON_SUCCESS;
}

//native JsonHandle:json_loads(const input[], flags, error[], len);
static cell AMX_NATIVE_CALL AMX_JsonLoads(AMX* amx, cell* params)
{   
    json_error_t error;
    char* input = MF_GetAmxString(amx, params[1], 0, NULL);

    json_t* root = json_loads(input, params[2], &error);
    
    if (!root)
    {
        MF_SetAmxString(amx, params[3], error.text, params[4]);
        return INVALID_HANDLE;
    }

    return JsonFunc::create(amx,
        [&root]()
        {
            return root;
        }
    );
}

//native JsonHandle:json_load_file(const path[], flags, error[], len);
static cell AMX_NATIVE_CALL AMX_JsonLoadFile(AMX* amx, cell* params)
{   
    json_error_t error;
    char* path = MF_GetAmxString(amx, params[1], 0, NULL);

    json_t* root = json_load_file(path, params[2], &error);
    
    if (!root)
    {
        MF_SetAmxString(amx, params[3], error.text, params[4]);
        return INVALID_HANDLE;
    }

    return JsonFunc::create(amx,
        [&root]()
        {
            return root;
        }
    );
}

AMX_NATIVE_INFO JSON_NATIVES[] = 
{
    {"destroy_handle",              AMX_DestroyHandle},

    {"json_typeof",                 AMX_JsonTypeof},
    {"json_true",                   AMX_JsonTrue},
    {"json_false",                  AMX_JsonFalse},
    {"json_null",                   AMX_JsonNull},

    {"json_integer",                AMX_JsonInteger},
    {"json_integer_value",          AMX_JsonIntegerValue},
    {"json_integer_set",            AMX_JsonIntegerSet},
    {"json_real",                   AMX_JsonReal},
    {"json_real_value",             AMX_JsonRealValue},
    {"json_real_set",               AMX_JsonRealSet},

    {"json_array",                  AMX_JsonArray},
    {"json_array_size",             AMX_JsonArraySize},
    {"json_array_get",              AMX_JsonArrayGet},
    {"json_array_set",              AMX_JsonArraySet},
    {"json_array_set_new",          AMX_JsonArraySetNew},
    {"json_array_append",           AMX_JsonArrayAppend},
    {"json_array_append_new",       AMX_JsonArrayAppendNew},
    {"json_array_insert",           AMX_JsonArrayInsert},
    {"json_array_insert_new",       AMX_JsonArrayInsertNew},
    {"json_array_extend",           AMX_JsonArrayExtend},
    {"json_array_remove",           AMX_JsonArrayRemove},
    {"json_array_clear",            AMX_JsonArrayClear},

    {"json_string",                 AMX_JsonString},
    {"json_string_value",           AMX_JsonStringValue},
    {"json_string_set",             AMX_JsonStringSet},
    {"json_string_length",          AMX_JsonStringLength},
    {"json_stringn",                AMX_JsonStringn},

    {"json_object",                 AMX_JsonObject},
    {"json_object_size",            AMX_JsonObjectSize},
    {"json_object_del",             AMX_JsonObjectDel},
    {"json_object_get",             AMX_JsonObjectGet},
    {"json_object_set",             AMX_JsonObjectSet},
    {"json_object_set_new",         AMX_JsonObjectSetNew},
    {"json_object_clear",           AMX_JsonObjectClear},
    {"json_object_iter",            AMX_JsonObjectIter},
    {"json_object_iter_at",         AMX_JsonObjectIterAt},
    {"json_object_iter_next",       AMX_JsonObjectIterNext},
    {"json_object_iter_key",        AMX_JsonObjectIterKey},
    {"json_object_iter_value",      AMX_JsonObjectIterValue},
    {"json_object_iter_set",        AMX_JsonObjectIterSet},
    {"json_object_iter_set_new",    AMX_JsonObjectIterSetNew},
    {"json_object_key_to_iter",     AMX_JsonObjectKeyToIter},

    {"json_dumps",                  AMX_JsonDumps},
    {"json_dump_file",              AMX_JsonDumpFile},

    {"json_loads",                  AMX_JsonLoads},
    {"json_load_file",              AMX_JsonLoadFile},

    {NULL,                          NULL}
};

void OnAmxxAttach()
{
    g_HandleTable = new HandleTable();
    MF_AddNatives(JSON_NATIVES);
}

void OnAmxxDetach()
{
    delete g_HandleTable;
}

void OnPluginsLoaded(void)
{

}