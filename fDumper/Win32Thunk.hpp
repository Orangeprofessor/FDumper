#include "pch.h"


// this was VERY annoying to figure out
#pragma pack(push, 1)
struct Thunk
{
	/*
	mov rax, pInstance
	mov gs:[0x28], rax
	mov rax, pFuncPtr
	jmp rax
	*/
	void* pInstace = nullptr;
	void* pFuncPtr = nullptr;
	uint16_t mov1 = '\x48\xB8';
	uint16_t mov2 = '\x48\xB8';
	uint16_t jmp1 = '\xFF\xE0';
	uint32_t fs1 = '\x65\x48\x89\x04';
	uint8_t fs2 = 0x25;
	uint8_t fs3 = FIELD_OFFSET(NT_TIB, ArbitraryUserPointer);
	uint8_t fs4 = 0x0;
	uint16_t fs5 = 0x0;

	void setup(void* pInst, void* pFunc)
	{
		pInstace = pInst;
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
	Win32Thunk(TypeMember pfn, C* pInstance) : 


private:
	TypeMember pMethod = nullptr;
	C* pInstace = nulptr;
	ThunkData 
};