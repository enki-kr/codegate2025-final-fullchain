#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_MINISHELL_MINISHELL_MANAGER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_MINISHELL_MINISHELL_MANAGER_H_

// mojom
#include "third_party/blink/public/mojom/cfs/cfs.mojom-blink.h"

// blink dependency
#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_resolver.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/modules/minishell/mini_shell.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/bindings/script_state.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/mojo/heap_mojo_remote.h"

// Window Obj
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/platform/supplementable.h"

namespace blink {

class MODULES_EXPORT MiniShellManager final : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  explicit MiniShellManager(ExecutionContext* context);

  // IDL methods
  // [CallWith=ScriptState, RaisesException] Promise<MiniShell> CreateShell();
  ScriptPromise<MiniShell> CreateShell(ScriptState* script_state,
                                       ExceptionState& exception_state);

  // [CallWith=ScriptState, RaisesException] Promise<bool> DeleteShell();
  ScriptPromise<IDLBoolean> DeleteShell(ScriptState* script_state,
                                        uint32_t shell_id,
                                        ExceptionState& exception_state);

  // [CallWith=ScriptState, RaisesException] Promise<MiniShell> get(uint32_t
  // id);
  ScriptPromise<MiniShell> get(ScriptState* script_state,
                               uint32_t shell_id,
                               ExceptionState& exception_state);

  // Callback
  void OnCreateFileSystem(
      ScriptPromiseResolver<MiniShell>* resolver,
      uint32_t id, mojo::PendingRemote<mojom::cfs::blink::CodegateDirectory> new_remote);
  void OnDeleteFileSystem(ScriptPromiseResolver<IDLBoolean>* resolver,
                          uint32_t id,
                          bool success);
  void Add(uint32_t id, MiniShell* minishell);
  void Sub(uint32_t id);
  void Trace(Visitor* visitor) const override;

 private:
  mojom::cfs::blink::CodegateFSManager* GetFSManagerService(
      ScriptState* script_state);

  HeapMojoRemote<mojom::cfs::blink::CodegateFSManager> fs_manager_remote_;
  HeapHashMap<uint32_t, Member<MiniShell>> active_shells_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_MINISHELL_MINISHELL_MANAGER_H_
