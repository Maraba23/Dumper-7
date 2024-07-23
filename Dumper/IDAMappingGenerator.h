#pragma once
#include <iostream>
#include <string>

#include "PredefinedMembers.h"
#include "ObjectArray.h"

class IDAMappingGenerator
{
public:
    static inline PredefinedMemberLookupMapType PredefinedMembers;

    static inline std::wstring MainFolderName = L"IDAMappings";
    static inline std::wstring SubfolderName = L"";

    static inline fs::path MainFolder;
    static inline fs::path Subfolder;

private:
    using StreamType = std::wofstream;

private:
    template<typename InStreamType, typename T>
    static void WriteToStream(InStreamType& InStream, T Value)
    {
        InStream.write(reinterpret_cast<const wchar_t*>(&Value), sizeof(T));
    }

    template<typename InStreamType, typename T>
    static void WriteToStream(InStreamType& InStream, T* Value, int32 Size)
    {
        InStream.write(reinterpret_cast<const wchar_t*>(Value), Size);
    }

private:
    static std::wstring MangleFunctionName(const std::wstring& ClassName, const std::wstring& FunctionName);

private:
    static void WriteReadMe(StreamType& ReadMe);

    static void GenerateVTableName(StreamType& IdmapFile, UEObject DefaultObject);
    static void GenerateClassFunctions(StreamType& IdmapFile, UEClass Class);

public:
    static void Generate();

    /* Always empty, there are no predefined members for IDAMappings */
    static void InitPredefinedMembers() { }
    static void InitPredefinedFunctions() { }
};