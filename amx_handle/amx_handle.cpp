#include "amx_handle.hpp"

HandleTable::~HandleTable()
{
	for (size_t i = 0; i < table.size(); ++i)
	{
		destroy(i);
	}
}

HandleKey HandleTable::create(HandleData ptr, IHandleDispatch* dispatch)
{	
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

	auto handle = table[key];

	if (handle.free)
	{
		return false;
	}

	handle.dispatch->free(handle.ptr);
	handle.ptr = nullptr;
	handle.dispatch = nullptr;
	handle.free = true;

	freeHandles.push(key);

	return true;
}

HandleData HandleTable::read(const HandleKey& key)
{
	if (key >= table.size() || table[key].free)
	{
		return nullptr;
	}
	return table[key].ptr;
}