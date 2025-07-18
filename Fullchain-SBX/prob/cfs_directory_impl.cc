// content/browser/CFS/cfs_directory_impl.cc

#include "content/browser/CFS/cfs_directory_impl.h"

#include "content/browser/CFS/cfs_file_impl.h"
#include "content/browser/CFS/cfs_manager_impl.h"

// Base
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/thread_pool.h"

CodegateDirectoryImpl::CodegateDirectoryImpl(const std::string& path)
    : CodegateItem(path, TYPE_DIRECTORY) {}

CodegateDirectoryImpl::~CodegateDirectoryImpl() = default;

void CodegateDirectoryImpl::GetItemHandle(const std::string& itemname,
                                          GetItemHandleCallback callback) {
  blink::mojom::cfs::ITEMTYPE item_type;

  CodegateItem* item = FindItemByName(itemname);
  if (!item) {
    LOG(ERROR) << "Failed to find item: " << itemname;
    item_type = blink::mojom::cfs::ITEMTYPE::kFailed;
    std::move(callback).Run(item_type, nullptr);
    return;
  }

  int filetype = item->GetItemType();
  switch (filetype) {
    case TYPE_FILE: {
      item_type = blink::mojom::cfs::ITEMTYPE::kFile;
      auto* file = static_cast<CodegateFileImpl*>(item);
      auto remote_file = file->GenerateConnection();
      blink::mojom::cfs::CodegateItemResponsePtr response =
          blink::mojom::cfs::CodegateItemResponse::NewRemoteFile(
              std::move(remote_file));
      std::move(callback).Run(item_type, std::move(response));
    } break;
    case TYPE_DIRECTORY: {
      item_type = blink::mojom::cfs::ITEMTYPE::kDir;
      auto* dir = static_cast<CodegateDirectoryImpl*>(item);
      auto remote_dir = dir->GenerateConnection();
      blink::mojom::cfs::CodegateItemResponsePtr response =
          blink::mojom::cfs::CodegateItemResponse::NewRemoteDir(
              std::move(remote_dir));
      std::move(callback).Run(item_type, std::move(response));
    } break;
    default:
      item_type = blink::mojom::cfs::ITEMTYPE::kFailed;
      std::move(callback).Run(item_type, nullptr);
      break;
  }
}

void CodegateDirectoryImpl::CreateItem(const std::string& itemname,
                                       blink::mojom::cfs::ITEMTYPE type,
                                       CreateItemCallback callback) {
  blink::mojom::cfs::ITEMTYPE item_type;
  switch (type) {
    case blink::mojom::cfs::ITEMTYPE::kFile: {
      auto new_file = std::make_unique<CodegateFileImpl>(itemname);
      auto remote_file = new_file->GenerateConnection();
      if (AddItemInternal(std::move(new_file))) {
        item_type = blink::mojom::cfs::ITEMTYPE::kFile;

        blink::mojom::cfs::CodegateItemResponsePtr response =
            blink::mojom::cfs::CodegateItemResponse::NewRemoteFile(
                std::move(remote_file));
        std::move(callback).Run(item_type, std::move(response));
      }
    } break;

    case blink::mojom::cfs::ITEMTYPE::kDir: {
      auto new_directory = std::make_unique<CodegateDirectoryImpl>(itemname);
      auto remote_dir = new_directory->GenerateConnection();
      if (AddItemInternal(std::move(new_directory))) {
        item_type = blink::mojom::cfs::ITEMTYPE::kDir;
        blink::mojom::cfs::CodegateItemResponsePtr response =
            blink::mojom::cfs::CodegateItemResponse::NewRemoteDir(
                std::move(remote_dir));
        std::move(callback).Run(item_type, std::move(response));
      }
    } break;

    default:
      item_type = blink::mojom::cfs::ITEMTYPE::kFailed;
      LOG(ERROR) << "Failed to create item: " << itemname;
      std::move(callback).Run(item_type,
                              blink::mojom::cfs::CodegateItemResponsePtr());

      break;
  }
}

