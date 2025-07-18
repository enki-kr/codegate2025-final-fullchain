// content/browser/CFS/cfs_file_impl.cc

// content
#include "content/browser/CFS/cfs_file_impl.h"

#include "content/browser/CFS/cfs_directory_impl.h"
#include "content/browser/CFS/cfs_manager_impl.h"

// Base
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"

CodegateFileImpl::CodegateFileImpl(const std::string& filename)
    : CodegateItem(filename, TYPE_FILE) {}

CodegateFileImpl::~CodegateFileImpl() {}

void CodegateFileImpl::GetFilename(GetFilenameCallback callback) {
  std::move(callback).Run(GetItemName());
}

void CodegateFileImpl::Write(const std::vector<uint8_t>& data,
                             WriteCallback callback) {
  data_buffer_ = data;
  std::move(callback).Run(true);
}

void CodegateFileImpl::Read(ReadCallback callback) {
  std::vector<uint8_t> data = data_buffer_;
  std::move(callback).Run(true, data);
}

void CodegateFileImpl::Edit(uint32_t idx, uint8_t value, EditCallback callback) {
  if (idx < data_buffer_.size()) {
    data_buffer_[idx] = value;
    std::move(callback).Run(true);
  } else {
    std::move(callback).Run(false);
  }
}

void CodegateFileImpl::Close(CloseCallback callback) {
  OnReceiverDisconnect();
  std::move(callback).Run(true);
}

void CodegateFileImpl::AddReceiver(
    mojo::PendingReceiver<blink::mojom::cfs::CodegateFile> receiver) {
  receivers_.Add(this, std::move(receiver));
  receivers_.set_disconnect_handler(base::BindRepeating(
      &CodegateFileImpl::OnReceiverDisconnect, base::Unretained(this)));
}

mojo::PendingRemote<blink::mojom::cfs::CodegateFile>
CodegateFileImpl::GenerateConnection() {
  mojo::PendingRemote<blink::mojom::cfs::CodegateFile> remote;
  AddReceiver(remote.InitWithNewPipeAndPassReceiver());
  return remote;
}

  void CodegateFileImpl::OnReceiverDisconnect() {
    if (receivers_.current_receiver()) {
      receivers_.Remove(receivers_.current_receiver());
    }
  }
