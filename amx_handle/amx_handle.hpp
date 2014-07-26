#ifndef HANDLE_HPP
#define HANDLE_HPP

#include <map>
#include <stack>
#include <limits.h>

#define INVALID_HANDLE UINT_MAX
#define INIT_HANDLE_VALUE	0


typedef void* HandleData;
typedef unsigned int HandleKey;


class IHandleDispatch
{
public:

	virtual ~IHandleDispatch() {};
	virtual void free(HandleData data) = 0;
};


struct QHandle
{
	HandleData ptr;
	IHandleDispatch* dispatch;
};


class HandleTable
{
private:

	std::map<HandleKey, QHandle> table;
	std::stack<HandleKey> freeHandles;
	HandleKey ukey;
public:

	HandleTable();

	HandleKey create(HandleData ptr, IHandleDispatch* dispatch);
	HandleData read(const HandleKey& key);
	void destroy(const HandleKey& key);
	bool isValid(const HandleKey& key);
};

#endif // HANDLE_HPP
