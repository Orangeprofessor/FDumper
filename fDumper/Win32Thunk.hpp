#include "pch.h"
#include <winnt.h>


#pragma pack(push, 1)
struct Thunk
{
	/*
	mov rax, pInstance
	mov gs:[0x28], rax
	mov rax, pFuncPtr
	jmp rax
	*/
	uint16_t mov1 = '\x48\xB8';
	void*    pInstance = nullptr;
	uint32_t fs1 = '\x65\x48\x89\x04';
	uint8_t  fs2 = 0x25;
	uint8_t  fs3 = FIELD_OFFSET(NT_TIB, ArbitraryUserPointer);
	uint8_t  fs4 = 0;
	uint16_t fs5 = 0;
	uint16_t mov2 = '\x48\xB8';
	void*    pFuncPtr = nullptr;
	uint16_t jmp1 = '\xFF\xE0';

	void setup(void* pInst, void* pFunc)
	{
		pInstance = pInst;
		pFuncPtr = pFunc;
	}
};
#pragma pack(pop)

template<typename fn, typename C>
class Win32Thunk;

template<typename R, typename... Args, typename C>
class Win32Thunk < R(__stdcall*)(Args...), C>
{
public:
	using TypeMember = R(C::*)(Args...);
	using TypeFree = R(__stdcall*)(Args...);

public:
	Win32Thunk(TypeMember pfn, C* pInstance) : pMethod(pfn), pInstance(pInstance)
	{
		DWORD dwOldProtect = 0;
		VirtualProtect(&thunk, sizeof(thunk), PAGE_EXECUTE_READWRITE, &dwOldProtect);
		thunk.setup(this, &Win32Thunk::Redirect);
	}

	static R __stdcall Redirect(Args... args)
	{
		auto self = reinterpret_cast<Win32Thunk*>(((PNT_TIB)NtCurrentTeb())->ArbitraryUserPointer);
		return (self->pInstance->*self->pMethod)(args...);	
	}

	TypeFree GetThunk() 
	{
		return reinterpret_cast<TypeFree>(&thunk);
	}

private:
	TypeMember pMethod = nullptr;
	C* pInstance = nullptr;
	Thunk thunk;
};