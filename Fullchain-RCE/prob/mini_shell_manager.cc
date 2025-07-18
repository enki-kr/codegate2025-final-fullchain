// third_party/blink/renderer/modules/minishell/mini_shell_manager.cc

#include "third_party/blink/renderer/modules/minishell/mini_shell_manager.h"

#include "base/functional/bind.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"

namespace blink {

MiniShellManager::MiniShellManager(ExecutionContext* context)
    : fs_manager_remote_(context) {}

mojom::cfs::blink::CodegateFSManager* MiniShellManager::GetFSManagerService(
    ScriptState* script_state) {
  if (!fs_manager_remote_.is_bound()) {
    ExecutionContext::From(script_state)
        ->GetBrowserInterfaceBroker()
        .GetInterface(fs_manager_remote_.BindNewPipeAndPassReceiver(
            ExecutionContext::From(script_state)
                ->GetTaskRunner(TaskType::kInternalDefault)));
  }
  return fs_manager_remote_.get();
}

ScriptPromise<MiniShell> MiniShellManager::CreateShell(
    ScriptState* script_state,
    ExceptionState& exception_state) {
  auto* resolver =
      MakeGarbageCollected<ScriptPromiseResolver<MiniShell>>(script_state);
  ScriptPromise<MiniShell> promise = resolver->Promise();

  GetFSManagerService(script_state)
      ->CreateFileSystem(WTF::BindOnce(&MiniShellManager::OnCreateFileSystem,
                                       WrapPersistent(this),
                                       WrapPersistent(resolver)));

  return promise;
}

ScriptPromise<IDLBoolean> MiniShellManager::DeleteShell(
    ScriptState* script_state,
    uint32_t shell_id,
    ExceptionState& exception_state) {
  auto* resolver =
      MakeGarbageCollected<ScriptPromiseResolver<IDLBoolean>>(script_state);
  ScriptPromise<IDLBoolean> promise = resolver->Promise();

  GetFSManagerService(script_state)
      ->DeleteFileSystem(shell_id,
                         WTF::BindOnce(&MiniShellManager::OnDeleteFileSystem,
                                       WrapPersistent(this),
                                       WrapPersistent(resolver), shell_id));

  return promise;
}

ScriptPromise<MiniShell> MiniShellManager::get(
    ScriptState* script_state,
    uint32_t shell_id,
    ExceptionState& exception_state) {
  auto* resolver =
      MakeGarbageCollected<ScriptPromiseResolver<MiniShell>>(script_state);
  ScriptPromise<MiniShell> promise = resolver->Promise();

  auto it = active_shells_.find(shell_id);

  if (it != active_shells_.end()) {
    resolver->Resolve(it->value);
  } else {
    GetFSManagerService(script_state)
        ->GetFileSystemHandle(
            shell_id,
            WTF::BindOnce(
                [](MiniShellManager* minishellmanager, uint32_t shell_id,
                   ScriptPromiseResolver<MiniShell>* resolver, bool success,
                   mojo::PendingRemote<mojom::cfs::blink::CodegateDirectory>
                       new_dir_remote) {
                  if (!success) {
                    resolver->Reject(MakeGarbageCollected<DOMException>(
                        DOMExceptionCode::kOperationError,
                        "Failed to get file system handle."));
                    return;
                  }
                  auto* new_shell = MakeGarbageCollected<MiniShell>(
                      resolver->GetScriptState(), std::move(new_dir_remote),
                      shell_id);
                  minishellmanager->Add(shell_id, new_shell);
                  resolver->Resolve(new_shell);
                },
                WrapPersistent(this), shell_id, WrapPersistent(resolver)));
  }

  return promise;
}

void MiniShellManager::OnCreateFileSystem(
    ScriptPromiseResolver<MiniShell>* resolver,
    uint32_t id,
    mojo::PendingRemote<mojom::cfs::blink::CodegateDirectory> new_remote) {
  auto* new_shell = MakeGarbageCollected<MiniShell>(resolver->GetScriptState(),
                                                    std::move(new_remote), id);
  Add(id, new_shell);
  resolver->Resolve(new_shell);
}

void MiniShellManager::OnDeleteFileSystem(
    ScriptPromiseResolver<IDLBoolean>* resolver,
    uint32_t id,
    bool success) {
  if (!success) {
    Sub(id);
    resolver->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kOperationError, "Failed to delete file system."));
    return;
  } else {
    resolver->Resolve(true);
  }
}

void MiniShellManager::Add(uint32_t id, MiniShell* minishell) {
  active_shells_.Set(id, minishell);
}
void MiniShellManager::Sub(uint32_t id) {
  active_shells_.Take(id);
}
void MiniShellManager::Trace(Visitor* visitor) const {
  visitor->Trace(fs_manager_remote_);
  visitor->Trace(active_shells_);
  ScriptWrappable::Trace(visitor);
}
}  // namespace blink
