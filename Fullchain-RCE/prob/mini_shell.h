#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_MINISHELL_MINISHELL_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_MINISHELL_MINISHELL_H_

// mojom
#include "third_party/blink/public/mojom/cfs/cfs.mojom-blink.h"

// blink dependency
#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_resolver.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/bindings/script_state.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/mojo/heap_mojo_remote.h"
#include "base/memory/scoped_refptr.h"

#define FILESIZE_MAX 1024

namespace blink {
class FileBuffer;

class MODULES_EXPORT MiniShell final : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  explicit MiniShell(
      ScriptState* script_state,
      mojo::PendingRemote<mojom::cfs::blink::CodegateDirectory> new_remote,
      uint32_t id);
  ~MiniShell() override;

  // IDL methods
  // [CallWith=ScriptState, RaisesException] uint32_t get_id();
  uint32_t get_id(ScriptState* script_state, ExceptionState& exception_state);

  // [CallWith=ScriptState, RaisesException] Promise<DOMString>
  // execute(DOMString command);
  ScriptPromise<IDLString> execute(ScriptState* script_state,
                                   const String& raw_input,
                                   ExceptionState& exception_state);

  void Trace(Visitor* visitor) const override;

  private:
  
  void FUNC_HELP(ScriptPromiseResolver<IDLString>* resolver,
                 const Vector<String>& cmd_input);
  void FUNC_PWD(ScriptPromiseResolver<IDLString>* resolver,
                const Vector<String>& cmd_input);
  void FUNC_MKDIR(ScriptPromiseResolver<IDLString>* resolver,
                  const Vector<String>& cmd_input);
  void FUNC_LS(ScriptPromiseResolver<IDLString>* resolver,
               const Vector<String>& cmd_input);
  void FUNC_CD(ScriptPromiseResolver<IDLString>* resolver,
               const Vector<String>& cmd_input);
  void FUNC_TOUCH(ScriptPromiseResolver<IDLString>* resolver,
                  const Vector<String>& cmd_input);
  void FUNC_DELETE(ScriptPromiseResolver<IDLString>* resolver,
                   const Vector<String>& cmd_input);
  void FUNC_RENAME(ScriptPromiseResolver<IDLString>* resolver,
                   const Vector<String>& cmd_input);
  void FUNC_EXEC(ScriptPromiseResolver<IDLString>* resolver,
                 const Vector<String>& cmd_input);
  void FUNC_MVDIR(ScriptPromiseResolver<IDLString>* resolver,
                  const Vector<String>& cmd_input);
  void FUNC_OPEN(ScriptPromiseResolver<IDLString>* resolver,
                 const Vector<String>& cmd_input);
  void FUNC_READ(ScriptPromiseResolver<IDLString>* resolver,
                 const Vector<String>& cmd_input);
  void FUNC_WRITE(ScriptPromiseResolver<IDLString>* resolver,
                  const Vector<String>& cmd_input);
  void FUNC_SEEK(ScriptPromiseResolver<IDLString>* resolver,
                 const Vector<String>& cmd_input);
  void FUNC_SAVE(ScriptPromiseResolver<IDLString>* resolver,
                 const Vector<String>& cmd_input);

  void SetDirectory(
      mojo::PendingRemote<mojom::cfs::blink::CodegateDirectory> new_dir_remote,
      ExecutionContext* execution_context);
  void SetBuffer(
      mojo::PendingRemote<mojom::cfs::blink::CodegateFile> new_file_remote,
      ScriptPromiseResolver<IDLString>* resolver);
  void ResetBuffer();

  mojom::cfs::blink::CodegateDirectory* GetDirectoryRemote();
  mojom::cfs::blink::CodegateFile* GetFileRemote();
  FileBuffer* GetBuffer();

  HeapMojoRemote<mojom::cfs::blink::CodegateDirectory> dir_remote_;
  Member<FileBuffer> file_descriptor_;
  uint32_t shell_id_;
};

class FileBuffer : public GarbageCollected<FileBuffer> {

 public:
  explicit FileBuffer(
      mojo::PendingRemote<mojom::cfs::blink::CodegateFile> new_file_remote,
      ScriptPromiseResolver<IDLString>* resolver);

  Vector<uint8_t> read(uint64_t count);
  void write(const Vector<uint8_t>& data);

  void SetIdx(uint64_t idx);

  mojom::cfs::blink::CodegateFile* GetRemote();

  void Trace(Visitor* visitor) const;

 private:
  HeapMojoRemote<mojom::cfs::blink::CodegateFile> remote_;
  uint64_t idx_;
  char buffer_[FILESIZE_MAX];

};
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_MINISHELL_MINISHELL_H_
