#pragma once
#include "ObjectArray.h"
#include "EnumWrapper.h"
#include "StructManager.h"
#include "StructWrapper.h"
#include "MemberWrappers.h"
#include "PackageManager.h"

#include "ExternalDependencies/Dumpspace/DSGen.h"

class DumpspaceGenerator
{
private:
    friend class CppGeneratorTest;
    friend class Generator;

private:
    using StreamType = std::wofstream;

public:
    static inline PredefinedMemberLookupMapType PredefinedMembers;

    static inline std::wstring MainFolderName = L"Dumpspace";
    static inline std::wstring SubfolderName = L"";

    static inline fs::path MainFolder;
    static inline fs::path Subfolder;

private:
    static std::wstring GetStructPrefixedName(const StructWrapper& Struct);
    static std::wstring GetEnumPrefixedName(const EnumWrapper& Enum);

private:
    static std::wstring EnumSizeToType(const int32 Size);

private:
    static DSGen::EType GetMemberEType(const PropertyWrapper& Property);
    static DSGen::EType GetMemberEType(UEProperty Property);
    static std::wstring GetMemberTypeStr(UEProperty Property, std::string& OutExtendedType, std::vector<DSGen::MemberType>& OutSubtypes);
    static DSGen::MemberType GetMemberType(const StructWrapper& Struct);
    static DSGen::MemberType GetMemberType(UEProperty Property, bool bIsReference = false);
    static DSGen::MemberType GetMemberType(const PropertyWrapper& Property, bool bIsReference = false);
    static DSGen::MemberType ManualCreateMemberType(DSGen::EType Type, const std::wstring& TypeName, const std::wstring& ExtendedType = L"");
    static void AddMemberToStruct(DSGen::ClassHolder& Struct, const PropertyWrapper& Property);

    static void RecursiveGetSuperClasses(const StructWrapper& Struct, std::vector<std::string>& OutSupers);
    static std::vector<std::string> GetSuperClasses(const StructWrapper& Struct);

private:
    static DSGen::ClassHolder GenerateStruct(const StructWrapper& Struct);
    static DSGen::EnumHolder GenerateEnum(const EnumWrapper& Enum);
    static DSGen::FunctionHolder GenearateFunction(const FunctionWrapper& Function);

    static void GeneratedStaticOffsets();

public:
    static void Generate();

    static void InitPredefinedMembers() { };
    static void InitPredefinedFunctions() { };
};