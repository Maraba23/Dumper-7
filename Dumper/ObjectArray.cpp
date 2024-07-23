#include <iostream>
#include <fstream>
#include <format>
#include <filesystem>
#include "ObjectArray.h"
#include "Offsets.h"
#include "Utils.h"

namespace fs = std::filesystem;

/* Scuffed stuff up here */
struct FChunkedFixedUObjectArray
{
	void** ObjectsAbove;
	uint8_t Pad_0[0x08];
	int32_t MaxElements;
	int32_t NumElements;
	int32_t MaxChunks;
	int32_t NumChunks;
	void** ObjectsBelow;

	inline int32 IsValid(int32& OutObjectsPtrOffset)
	{
		void** ObjectsAboveButDecrypted = (void**)ObjectArray::DecryptPtr(ObjectsAbove);
		void** ObjectsBelowButDecrypted = (void**)ObjectArray::DecryptPtr(ObjectsBelow);

		if (NumChunks > 0x14 || NumChunks < 0x1)
			return false;

		if (MaxChunks > 0x22F || MaxChunks < 0x6)
			return false;

		if (NumElements > MaxElements || NumChunks > MaxChunks)
			return false;

		if (((NumElements / 0x10000) + 1) != NumChunks || (MaxElements / 0x10000) != MaxChunks)
			return false;

		const bool bAreObjectsAboveValid = (ObjectsAboveButDecrypted && !IsBadReadPtr(ObjectsAboveButDecrypted));
		const bool bAreObjectsBewlowValid = (ObjectsBelowButDecrypted && !IsBadReadPtr(ObjectsBelowButDecrypted));

		if (!bAreObjectsAboveValid && !bAreObjectsBewlowValid)
			return false;

		for (int i = 0; i < NumChunks; i++)
		{
#pragma warning(disable:6011)
			const bool bIsCurrentIndexValidAbove = bAreObjectsAboveValid ? !IsBadReadPtr(ObjectsAboveButDecrypted[i]) : false;
			const bool bIsCurrentIndexValidBelow = bAreObjectsBewlowValid ? !IsBadReadPtr(ObjectsBelowButDecrypted[i]) : false;
#pragma pop

			if (!bIsCurrentIndexValidAbove && !bIsCurrentIndexValidBelow)
				return false;
		}

		OutObjectsPtrOffset = 0x00;

		if (!bAreObjectsAboveValid && bAreObjectsBewlowValid)
			OutObjectsPtrOffset = 0x20;

		return true;
	}
};

struct FFixedUObjectArray
{
	struct FUObjectItem
	{
		void* Object;
		uint8_t Pad[0x10];
	};

	FUObjectItem* Objects;
	int32_t Max;
	int32_t Num;

	inline bool IsValid()
	{
		FUObjectItem* ObjectsButDecrypted = (FUObjectItem*)ObjectArray::DecryptPtr(Objects);

		if (Num > Max)
			return false;

		if (Max > 0x400000)
			return false;

		if (Num < 0x1000)
			return false;

		if (IsBadReadPtr(ObjectsButDecrypted))
			return false;

		if (IsBadReadPtr(ObjectsButDecrypted[5].Object))
			return false;

		if (*(int32_t*)(uintptr_t(ObjectsButDecrypted[5].Object) + 0xC) != 5)
			return false;

		return true;
	}
};



uint8* ObjectArray::GObjects = nullptr;
uint32 ObjectArray::NumElementsPerChunk = 0x10000;
uint32 ObjectArray::SizeOfFUObjectItem = 0x18;
uint32 ObjectArray::FUObjectItemInitialOffset = 0x0;
std::wstring ObjectArray::DecryptionLambdaStr = L"";

void ObjectArray::InitializeFUObjectItem(uint8_t* FirstItemPtr)
{
	for (int i = 0x0; i < 0x10; i += 4)
	{
		if (!IsBadReadPtr(*reinterpret_cast<uint8_t**>(FirstItemPtr + i)))
		{
			FUObjectItemInitialOffset = i;
			break;
		}
	}

	for (int i = FUObjectItemInitialOffset + 0x8; i <= 0x38; i += 4)
	{
		void* SecondObject = *reinterpret_cast<uint8**>(FirstItemPtr + i);
		void* ThirdObject  = *reinterpret_cast<uint8**>(FirstItemPtr + (i * 2) - FUObjectItemInitialOffset);

		if (!IsBadReadPtr(SecondObject) && !IsBadReadPtr(*reinterpret_cast<void**>(SecondObject)) && !IsBadReadPtr(ThirdObject) && !IsBadReadPtr(*reinterpret_cast<void**>(ThirdObject)))
		{
			SizeOfFUObjectItem = i - FUObjectItemInitialOffset;
			break;
		}
	}

	Off::InSDK::ObjArray::FUObjectItemInitialOffset = FUObjectItemInitialOffset;
	Off::InSDK::ObjArray::FUObjectItemSize = SizeOfFUObjectItem;
}

