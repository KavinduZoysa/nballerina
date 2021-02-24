#include "BalUnaryOpInsn.h"
#include "BalFunction.h"
#include "BalOperand.h"
#include "BalPackage.h"
#include <llvm-c/Core.h>
#include <string>

#ifndef unix
#define __attribute__(unused)
#endif

using namespace std;

namespace nballerina {

UnaryOpInsn::UnaryOpInsn(Location *pos, InstructionKind kind, Operand *lOp,
                         Operand *rOp)
    : NonTerminatorInsn(pos, kind, lOp), rhsOp(rOp) {}

Operand *UnaryOpInsn::getRhsOp() { return rhsOp; }
void UnaryOpInsn::setRhsOp(Operand *op) { rhsOp = op; }

void UnaryOpInsn::translate(__attribute__((unused)) LLVMModuleRef &modRef) {

  Function *funcObj = getFunction();
  assert(funcObj);
  LLVMBuilderRef builder = funcObj->getLLVMBuilder();
  assert(builder);
  Operand *lhsOp = getLhsOperand();
  assert(lhsOp);
  string lhsName = lhsOp->getName();
  assert(lhsOp->getVarDecl());
  string lhstmpName = lhsName + "_temp";

  LLVMValueRef lhsRef = funcObj->getLocalVarRefUsingId(lhsName);
  if (!lhsRef)
    lhsRef = getPkgAddress()->getGlobalVarRefUsingId(lhsName);

  assert(rhsOp);
  LLVMValueRef rhsOpref = funcObj->getLocalToTempVar(rhsOp);
  if (!rhsOpref)
    return;

  switch (getInstKind()) {
  case INSTRUCTION_KIND_UNARY_NOT:
  case INSTRUCTION_KIND_UNARY_NEG: {
    LLVMValueRef ifReturn = LLVMBuildNot(builder, rhsOpref, lhstmpName.c_str());
    LLVMBuildStore(builder, ifReturn, lhsRef);
    break;
  }
  default:
    break;
  }
  return;
}

} // namespace nballerina