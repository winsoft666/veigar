#include "name_manager.h"
#include "log.h"
#include <boost/foreach.hpp>

namespace veigar {
namespace {
const uint32_t kMaxNameCount = 50000;
const uint32_t kNameFixedSize = 60;
const char* kUniqueName = "NameManagerCB416192EA854F1F8AC34A1094B3B728";
}  // namespace
NameManager::NameManager() noexcept {
}

NameManager::~NameManager() noexcept {
    assert(!init_);
    uninit();
}

bool NameManager::init() noexcept {
    if (init_) {
        return false;
    }

    uint32_t shmSize = kMaxNameCount * kNameFixedSize + 4;

    try {
        sharedMemory_ = itp::managed_shared_memory(itp::open_or_create, kUniqueName, shmSize);
    } catch (itp::interprocess_exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on create shared memory: %s\n", exc.what());
        return false;
    } catch (std::exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on create shared memory: %s\n", exc.what());
        return false;
    }

    try {
        StringAllocator stringAlloc(sharedMemory_.get_segment_manager());
        names_ = sharedMemory_.find_or_construct<StringDeque>("Names")(stringAlloc);
    } catch (itp::interprocess_exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on find_or_construct Names: %s\n", exc.what());
        return false;
    } catch (std::exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on find_or_construct Names: %s\n", exc.what());
        return false;
    }

    if (!names_) {
        return false;
    }

    init_ = true;
    return init_;
}

bool NameManager::isInit() const noexcept {
    return init_;
}

void NameManager::uninit() noexcept {
    if (!init_) {
        return;
    }

    itp::shared_memory_object::remove(kUniqueName);
    init_ = false;
}

bool NameManager::isExist(const std::string& name) noexcept {
    if (!init_) {
        return false;
    }

    if (!names_) {
        return false;
    }

    bool exist = false;
    try {
        for (auto& it = names_->cbegin(); it != names_->cend(); ++it) {
            const char* d = (*it).c_str();
            if (strcmp(d, name.c_str()) == 0) {
                exist = true;
                break;
            }
        }
    } catch (itp::interprocess_exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on check name exist: %s\n", exc.what());
        return false;
    } catch (std::exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on check name exist: %s\n", exc.what());
        return false;
    }

    return exist;
}

bool NameManager::reserveName(const std::string& name) noexcept {
    if (!init_) {
        return false;
    }

    if (name.length() > kNameFixedSize) {
        return false;
    }

    try {
        if (isExist(name)) {
            return false;
        }

        assert(names_);
        if (names_) {
            CharAllocator charAlloc(sharedMemory_.get_segment_manager());
            String nameInShm(charAlloc);
            nameInShm = name.c_str();

            names_->push_back(nameInShm);
        }
    } catch (itp::interprocess_exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on reserve name: %s\n", exc.what());
        return false;
    } catch (std::exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on reserve name: %s\n", exc.what());
        return false;
    }

    return true;
}

bool NameManager::releaseName(const std::string& name) noexcept {
    if (!init_) {
        return false;
    }

    bool erased = false;
    try {
        for (auto& it = names_->cbegin(); it != names_->cend(); ++it) {
            const char* d = (*it).c_str();
            if (strcmp(d, name.c_str()) == 0) {
                names_->erase(it);
                erased = true;
                break;
            }
        }
    } catch (itp::interprocess_exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on release name: %s : %s\n", name.c_str(), exc.what());
        return false;
    } catch (std::exception& exc) {
        assert(false);
        veigar::log("Veigar: interprocess exception on release name: %s : %s\n", name.c_str(), exc.what());
        return false;
    }

    return erased;
}

}  // namespace veigar