void ObjectArray::InitDecryption(uint8_t* (*DecryptionFunction)(void* ObjPtr), const wchar_t* DecryptionLambdaAsStr)
{
	DecryptPtr = DecryptionFunction;
	DecryptionLambdaStr = DecryptionLambdaAsStr;
}

void ObjectArray::InitializeChunkSize(uint8_t* ChunksPtr)
{
	int IndexOffset = 0x0;
	uint8* ObjAtIdx374 = (uint8*)ByIndex(ChunksPtr, 0x374, SizeOfFUObjectItem, FUObjectItemInitialOffset, 0x10000);
	uint8* ObjAtIdx106 = (uint8*)ByIndex(ChunksPtr, 0x106, SizeOfFUObjectItem, FUObjectItemInitialOffset, 0x10000);

	for (int i = 0x8; i < 0x20; i++)
	{
		if (*reinterpret_cast<int32*>(ObjAtIdx374 + i) == 0x374 && *reinterpret_cast<int32*>(ObjAtIdx106 + i) == 0x106)
		{
			IndexOffset = i;
			break;
		}
	}

	int IndexToCheck = 0x10400;
	while (ObjectArray::Num() > IndexToCheck)
	{
		if (void* Obj = ByIndex(ChunksPtr, IndexToCheck, SizeOfFUObjectItem, FUObjectItemInitialOffset, 0x10000))
		{
			const bool bIsTrue = *reinterpret_cast<int32*>((uint8*)Obj + IndexOffset) != IndexToCheck;
			NumElementsPerChunk = bIsTrue ? 0x10400 : 0x10000;
			break;
		}
		IndexToCheck += 0x10400;
	}

	Off::InSDK::ObjArray::ChunkSize = NumElementsPerChunk;
}

