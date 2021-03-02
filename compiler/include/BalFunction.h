#ifndef __BALFUNCTION__H__
#define __BALFUNCTION__H__

#include "interfaces/Debuggable.h"
#include "interfaces/PackageNode.h"
#include "interfaces/Translatable.h"
#include <map>
#include <string>
#include <vector>

namespace nballerina {

// Forward Declaration
class BasicBlock;
class Operand;
class VarDecl;
class InvokableType;
class Param;
class Type;

class Function : public PackageNode, public Debuggable, public Translatable {
private:
  std::string name;
  int flags;
  InvokableType *type;
  std::vector<Operand *> requiredParams;
  VarDecl *receiver;
  Param *restParam;
  int paramCount;
  std::map<std::string, VarDecl *> localVars;
  VarDecl *returnVar;
  std::vector<BasicBlock *> basicBlocks;
  std::string workerName;
  LLVMBuilderRef builder;
  LLVMValueRef newFunction;
  std::map<std::string, LLVMValueRef> localVarRefs;
  std::map<std::string, LLVMValueRef> branchComparisonList;

public:
  Function() = default;
  Function(Location *pos, std::string pname, int pflags, InvokableType *ptype,
           std::string pworkerName);
  Function(const Function &) = delete;
  ~Function() = default;

  std::string getName();
  int getFlags();
  InvokableType *getInvokableType();
  std::vector<Operand *> getParams();
  Operand *getParam(int i);
  VarDecl *getReceiver();
  Param *getRestParam();
  VarDecl *getReturnVar();
  std::vector<BasicBlock *> getBasicBlocks();
  size_t numBasicBlocks();
  BasicBlock *getBasicBlock(int i);
  std::string getWorkerName();
  LLVMBuilderRef getLLVMBuilder();
  int getNumParams();
  LLVMValueRef getNewFunctionRef();
  std::map<std::string, LLVMValueRef> getLocalVarRefs();
  std::map<std::string, LLVMValueRef> getBranchComparisonList();
  LLVMValueRef getValueRefBasedOnName(std::string lhsName);

  void setName(std::string newName);
  void setFlags(int newFlags);
  void setInvokableType(InvokableType *t);
  void setParams(std::vector<Operand *> p);
  void setParam(Operand *param);
  void setReceiver(VarDecl *var);
  void setRestParam(Param *param);
  void setNumParams(int paramcount);
  void setLocalVar(std::string name, VarDecl *var);
  void setReturnVar(VarDecl *var);
  void setBasicBlocks(std::vector<BasicBlock *> b);
  void addBasicBlock(BasicBlock *bb);
  void setWorkerName(std::string newName);
  void setLLVMBuilder(LLVMBuilderRef b);
  void setLocalVarRefs(std::map<std::string, LLVMValueRef> newLocalVarRefs);
  void setNewFunctionRef(LLVMValueRef newFuncRef);
  void setBranchComparisonlist(std::map<std::string, LLVMValueRef> brCompl);
  void addNewbranchComparison(std::string name, LLVMValueRef compRef);

  BasicBlock *searchBb(std::string name);

  LLVMTypeRef getLLVMTypeRefOfType(Type *typeD);
  LLVMValueRef getLocalVarRefUsingId(std::string locVar);
  LLVMValueRef getLocalToTempVar(Operand *op);
  void translateFunctionBody(LLVMModuleRef &modRef);
  // void patchInsn(llvm::Function *llvnFun);
  LLVMTypeRef getLLVMFuncRetTypeRefOfType(VarDecl *vDecl);
  VarDecl *getNameVarDecl(std::string opName);
  void translate(LLVMModuleRef &modRef) final;
};
} // namespace nballerina

#endif //!__BALFUNCTION__H__