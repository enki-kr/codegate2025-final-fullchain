// third_party/blink/renderer/modules/minishell/mini_shell.cc

#include "third_party/blink/renderer/modules/minishell/mini_shell.h"

#include "base/memory/raw_ptr.h"
#include "third_party/blink/renderer/platform/wtf/text/string_builder.h"

namespace blink {

WTF::String ConvertVectorToHexString(const WTF::Vector<uint8_t>& vec,
                                     size_t size) {
  StringBuilder res;
  size_t count = 0;

  for (uint8_t i : vec) {
    if (count >= size) {
      break;
    }

    if (count++ > 0) {
      res.Append(" ");
    }

    char data[3];
    snprintf(data, sizeof(data), "%02x", i);
    res.Append(data);
  }
  return res.ToString();
}

Vector<WTF::String> ParseShellArguments(const WTF::String& input) {
  Vector<WTF::String> cmd_input;
  if (input.IsNull() || input.empty()) {
    return cmd_input;
  }

  std::string s = input.Utf8();
  size_t total_idx = 0;
  const size_t len = s.length();

  while (total_idx < len) {
    while (total_idx < len &&
           std::isspace(static_cast<unsigned char>(s[total_idx]))) {
      total_idx++;
    }

    if (total_idx == len) {
      break;
    }

    std::string cur_tok;
    if (s[total_idx] == '"') {
      total_idx++;
      while (total_idx < len && s[total_idx] != '"') {
        cur_tok += s[total_idx];
        total_idx++;
      }
      if (total_idx < len && s[total_idx] == '"') {
        total_idx++;
      }
    } else {
      while (total_idx < len &&
             !std::isspace(static_cast<unsigned char>(s[total_idx]))) {
        cur_tok += s[total_idx];
        total_idx++;
      }
    }
    cmd_input.push_back(WTF::String::FromUTF8(cur_tok.c_str()));
  }
  return cmd_input;
}

MiniShell::MiniShell(
    ScriptState* script_state,
    mojo::PendingRemote<mojom::cfs::blink::CodegateDirectory> new_remote,
    uint32_t id)
    : dir_remote_(ExecutionContext::From(script_state)),
      file_descriptor_(nullptr),
      shell_id_(id) {
  dir_remote_.Bind(std::move(new_remote),
                   ExecutionContext::From(script_state)
                       ->GetTaskRunner(TaskType::kInternalDefault));
}

MiniShell::~MiniShell() = default;

uint32_t MiniShell::get_id(ScriptState* script_state,
                           ExceptionState& exception_state) {
  return shell_id_;
}

ScriptPromise<IDLString> MiniShell::execute(ScriptState* script_state,
                                            const String& raw_input,
                                            ExceptionState& exception_state) {
  auto* resolver =
      MakeGarbageCollected<ScriptPromiseResolver<IDLString>>(script_state);
  ScriptPromise<IDLString> promise = resolver->Promise();

  if (!dir_remote_.is_bound()) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kInvalidStateError, "Dead Pipe"));
    return promise;
  }

  Vector<String> cmd_input = ParseShellArguments(raw_input);

  if (cmd_input.empty()) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "Input Error"));
    return promise;
  }

  String command = cmd_input[0];

  if (command == "help") {
    FUNC_HELP(resolver, cmd_input);
  } else if (command == "pwd") {
    FUNC_PWD(resolver, cmd_input);
  } else if (command == "ls") {
    FUNC_LS(resolver, cmd_input);
  } else if (command == "mkdir") {
    FUNC_MKDIR(resolver, cmd_input);
  } else if (command == "cd") {
    FUNC_CD(resolver, cmd_input);
  } else if (command == "touch") {
    FUNC_TOUCH(resolver, cmd_input);
  } else if (command == "delete") {
    FUNC_DELETE(resolver, cmd_input);
  } else if (command == "rename") {
    FUNC_RENAME(resolver, cmd_input);
  } else if (command == "exec") {
    FUNC_EXEC(resolver, cmd_input);
  } else if (command == "mvdir") {
    FUNC_MVDIR(resolver, cmd_input);
  } else if (command == "open") {
    FUNC_OPEN(resolver, cmd_input);
  } else if (command == "read") {
    FUNC_READ(resolver, cmd_input);
  } else if (command == "write") {
    FUNC_WRITE(resolver, cmd_input);
  } else if (command == "seek") {
    FUNC_SEEK(resolver, cmd_input);
  } else if (command == "save") {
    FUNC_SAVE(resolver, cmd_input);
  } else {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "Unknown command: " + command));
  }
  return promise;
}