void CodegateDirectoryImpl::DeleteItem(const std::string& itemname,
                                       DeleteItemCallback callback) {
  std::unique_ptr<CodegateItem> removed_item = RemoveItemByName(itemname);
  bool success = (removed_item != nullptr);

  std::move(callback).Run(success);
}

void CodegateDirectoryImpl::RenameItem(const std::string& filename_orig,
                                       const std::string& filename_new,
                                       RenameItemCallback callback) {
  if (!IsItemNameExists(filename_orig) && IsItemNameExists(filename_new)) {
    std::move(callback).Run(false);
    return;
  }

  CodegateItem* target_file = FindItemByName(filename_orig);
  target_file->SetItemName(filename_new);

  std::move(callback).Run(true);
}

void CodegateDirectoryImpl::ChangeItemLocation(
    const std::string& filename_src,
    const std::string& dst_dir,
    ChangeItemLocationCallback callback) {
  CodegateDirectoryImpl* destination_directory =
      ValidateChangeLocation(filename_src, dst_dir);

  if (!destination_directory) {
    blink::mojom::cfs::ITEMTYPE item_type =
        blink::mojom::cfs::ITEMTYPE::kFailed;
    std::move(callback).Run(item_type,
                            blink::mojom::cfs::CodegateItemResponsePtr());
    return;
  }

  std::unique_ptr<CodegateItem> file_to_move = RemoveItemByName(filename_src);

  struct backup backup_info;
  backup_info.itemname = file_to_move->GetItemName();
  backup_info.filetype = file_to_move->GetItemType();

  if (!destination_directory->AddItemInternal(std::move(file_to_move))) {
    base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(&CodegateDirectoryImpl::RecoverItem,
                                  weak_factory_.GetWeakPtr(), backup_info,
                                  destination_directory, std::move(callback)));
    return;
  }

  blink::mojom::cfs::ITEMTYPE item_type = blink::mojom::cfs::ITEMTYPE::kFailed;
  std::move(callback).Run(item_type,
                          blink::mojom::cfs::CodegateItemResponsePtr());
}

void CodegateDirectoryImpl::ListItems(ListItemsCallback callback) {
  std::vector<std::string> item_list = GetItemNameList();
  std::move(callback).Run(item_list);
}

void CodegateDirectoryImpl::GetPwd(GetPwdCallback callback) {
  std::string full_path;
  CodegateDirectoryImpl* current_directory = this;

  while (current_directory != nullptr) {
    full_path = "/" + current_directory->GetItemName() + full_path;
    current_directory = current_directory->GetParentDir();
  }

  std::move(callback).Run(full_path);
}

void CodegateDirectoryImpl::AddReceiver(
    mojo::PendingReceiver<blink::mojom::cfs::CodegateDirectory> receiver) {
  receivers_.Add(this, std::move(receiver));
  receivers_.set_disconnect_handler(base::BindRepeating(
      &CodegateDirectoryImpl::OnReceiverDisconnect, base::Unretained(this)));
}

mojo::PendingRemote<blink::mojom::cfs::CodegateDirectory>
CodegateDirectoryImpl::GenerateConnection() {
  mojo::PendingRemote<blink::mojom::cfs::CodegateDirectory> remote;
  AddReceiver(remote.InitWithNewPipeAndPassReceiver());
  return remote;
}

void CodegateDirectoryImpl::OnReceiverDisconnect() {
  if (receivers_.current_receiver()) {
    receivers_.Remove(receivers_.current_receiver());
  }
}
// Private
bool CodegateDirectoryImpl::AddItemInternal(
    std::unique_ptr<CodegateItem> new_item) {
  if (IsItemNameExists(new_item->GetItemName())) {
    return false;
  }

  new_item->SetParentDir(this);
  item_list_.push_back(std::move(new_item));
  return true;
}

