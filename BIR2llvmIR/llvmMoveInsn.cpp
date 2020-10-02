#include "BIR.h"

MoveInsn::MoveInsn(){
}

MoveInsn::MoveInsn(Location *pos, InstructionKind kind,Operand *lOp, Operand *rOp):
	NonTerminatorInsn(pos, kind, lOp), rhsOp(rOp) {
}

MoveInsn::~MoveInsn(){
}

void MoveInsn::translate(LLVMModuleRef &modRef){
  LLVMBuilderRef builder = getFunction()->getLLVMBuilder();

  LLVMValueRef lhsRef;
  if (getLhsOperand() && getLhsOperand()->getVarDecl()) {
    if (getLhsOperand()->getVarDecl()->getVarKind() == GLOBAL_VAR_KIND) {
      lhsRef = getpkgAddress()->getGlobalVarRefUsingId(
                        getLhsOperand()->getVarDecl()->getVarName());
      LLVMValueRef rhsVarOpRef = getFunction()->getLocalToTempVar(rhsOp);
      LLVMBuildStore(builder, rhsVarOpRef, lhsRef);
    }

    else if (getLhsOperand()->getVarDecl()->getVarKind() == LOCAL_VAR_KIND ||
	     getLhsOperand()->getVarDecl()->getVarKind() == TEMP_VAR_KIND ) {
      lhsRef = getFunction()->getLocalVarRefUsingId(
			getLhsOperand()->getVarDecl()->getVarName());
      LLVMValueRef rhsVarOpRef = getFunction()->getLocalToTempVar(rhsOp);
      LLVMBuildStore(builder, rhsVarOpRef, lhsRef);       
    }
  }
}