void MiniShell::FUNC_HELP(ScriptPromiseResolver<IDLString>* resolver,
                          const Vector<String>& cmd_input) {
  if (cmd_input.size() != 1) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "help: too many arguments."));
    return;
  }
  WTF::StringBuilder res;
  res.Append("Available commands:\n");
  res.Append("  help\n");
  res.Append("  pwd\n");
  res.Append("  ls\n");
  res.Append("  mkdir <dirname>\n");
  res.Append("  cd <path>\n");
  res.Append("  touch <filename>\n");
  res.Append("  delete <filename>\n");
  res.Append("  rename <oldname> <newname>\n");
  res.Append("  exec <filename>\n");
  res.Append("  mvdir <src> <dst_parent_dir | new_dir_name_if_renaming>\n");

  res.Append("  open <filepath>\n");
  res.Append("  read <count>\n");
  res.Append("  write <count> {hex1} {hex2} {hex3} . . .\n");
  res.Append("  seek <idx>\n");
  res.Append("  save\n");
  resolver->Resolve(res.ToString());
  return;
}

void MiniShell::FUNC_PWD(ScriptPromiseResolver<IDLString>* resolver,
                         const Vector<String>& cmd_input) {
  if (cmd_input.size() != 1) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "pwd: too many arguments."));
    return;
  }

  GetDirectoryRemote()->GetPwd(WTF::BindOnce(
      [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver,
         const String& current_path) { resolver->Resolve(current_path); },
      WrapPersistent(this), WrapPersistent(resolver)));
}

void MiniShell::FUNC_LS(ScriptPromiseResolver<IDLString>* resolver,
                        const Vector<String>& cmd_input) {
  if (cmd_input.size() != 1) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "ls: too many arguments."));
    return;
  }
  GetDirectoryRemote()->ListItems(WTF::BindOnce(
      [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver,
         const Vector<String>& entries) {
        StringBuilder output;
        for (const auto& entry : entries) {
          output.Append(entry);
          output.Append("\n");
        }
        resolver->Resolve(output.ToString());
      },
      WrapPersistent(this), WrapPersistent(resolver)));
}

void MiniShell::FUNC_MKDIR(ScriptPromiseResolver<IDLString>* resolver,
                           const Vector<String>& cmd_input) {
  if (cmd_input.size() != 2) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "mkdir <dirname>"));
    return;
  }
  String dirname = cmd_input[1];

  GetDirectoryRemote()->CreateItem(
      dirname, blink::mojom::cfs::ITEMTYPE::kDir,
      WTF::BindOnce(
          [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver,
             blink::mojom::cfs::ITEMTYPE type,
             blink::mojom::cfs::blink::CodegateItemResponsePtr item) {
            if (type == blink::mojom::cfs::ITEMTYPE::kDir) {
              resolver->Resolve("Directory created.");
            } else {
              resolver->Reject(MakeGarbageCollected<DOMException>(
                  DOMExceptionCode::kOperationError,
                  "Failed to create directory."));
            }
          },
          WrapPersistent(this), WrapPersistent(resolver)));
}

void MiniShell::FUNC_CD(ScriptPromiseResolver<IDLString>* resolver,
                        const Vector<String>& cmd_input) {
  if (cmd_input.size() != 2) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "cd <path>"));
    return;
  }

  String path = cmd_input[1];
  GetDirectoryRemote()->GetItemHandle(
      path,
      WTF::BindOnce(
          [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver,
             blink::mojom::cfs::ITEMTYPE type,
             blink::mojom::cfs::blink::CodegateItemResponsePtr item) {
            if (type == blink::mojom::cfs::ITEMTYPE::kDir) {
              minishell->SetDirectory(std::move(item->get_remote_dir()),
                                      resolver->GetExecutionContext());
              resolver->Resolve("Changed directory");
            } else if (type == blink::mojom::cfs::ITEMTYPE::kFile) {
              resolver->Reject(MakeGarbageCollected<DOMException>(
                  DOMExceptionCode::kOperationError,
                  "Cannot change directory to a file."));
              return;
            } else {
              resolver->Reject(MakeGarbageCollected<DOMException>(
                  DOMExceptionCode::kNotFoundError, "Directory not found."));
              return;
            }
          },
          WrapPersistent(this), WrapPersistent(resolver)));
}

