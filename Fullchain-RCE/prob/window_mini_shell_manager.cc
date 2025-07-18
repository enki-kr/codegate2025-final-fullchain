// third_party/blink/renderer/modules/minishell/minishell_manager.cc

#include "third_party/blink/renderer/modules/minishell/window_mini_shell_manager.h"
#include "third_party/blink/renderer/modules/minishell/mini_shell_manager.h"

#include "base/functional/bind.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"


namespace blink {

const char WindowMiniShellManager::kSupplementName[] = "WindowMiniShellManager";

WindowMiniShellManager::WindowMiniShellManager(LocalDOMWindow& window)
    : Supplement<LocalDOMWindow>(window) {}

WindowMiniShellManager& WindowMiniShellManager::From(LocalDOMWindow& window) {
  WindowMiniShellManager* supplement =
      Supplement<LocalDOMWindow>::From<WindowMiniShellManager>(window);
  if (!supplement) {
    supplement = MakeGarbageCollected<WindowMiniShellManager>(window);
    ProvideTo(window, supplement);
  }
  return *supplement;
}

MiniShellManager* WindowMiniShellManager::miniShellManager(LocalDOMWindow& window) {
  return WindowMiniShellManager::From(window).miniShellManager(&window);
}

MiniShellManager* WindowMiniShellManager::miniShellManager(ExecutionContext* context) {
  if(!mini_shell_manager_) {
    mini_shell_manager_ = MakeGarbageCollected<MiniShellManager>(context);
  }
  return mini_shell_manager_.Get();
}

void WindowMiniShellManager::Trace(Visitor* visitor) const {
  visitor->Trace(mini_shell_manager_);
  Supplement<LocalDOMWindow>::Trace(visitor);

}
}  // namespace blink
