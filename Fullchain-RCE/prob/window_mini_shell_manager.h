#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_MINISHELL_WINDOW_MINISHELL_MANAGER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_MINISHELL_WINDOW_MINISHELL_MANAGER_H_

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

class MiniShellManager;
class WindowMiniShellManager final
    : public GarbageCollected<WindowMiniShellManager>,
      public Supplement<LocalDOMWindow> {
 public:
  static const char kSupplementName[];

  static WindowMiniShellManager& From(LocalDOMWindow& window);
  static MiniShellManager* miniShellManager(LocalDOMWindow&);

  explicit WindowMiniShellManager(LocalDOMWindow& window);

  MiniShellManager* miniShellManager(ExecutionContext* context);

  void Trace(Visitor* visitor) const override;

 private:
  Member<MiniShellManager> mini_shell_manager_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_MINISHELL_WINDOW_MINISHELL_MANAGER_H_
