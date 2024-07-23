#include <format>

#include "UnrealTypes.h"
#include "NameArray.h"


std::wstring MakeNameValid(std::wstring&& Name)
{
	return Name;

	static constexpr const wchar_t* Numbers[10] =
	{
		L"Zero",
		L"One",
		L"Two",
		L"Three",
		L"Four",
		L"Five",
		L"Six",
		L"Seven",
		L"Eight",
		L"Nine"
	};

	if (Name == L"bool")
		return L"Bool";

	if (Name == L"TRUE")
		return L"TURR";

	if (Name == L"FALSE")
		return L"FLASE";

	if (Name == L"NULL")
		return L"NULLL";

	if (Name[0] <= L'9' && Name[0] >= L'0')
	{
		Name.replace(0, 1, Numbers[Name[0] - L'0']);
	}
	else if ((Name[0] <= L'z' && Name[0] >= L'a') && Name[0] != L'b')
	{
		Name[0] -= 0x20;
	}

	for (int i = 0; i < Name.length(); i++)
	{
		switch (Name[i])
		{
		case '+':
			Name.replace(i, 1, L"Plus");
			continue;
		case '-':
			Name.replace(i, 1, L"Minus");
			continue;
		case '*':
			Name.replace(i, 1, L"Star");
			continue;
		case '/':
			Name.replace(i, 1, L"Slash");
			continue;
		default:
			break;
		}

		char c = Name[i];

		if (c != L'_' && !((c <= L'z' && c >= L'a') || (c <= L'Z' && c >= L'A') || (c <= L'9' && c >= L'0')))
		{
			Name[i] = L'_';
		}
	}

	return Name;
}


FName::FName(const void* Ptr)
	: Address(static_cast<const uint8*>(Ptr))
{
}

void FName::Init(bool bForceGNames)
{
	constexpr std::array<const char*, 5> PossibleSigs = 
	{ 
		"48 8D ? ? 48 8D ? ? E8",
		"48 8D ? ? ? 48 8D ? ? E8",
		"48 8D ? ? 49 8B ? E8",
		"48 8D ? ? ? 49 8B ? E8",
		"48 8D ? ? 48 8B ? E8"
		"48 8D ? ? ? 48 8B ? E8",
	};

	MemAddress StringRef = FindByStringInAllSections("ForwardShadingQuality_");

	int i = 0;
	while (!AppendString && i < PossibleSigs.size())
	{
		AppendString = static_cast<void(*)(const void*, FString&)>(StringRef.RelativePattern(PossibleSigs[i], 0x50, -1 /* auto */));

		i++;
	}

	Off::InSDK::Name::AppendNameToString = AppendString && !bForceGNames ? GetOffset(AppendString) : 0x0;

	if (!AppendString || bForceGNames)
	{
		const bool bInitializedSuccessfully = NameArray::TryInit();

		if (bInitializedSuccessfully)
		{
			ToStr = [](const void* Name) -> std::wstring
			{
				if (!Settings::Internal::bUseUoutlineNumberName)
				{
					const int32 Number = FName(Name).GetNumber();

					if (Number > 0)
						return NameArray::GetNameEntry(Name).GetString() + L"_" + std::to_wstring(Number - 1);
				}

				return NameArray::GetNameEntry(Name).GetString();
			};

			return;
		}
		else /* Attempt to find FName::ToString as a final fallback */
		{
			/* Initialize GNames offset without committing to use GNames during the dumping process or in the SDK */
			NameArray::SetGNamesWithoutCommiting();
			FName::InitFallback();
		}
	}

	/* Initialize GNames offset without committing to use GNames during the dumping process or in the SDK */
	NameArray::SetGNamesWithoutCommiting();

	std::wcout << std::format(L"Found FName::{} at Offset 0x{:X}\n\n", (Off::InSDK::Name::bIsUsingAppendStringOverToString ? L"AppendString" : L"ToString"), Off::InSDK::Name::AppendNameToString);

	ToStr = [](const void* Name) -> std::wstring
	{
		thread_local FFreableString TempString(1024);

		AppendString(Name, TempString);

		std::wstring OutputString = TempString.ToString();
		TempString.ResetNum();

		return OutputString;
	};
}

