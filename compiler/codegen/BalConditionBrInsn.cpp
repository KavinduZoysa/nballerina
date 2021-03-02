#include "BalConditionBrInsn.h"
#include "BalBasicBlock.h"
#include "BalFunction.h"
#include "BalOperand.h"
#include "BalType.h"
#include "BalVarDecl.h"
#include "llvm-c/Core.h"

#ifndef unix
#define __attribute__(unused)
#endif

using namespace std;

namespace nballerina {

ConditionBrInsn::ConditionBrInsn(Operand *lOp, BasicBlock *currentBB,
                                 BasicBlock *_ifThenBB, BasicBlock *_elseBB)
    : TerminatorInsn(lOp, currentBB, nullptr, true), ifThenBB(_ifThenBB),
      elseBB(_elseBB) {
  kind = INSTRUCTION_KIND_CONDITIONAL_BRANCH;
}

BasicBlock *ConditionBrInsn::getIfThenBB() { return ifThenBB; }
BasicBlock *ConditionBrInsn::getElseBB() { return elseBB; }
void ConditionBrInsn::setIfThenBB(BasicBlock *bb) { ifThenBB = bb; }
void ConditionBrInsn::setElseBB(BasicBlock *bb) { elseBB = bb; }

void ConditionBrInsn::translate(__attribute__((unused)) LLVMModuleRef &modRef) {

  Operand *lhsOp = getLhsOperand();
  assert(lhsOp->getVarDecl());

  LLVMBuilderRef builder = getFunction()->getLLVMBuilder();
  string lhsName = lhsOp->getName();
  assert(lhsName != "");
  LLVMValueRef brCondition = getFunction()->getValueRefBasedOnName(lhsName);

  if (!brCondition) {
    VarDecl *lhsVarDecl = getFunction()->getLocalVarFromName(lhsName);
    if (lhsVarDecl->getTypeDecl()->getTypeTag() == TYPE_TAG_BOOLEAN) {
      brCondition = LLVMBuildIsNotNull(
          builder, getFunction()->getLocalToTempVar(lhsOp), lhsName.c_str());
    }
  }

  LLVMBasicBlockRef ifLLVMBB = ifThenBB->getLLVMBBRef();
  LLVMBasicBlockRef elseLLVMBB = elseBB->getLLVMBBRef();

  if (brCondition && ifLLVMBB && elseLLVMBB) {
    LLVMBuildCondBr(builder, brCondition, ifLLVMBB, elseLLVMBB);
  } else
    llvm_unreachable("Unknown State");
}

} // namespace nballerina