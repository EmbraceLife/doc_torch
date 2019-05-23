#pragma once

#include <torch/csrc/jit/script/tree.h>
#include <c10/util/Optional.h>

namespace torch {
namespace jit {
namespace script {

struct ErrorReport : public std::exception {
  ErrorReport(const ErrorReport& e)
      : ss(e.ss.str()), context(e.context), the_message(e.the_message) {}

  ErrorReport() : context(c10::nullopt) {}
  explicit ErrorReport(SourceRange r)
      : context(std::move(r)) {}
  explicit ErrorReport(const TreeRef& tree) : ErrorReport(tree->range()) {}
  explicit ErrorReport(const Token& tok) : ErrorReport(tok.range) {}
  const char* what() const noexcept override {
    std::stringstream msg;
    msg << "\n" << ss.str();
    if (context) {
      msg << ":\n";
      context->highlight(msg);
    } else {
      msg << ".\n";
    }
    the_message = msg.str();
    return the_message.c_str();
  }

 private:
  template <typename T>
  friend const ErrorReport& operator<<(const ErrorReport& e, const T& t);

  mutable std::stringstream ss;
  c10::optional<SourceRange> context;
  mutable std::string the_message;
};

template <typename T>
const ErrorReport& operator<<(const ErrorReport& e, const T& t) {
  e.ss << t;
  return e;
}

#define JIT_SCRIPT_ASSERT(ctx, cond)                                       \
  if (!(cond)) {                                                           \
    throw ::torch::jit::script::ErrorReport(ctx)                           \
        << __FILE__ << ":" << __LINE__ << ": assertion failed: " << #cond; \
  }
} // namespace script
} // namespace jit
} // namespace torch