void FName::Init(int32 OverrideOffset, EOffsetOverrideType OverrideType, bool bIsNamePool)
{
	if (OverrideType == EOffsetOverrideType::GNames)
	{
		const bool bInitializedSuccessfully = NameArray::TryInit(OverrideOffset, bIsNamePool);

		if (bInitializedSuccessfully)
		{
			ToStr = [](const void* Name) -> std::wstring
			{
				if (!Settings::Internal::bUseUoutlineNumberName)
				{
					const int32 Number = FName(Name).GetNumber();

					if (Number > 0)
						return NameArray::GetNameEntry(Name).GetString() + L"_" + std::to_wstring(Number - 1);
				}

				return NameArray::GetNameEntry(Name).GetString();
			};
		}

		return;
	}

	AppendString = reinterpret_cast<void(*)(const void*, FString&)>(GetImageBase() + OverrideOffset);

	Off::InSDK::Name::AppendNameToString = OverrideOffset;
	Off::InSDK::Name::bIsUsingAppendStringOverToString = OverrideType == EOffsetOverrideType::AppendString;

	ToStr = [](const void* Name) -> std::wstring
	{
		thread_local FFreableString TempString(1024);

		AppendString(Name, TempString);

		std::wstring OutputString = TempString.ToString();
		TempString.ResetNum();

		return OutputString;
	};

	std::wcout << std::format(L"Manual-Override: FName::{} --> Offset 0x{:X}\n\n", (Off::InSDK::Name::bIsUsingAppendStringOverToString ? L"AppendString" : L"ToString"), Off::InSDK::Name::AppendNameToString);
}

void FName::InitFallback()
{
	Off::InSDK::Name::bIsUsingAppendStringOverToString = false;

	MemAddress Conv_NameToStringAddress = FindUnrealExecFunctionByString(L"Conv_NameToString");

	constexpr std::array<const char*, 3> PossibleSigs =
	{
		"89 44 ? ? 48 01 ? ? E8",
		"48 89 ? ? 48 8D ? ? ? E8",
		"48 89 ? ? ? 48 89 ? ? E8",
	};

	int i = 0;
	while (!AppendString && i < PossibleSigs.size())
	{
		AppendString = static_cast<void(*)(const void*, FString&)>(Conv_NameToStringAddress.RelativePattern(PossibleSigs[i], 0x90, -1 /* auto */));

		i++;
	}

	Off::InSDK::Name::AppendNameToString = AppendString ? GetOffset(AppendString) : 0x0;
}

std::wstring FName::ToString() const
{
	if (!Address)
		return L"None";

	std::wstring OutputString = ToStr(Address);

	size_t pos = OutputString.rfind('/');

	if (pos == std::wstring::npos)
		return OutputString;

	return OutputString.substr(pos + 1);
}

std::wstring FName::ToRawString() const
{
	if (!Address)
		return L"None";

	return ToStr(Address);
}

std::wstring FName::ToValidString() const
{
	return MakeNameValid(ToString());
}

int32 FName::GetCompIdx() const 
{
	return *reinterpret_cast<const int32*>(Address + Off::FName::CompIdx);
}

int32 FName::GetNumber() const
{
	return !Settings::Internal::bUseUoutlineNumberName ? *reinterpret_cast<const int32*>(Address + Off::FName::Number) : 0x0;
}

bool FName::operator==(FName Other) const
{
	return GetCompIdx() == Other.GetCompIdx();
}

bool FName::operator!=(FName Other) const
{
	return GetCompIdx() != Other.GetCompIdx();
}

std::wstring FName::CompIdxToString(int CmpIdx)
{
	if (!Settings::Internal::bUseCasePreservingName)
	{
		struct FakeFName
		{
			int CompIdx;
			uint8 Pad[0x4];
		} Name(CmpIdx);

		return FName(&Name).ToString();
	}
	else
	{
		struct FakeFName
		{
			int CompIdx;
			uint8 Pad[0xC];
		} Name(CmpIdx);

		return FName(&Name).ToString();
	}
}

void* FName::DEBUGGetAppendString()
{
	return (void*)(AppendString);
}
