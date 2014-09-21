#include "json_module.hpp" 
#include <jansson.h>
#include <amx_handle.hpp>

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
v 1.3 - no stl (see https://forums.alliedmods.net/showthread.php?t=244605&page=2)
*/

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
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return json_typeof(object);
}

//native JsonHandle:json_true();
static cell AMX_NATIVE_CALL AMX_JsonTrue(AMX* amx, cell* params)
{
    json_t* object = json_true();

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler); 
}

//native JsonHandle:json_false();
static cell AMX_NATIVE_CALL AMX_JsonFalse(AMX* amx, cell* params)
{
    json_t* object = json_false();

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
}

//native JsonHandle:json_null();
static cell AMX_NATIVE_CALL AMX_JsonNull(AMX* amx, cell* params)
{
    json_t* object = json_null();

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
}

//native JsonHandle:json_integer(value);
static cell AMX_NATIVE_CALL AMX_JsonInteger(AMX* amx, cell* params)
{
    json_t* object = json_integer(params[1]);

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
}

//native json_integer_value(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonIntegerValue(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return json_integer_value(object);
}

//native json_integer_set(JsonHandle:handle, value);
static cell AMX_NATIVE_CALL AMX_JsonIntegerSet(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return json_integer_set(object, params[2]) == JSON_SUCCESS;
}

//native JsonHandle:json_real(Float:value);
static cell AMX_NATIVE_CALL AMX_JsonReal(AMX* amx, cell* params)
{
    json_t* object = json_real(amx_ctof(params[1]));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
}

//native Float:json_real_value(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonRealValue(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return amx_ftoc(json_real_value(object));
}

//native Float:json_real_set(JsonHandle:handle, Float:value);
static cell AMX_NATIVE_CALL AMX_JsonRealSet(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return json_real_set(object, amx_ctof(params[2])) == JSON_SUCCESS;
}

//native JsonHandle:json_array();
static cell AMX_NATIVE_CALL AMX_JsonArray(AMX* amx, cell* params)
{
    json_t* object = json_array();

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
}

//native json_array_size(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonArraySize(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return json_array_size(object);
}

/*in get functions there is a borrowed reference, 
but have to create a new json wrapper with this reference and user have to close it
so we have to increase reference on object  */

//native JsonHandle:json_array_get(JsonHandle:handle, index);
static cell AMX_NATIVE_CALL AMX_JsonArrayGet(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return INVALID_HANDLE;
    }

    json_t* result = json_array_get(object, params[2]);

    if (!result)
    {
        return INVALID_HANDLE;
    }

    json_incref(result);

    return g_HandleTable->create(result, &g_JsonTypeHandler);
}

//native json_array_set(JsonHandle:handle, index, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArraySet(AMX* amx, cell* params)
{  
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hvalue = static_cast<HandleKey>(params[3]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    return json_array_set(object, params[2], value) == JSON_SUCCESS;
}

//native json_array_set_new(JsonHandle:handle, index, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArraySetNew(AMX* amx, cell* params)
{  
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hvalue = static_cast<HandleKey>(params[3]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    return json_array_set(object, params[2], value) == JSON_SUCCESS && 
        g_HandleTable->destroy(hvalue);
}

//native json_array_append(JsonHandle:handle, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArrayAppend(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hvalue = static_cast<HandleKey>(params[2]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    return json_array_append(object, value) == JSON_SUCCESS;
}

//native json_array_append_new(JsonHandle:handle, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArrayAppendNew(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hvalue = static_cast<HandleKey>(params[2]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    return json_array_append(object, value) == JSON_SUCCESS && 
        g_HandleTable->destroy(hvalue);
}

//native json_array_insert(JsonHandle:handle, index, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArrayInsert(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hvalue = static_cast<HandleKey>(params[3]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    return json_array_insert(object, params[2], value) == JSON_SUCCESS;
}

//native json_array_insert_new(JsonHandle:handle, index, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonArrayInsertNew(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hvalue = static_cast<HandleKey>(params[3]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    return json_array_insert(object, params[2], value) == JSON_SUCCESS &&
        g_HandleTable->destroy(hvalue);
}

