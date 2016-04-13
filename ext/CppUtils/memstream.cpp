#include "memstream.h"


void MemStream::EnsureCapacity(int newCapacity){
	if(newCapacity > capacity){
		void* newBase = malloc(newCapacity);
		
		int writeOffset = VOID_PTR_DIST(writeHead, readHead);
		MemCpy(newBase, readHead, writeOffset);
		
		free(base);
		base = newBase;
		readHead = base;
		writeHead = VOID_PTR_ADD(base, writeOffset);
	}
}

void MemStream::WriteString(const char* str){
	int length = StrLen(str);
	
	WriteString(str, length);
}

void MemStream::WriteString(const char* str, int len){
	int newCapacity = capacity;
	int neededCapacity = VOID_PTR_DIST(writeHead, base) + len;
	while(newCapacity < neededCapacity){
		newCapacity = (newCapacity > 0) ? newCapacity * 2 : 16;
	}
	
	EnsureCapacity(newCapacity);
		
	MemCpy(writeHead, str, len);
	writeHead = VOID_PTR_ADD(writeHead, len);
}


#if defined(MEMSTREAM_TEST_MAIN)

struct TestStr{
	char name[5];
	int x;
	
	bool operator==(const TestStr& other){
		return StrEqualN(other.name, name, 5) && x == other.x;
	}
};

TestStr GetStruct(){
	TestStr c = {"ergg", -12345};
	
	return c;
}

int main(int argc, char** argv){
	
	{
		MemStream str;
		
		str.Write(1322);
		str.Write(-235.03423f);
		str.Write(17);
		str.Write(12);
		str.Write(13);
		str.Write(14);
		
		ASSERT(str.Read<int>() == 1322);
		ASSERT(str.Read<float>() == -235.03423f);
		ASSERT(str.Read<int>() == 17);
		
		int arr[3];
		
		str.ReadArray(arr, 3);
		for(int i = 0; i < 3; i++){
			ASSERT(arr[i] == 12 + i);
		}
	}
	
	{
		MemStream str;
		
		str.Write(1422);
		ASSERT(str.Read<int>() == 1422);
		
		for(int i = 0; i < 24; i++){
			str.Write(GetStruct());
			ASSERT(str.Read<TestStr>() == GetStruct());
		}
	}
	
	return 0;
}

#endif

