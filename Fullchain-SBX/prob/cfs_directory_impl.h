#ifndef CONTENT_BROWSER_CFS_CFS_DIRECTORY_IMPL_H_
#define CONTENT_BROWSER_CFS_CFS_DIRECTORY_IMPL_H_

// library
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

// content
#include "content/browser/CFS/cfs_file_impl.h"
#include "content/browser/CFS/cfs_item.h"
#include "content/browser/CFS/cfs_manager_impl.h"

// base
#include "base/memory/weak_ptr.h"
#include "base/task/sequenced_task_runner.h"

// mojo dependency
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

// mojo IPC Rule
#include "third_party/blink/public/mojom/CFS/cfs.mojom.h"

struct backup {
  std::string itemname;
  int filetype;
};

class CodegateFileImpl;

class CodegateDirectoryImpl
    : public blink::mojom::cfs::CodegateDirectory,
      public CodegateItem {
 public:
  explicit CodegateDirectoryImpl(const std::string& path);
  ~CodegateDirectoryImpl() override;

  CodegateDirectoryImpl(const CodegateDirectoryImpl&) = delete;
  CodegateDirectoryImpl& operator=(const CodegateDirectoryImpl&) = delete;

  // Mojo IDL
  void GetItemHandle(const std::string& itemname,
                     GetItemHandleCallback callback) override;

  void CreateItem(const std::string& itemname,
                  blink::mojom::cfs::ITEMTYPE type,
                  CreateItemCallback callback) override;

  void DeleteItem(const std::string& itemname,
                  DeleteItemCallback callback) override;

  void RenameItem(const std::string& itemname_orig,
                  const std::string& itemname_new,
                  RenameItemCallback callback) override;

  void ChangeItemLocation(const std::string& itemname_src,
                          const std::string& itemname_dst,
                          ChangeItemLocationCallback callback) override;

  void ListItems(ListItemsCallback callback) override;

  void GetPwd(GetPwdCallback callback) override;

  void AddReceiver(mojo::PendingReceiver<blink::mojom::cfs::CodegateDirectory> receiver);
  mojo::PendingRemote<blink::mojom::cfs::CodegateDirectory> GenerateConnection();
    void OnReceiverDisconnect();



 private:
  bool AddItemInternal(std::unique_ptr<CodegateItem> new_item);
  void RecoverItem(struct backup backup_info,
                   CodegateDirectoryImpl* destination_directory,
                   ChangeItemLocationCallback callback);

  CodegateDirectoryImpl* ValidateChangeLocation(const std::string& itemname,
                                                const std::string& dst_dir);

  CodegateItem* FindItemByName(const std::string& name) const;
  std::unique_ptr<CodegateItem> RemoveItemByName(const std::string& name);

  bool IsItemNameExists(const std::string& itemname) const;
  bool IsValidFile(const std::string& itemname) const;
  bool IsValidDirectory(const std::string& dirname) const;

  std::vector<std::string> GetItemNameList() const;

  mojo::ReceiverSet<blink::mojom::cfs::CodegateDirectory> receivers_;
  std::vector<std::unique_ptr<CodegateItem>> item_list_;
  base::WeakPtrFactory<CodegateDirectoryImpl> weak_factory_{this};
};
#endif  // CONTENT_BROWSER_CFS_CFS_DIRECTORY_IMPL_H_