/* We don't speak about this function... */
void ObjectArray::Init(bool bScanAllMemory)
{
	if (!bScanAllMemory)
		std::wcout << L"\nDumper-7 by me, you & him\n\n\n";

	const auto [ImageBase, ImageSize] = GetImageBaseAndSize();

	uintptr_t SearchBase = ImageBase;
	DWORD SearchRange = ImageSize;

	if (!bScanAllMemory)
	{
		const auto [DataSection, DataSize] = GetSectionByName(ImageBase, ".data");

		if (DataSection != 0x0 && DataSize != 0x0)
		{
			SearchBase = DataSection;
			SearchRange = DataSize;
		}
		else
		{
			bScanAllMemory = true;
		}
	}

	/* Sub 0x50 so we don't try to read out of bounds memory when checking FixedArray->IsValid() or ChunkedArray->IsValid() */
	SearchRange -= 0x50;

	if (!bScanAllMemory)
		std::wcout << L"Searching for GObjects...\n\n";

	for (int i = 0; i < SearchRange; i += 0x4)
	{
		auto FixedArray = reinterpret_cast<FFixedUObjectArray*>(SearchBase + i);
		auto ChunkedArray = reinterpret_cast<FChunkedFixedUObjectArray*>(SearchBase + i);

		if (FixedArray->IsValid())
		{
			GObjects = reinterpret_cast<uint8_t*>(SearchBase + i);
			Off::FUObjectArray::Num = 0xC;
			NumElementsPerChunk = -1;

			Off::InSDK::ObjArray::GObjects = (SearchBase + i) - ImageBase;

			std::wcout << L"Found FFixedUObjectArray GObjects at offset 0x" << std::hex << Off::InSDK::ObjArray::GObjects << std::dec << L"\n\n";

			ByIndex = [](void* ObjectsArray, int32 Index, uint32 FUObjectItemSize, uint32 FUObjectItemOffset, uint32 PerChunk) -> void*
			{
				if (Index < 0 || Index > Num())
					return nullptr;

				uint8_t* ChunkPtr = DecryptPtr(*reinterpret_cast<uint8_t**>(ObjectsArray));

				return *reinterpret_cast<void**>(ChunkPtr + FUObjectItemOffset + (Index * FUObjectItemSize));
			};

			uint8_t* ChunksPtr = DecryptPtr(*reinterpret_cast<uint8_t**>(GObjects + Off::FUObjectArray::Ptr));

			ObjectArray::InitializeFUObjectItem(*reinterpret_cast<uint8_t**>(ChunksPtr));

			return;
		}
		else if (ChunkedArray->IsValid(Off::FUObjectArray::Ptr))
		{
			GObjects = reinterpret_cast<uint8_t*>(SearchBase + i);
			NumElementsPerChunk = 0x10000;
			SizeOfFUObjectItem = 0x18;
			Off::FUObjectArray::Num = 0x14;
			FUObjectItemInitialOffset = 0x0;

			Off::InSDK::ObjArray::GObjects = (SearchBase + i) - ImageBase;

			std::wcout << L"Found FChunkedFixedUObjectArray GObjects at offset 0x" << std::hex << Off::InSDK::ObjArray::GObjects << std::dec << L"\n\n";

			ByIndex = [](void* ObjectsArray, int32 Index, uint32 FUObjectItemSize, uint32 FUObjectItemOffset, uint32 PerChunk) -> void*
			{
				if (Index < 0 || Index > Num())
					return nullptr;

				const int32 ChunkIndex = Index / PerChunk;
				const int32 InChunkIdx = Index % PerChunk;

				uint8_t* ChunkPtr = DecryptPtr(*reinterpret_cast<uint8_t**>(ObjectsArray));

				uint8_t* Chunk = reinterpret_cast<uint8_t**>(ChunkPtr)[ChunkIndex];
				uint8_t* ItemPtr = Chunk + (InChunkIdx * FUObjectItemSize);

				return *reinterpret_cast<void**>(ItemPtr + FUObjectItemOffset);
			};
			
			uint8_t* ChunksPtr = DecryptPtr(*reinterpret_cast<uint8_t**>(GObjects + Off::FUObjectArray::Ptr));

			ObjectArray::InitializeFUObjectItem(*reinterpret_cast<uint8_t**>(ChunksPtr));

			ObjectArray::InitializeChunkSize(GObjects + Off::FUObjectArray::Ptr);

			return;
		}
	}

	if (!bScanAllMemory)
	{
		ObjectArray::Init(true);
		return;
	}

	if (!bScanAllMemory)
	{
		std::wcout << L"\nGObjects couldn't be found!\n\n\n";
		Sleep(3000);
		exit(1);
	}
}

void ObjectArray::Init(int32 GObjectsOffset, int32 ElementsPerChunk, bool bIsChunked)
{
	GObjects = reinterpret_cast<uint8_t*>(GetImageBase() + GObjectsOffset);

	Off::InSDK::ObjArray::GObjects = GObjectsOffset;

	std::wcout << L"GObjects: 0x" << (void*)GObjects << L"\n" << std::endl;

	if (!bIsChunked)
	{
		Off::FUObjectArray::Num = 0xC;

		ByIndex = [](void* ObjectsArray, int32 Index, uint32 FUObjectItemSize, uint32 FUObjectItemOffset, uint32 PerChunk) -> void*
		{
			if (Index < 0 || Index > Num())
				return nullptr;

			uint8_t* ItemPtr = *reinterpret_cast<uint8_t**>(ObjectsArray) + (Index * FUObjectItemSize);

			return *reinterpret_cast<void**>(ItemPtr + FUObjectItemOffset);
		};

		uint8_t* ChunksPtr = DecryptPtr(*reinterpret_cast<uint8_t**>(GObjects));

		ObjectArray::InitializeFUObjectItem(*reinterpret_cast<uint8_t**>(ChunksPtr));
	}
	else
	{
		Off::FUObjectArray::Num = 0x14;

		ByIndex = [](void* ObjectsArray, int32 Index, uint32 FUObjectItemSize, uint32 FUObjectItemOffset, uint32 PerChunk) -> void*
		{
			if (Index < 0 || Index > Num())
				return nullptr;

			const int32 ChunkIndex = Index / PerChunk;
			const int32 InChunkIdx = Index % PerChunk;

			uint8_t* Chunk = (*reinterpret_cast<uint8_t***>(ObjectsArray))[ChunkIndex];
			uint8_t* ItemPtr = reinterpret_cast<uint8_t*>(Chunk) + (InChunkIdx * FUObjectItemSize);

			return *reinterpret_cast<void**>(ItemPtr + FUObjectItemOffset);
		};

		uint8_t* ChunksPtr = DecryptPtr(*reinterpret_cast<uint8_t**>(GObjects));

		ObjectArray::InitializeFUObjectItem(*reinterpret_cast<uint8_t**>(ChunksPtr));
	}

	NumElementsPerChunk = ElementsPerChunk;
	Off::InSDK::ObjArray::ChunkSize = ElementsPerChunk;
}

