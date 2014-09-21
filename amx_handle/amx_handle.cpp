#include "amx_handle.hpp"

HandleTable::HandleTable()
{
	
}

HandleTable::~HandleTable()
{
	#if defined(SHOW_HANDLE_LEAK)

		size_t unclosedHandles = 0;

		for (size_t i = 0; i < table.size(); ++i)
		{
			if (destroy(i))
			{
				++unclosedHandles;
			}
		}

		printf("[JSON MODULE] : there are %u unclosed handles\n", unclosedHandles);

	#else

		for (size_t i = 0; i < table.size(); ++i)
		{
			destroy(i);
		}

	#endif
}

HandleKey HandleTable::create(HandleData ptr, IHandleDispatch* dispatch)
{	
	if (!ptr || !dispatch)
	{
		return INVALID_HANDLE;
	}

	HandleKey key = INIT_HANDLE_VALUE;
	QHandle handle = {ptr, dispatch, false};

	if (freeHandles.size())
	{
		key = freeHandles.front();
		freeHandles.pop();
		table[key] = handle;
	}
	else
	{
		table.push_back(handle);
		key = static_cast<HandleKey>(table.size() - 1);
	}

	return key;
}

bool HandleTable::destroy(const HandleKey& key)
{
	if (table.size() <= key || key == INVALID_HANDLE)
	{
		return false;
	}

	QHandle& handle = table[key];

	if (handle.free)
	{
		return false;
	}

	handle.dispatch->free(handle.ptr);
	handle.ptr = NULL;
	handle.dispatch = NULL;
	handle.free = true;

	freeHandles.push(key);

	return true;
}

HandleData HandleTable::read(const HandleKey& key)
{
	if (key >= table.size() || table[key].free)
	{
		return NULL;
	}
	return table[key].ptr;
}