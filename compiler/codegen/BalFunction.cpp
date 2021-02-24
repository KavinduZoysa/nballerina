#include "BalFunction.h"
#include "BalBasicBlock.h"
#include "BalOperand.h"
#include "BalPackage.h"
#include "BalTypeDecl.h"
#include "BalVarDecl.h"
#include "llvm-c/Core.h"

namespace nballerina {

Function::Function(Location *pos, std::string namep, int flagsp,
                         InvokableType *typep, std::string workerNamep)
    : name(namep), flags(flagsp), type(typep), workerName(workerNamep) {
  setLocation(pos);
}

// Search basic block based on the basic block ID
BasicBlock *Function::searchBb(std::string name) {
  std::vector<BasicBlock *>::iterator itr;
  for (itr = basicBlocks.begin(); itr != basicBlocks.end(); itr++) {
    if ((*itr)->getId() == name) {
      return (*itr);
    }
  }
  return NULL;
}

std::string Function::getName() { return name; }
int Function::getFlags() { return flags; }
InvokableType *Function::getInvokableType() { return type; }
std::vector<Operand *> Function::getParams() { return requiredParams; }
Operand *Function::getParam(int i) { return requiredParams[i]; }
VarDecl *Function::getReceiver() { return receiver; }
Param *Function::getRestParam() { return restParam; }
VarDecl *Function::getReturnVar() { return returnVar; }
std::vector<BasicBlock *> Function::getBasicBlocks() { return basicBlocks; }
size_t Function::numBasicBlocks() { return basicBlocks.size(); }
BasicBlock *Function::getBasicBlock(int i) { return basicBlocks[i]; }
std::string Function::getWorkerName() { return workerName; }
LLVMBuilderRef Function::getLLVMBuilder() { return builder; }
int Function::getNumParams() { return paramCount; }
LLVMValueRef Function::getNewFunctionRef() { return newFunction; }
std::map<std::string, LLVMValueRef> Function::getLocalVarRefs() {
  return localVarRefs;
}
std::map<std::string, LLVMValueRef> Function::getBranchComparisonList() {
  return branchComparisonList;
}

LLVMValueRef Function::getValueRefBasedOnName(std::string lhsName) {
  std::map<std::string, LLVMValueRef>::iterator it;
  it = branchComparisonList.find(lhsName);

  if (it == branchComparisonList.end()) {
    return NULL;
  } else
    return it->second;
}

LLVMValueRef Function::getLocalVarRefUsingId(std::string locVar) {
  for (std::map<std::string, LLVMValueRef>::iterator iter =
           localVarRefs.begin();
       iter != localVarRefs.end(); iter++) {
    if (iter->first == locVar)
      return iter->second;
  }
  return NULL;
}

LLVMValueRef Function::getLocalToTempVar(Operand *operand) {
  std::string refOp = operand->getVarDecl()->getVarName();
  std::string tempName = refOp + "_temp";
  LLVMValueRef locVRef = getLocalVarRefUsingId(refOp);
  if (!locVRef)
    locVRef = getPkgAddress()->getGlobalVarRefUsingId(refOp);
  return LLVMBuildLoad(builder, locVRef, tempName.c_str());
}

static bool isParamter(VarDecl *locVar) {
  switch (locVar->getVarKind()) {
  case LOCAL_VAR_KIND:
  case TEMP_VAR_KIND:
  case RETURN_VAR_KIND:
  case GLOBAL_VAR_KIND:
  case SELF_VAR_KIND:
  case CONSTANT_VAR_KIND:
    return false;
  case ARG_VAR_KIND:
    return true;
  default:
    return false;
  }
}

LLVMTypeRef Function::getLLVMFuncRetTypeRefOfType(VarDecl *vDecl) {
  int typeTag = 0;
  if (vDecl->getTypeDecl())
    typeTag = vDecl->getTypeDecl()->getTypeTag();
  // if main function return type is void, but user wants to return some
  // value using _bal_result (global variable from BIR), change main function
  // return type from void to global variable (_bal_result) type.
  if (typeTag == TYPE_TAG_NIL || typeTag == TYPE_TAG_VOID) {
    std::vector<VarDecl *> globVars = getPkgAddress()->getGlobalVars();
    for (unsigned int i = 0; i < globVars.size(); i++) {
      VarDecl *globVar = globVars[i];
      if (globVar->getVarName() == "_bal_result") {
        typeTag = globVar->getTypeDecl()->getTypeTag();
        break;
      }
    }
  }

  switch (typeTag) {
  case TYPE_TAG_INT:
    return LLVMInt32Type();
  case TYPE_TAG_BYTE:
  case TYPE_TAG_FLOAT:
    return LLVMFloatType();
  case TYPE_TAG_BOOLEAN:
    return LLVMInt8Type();
  case TYPE_TAG_CHAR_STRING:
  case TYPE_TAG_STRING:
    return LLVMPointerType(LLVMInt8Type(), 0);
  default:
    return LLVMVoidType();
  }
}

void Function::translateFunctionBody(LLVMModuleRef &modRef) {
  LLVMBasicBlockRef BbRef;
  int paramIndex = 0;
  BbRef = LLVMAppendBasicBlock(newFunction, "entry");
  LLVMPositionBuilderAtEnd(builder, BbRef);

  // iterate through all local vars.
  for (auto const &it : localVars) {
    VarDecl *locVar = it.second;
    LLVMTypeRef varType = getLLVMTypeRefOfType(locVar->getTypeDecl());
    LLVMValueRef localVarRef;
    if (locVar->getTypeDecl()->getTypeTag() == TYPE_TAG_ANY) {
      varType = wrap(getPkgAddress()->getStructType());
    }
    localVarRef =
        LLVMBuildAlloca(builder, varType, (locVar->getVarName()).c_str());
    localVarRefs.insert({locVar->getVarName(), localVarRef});
    if (isParamter(locVar)) {
      LLVMValueRef parmRef = LLVMGetParam(newFunction, paramIndex);
      std::string paramName = getParam(paramIndex)->getVarDecl()->getVarName();
      LLVMSetValueName2(parmRef, paramName.c_str(), paramName.length());
      if (parmRef)
        LLVMBuildStore(builder, parmRef, localVarRef);
      paramIndex++;
    }
  }

  // iterate through with each basic block in the function and create them
  // first.
  for (unsigned int i = 0; i < basicBlocks.size(); i++) {
    BasicBlock *bb = basicBlocks[i];
    LLVMBasicBlockRef bbRef =
        LLVMAppendBasicBlock(this->getNewFunctionRef(), bb->getId().c_str());
    bb->setLLVMBBRef(bbRef);
    bb->setFunction(this);
    bb->setLLVMBuilderRef(builder);
    bb->setPkgAddress(getPkgAddress());
  }

  // creating branch to next basic block.
  if (basicBlocks.size() != 0 && basicBlocks[0] &&
      basicBlocks[0]->getLLVMBBRef())
    LLVMBuildBr(builder, basicBlocks[0]->getLLVMBBRef());

  // Now translate the basic blocks (essentially add the instructions in them)
  for (unsigned int i = 0; i < basicBlocks.size(); i++) {
    BasicBlock *bb = basicBlocks[i];
    LLVMPositionBuilderAtEnd(builder, bb->getLLVMBBRef());
    bb->translate(modRef);
  }
}

void Function::translate(LLVMModuleRef &modRef) {
  translateFunctionBody(modRef);
}

void Function::setName(std::string newName) { name = newName; }
void Function::setFlags(int newFlags) { flags = newFlags; }
void Function::setInvokableType(InvokableType *t) { type = t; }
void Function::setParams(std::vector<Operand *> p) { requiredParams = p; }
void Function::setParam(Operand *param) { requiredParams.push_back(param); }
void Function::setReceiver(VarDecl *var) { receiver = var; }
void Function::setRestParam(Param *param) { restParam = param; }
void Function::setNumParams(int paramcount) { paramCount = paramcount; }
void Function::setLocalVar(std::string name, VarDecl *var) {
  localVars.insert(std::pair<std::string, VarDecl *>(name, var));
}
void Function::setReturnVar(VarDecl *var) { returnVar = var; }
void Function::setBasicBlocks(std::vector<BasicBlock *> b) {
  basicBlocks = b;
}
void Function::addBasicBlock(BasicBlock *bb) { basicBlocks.push_back(bb); }
void Function::setWorkerName(std::string newName) { workerName = newName; }
void Function::setLLVMBuilder(LLVMBuilderRef b) { builder = b; }
void Function::setLocalVarRefs(
    std::map<std::string, LLVMValueRef> newLocalVarRefs) {
  localVarRefs = newLocalVarRefs;
}
void Function::setNewFunctionRef(LLVMValueRef newFuncRef) {
  newFunction = newFuncRef;
}

void Function::setBranchComparisonlist(
    std::map<std::string, LLVMValueRef> brCompl) {
  branchComparisonList = brCompl;
}
void Function::addNewbranchComparison(std::string name,
                                         LLVMValueRef compRef) {
  branchComparisonList.insert(
      std::pair<std::string, LLVMValueRef>(name, compRef));
}

LLVMTypeRef Function::getLLVMTypeRefOfType(TypeDecl *typeD) {
  int typeTag = typeD->getTypeTag();
  switch (typeTag) {
  case TYPE_TAG_INT:
    return LLVMInt32Type();
  case TYPE_TAG_BYTE:
  case TYPE_TAG_FLOAT:
    return LLVMFloatType();
  case TYPE_TAG_BOOLEAN:
    return LLVMInt8Type();
  case TYPE_TAG_CHAR_STRING:
  case TYPE_TAG_STRING:
    return LLVMPointerType(LLVMInt8Type(), 0);
  case TYPE_TAG_ANY:
  case TYPE_TAG_MAP:
    return LLVMPointerType(LLVMInt64Type(), 0);
  default:
    return LLVMInt32Type();
  }
}

VarDecl *Function::getNameVarDecl(std::string opName) {

  auto varIt = localVars.find(opName);
  if (varIt == localVars.end())
    return nullptr;

  return varIt->second;
}

const char *Function::getTypeNameOfTypeTag(TypeTagEnum typeTag) {
  switch (typeTag) {
  case TYPE_TAG_INT:
    return "int";
  case TYPE_TAG_FLOAT:
    return "float";
  case TYPE_TAG_CHAR_STRING:
  case TYPE_TAG_STRING:
    return "string";
  case TYPE_TAG_BOOLEAN:
    return "bool";
  case TYPE_TAG_ANY:
    return "any";
  default:
    return "";
  }
}

} // namespace nballerina