/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_FG_RESOURCEENTRY_H
#define TNT_FILAMENT_FG_RESOURCEENTRY_H

#include "VirtualResource.h"

#include <type_traits>

#include <stdint.h>

namespace filament {

class FrameGraph;

namespace fg {

struct PassNode;

class ResourceEntryBase : public VirtualResource {
public:
    explicit ResourceEntryBase(const char* name, uint16_t id) noexcept;
    ResourceEntryBase(ResourceEntryBase const&) = default;
    ~ResourceEntryBase() override;

    // constants
    const char* const name;
    const uint16_t id;                      // for debugging and graphing
    bool imported{};

    // updated by builder
    uint8_t version = 0;

    // computed during compile()
    uint32_t refs = 0;                      // final reference count
};


template<typename T>
class ResourceEntry : public ResourceEntryBase {
    using Descriptor = typename T::Descriptor;
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    Descriptor descriptor;
    Storage resource;

public:
    explicit ResourceEntry(const char* name, uint16_t id, Descriptor const& desc) noexcept
        : ResourceEntryBase(name, id), descriptor(desc) {
    }

    Descriptor const& getDescriptor() const noexcept { return descriptor; }

    T const& getResource() const noexcept { return reinterpret_cast<T const&>(resource); }

    T& getResource() noexcept { return reinterpret_cast<T&>(resource); }

    void create(FrameGraph& fg) noexcept override {
        // TODO: handle imported resources
        new(&resource) T(fg, name, descriptor);
    }

    void destroy(FrameGraph& fg) noexcept override {
        // TODO: handle imported resources
        getResource().terminate(fg);
        reinterpret_cast<T*>(&resource)->~T();
    }
};

} // namespace fg
} // namespace filament

#endif //TNT_FILAMENT_FG_RESOURCEENTRY_H
