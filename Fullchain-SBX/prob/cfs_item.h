#ifndef CONTENT_BROWSER_CFS_CFS_ITEM_H
#define CONTENT_BROWSER_CFS_CFS_ITEM_H

#define TYPE_FILE 1
#define TYPE_DIRECTORY 2

#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"

class CodegateDirectoryImpl;

class CodegateItem {
 public:
  CodegateItem(std::string itemname, int itemtype) : 
      itemtype_(itemtype), itemname_(std::move(itemname)), parents_dir_(nullptr) {}
  virtual ~CodegateItem() = default;

  CodegateDirectoryImpl* GetParentDir() const { return parents_dir_.get(); }
  void SetParentDir(CodegateDirectoryImpl* t) { parents_dir_ = t; }

  std::string GetItemName() const { return itemname_; }
  void SetItemName(std::string newname) { itemname_ = newname; }
  int GetItemType() const { return itemtype_; }

 private:
  int itemtype_;
  std::string itemname_;
  raw_ptr<CodegateDirectoryImpl> parents_dir_;
};
#endif  // CONTENT_BROWSER_CFS_CFS_ITEM_H
