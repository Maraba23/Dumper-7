#pragma once
#include "EnumManager.h"

class EnumWrapper
{
private:
    const UEEnum Enum;
    EnumInfoHandle InfoHandle;

public:
    EnumWrapper(const UEEnum Enm);

public:
    UEEnum GetUnrealEnum() const;

public:
    std::wstring GetName() const;
    std::wstring GetRawName() const;
    std::wstring GetFullName() const;

    std::pair<std::wstring, bool> GetUniqueName() const;
    uint8 GetUnderlyingTypeSize() const;

    int32 GetNumMembers() const;

    CollisionInfoIterator GetMembers() const;

    bool IsValid() const;
};
