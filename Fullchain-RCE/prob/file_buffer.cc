#include "third_party/blink/renderer/modules/minishell/mini_shell.h"

namespace blink {

FileBuffer::FileBuffer(
    mojo::PendingRemote<mojom::cfs::blink::CodegateFile> new_file_remote,
    ScriptPromiseResolver<IDLString>* resolver)
    : remote_(ExecutionContext::From(resolver->GetScriptState())), idx_(0) {
  remote_.Bind(std::move(new_file_remote),
               ExecutionContext::From(resolver->GetScriptState())
                   ->GetTaskRunner(TaskType::kInternalDefault));
  GetRemote()->Read(WTF::BindOnce(
      [](FileBuffer* filebuffer, ScriptPromiseResolver<IDLString>* resolver,
         bool success, const std::optional<WTF::Vector<uint8_t>>& content) {
        if (success) {
          filebuffer->write(content.value());
          resolver->Resolve("File opened");
        } else {
          resolver->Reject(MakeGarbageCollected<DOMException>(
              DOMExceptionCode::kOperationError, "Failed to open file."));
        }
      },
      WrapPersistent(this), WrapPersistent(resolver)));
}

Vector<uint8_t> FileBuffer::read(uint64_t count) {
  Vector<uint8_t> res(count);

  if (count > FILESIZE_MAX) {
    return Vector<uint8_t>();
  }

  for (uint64_t i = 0; i < count; i++) {
    res[i] = buffer_[idx_ + i];
  }
  return res;
}

void FileBuffer::write(const Vector<uint8_t>& data) {
  if (data.size() > FILESIZE_MAX) {
    return;
  }

  for (uint64_t i = 0; i < data.size(); i++) {
    buffer_[idx_ + i] = data[i];
  }
}

void FileBuffer::SetIdx(uint64_t idx) {
  idx_ = idx;
}

void FileBuffer::Trace(Visitor* visitor) const {
  visitor->Trace(remote_);
}

mojom::cfs::blink::CodegateFile* FileBuffer::GetRemote() {
  return remote_.get();
}

}  // namespace blink
