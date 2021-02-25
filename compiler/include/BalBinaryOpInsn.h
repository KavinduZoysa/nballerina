#ifndef __BALBINARYOPINSN__H__
#define __BALBINARYOPINSN__H__

#include "BalEnums.h"
#include "BalNonTerminatorInsn.h"

namespace nballerina {

// Forward Declaration
class Operand;
class BasicBlock;

class BinaryOpInsn : public NonTerminatorInsn {
private:
  Operand *rhsOp1;
  Operand *rhsOp2;
  InstructionKind kind;

public:
  BinaryOpInsn() = delete;
  BinaryOpInsn(Operand *lOp, BasicBlock *currentBB, Operand *rOp1,
               Operand *rOp2);
  ~BinaryOpInsn() = default;

  void setInstKind(InstructionKind _kind);
  void translate(LLVMModuleRef &modRef) final;
};

} // namespace nballerina

#endif //!__BALBINARYOPINSN__H__