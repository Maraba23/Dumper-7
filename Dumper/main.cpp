#include <Windows.h>
#include <iostream>
#include <chrono>
#include <fstream>

#include "CppGenerator.h"
#include "MappingGenerator.h"
#include "IDAMappingGenerator.h"
#include "DumpspaceGenerator.h"

#include "StructManager.h"
#include "EnumManager.h"

#include "Generator.h"


enum class EFortToastType : uint8
{
        Default                        = 0,
        Subdued                        = 1,
        Impactful                      = 2,
        EFortToastType_MAX             = 3,
};

DWORD MainThread(HMODULE Module)
{
	AllocConsole();
	FILE* Dummy;
	freopen_s(&Dummy, "CONOUT$", "w", stdout);
	freopen_s(&Dummy, "CONIN$", "r", stdin);

	auto t_1 = std::chrono::high_resolution_clock::now();

	std::wcout << L"Started Generation [Dumper-7]!\n";

	Generator::InitEngineCore();
	Generator::InitInternal();

	if (Settings::Generator::GameName.empty() && Settings::Generator::GameVersion.empty())
	{
		// Only Possible in Main()
		FString Name;
		FString Version;
		UEClass Kismet = ObjectArray::FindClassFast(L"KismetSystemLibrary");
		UEFunction GetGameName = Kismet.GetFunction(L"KismetSystemLibrary", L"GetGameName");
		UEFunction GetEngineVersion = Kismet.GetFunction(L"KismetSystemLibrary", L"GetEngineVersion");

		Kismet.ProcessEvent(GetGameName, &Name);
		Kismet.ProcessEvent(GetEngineVersion, &Version);

		Settings::Generator::GameName = Name.ToString();
		Settings::Generator::GameVersion = Version.ToString();
	}

	std::wcout << L"GameName: L" << Settings::Generator::GameName << L"\n";
	std::wcout << L"GameVersion: L" << Settings::Generator::GameVersion << L"\n\n";

	Generator::Generate<CppGenerator>();
	Generator::Generate<MappingGenerator>();
	Generator::Generate<IDAMappingGenerator>();
	Generator::Generate<DumpspaceGenerator>();

	for (UEObject Obj : ObjectArray())
	{
		if (!Obj.IsA(EClassCastFlags::Enum) || Obj.HasAnyFlags(EObjectFlags::ClassDefaultObject))
			continue;

		UEEnum Enum = Obj.Cast<UEEnum>();

		if (Enum.GetEnumFlags() & EEnumFlags::Flags)
		{
			std::wcout << L"Enum: L" << Enum.GetCppName() << L" : L" << Enum.GetEnumTypeAsStr() << L"\n";

			for (auto& NameValuePair : Enum.GetNameValuePairs())
			{
				std::wcout << L"Val: L" << NameValuePair.first.ToString() << L" = L" << NameValuePair.second << std::endl;
			}

			std::wcout << L"\n\n";
		}
	}

	auto t_C = std::chrono::high_resolution_clock::now();
	
	auto ms_int_ = std::chrono::duration_cast<std::chrono::milliseconds>(t_C - t_1);
	std::chrono::duration<double, std::milli> ms_double_ = t_C - t_1;
	
	std::wcout << L"\n\nGenerating SDK took (L" << ms_double_.count() << L"ms)\n\n\n";

	while (true)
	{
		if (GetAsyncKeyState(VK_F6) & 1)
		{
			fclose(stdout);
			if (Dummy) fclose(Dummy);
			FreeConsole();

			FreeLibraryAndExitThread(Module, 0);
		}

		Sleep(100);
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
		break;
	}

	return TRUE;
}