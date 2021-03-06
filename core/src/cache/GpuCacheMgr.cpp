// Copyright (C) 2019-2020 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under the License.

#include "cache/GpuCacheMgr.h"
#include "server/Config.h"
#include "utils/Log.h"

#include <fiu-local.h>
#include <sstream>
#include <utility>

namespace milvus {
namespace cache {

#ifdef MILVUS_GPU_VERSION
std::mutex GpuCacheMgr::mutex_;
std::unordered_map<uint64_t, GpuCacheMgrPtr> GpuCacheMgr::instance_;

namespace {
constexpr int64_t G_BYTE = 1024 * 1024 * 1024;
}

GpuCacheMgr::GpuCacheMgr() {
    // All config values have been checked in Config::ValidateConfig()
    server::Config& config = server::Config::GetInstance();

    config.GenUniqueIdentityID("GpuCacheMar", identity_);

    config.GetGpuResourceConfigEnable(gpu_enable_);
    server::ConfigCallBackF lambda = [this](const std::string& value) -> Status {
        auto& config = server::Config::GetInstance();
        return config.GetGpuResourceConfigEnable(this->gpu_enable_);
    };
    config.RegisterCallBack(server::CONFIG_GPU_RESOURCE, server::CONFIG_GPU_RESOURCE_ENABLE, identity_, lambda);

    int64_t gpu_cache_cap;
    config.GetGpuResourceConfigCacheCapacity(gpu_cache_cap);
    int64_t cap = gpu_cache_cap * G_BYTE;
    cache_ = std::make_shared<Cache<DataObjPtr>>(cap, 1UL << 32);

    float gpu_mem_threshold;
    config.GetGpuResourceConfigCacheThreshold(gpu_mem_threshold);
    cache_->set_freemem_percent(gpu_mem_threshold);
}

GpuCacheMgr::~GpuCacheMgr() {
    server::Config& config = server::Config::GetInstance();
    config.CancelCallBack(server::CONFIG_GPU_RESOURCE, server::CONFIG_GPU_RESOURCE_ENABLE, identity_);
}

GpuCacheMgr*
GpuCacheMgr::GetInstance(uint64_t gpu_id) {
    if (instance_.find(gpu_id) == instance_.end()) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_.find(gpu_id) == instance_.end()) {
            instance_.insert(std::pair<uint64_t, GpuCacheMgrPtr>(gpu_id, std::make_shared<GpuCacheMgr>()));
        }
        return instance_[gpu_id].get();
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        return instance_[gpu_id].get();
    }
}

DataObjPtr
GpuCacheMgr::GetIndex(const std::string& key) {
    DataObjPtr obj = GetItem(key);
    return obj;
}

void
GpuCacheMgr::InsertItem(const std::string& key, const milvus::cache::DataObjPtr& data) {
    if (gpu_enable_) {
        CacheMgr<DataObjPtr>::InsertItem(key, data);
    }
}

#endif

}  // namespace cache
}  // namespace milvus
