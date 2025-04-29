/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ohos.pasteboard.pasteboard.impl.hpp"
#include "ohos.pasteboard.pasteboard.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace ohos::pasteboard::pasteboard;

namespace {
// To be implemented.

class PasteDataRecordImpl {
public:
    PasteDataRecordImpl()
    {
        // Don't forget to implement the constructor.
    }

    string GetmimeType()
    {
        TH_THROW(std::runtime_error, "GetmimeType not implemented");
    }

    string GetplainText()
    {
        TH_THROW(std::runtime_error, "GetplainText not implemented");
    }

    string Geturi()
    {
        TH_THROW(std::runtime_error, "Geturi not implemented");
    }

    uintptr_t GetpixelMap()
    {
        TH_THROW(std::runtime_error, "GetpixelMap not implemented");
    }

    ValueType GetDataSync(string_view type)
    {
        TH_THROW(std::runtime_error, "GetDataSync not implemented");
    }

    string ToPlainText()
    {
        TH_THROW(std::runtime_error, "ToPlainText not implemented");
    }

    int64_t GetRecordImpl()
    {
        TH_THROW(std::runtime_error, "GetRecordImpl not implemented");
    }
};

class PasteDataImpl {
public:
    PasteDataImpl()
    {
        // Don't forget to implement the constructor.
    }

    void AddRecordData(weak::PasteDataRecord record)
    {
        TH_THROW(std::runtime_error, "AddRecordData not implemented");
    }

    void AddRecordValue(string_view mimeType, ValueType const &value)
    {
        TH_THROW(std::runtime_error, "AddRecordValue not implemented");
    }

    array<string> GetMimeTypes()
    {
        TH_THROW(std::runtime_error, "GetMimeTypes not implemented");
    }

    string GetPrimaryHtml()
    {
        TH_THROW(std::runtime_error, "GetPrimaryHtml not implemented");
    }

    uintptr_t GetPrimaryWant()
    {
        TH_THROW(std::runtime_error, "GetPrimaryWant not implemented");
    }

    string GetPrimaryMimeType()
    {
        TH_THROW(std::runtime_error, "GetPrimaryMimeType not implemented");
    }

    string GetPrimaryText()
    {
        TH_THROW(std::runtime_error, "GetPrimaryText not implemented");
    }

    string GetPrimaryUri()
    {
        TH_THROW(std::runtime_error, "GetPrimaryUri not implemented");
    }

    PasteDataProperty GetProperty()
    {
        TH_THROW(std::runtime_error, "GetProperty not implemented");
    }

    void SetProperty(PasteDataProperty const &property)
    {
        TH_THROW(std::runtime_error, "SetProperty not implemented");
    }

    PasteDataRecord GetRecord(uint32_t index)
    {
        // The parameters in the make_holder function should be of the same type
        // as the parameters in the constructor of the actual implementation class.
        return make_holder<PasteDataRecordImpl, PasteDataRecord>();
    }

    uint32_t GetRecordCount()
    {
        TH_THROW(std::runtime_error, "GetRecordCount not implemented");
    }

    string GetTag()
    {
        TH_THROW(std::runtime_error, "GetTag not implemented");
    }

    int64_t GetPasteDataImpl()
    {
        TH_THROW(std::runtime_error, "GetPasteDataImpl not implemented");
    }
};

class SystemPasteboardImpl {
public:
    SystemPasteboardImpl()
    {
        // Don't forget to implement the constructor.
    }

    void OnUpdate(callback_view<void()> callback)
    {
        TH_THROW(std::runtime_error, "OnUpdate not implemented");
    }

    void OffUpdate(optional_view<callback<void()>> callback)
    {
        TH_THROW(std::runtime_error, "OffUpdate not implemented");
    }

    void ClearDataSync()
    {
        TH_THROW(std::runtime_error, "ClearDataSync not implemented");
    }

    PasteData GetDataSync()
    {
        // The parameters in the make_holder function should be of the same type
        // as the parameters in the constructor of the actual implementation class.
        return make_holder<PasteDataImpl, PasteData>();
    }

    bool HasDataSync()
    {
        TH_THROW(std::runtime_error, "HasDataSync not implemented");
    }

    void SetDataSync(weak::PasteData data)
    {
        TH_THROW(std::runtime_error, "SetDataSync not implemented");
    }

    array<string> GetMimeTypesSync()
    {
        TH_THROW(std::runtime_error, "GetMimeTypesSync not implemented");
    }

    string GetDataSource()
    {
        TH_THROW(std::runtime_error, "GetDataSource not implemented");
    }

    bool HasDataType(string_view mimeType)
    {
        TH_THROW(std::runtime_error, "HasDataType not implemented");
    }
};

PasteDataRecord MakePasteDataRecord()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<PasteDataRecordImpl, PasteDataRecord>();
}

PasteData CreatePasteData()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<PasteDataImpl, PasteData>();
}

SystemPasteboard CreateSystemPasteboard()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<SystemPasteboardImpl, SystemPasteboard>();
}

PasteData CreateDataByValue(string_view mimeType, ValueType const &value)
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<PasteDataImpl, PasteData>();
}

PasteData CreateDataByRecord(map_view<string, ValueType> data)
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<PasteDataImpl, PasteData>();
}

SystemPasteboard GetSystemPasteboard()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<SystemPasteboardImpl, SystemPasteboard>();
}
} // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_MakePasteDataRecord(MakePasteDataRecord);
TH_EXPORT_CPP_API_CreatePasteData(CreatePasteData);
TH_EXPORT_CPP_API_CreateSystemPasteboard(CreateSystemPasteboard);
TH_EXPORT_CPP_API_CreateDataByValue(CreateDataByValue);
TH_EXPORT_CPP_API_CreateDataByRecord(CreateDataByRecord);
TH_EXPORT_CPP_API_GetSystemPasteboard(GetSystemPasteboard);
// NOLINTEND