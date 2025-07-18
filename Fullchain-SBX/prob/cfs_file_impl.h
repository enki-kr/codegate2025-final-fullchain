#ifndef CONTENT_BROWSER_CFS_CFS_FILE_IMPL_H_
#define CONTENT_BROWSER_CFS_CFS_FILE_IMPL_H_

// library
#include <map>
#include <string>
#include <vector>

// content
#include "content/browser/CFS/cfs_directory_impl.h"
#include "content/browser/CFS/cfs_item.h"
#include "content/browser/CFS/cfs_manager_impl.h"

// mojo dependency
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

// mojo IPC Rule
#include "third_party/blink/public/mojom/CFS/cfs.mojom.h"

class CodegateDirectoryImpl;

class CodegateFileImpl
    : public blink::mojom::cfs::CodegateFile,
      public CodegateItem {
 public:
  explicit CodegateFileImpl(const std::string& filename);
  ~CodegateFileImpl() override;

  CodegateFileImpl(const CodegateFileImpl&) = delete;
  CodegateFileImpl& operator=(const CodegateFileImpl&) = delete;

  // Mojo IDL
  void GetFilename(GetFilenameCallback callback) override;
  void Write(const std::vector<uint8_t>& data, WriteCallback callback) override;
  void Read(ReadCallback callback) override;
  void Edit(uint32_t idx, uint8_t value, EditCallback callback) override;
  void Close(CloseCallback callback) override;

    void AddReceiver(mojo::PendingReceiver<blink::mojom::cfs::CodegateFile> receiver);
  mojo::PendingRemote<blink::mojom::cfs::CodegateFile> GenerateConnection();
  void OnReceiverDisconnect();
 private:
  mojo::ReceiverSet<blink::mojom::cfs::CodegateFile> receivers_;
  std::vector<uint8_t> data_buffer_;
  base::WeakPtrFactory<CodegateFileImpl> weak_factory_{this};
};
#endif  // CONTENT_BROWSER_CFS_CFS_FILE_IMPL_H_