void MiniShell::FUNC_TOUCH(ScriptPromiseResolver<IDLString>* resolver,
                           const Vector<String>& cmd_input) {
  if (cmd_input.size() != 2) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "touch <filename>"));
    return;
  }
  String filename = cmd_input[1];

  GetDirectoryRemote()->CreateItem(
      filename, blink::mojom::cfs::ITEMTYPE::kFile,
      WTF::BindOnce(
          [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver,
             blink::mojom::cfs::ITEMTYPE type,
             blink::mojom::cfs::blink::CodegateItemResponsePtr item) {
            if (type == blink::mojom::cfs::ITEMTYPE::kFile) {
              resolver->Resolve("File touched.");
            } else {
              resolver->Reject(MakeGarbageCollected<DOMException>(
                  DOMExceptionCode::kOperationError, "Failed to touch file."));
            }
          },
          WrapPersistent(this), WrapPersistent(resolver)));
}

void MiniShell::FUNC_DELETE(ScriptPromiseResolver<IDLString>* resolver,
                            const Vector<String>& cmd_input) {
  if (cmd_input.size() != 2) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "delete <filename>"));
    return;
  }

  if (GetBuffer()) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kAbortError, "Save File First"));
    return;
  }

  String filename_to_delete = cmd_input[1];

  GetDirectoryRemote()->DeleteItem(
      filename_to_delete,
      WTF::BindOnce(
          [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver,
             bool success) {
            if (success) {
              resolver->Resolve("File deleted.");
            } else {
              resolver->Reject(MakeGarbageCollected<DOMException>(
                  DOMExceptionCode::kOperationError, "Failed to delete file."));
            }
          },
          WrapPersistent(this), WrapPersistent(resolver)));
}

void MiniShell::FUNC_RENAME(ScriptPromiseResolver<IDLString>* resolver,
                            const Vector<String>& cmd_input) {
  if (cmd_input.size() != 3) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "rename <oldname> <newname>"));
    return;
  }
  String old_name = cmd_input[1];
  String new_name = cmd_input[2];

  GetDirectoryRemote()->RenameItem(
      old_name, new_name,
      WTF::BindOnce(
          [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver,
             bool success) {
            if (success) {
              resolver->Resolve("File renamed.");
            } else {
              resolver->Reject(MakeGarbageCollected<DOMException>(
                  DOMExceptionCode::kOperationError, "Failed to rename file."));
            }
          },
          WrapPersistent(this), WrapPersistent(resolver)));
}

void MiniShell::FUNC_EXEC(ScriptPromiseResolver<IDLString>* resolver,
                          const Vector<String>& cmd_input) {
  resolver->Reject(MakeGarbageCollected<DOMException>(
      DOMExceptionCode::kAbortError, "Not Implement."));
}

void MiniShell::FUNC_MVDIR(ScriptPromiseResolver<IDLString>* resolver,
                           const Vector<String>& cmd_input) {
  if (cmd_input.size() != 3) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "mvdir <src> <dst>"));
    return;
  }
  String source = cmd_input[1];
  String destination = cmd_input[2];

  GetDirectoryRemote()->ChangeItemLocation(
      source, destination,
      WTF::BindOnce(
          [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver,
             blink::mojom::cfs::ITEMTYPE type,
             blink::mojom::cfs::blink::CodegateItemResponsePtr item) {
            if (type == blink::mojom::cfs::ITEMTYPE::kFailed) {
              resolver->Reject(MakeGarbageCollected<DOMException>(
                  DOMExceptionCode::kOperationError, "Failed to move."));
            } else {
              resolver->Resolve("Moved successfully.");
            }
          },
          WrapPersistent(this), WrapPersistent(resolver)));
}

void MiniShell::FUNC_OPEN(ScriptPromiseResolver<IDLString>* resolver,
                          const Vector<String>& cmd_input) {
  if (cmd_input.size() != 2) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "open <filepath>"));
    return;
  }

  if (GetBuffer() != nullptr) {
    resolver->Resolve("Close Current File Handle");
    return;
  }

  String filepath = cmd_input[1];

  GetDirectoryRemote()->GetItemHandle(
      filepath,
      WTF::BindOnce(
          [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver,
             blink::mojom::cfs::ITEMTYPE type,
             blink::mojom::cfs::blink::CodegateItemResponsePtr item) {
            if (type != blink::mojom::cfs::ITEMTYPE::kFile) {
              resolver->Reject(MakeGarbageCollected<DOMException>(
                  DOMExceptionCode::kNotFoundError, "Failed to open file."));
              return;
            }
            minishell->SetBuffer(std::move(item->get_remote_file()),
                                 WrapPersistent(resolver));
          },
          WrapPersistent(this), WrapPersistent(resolver)));
}