//native json_array_extend(JsonHandle:handle, JsonHandle:other);
static cell AMX_NATIVE_CALL AMX_JsonArrayExtend(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hvalue = static_cast<HandleKey>(params[2]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    return json_array_extend(object, value) == JSON_SUCCESS;
}

//native json_array_remove(JsonHandle:handle, index);
static cell AMX_NATIVE_CALL AMX_JsonArrayRemove(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return json_array_remove(object, params[2]) == JSON_SUCCESS;
}

//native json_array_clear(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonArrayClear(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return json_array_clear(object) == JSON_SUCCESS;
}

//native JsonHandle:json_string(const value[]);
static cell AMX_NATIVE_CALL AMX_JsonString(AMX* amx, cell* params)
{
    json_t* object = json_string(MF_GetAmxString(amx, params[1], 0, NULL));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
}

//native JsonHandle:json_stringn(const value[], len);
static cell AMX_NATIVE_CALL AMX_JsonStringn(AMX* amx, cell* params)
{
    json_t* object = json_stringn(MF_GetAmxString(amx, params[1], 0, NULL), params[2]);

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
}

//native json_string_value(JsonHandle:handle, buffer[], len);
static cell AMX_NATIVE_CALL AMX_JsonStringValue(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    const char* value = json_string_value(object);

    return MF_SetAmxString(amx, params[2], value, params[3]) > 0;
}

//native json_string_length(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonStringLength(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }
    
    return json_string_length(object);
}

//native json_string_set(JsonHandle:handle, const value[]);
static cell AMX_NATIVE_CALL AMX_JsonStringSet(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }
        
    char* data = MF_GetAmxString(amx, params[2], 0, NULL);

    return json_string_set(object, data) == JSON_SUCCESS;
}

//native JsonHandle:json_object();
static cell AMX_NATIVE_CALL AMX_JsonObject(AMX* amx, cell* params)
{
    json_t* object = json_object();

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
}

//native json_object_size(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonObjectSize(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return json_object_size(object);
}

//native json_object_del(JsonHandle:handle, const key[]);
static cell AMX_NATIVE_CALL AMX_JsonObjectDel(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    char* key = MF_GetAmxString(amx, params[2], 0, NULL);

    return json_object_del(object, key) == JSON_SUCCESS;
}

//native JsonHandle:json_object_get(JsonHandle:handle, const key[]);
static cell AMX_NATIVE_CALL AMX_JsonObjectGet(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return INVALID_HANDLE;
    }

    char* key = MF_GetAmxString(amx, params[2], 0, NULL);
    json_t* result = json_object_get(object, key);

    if (!result)
    {
        return INVALID_HANDLE;
    }

    json_incref(result);

    return g_HandleTable->create(result, &g_JsonTypeHandler);
}

//native json_object_set(JsonHandle:handle, const key[], JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonObjectSet(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hvalue = static_cast<HandleKey>(params[3]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    char* key = MF_GetAmxString(amx, params[2], 0, NULL);

    return json_object_set(object, key, value) == JSON_SUCCESS;
}

//native json_object_set_new(JsonHandle:handle, const key[], JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonObjectSetNew(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hvalue = static_cast<HandleKey>(params[3]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    char* key = MF_GetAmxString(amx, params[2], 0, NULL);

    return json_object_set(object, key, value) == JSON_SUCCESS && 
        g_HandleTable->destroy(hvalue);
}

//native json_object_clear(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonObjectClear(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    return json_object_clear(object) == JSON_SUCCESS;
}