void ObjectArray::DumpObjects(const fs::path& Path, bool bWithPathname)
{
	std::wofstream DumpStream(Path / "GObjects-Dump.txt");

	DumpStream << L"Object dump by Dumper-7\n\n";
	DumpStream << (!Settings::Generator::GameVersion.empty() && !Settings::Generator::GameName.empty() ? (Settings::Generator::GameVersion + L'-' + Settings::Generator::GameName) + L"\n\n" : L"");
	DumpStream << L"Count: L" << Num() << L"\n\n\n";

	for (auto Object : ObjectArray())
	{
		if (!bWithPathname)
		{
			DumpStream << std::format(L"[{:08X}] {{{}}} {}\n", Object.GetIndex(), Object.GetAddress(), Object.GetFullName());
		}
		else
		{
			DumpStream << std::format(L"[{:08X}] {{{}}} {}\n", Object.GetIndex(), Object.GetAddress(), Object.GetPathName());
		}
	}

	DumpStream.close();
}


int32 ObjectArray::Num()
{
	return *reinterpret_cast<int32*>(GObjects + Off::FUObjectArray::Num);
}

template<typename UEType>
static UEType ObjectArray::GetByIndex(int32 Index)
{
	return UEType(ByIndex(GObjects + Off::FUObjectArray::Ptr, Index, SizeOfFUObjectItem, FUObjectItemInitialOffset, NumElementsPerChunk));
}

template<typename UEType>
UEType ObjectArray::FindObject(std::wstring FullName, EClassCastFlags RequiredType)
{
	for (UEObject Object : ObjectArray())
	{
		if (Object.IsA(RequiredType) && Object.GetFullName() == FullName)
		{
			return Object.Cast<UEType>();
		}
	}

	return UEType();
}

template<typename UEType>
UEType ObjectArray::FindObjectFast(std::wstring Name, EClassCastFlags RequiredType)
{
	auto ObjArray = ObjectArray();

	for (UEObject Object : ObjArray)
	{
		if (Object.IsA(RequiredType) && Object.GetName() == Name)
		{
			return Object.Cast<UEType>();
		}
	}

	return UEType();
}

template<typename UEType>
static UEType ObjectArray::FindObjectFastInOuter(std::wstring Name, std::wstring Outer)
{
	auto ObjArray = ObjectArray();

	for (UEObject Object : ObjArray)
	{
		if (Object.GetName() == Name && Object.GetOuter().GetName() == Outer)
		{
			return Object.Cast<UEType>();
		}
	}

	return UEType();
}

UEClass ObjectArray::FindClass(std::wstring FullName)
{
	return FindObject<UEClass>(FullName, EClassCastFlags::Class);
}

UEClass ObjectArray::FindClassFast(std::wstring Name)
{
	return FindObjectFast<UEClass>(Name, EClassCastFlags::Class);
}

ObjectArray::ObjectsIterator ObjectArray::begin()
{
	return ObjectsIterator(*this);
}
ObjectArray::ObjectsIterator ObjectArray::end()
{
	return ObjectsIterator(*this, Num());
}


ObjectArray::ObjectsIterator::ObjectsIterator(ObjectArray& Array, int32 StartIndex)
	: IteratedArray(Array), CurrentIndex(StartIndex), CurrentObject(ObjectArray::GetByIndex(StartIndex))
{
}

UEObject ObjectArray::ObjectsIterator::operator*()
{
	return CurrentObject;
}

ObjectArray::ObjectsIterator& ObjectArray::ObjectsIterator::operator++()
{
	CurrentObject = ObjectArray::GetByIndex(++CurrentIndex);

	while (!CurrentObject && CurrentIndex < (ObjectArray::Num() - 1))
	{
		CurrentObject = ObjectArray::GetByIndex(++CurrentIndex);
	}

	if (!CurrentObject && CurrentIndex == (ObjectArray::Num() - 1)) [[unlikely]]
		CurrentIndex++;

	return *this;
}

