#include <amxmodx>
#include <json>

#define ERROR 	0
#define SUCCESS 1

new testN = 0

#define TEST_INIT(%1)	server_print("[Testing %s]", %1);	testN = 0

#define TEST_ASSERT(%1,%2)	server_print("test%d %s", ++testN, cell:%1==cell:%2 ? "OK" : "FAIL")

#define TEST_ASSERT_SUCCESS(%1)	TEST_ASSERT(%1, SUCCESS)

#define TEST_ASSERT_ERROR(%1)	TEST_ASSERT(%1, ERROR)

public plugin_modules()
{
	require_module("json")
}

stock json_test()
{
	/*
		for checking json_typeof and json is valid we can "kill two birds with one stone" 
		by using macro json_is_[type]
	*/
	{
		TEST_INIT("json_typeof")

		new JsonHandle:jtrue = json_true()
		new JsonHandle:jfalse = json_false()
		TEST_ASSERT_SUCCESS(json_is_true(jtrue))
		TEST_ASSERT_SUCCESS(json_is_false(jfalse))

		destroy_json(jtrue)
		destroy_json(jfalse)
	}
	
	{
		TEST_INIT("json_integer")

		new integer = 1234567
		new JsonHandle:jint = json_integer(integer)

		TEST_ASSERT_SUCCESS(json_is_integer(jint))
		TEST_ASSERT(json_integer_value(jint), integer)
		TEST_ASSERT_SUCCESS(json_integer_set(jint, ++integer))
		TEST_ASSERT(json_integer_value(jint), integer)

		destroy_json(jint)
	}

	{
		TEST_INIT("json_real")

		new Float:real = 0.123456789
		new JsonHandle:jreal = json_real(real)

		TEST_ASSERT_SUCCESS(json_is_real(jreal))
		TEST_ASSERT(json_real_value(jreal), real)

		real -= 40321.2

		TEST_ASSERT_SUCCESS(json_real_set(jreal, real))
		TEST_ASSERT(json_real_value(jreal), real)

		destroy_json(jreal)
	}

	{
		TEST_INIT("json_array p1")

		new size = 0
		new JsonHandle:jarray = json_array()
		new JsonHandle:jinserted = json_integer(21)

		TEST_ASSERT_SUCCESS(json_is_array(jarray))

		TEST_ASSERT(json_array_size(jarray), size)

		for (new i = 0; i < 5; ++i)
		{
			TEST_ASSERT_SUCCESS(json_array_append(jarray, jinserted))
			TEST_ASSERT(json_array_size(jarray), 1)
			TEST_ASSERT_SUCCESS(json_array_clear(jarray))
		}

		for (new i = 0; i < 5; ++i)
		{
			TEST_ASSERT_SUCCESS(json_array_append(jarray, jinserted))
			size = json_array_size(jarray)
			new JsonHandle:value = json_array_get(jarray, size - 1)
			TEST_ASSERT(json_integer_value(value), json_integer_value(jinserted))
			destroy_json(value)
		}

		TEST_ASSERT_SUCCESS(json_array_clear(jarray))

		destroy_json(jinserted)
		destroy_json(jarray)
	}

	{
		TEST_INIT("json_array p2")

		new JsonHandle:jarray = json_array()
		new JsonHandle:jint = json_integer(1)
		new JsonHandle:jreal = json_real(0.1232)
		new JsonHandle:j2array = json_array()
		new JsonHandle:jget = INVALID_JSON

		TEST_ASSERT_ERROR(json_array_set(jarray, 0, jint))
		/*json_array_get return new Handle or INVALID_HANDLE so you have not to
			do like this. But i know that returned value will be invalid so i can.
			By the way this is my module i can do what ever i want :p
		*/
		TEST_ASSERT_ERROR(json_is_integer(json_array_get(jarray, 0)))

		TEST_ASSERT_SUCCESS(json_array_append(jarray, jint))
		TEST_ASSERT_SUCCESS(json_array_insert(jarray, 0, jreal))

		jget = json_array_get(jarray, 0)
		TEST_ASSERT_SUCCESS(json_is_real(jget))
		destroy_json(jget)

		jget = json_array_get(jarray, 1)
		TEST_ASSERT_SUCCESS(json_is_integer(jget))
		destroy_json(jget)

		jget = json_array_get(jarray, 2)
		TEST_ASSERT(jget, INVALID_JSON)
		destroy_json(jget)

		json_array_append(j2array, jarray)
		TEST_ASSERT(json_array_size(j2array), 1)

		json_array_clear(j2array)
		json_array_clear(jarray)

		destroy_json(j2array)
		destroy_json(jreal)
		destroy_json(jint)
		destroy_json(jarray)
	}

	{
		TEST_INIT("json_array p3")

		new JsonHandle:jarray = json_array()

		for (new i = 0; i < 5; ++i)
		{
			TEST_ASSERT_SUCCESS(json_array_append_new(jarray, json_integer(i)))
		}

		for (new i = 0; i < json_array_size(jarray); ++i)
		{
			new JsonHandle:ret = json_array_get(jarray, i)
			TEST_ASSERT_SUCCESS(json_is_integer(ret))
			TEST_ASSERT(json_integer_value(ret), i)
			destroy_json(ret)
		}

		while (json_array_size(jarray))
		{
			TEST_ASSERT_SUCCESS(json_array_remove(jarray, 0))
		}

		destroy_json(jarray)
	}

	{
		TEST_INIT("json_array p4")

		new JsonHandle:jarray = json_array()
		new num = 5

		json_array_append_new(jarray, json_real(12.221321))

		for (new i = 0; i <= num; ++i)
		{
			TEST_ASSERT_SUCCESS(json_array_set_new(jarray, 0, json_integer(i)))
		}

		TEST_ASSERT(json_array_size(jarray), 1)
		new JsonHandle:ret = json_array_get(jarray, 0)
		TEST_ASSERT_SUCCESS(json_is_integer(ret))
		TEST_ASSERT(json_integer_value(ret), num)

		destroy_json(ret)
		destroy_json(jarray)
	}

	{
		TEST_INIT("json_array p5")

		new num = 5
		new JsonHandle:jarray = json_array()
		new JsonHandle:jarray2 = json_array()

		for (new i = 0; i < num; ++i)
		{
			json_array_append_new(jarray, json_real(12.221321 + i))
		}
		
		json_array_append_new(jarray2, json_integer(111))

		TEST_ASSERT_SUCCESS(json_array_extend(jarray2, jarray))

		TEST_ASSERT(json_array_size(jarray2), (num + 1))

		new i = 0
		new JsonHandle:value = INVALID_JSON

		json_array_foreach(jarray, i, value)
		{
			TEST_ASSERT_SUCCESS(json_is_real(value))
			TEST_ASSERT(json_real_value(value), (12.221321 + i))
			//has to be closed (see `json.inc`)
			destroy_json(value)
		}

		TEST_ASSERT(i, num)

		destroy_json(jarray)
		destroy_json(jarray2)
	}

	{
		TEST_INIT("json_string")

		new buffer[100]
		new src[100]
		format(src, sizeof(src), "%s", "ababaca")

		new JsonHandle:jstr = json_string(src)

		TEST_ASSERT_SUCCESS(json_is_string(jstr))
		TEST_ASSERT_SUCCESS(json_string_value(jstr, buffer, sizeof(buffer)))
		TEST_ASSERT_SUCCESS(equal(buffer, src))

		TEST_ASSERT_SUCCESS(json_string_set(jstr, "ababaca1"))
		TEST_ASSERT_SUCCESS(json_string_value(jstr, buffer, sizeof(buffer)))
		TEST_ASSERT_ERROR(equal(buffer, src))

		destroy_json(jstr)

		format(src, sizeof(src), "%s", "mega_test_ababaca")

		jstr = json_stringn(src, 9)
		TEST_ASSERT_SUCCESS(json_is_string(jstr))
		TEST_ASSERT(json_string_length(jstr), 9)

		destroy_json(jstr)
	}

	{
		TEST_INIT("json_object p1")

		new JsonHandle:jroot = json_object()

		TEST_ASSERT_SUCCESS(json_is_object(jroot))

		TEST_ASSERT(json_object_size(jroot), 0)

		new JsonHandle:jarray = json_array()
		TEST_ASSERT_SUCCESS(json_array_append_new(jarray, json_string("super")))
		TEST_ASSERT_SUCCESS(json_array_append_new(jarray, json_string("test")))
		TEST_ASSERT_SUCCESS(json_array_append_new(jarray, json_integer(1111)))

		TEST_ASSERT_SUCCESS(json_object_set(jroot, "mega", jarray))
		destroy_json(jarray)
		jarray = json_object_get(jroot, "mega")

		TEST_ASSERT_SUCCESS(json_is_array(jarray))
		TEST_ASSERT(json_array_size(jarray), 3)

		TEST_ASSERT_SUCCESS(json_object_del(jroot, "mega"))
		TEST_ASSERT(json_object_size(jroot), 0)

		destroy_json(jroot)
		destroy_json(jarray)
	}

	{
		TEST_INIT("json_object p2")

		new JsonHandle:jroot = json_object()

		TEST_ASSERT_SUCCESS(json_object_set_new(jroot, "1233", json_object()))
		TEST_ASSERT_SUCCESS(json_object_clear(jroot))
		TEST_ASSERT(json_object_size(jroot), 0)

		destroy_json(jroot)
	}


	{
		TEST_INIT("json object iter p1")

		new JsonHandle:root = json_object()
		/*if root will be invalid, MF_LogError will be called*/
		new JsonIter:iter = json_object_iter(root)

		TEST_ASSERT(iter, INVALID_ITER)

		json_object_set_new(root, "i", json_integer(1))
		json_object_set_new(root, "t", json_integer(3))
		json_object_set_new(root, "e", json_integer(4))
		json_object_set_new(root, "r", json_integer(2))

		iter = json_object_iter(root)

		new key[32]

		while (iter != INVALID_ITER)
		{
			TEST_ASSERT_SUCCESS(json_object_iter_key(iter, key, sizeof(key)))

			new JsonHandle:value = json_object_iter_value(iter)

			TEST_ASSERT_SUCCESS(json_is_integer(value))

			destroy_iter(iter)

			iter = json_object_iter_at(root, key)

			new JsonIter:next = json_object_iter_next(root, iter)
			destroy_iter(iter)
			iter = next
			destroy_json(value)
		}

		destroy_json(root)
		destroy_iter(iter)
	}

	{
		// TEST_INIT("json object iter p2")

		//to do test on set, set_new, key_to_iter
	}

	{
		TEST_INIT("json encode/decode")

		new buffer[128]
		new JsonHandle:jroot = json_object()
		new JsonHandle:jpref = json_object()
		new JsonHandle:tmp = INVALID_JSON

		
		TEST_ASSERT_SUCCESS(json_object_set_new(jpref, "start_round_sound", json_false()))
		TEST_ASSERT_SUCCESS(json_object_set_new(jpref, "start_quake_sound", json_true()))
		TEST_ASSERT_SUCCESS(json_object_set_new(jpref, "last_time", json_integer(get_systime())))
		TEST_ASSERT_SUCCESS(json_object_set(jroot, "steam_id:3232321", jpref))

		TEST_ASSERT_SUCCESS(json_dumps(jroot, JSON_ENCODE_ANY, buffer, sizeof(buffer)))

		// TEST_ASSERT_SUCCESS(json_dump_file(jroot, "test", JSON_INDENT(2))

		destroy_json(jroot)
		destroy_json(jpref)

		new ebuf[32]

		jroot = json_loads(buffer, JSON_REJECT_DUPLICATES, ebuf, sizeof(ebuf))
		TEST_ASSERT_SUCCESS(json_is_object(jroot))

		jpref = json_object_get(jroot, "steam_id:3232321")
		TEST_ASSERT_SUCCESS(json_is_object(jpref))

		tmp = json_object_get(jpref, "start_round_sound")
		TEST_ASSERT_SUCCESS(json_is_false(tmp))
		destroy_json(tmp)

		tmp = json_object_get(jpref, "start_quake_sound")
		TEST_ASSERT_SUCCESS(json_is_true(tmp))
		destroy_json(tmp)

		tmp = json_object_get(jpref, "last_time")
		TEST_ASSERT_SUCCESS(json_is_integer(tmp))
		destroy_json(tmp)

		destroy_json(jroot)
		destroy_json(jpref)
	}
}

public plugin_init()
{
	register_plugin("json_unit_test", "1.1", "Exeorb Dev Team")

	json_test()
}