//native JsonIter:json_object_iter(JsonHandle:handle);
static cell AMX_NATIVE_CALL AMX_JsonObjectIter(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    JsonIterT iter = json_object_iter(object);

    if (!iter)
    {
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(iter, &g_JsonIterTypeHandler);
}

//native JsonIter:json_object_iter_at(JsonHandle:handle, const key[])
static cell AMX_NATIVE_CALL AMX_JsonObjectIterAt(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    char* key = MF_GetAmxString(amx, params[2], 0, NULL);
    JsonIterT iter = json_object_iter_at(object, key);

    if (!iter)
    {
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(iter, &g_JsonIterTypeHandler);
}

//native JsonIter:json_object_iter_next(JsonHandle:handle, JsonIter:iter);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterNext(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    HandleKey hiter = static_cast<HandleKey>(params[2]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    JsonIterT iter = reinterpret_cast<JsonIterT>(g_HandleTable->read(hiter));

    if (!iter)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hiter);
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(json_object_iter_next(object, iter),
         &g_JsonIterTypeHandler);
}

//native json_object_iter_key(JsonIter:iter, buf[], len);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterKey(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    JsonIterT iter = reinterpret_cast<JsonIterT>(g_HandleTable->read(handle));

    if (!iter)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid iter handle: %d", handle);
        return ERROR;
    }

    return MF_SetAmxString(amx, params[2], 
        json_object_iter_key(iter), params[3]) != ERROR;
}

//native JsonHandle:json_object_iter_value(JsonIter:iter);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterValue(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    JsonIterT iter = reinterpret_cast<JsonIterT>(g_HandleTable->read(handle));

    if (!iter)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    json_t* value = json_object_iter_value(iter);

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create json handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(value, &g_JsonTypeHandler);
}

//native json_object_iter_set(JsonHandle:handle, JsonIter:iter, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterSet(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]),
        hiter = static_cast<HandleKey>(params[2]),
        hvalue = static_cast<HandleKey>(params[3]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    JsonIterT iter = reinterpret_cast<JsonIterT>(g_HandleTable->read(hiter));

    if (!iter)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hiter);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    return json_object_iter_set(object, iter, value) == JSON_SUCCESS;
}

//native json_object_iter_set_new(JsonHandle:handle, JsonIter:iter, JsonHandle:value);
static cell AMX_NATIVE_CALL AMX_JsonObjectIterSetNew(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]),
        hiter = static_cast<HandleKey>(params[2]),
        hvalue = static_cast<HandleKey>(params[3]);

    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

    if (!object)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", handle);
        return ERROR;
    }

    JsonIterT iter = reinterpret_cast<JsonIterT>(g_HandleTable->read(hiter));

    if (!iter)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hiter);
        return ERROR;
    }

    json_t* value = reinterpret_cast<json_t*>(g_HandleTable->read(hvalue));

    if (!value)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid handle: %d", hvalue);
        return ERROR;
    }

    return json_object_iter_set(object, iter, value) == JSON_SUCCESS &&
        g_HandleTable->destroy(hvalue);
}

//native JsonIter:json_object_key_to_iter(const key[]);
static cell AMX_NATIVE_CALL AMX_JsonObjectKeyToIter(AMX* amx, cell* params)
{
    JsonIterT iter = json_object_key_to_iter(MF_GetAmxString(amx, params[1], 0, NULL));

    if (!iter)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Couldn't create iter handle");
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(iter, &g_JsonIterTypeHandler);
}

//native json_dumps(JsonHandle:handle, flags, buffer[], len);
static cell AMX_NATIVE_CALL AMX_JsonDumps(AMX* amx, cell* params)
{
    HandleKey handle = static_cast<HandleKey>(params[1]);
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

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
    json_t* object = reinterpret_cast<json_t*>(g_HandleTable->read(handle));

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

    json_t* object = json_loads(input, params[2], &error);
    
    if (!object)
    {
        MF_SetAmxString(amx, params[3], error.text, params[4]);
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
}

//native JsonHandle:json_load_file(const path[], flags, error[], len);
static cell AMX_NATIVE_CALL AMX_JsonLoadFile(AMX* amx, cell* params)
{   
    json_error_t error;
    char* path = MF_GetAmxString(amx, params[1], 0, NULL);

    json_t* object = json_load_file(path, params[2], &error);
    
    if (!object)
    {
        MF_SetAmxString(amx, params[3], error.text, params[4]);
        return INVALID_HANDLE;
    }

    return g_HandleTable->create(object, &g_JsonTypeHandler);
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