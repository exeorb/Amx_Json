#include "amx_handle.hpp"

HandleKey HandleTable::create(HandleData ptr, IHandleDispatch* dispatch)
{	
	auto key = ukey;
	if (freeHandles.size())
	{
		key = freeHandles.top();
		freeHandles.pop();
	}
	else
	{
		++ukey;
	}
	//tnx mr c++11 :)
	table[key] = {ptr, dispatch};
	return key;
}

HandleTable::HandleTable()
{
	ukey = INIT_HANDLE_VALUE;
}

bool HandleTable::isValid(const HandleKey& key)
{
	return table.find(key) != table.end();
}

void HandleTable::destroy(const HandleKey& key)
{
	auto iter = table.find(key);
	if (iter == table.end())
		return;
	auto handle = iter->second;
	handle.dispatch->free(handle.ptr);
	table.erase(iter);
	freeHandles.push(key);
}

HandleData HandleTable::read(const HandleKey& key)
{
	if (!isValid(key))
		return nullptr;
	return table[key].ptr;
}