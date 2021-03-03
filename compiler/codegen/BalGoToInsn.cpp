#include "BalGoToInsn.h"
#include "BalBasicBlock.h"
#include "BalFunction.h"
#include "llvm-c/Core.h"

#ifndef unix
#define __attribute__(unused)
#endif

namespace nballerina {

GoToInsn::GoToInsn(BasicBlock *nextBB, BasicBlock *currentBB) : TerminatorInsn(nullptr, currentBB, nextBB, true) {
    kind = INSTRUCTION_KIND_GOTO;
}

void GoToInsn::translate(__attribute__((unused)) LLVMModuleRef &modRef) {
    LLVMBuilderRef builder = getFunction()->getLLVMBuilder();
    LLVMBuildBr(builder, getNextBB()->getLLVMBBRef());
}

} // namespace nballerina
