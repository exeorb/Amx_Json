#ifndef HANDLE_HPP
#define HANDLE_HPP

#include <limits.h>
#include "sh_stack.h"
#include "CVector.h"

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
	bool free;
};


class HandleTable
{
private:

	CVector<QHandle> table;
	CStack<HandleKey> freeHandles;
public:

	HandleTable() = default;
	~HandleTable();

	HandleKey create(HandleData ptr, IHandleDispatch* dispatch);
	HandleData read(const HandleKey& key);
	bool destroy(const HandleKey& key);
};

#endif // HANDLE_HPP
