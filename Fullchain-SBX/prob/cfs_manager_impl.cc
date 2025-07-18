// content/browser/CFS/cfs_manager_impl.cc

#include "content/browser/CFS/cfs_manager_impl.h"

CodegateFSManagerImpl::CodegateFSManagerImpl() : cnt_(0) {}

CodegateFSManagerImpl::~CodegateFSManagerImpl() = default;

void CodegateFSManagerImpl::CreateFileSystem(
    CreateFileSystemCallback callback) {
  std::unique_ptr<CodegateDirectoryImpl> root_dir =
      std::make_unique<CodegateDirectoryImpl>("root");
  auto remote = root_dir->GenerateConnection();
  file_system_list_.emplace(++cnt_, std::move(root_dir));
  std::move(callback).Run(cnt_, std::move(remote));
}

void CodegateFSManagerImpl::DeleteFileSystem(
    uint32_t id,
    DeleteFileSystemCallback callback) {
  if (!file_system_list_[id]) {
    std::move(callback).Run(false);
    return;
  } else {
    file_system_list_.erase(id);
    std::move(callback).Run(true);
    return;
  }
}

void CodegateFSManagerImpl::GetFileSystemHandle(
    uint32_t id,
    GetFileSystemHandleCallback callback) {
  if (!file_system_list_[id]) {
    std::move(callback).Run(false, mojo::PendingRemote<blink::mojom::cfs::CodegateDirectory>());
    return;
  } else {
    CodegateDirectoryImpl* target = file_system_list_[id].get();
    auto remote = target->GenerateConnection();
    std::move(callback).Run(true, std::move(remote));
    return;
  }
}

void CodegateFSManagerImpl::GetCode(GetCodeCallback callback) {
  std::move(callback).Run((uint64_t)(&CodegateFSManagerImpl::Create));
}
