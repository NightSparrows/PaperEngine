#pragma once

#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <typeindex>


#include <PaperEngine/core/Base.h>

namespace PaperEngine {

    /// <summary>
    /// 資源生命週期
    /// Load -> Create -> Get -> Release
    /// 
    /// Load還有分
    /// serialize load 
    /// 或
    /// package load
    /// 
    /// name可以是路徑，也可以是名稱（反正要有唯一性）
    /// 
    /// 只負責資源管理
    /// 不負責Load
    /// </summary>

    class ResourceManager {
    public:
        // 創建或取得資源
        template<typename T, typename... Args>
        Ref<T> create(const std::string& name, Args&&... args) {
            std::shared_lock readLock(m_mutex);
            auto typeIt = m_resources.find(std::type_index(typeid(T)));
            if (typeIt != m_resources.end()) {
                auto& innerMap = typeIt->second;
                auto it = innerMap.find(name);
                if (it != innerMap.end()) {
                    if (auto sharedRes = it->second.lock()) {
                        return std::static_pointer_cast<T>(sharedRes);
                    }
                }
            }
            readLock.unlock();

            // 寫鎖創建資源
            std::unique_lock writeLock(m_mutex);
            PE_CORE_WARN("create resource: {}", name);
            auto& innerMap = m_resources[std::type_index(typeid(T))];
            Ref<T> newRes = CreateRef<T>(std::forward<Args>(args)...);
            innerMap[name] = newRes;
            return newRes;
        }

        // 讀取資源
        template<typename T>
        Ref<T> load(const std::string& name) {
            std::shared_lock readLock(m_mutex);
            auto typeIt = m_resources.find(std::type_index(typeid(T)));
            if (typeIt == m_resources.end()) return nullptr;

            auto& innerMap = typeIt->second;
            auto it = innerMap.find(name);
            if (it == innerMap.end()) return nullptr;

            return std::static_pointer_cast<T>(it->second.lock());
        }

        // 分批垃圾回收
        void garbageCollectBatch(size_t maxItems = 50) {
            size_t count = 0;

            while (count < maxItems) {
                std::unique_lock lock(m_mutex);
                if (m_typeIt == m_resources.end()) {
                    // 完成一輪掃描，重置迭代器
                    m_typeIt = m_resources.begin();
                    if (m_typeIt != m_resources.end())
                        m_innerIt = m_typeIt->second.begin();
                    else
                        break; // 沒有資源
                }

                auto& innerMap = m_typeIt->second;
                if (m_innerIt == innerMap.end()) {
                    ++m_typeIt;
                    continue;
                }

                auto currentIt = m_innerIt++;
                if (currentIt->second.expired()) {
                    innerMap.erase(currentIt);
                }

                ++count;
            }
        }

    private:
        std::shared_mutex m_mutex;

        // 外層 key: type_index，內層 key: resource name
        std::unordered_map<std::type_index, std::unordered_map<std::string, std::weak_ptr<void>>> m_resources;

        // 分批掃描迭代器狀態
        std::unordered_map<std::type_index, std::unordered_map<std::string, std::weak_ptr<void>>>::iterator m_typeIt = m_resources.begin();
        std::unordered_map<std::string, std::weak_ptr<void>>::iterator m_innerIt;
    };

}