void MiniShell::FUNC_READ(ScriptPromiseResolver<IDLString>* resolver,
                          const Vector<String>& cmd_input) {
  if (cmd_input.size() != 2) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "read <count>"));
    return;
  }

  if (GetBuffer() == nullptr) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kAbortError, "Open File First"));
    return;
  }

  uint64_t read_size = std::stoull(cmd_input[1].Ascii().c_str());

  Vector<uint8_t> res = GetBuffer()->read(read_size);
  resolver->Resolve(ConvertVectorToHexString(res, res.size()));
  return;
}

void MiniShell::FUNC_WRITE(ScriptPromiseResolver<IDLString>* resolver,
                           const Vector<String>& cmd_input) {
  if (cmd_input.size() < 3) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "write <count> {hex1}  . . ."));
    return;
  }
  if (GetBuffer() == nullptr) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kAbortError, "Open File First"));
    return;
  }

  uint64_t write_size = std::stoull(cmd_input[1].Ascii().c_str());
  if (write_size + 2 != cmd_input.size()) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError,
        "Invalid Write Size : write <count> {hex1}  . . ."));
    return;
  }

  Vector<uint8_t> write_data;
  for (size_t i = 2; i < cmd_input.size(); ++i) {
    write_data.push_back(std::stoul(cmd_input[i].Ascii().c_str(), nullptr, 16));
  }

  GetBuffer()->write(write_data);
  resolver->Resolve("write successed");
}

void MiniShell::FUNC_SEEK(ScriptPromiseResolver<IDLString>* resolver,
                          const Vector<String>& cmd_input) {
  if (cmd_input.size() != 2) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "seek <idx>"));
    return;
  }

  if (GetBuffer() == nullptr) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kAbortError, "Open File First"));
    return;
  }

  uint64_t seek_idx = std::stoull(cmd_input[1].Ascii().c_str());
  GetBuffer()->SetIdx(seek_idx);
  resolver->Resolve("Seek successed");
}

void MiniShell::FUNC_SAVE(ScriptPromiseResolver<IDLString>* resolver,
                          const Vector<String>& cmd_input) {
  if (cmd_input.size() != 1) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kSyntaxError, "save: too many arguments."));
    return;
  }

  if (GetBuffer() == nullptr) {
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kAbortError, "Open File First"));
    return;
  }

  Vector<uint8_t> data_write = GetBuffer()->read(FILESIZE_MAX);

  auto file_write_callback = WTF::BindOnce(
      [](MiniShell* minishell, ScriptPromiseResolver<IDLString>* resolver, bool success) {
        if (success) {
          minishell->GetFileRemote()->Close(base::NullCallback());
          minishell->ResetBuffer();
          resolver->Resolve("File saved.");
        } else {
          resolver->Reject(MakeGarbageCollected<DOMException>(
              DOMExceptionCode::kOperationError, "Failed to write."));
        }
      },
      WrapPersistent(this), WrapPersistent(resolver));

  GetFileRemote()->Write(data_write, std::move(file_write_callback));
}

void MiniShell::SetDirectory(
    mojo::PendingRemote<mojom::cfs::blink::CodegateDirectory> new_dir_remote,
    ExecutionContext* execution_context) {
  dir_remote_.reset();
  dir_remote_.Bind(std::move(new_dir_remote), execution_context->GetTaskRunner(
                                                  TaskType::kInternalDefault));
}

void MiniShell::SetBuffer(
    mojo::PendingRemote<mojom::cfs::blink::CodegateFile> new_file_remote,
    ScriptPromiseResolver<IDLString>* resolver) {
  file_descriptor_ =
      MakeGarbageCollected<FileBuffer>(std::move(new_file_remote), resolver);
}

void MiniShell::ResetBuffer() {
  file_descriptor_ = nullptr;
}

mojom::cfs::blink::CodegateDirectory* MiniShell::GetDirectoryRemote() {
  return dir_remote_.get();
}

mojom::cfs::blink::CodegateFile* MiniShell::GetFileRemote() {
  return file_descriptor_ ? file_descriptor_->GetRemote() : nullptr;
}

FileBuffer* MiniShell::GetBuffer() {
  return file_descriptor_.Get();
}

void MiniShell::Trace(Visitor* visitor) const {
  visitor->Trace(dir_remote_);
  visitor->Trace(file_descriptor_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
