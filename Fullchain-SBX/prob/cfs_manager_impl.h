#ifndef CONTENT_BROWSER_CFS_CFS_MANAGER_IMPL_H_
#define CONTENT_BROWSER_CFS_CFS_MANAGER_IMPL_H_

// library
#include <cstdint>

// content
#include "content/browser/CFS/cfs_directory_impl.h"
#include "content/browser/CFS/cfs_file_impl.h"

// mojo dependency
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

// mojo IPC Rule
#include "third_party/blink/public/mojom/CFS/cfs.mojom.h"

class CodegateDirectoryImpl;

class CodegateFSManagerImpl : public blink::mojom::cfs::CodegateFSManager {
 public:
  static void Create(
      mojo::PendingReceiver<blink::mojom::cfs::CodegateFSManager> receiver) {
    mojo::MakeSelfOwnedReceiver(std::make_unique<CodegateFSManagerImpl>(),
                                std::move(receiver));
  }

  CodegateFSManagerImpl();
  ~CodegateFSManagerImpl() override;

  void CreateFileSystem(
      CreateFileSystemCallback callback) override;
  void DeleteFileSystem(uint32_t id,
                        DeleteFileSystemCallback callback) override;
  void GetFileSystemHandle(
      uint32_t id,
      GetFileSystemHandleCallback callback) override;

  void GetCode(GetCodeCallback callback) override;
 private:
  uint32_t cnt_;
  std::map<uint32_t, std::unique_ptr<CodegateDirectoryImpl>> file_system_list_;
};
#endif  // CONTENT_BROWSER_CFS_CFS_MANAGER_IMPL_H_