void CodegateDirectoryImpl::RecoverItem(
    struct backup backup_info,
    CodegateDirectoryImpl* destination_directory,
    ChangeItemLocationCallback callback) {
  blink::mojom::cfs::ITEMTYPE item_type;
  blink::mojom::cfs::CodegateItemResponsePtr response;
  std::unique_ptr<CodegateItem> recovered_file;

  if (backup_info.filetype == TYPE_DIRECTORY) {
    auto new_dir =
        std::make_unique<CodegateDirectoryImpl>(backup_info.itemname + "_swp");
    auto remote_dir = new_dir->GenerateConnection();
    response = blink::mojom::cfs::CodegateItemResponse::NewRemoteDir(
        std::move(remote_dir));
    item_type = blink::mojom::cfs::ITEMTYPE::kDir;
    recovered_file = std::move(new_dir);
  } else {
    auto new_file =
        std::make_unique<CodegateFileImpl>(backup_info.itemname + ".swp");
    auto remote_file = new_file->GenerateConnection();
    response = blink::mojom::cfs::CodegateItemResponse::NewRemoteFile(
        std::move(remote_file));
    item_type = blink::mojom::cfs::ITEMTYPE::kFile;
    recovered_file = std::move(new_file);
  }

  if (!destination_directory->AddItemInternal(std::move(recovered_file))) {
    std::move(callback).Run(item_type,
                            blink::mojom::cfs::CodegateItemResponsePtr());
    LOG(ERROR) << "Failed to recover item: " << backup_info.itemname;
    return;
  }

  std::move(callback).Run(item_type, std::move(response));
}

CodegateDirectoryImpl* CodegateDirectoryImpl::ValidateChangeLocation(
    const std::string& itemname,
    const std::string& dst_dir) {
  if (!IsItemNameExists(itemname) || !IsItemNameExists(dst_dir)) {
    return nullptr;
  }

  if(itemname == ".." || itemname == ".") {
    return nullptr;
  }
  CodegateDirectoryImpl* dst = nullptr;
  if (dst_dir == ".") {
    dst = this;
  } else if (dst_dir == "..") {
    dst = GetParentDir();
  } else if (IsValidDirectory(dst_dir)) {
    dst = static_cast<CodegateDirectoryImpl*>(FindItemByName(dst_dir));
  }

  return dst;
}

CodegateItem* CodegateDirectoryImpl::FindItemByName(
    const std::string& name) const {
  if (name == "..") {
    return static_cast<CodegateItem*>(GetParentDir());
  }

  auto it = std::find_if(
      item_list_.begin(), item_list_.end(),
      [&name](const auto& file) { return file->GetItemName() == name; });

  return it != item_list_.end() ? it->get() : nullptr;
}

std::unique_ptr<CodegateItem> CodegateDirectoryImpl::RemoveItemByName(
    const std::string& name) {
  auto it = std::find_if(
      item_list_.begin(), item_list_.end(),
      [&name](const auto& file) { return file->GetItemName() == name; });

  if (it != item_list_.end()) {
    std::unique_ptr<CodegateItem> result = std::move(*it);
    item_list_.erase(it);
    return result;
  }

  return nullptr;
}

bool CodegateDirectoryImpl::IsItemNameExists(
    const std::string& itemname) const {
  return FindItemByName(itemname) != nullptr;
}

bool CodegateDirectoryImpl::IsValidFile(const std::string& itemname) const {
  CodegateItem* target_file = FindItemByName(itemname);
  return target_file != nullptr && target_file->GetItemType() == TYPE_FILE;
}

bool CodegateDirectoryImpl::IsValidDirectory(const std::string& dirname) const {
  CodegateItem* target_dir = FindItemByName(dirname);
  return target_dir != nullptr && target_dir->GetItemType() == TYPE_DIRECTORY;
}

std::vector<std::string> CodegateDirectoryImpl::GetItemNameList() const {
  std::vector<std::string> result;
  result.reserve(item_list_.size());

  for (const auto& file : item_list_) {
    if (file->GetItemType() == TYPE_DIRECTORY) {
      result.push_back("/" + file->GetItemName());
    } else {
      result.push_back(file->GetItemName());
    }
  }

  return result;
}