bool ObjectArray::ObjectsIterator::operator!=(const ObjectsIterator& Other)
{
	return CurrentIndex != Other.CurrentIndex;
}

int32 ObjectArray::ObjectsIterator::GetIndex() const
{
	return CurrentIndex;
}

/*
* The compiler won't generate functions for a specific template type unless it's used in the .cpp file corresponding to the
* header it was declatred in.
*
* See https://stackoverflow.com/questions/456713/why-do-i-get-unresolved-external-symbol-errors-when-using-templates
*/
[[maybe_unused]] void TemplateTypeCreationForObjectArray(void)
{
	ObjectArray::FindObject<UEObject>(L"");
	ObjectArray::FindObject<UEField>(L"");
	ObjectArray::FindObject<UEEnum>(L"");
	ObjectArray::FindObject<UEStruct>(L"");
	ObjectArray::FindObject<UEClass>(L"");
	ObjectArray::FindObject<UEFunction>(L"");
	ObjectArray::FindObject<UEProperty>(L"");
	ObjectArray::FindObject<UEByteProperty>(L"");
	ObjectArray::FindObject<UEBoolProperty>(L"");
	ObjectArray::FindObject<UEObjectProperty>(L"");
	ObjectArray::FindObject<UEClassProperty>(L"");
	ObjectArray::FindObject<UEStructProperty>(L"");
	ObjectArray::FindObject<UEArrayProperty>(L"");
	ObjectArray::FindObject<UEMapProperty>(L"");
	ObjectArray::FindObject<UESetProperty>(L"");
	ObjectArray::FindObject<UEEnumProperty>(L"");

	ObjectArray::FindObjectFast<UEObject>(L"");
	ObjectArray::FindObjectFast<UEField>(L"");
	ObjectArray::FindObjectFast<UEEnum>(L"");
	ObjectArray::FindObjectFast<UEStruct>(L"");
	ObjectArray::FindObjectFast<UEClass>(L"");
	ObjectArray::FindObjectFast<UEFunction>(L"");
	ObjectArray::FindObjectFast<UEProperty>(L"");
	ObjectArray::FindObjectFast<UEByteProperty>(L"");
	ObjectArray::FindObjectFast<UEBoolProperty>(L"");
	ObjectArray::FindObjectFast<UEObjectProperty>(L"");
	ObjectArray::FindObjectFast<UEClassProperty>(L"");
	ObjectArray::FindObjectFast<UEStructProperty>(L"");
	ObjectArray::FindObjectFast<UEArrayProperty>(L"");
	ObjectArray::FindObjectFast<UEMapProperty>(L"");
	ObjectArray::FindObjectFast<UESetProperty>(L"");
	ObjectArray::FindObjectFast<UEEnumProperty>(L"");

	ObjectArray::FindObjectFastInOuter<UEObject>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEField>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEEnum>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEStruct>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEClass>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEFunction>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEProperty>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEByteProperty>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEBoolProperty>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEObjectProperty>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEClassProperty>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEStructProperty>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEArrayProperty>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEMapProperty>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UESetProperty>(L"", L"");
	ObjectArray::FindObjectFastInOuter<UEEnumProperty>(L"", L"");

	ObjectArray::GetByIndex<UEObject>(-1);
	ObjectArray::GetByIndex<UEField>(-1);
	ObjectArray::GetByIndex<UEEnum>(-1);
	ObjectArray::GetByIndex<UEStruct>(-1);
	ObjectArray::GetByIndex<UEClass>(-1);
	ObjectArray::GetByIndex<UEFunction>(-1);
	ObjectArray::GetByIndex<UEProperty>(-1);
	ObjectArray::GetByIndex<UEByteProperty>(-1);
	ObjectArray::GetByIndex<UEBoolProperty>(-1);
	ObjectArray::GetByIndex<UEObjectProperty>(-1);
	ObjectArray::GetByIndex<UEClassProperty>(-1);
	ObjectArray::GetByIndex<UEStructProperty>(-1);
	ObjectArray::GetByIndex<UEArrayProperty>(-1);
	ObjectArray::GetByIndex<UEMapProperty>(-1);
	ObjectArray::GetByIndex<UESetProperty>(-1);
	ObjectArray::GetByIndex<UEEnumProperty>(-1);
}
