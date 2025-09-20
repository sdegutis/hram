#include <intsafe.h>
#include <asmjit/host.h>
#include <asmtk/asmtk.h>
#include <string>

extern "C" const char* assemble_string(void* dst, size_t* dst_size, char* src) {
	asmjit::Environment env(asmjit::Arch::kHost);
	asmjit::CodeHolder code;
	code.init(env, (uint64_t)dst);
	asmjit::x86::Assembler a(&code);
	asmtk::AsmParser p(&a);

	asmjit::Error err = p.parse(src);
	if (err) return asmjit::DebugUtils::errorAsString(err);

	err = code.flatten();
	if (err) return asmjit::DebugUtils::errorAsString(err);

	err = code.copyFlattenedData(dst, *dst_size, asmjit::CopySectionFlags::kPadSectionBuffer);
	if (err) return asmjit::DebugUtils::errorAsString(err);

	*dst_size = code.codeSize();
	return NULL;
}

void* BASEPTR;
UINT64 BASEPTRV;

#include <Windows.h>



extern "C"
__declspec(noinline)
int mytestfn2(float a, int x1, int x2, int x3, int x4, int x5, int x6, int x7, unsigned long long b, char* c, char* d) {
	printf("BASEPTR = [%llu]\n", BASEPTR);
	printf("BASEPTRV = [%llu]\n", BASEPTRV);

	printf("a = [%f]\n", a);
	printf("b = [%llu]\n", b);
	printf("c = [%s]\n", c);
	printf("d = [%s]\n", d);

	MessageBoxA(NULL, "hey world", "cap thing", 0);

	return a + b;
}

class Assembler {

	void* mem = VirtualAlloc(NULL, 40, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	UINT8* bytes = (UINT8*)mem;
	int i = 0;

public:

	void movRaxTo(UINT64 val) {
		if (val > 0x7fffffff) {
			// 10 bytes
			bytes[i++] = 0x48;
			bytes[i++] = 0xA3;
			*(UINT64*)(bytes + i) = val;
			i += sizeof(UINT64);
		}
		else {
			// 8 bytes
			bytes[i++] = 0x48;
			bytes[i++] = 0x89;
			bytes[i++] = 0x04;
			bytes[i++] = 0x25;
			*(UINT32*)(bytes + i) = val;
			i += sizeof(UINT32);
		}
	}

	void movToRax(UINT64 val) {
		if (val > 0x7fffffff) {
			// 48 B8 addr(8 bytes, 64 bits)
			// 10 bytes
			bytes[i++] = 0x48;
			bytes[i++] = 0xb8;
			*(UINT64*)(bytes + i) = val;
			i += sizeof(UINT64);
		}
		else {
			// 48 C7 C0 addr(4 bytes, 32 bits)
			// 7 bytes
			bytes[i++] = 0x48;
			bytes[i++] = 0xc7;
			bytes[i++] = 0xc0;
			*(UINT32*)(bytes + i) = val;
			i += sizeof(UINT32);
		}
	}

	void jmpToRax() {
		bytes[i++] = 0xff;
		bytes[i++] = 0xe0;
	}

	template<typename T>
	T memory() {
		return (T)mem;
	}

};

extern "C" void my_test_fn() {
	Assembler as;
	as.movToRax(1234);
	as.movRaxTo((UINT64)&BASEPTR);
	as.movToRax(5678);
	as.movRaxTo((UINT64)&BASEPTRV);
	as.movToRax((UINT64)mytestfn2);
	as.jmpToRax();
	auto fnptr = as.memory<int (*)(float a, int x1, int x2, int x3, int x4, int x5, int x6, int x7, unsigned long long b, const char* c, const char* d)>();

	int res = fnptr(3.12, 0, 0, 0, 0, 0, 0, 1, 245, "hi", "world");

	printf("res = %d\n", res);
}
