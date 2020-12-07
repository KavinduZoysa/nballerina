#include "BIRReader.h"
#include "BIR.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

// Read 1 byte from the stream
uint8_t BIRReader::readU1() {
  uint8_t value;
  is.read(reinterpret_cast<char *>(&value), sizeof(value));
  return value;
}

// Read 1 byte from the stream but do not move pointer
uint8_t BIRReader::peekReadU1() {
  uint8_t value;
  value = is.peek();
  return value;
}

// Read 2 bytes from the stream
uint16_t BIRReader::readS2be() {
  uint16_t value;
  uint16_t result;
  is.read(reinterpret_cast<char *>(&value), sizeof(value));
  char *p = (char *)&value;
  char tmp;
#if __BYTE_ORDER == __LITTLE_ENDIAN
  tmp = p[0];
  p[0] = p[1];
  p[1] = tmp;
#endif
  result = value;
  return result;
}

// Read 4 bytes from the stream
uint32_t BIRReader::readS4be() {
  uint32_t value;
  is.read(reinterpret_cast<char *>(&value), sizeof(value));
#if __BYTE_ORDER == __LITTLE_ENDIAN
  uint32_t result = 0;
  result |= (value & 0x000000FF) << 24;
  result |= (value & 0x0000FF00) << 8;
  result |= (value & 0x00FF0000) >> 8;
  result |= (value & 0xFF000000) >> 24;
  value = result;
#endif
  return value;
}

// Read 8 bytes from the stream
uint64_t BIRReader::readS8be() {
  uint64_t value;
  uint64_t result;
  is.read(reinterpret_cast<char *>(&value), sizeof(value));
#if __BYTE_ORDER == __LITTLE_ENDIAN
  char *p = (char *)&value;
  char tmp;

  tmp = p[0];
  p[0] = p[7];
  p[7] = tmp;

  tmp = p[1];
  p[1] = p[6];
  p[6] = tmp;

  tmp = p[2];
  p[2] = p[5];
  p[5] = tmp;

  tmp = p[3];
  p[3] = p[4];
  p[4] = tmp;
#endif
  result = value;
  return result;
}

// Search string from the constant pool based on index
std::string ConstantPoolSet::getStringCp(int32_t index,
                                         ConstantPoolSet *constantPool) {
  ConstantPoolEntry *poolEntry = constantPool->getEntry(index);
  StringCpInfo *stringCp = static_cast<StringCpInfo *>(poolEntry);
  return stringCp->getValue();
}

// Search string from the constant pool based on index
int32_t ConstantPoolSet::getIntCp(int32_t index,
                                  ConstantPoolSet *constantPool) {
  ConstantPoolEntry *poolEntry = constantPool->getEntry(index);
  IntCpInfo *intCp = static_cast<IntCpInfo *>(poolEntry);
  return intCp->getValue();
}

// Search type from the constant pool based on index
TypeDecl *ConstantPoolSet::getTypeCp(int32_t index,
                                     ConstantPoolSet *constantPool,
                                     bool voidToInt) {
  ConstantPoolEntry *poolEntry = constantPool->getEntry(index);
  ShapeCpInfo *shapeCp = static_cast<ShapeCpInfo *>(poolEntry);
  TypeDecl *typeDecl = new TypeDecl();
  typeDecl->setTypeDeclName(
      constantPool->getStringCp(shapeCp->getNameIndex(), constantPool));
  if (typeDecl->getTypeDeclName() == "") {
    char newName[20];
    char *p;
    p = stpcpy(newName, "anon-");
    sprintf(p, "%5ld", random() % 100000);
    typeDecl->setTypeDeclName(newName);
  }
  typeDecl->setTypeTag(shapeCp->getTypeTag());
  if (shapeCp->getTypeTag() == TYPE_TAG_NIL && voidToInt)
    typeDecl->setTypeTag(TYPE_TAG_INT);
  typeDecl->setFlags(shapeCp->getTypeFlag());
  return typeDecl;
}

// Get the Type tag from the constant pool based on the index passed
typeTagEnum ConstantPoolSet::getTypeTag(int32_t index,
                                        ConstantPoolSet *constantPool) {
  ConstantPoolEntry *poolEntry = constantPool->getEntry(index);
  ShapeCpInfo *shapeCp = static_cast<ShapeCpInfo *>(poolEntry);
  return shapeCp->getTypeTag();
}

// Search type from the constant pool based on index
InvokableType *
ConstantPoolSet::getInvokableType(int32_t index,
                                  ConstantPoolSet *constantPool) {
  InvokableType *invokableType = new InvokableType();
  ConstantPoolEntry *poolEntry = constantPool->getEntry(index);
  ShapeCpInfo *shapeCp = static_cast<ShapeCpInfo *>(poolEntry);
  for (int i = 0; i < shapeCp->getParamCount(); i++) {
    TypeDecl *typeDecl =
        constantPool->getTypeCp(shapeCp->getParam(i), constantPool, false);
    invokableType->addParamType(typeDecl);
  }
  if (shapeCp->getRestType()) {
    TypeDecl *typeDecl = constantPool->getTypeCp(shapeCp->getRestTypeIndex(),
                                                 constantPool, false);
    invokableType->setRestType(typeDecl);
  }
  TypeDecl *typeDecl = constantPool->getTypeCp(shapeCp->getReturnTypeIndex(),
                                               constantPool, false);
  invokableType->setReturnType(typeDecl);
  return invokableType;
}

// Read Global Variable and push it to BIRPackage
void BIRReader::readGlobalVar() {
  VarDecl *varDecl = new VarDecl();
  uint8_t kind = readU1();
  varDecl->setVarKind((VarKind)kind);

  uint32_t varDclNameCpIndex = readS4be();
  varDecl->setVarName(
      (constantPool->getStringCp(varDclNameCpIndex, constantPool)));

  uint32_t flags __attribute__((unused)) = readS4be();
  uint32_t length __attribute__((unused)) = readS4be();
  uint8_t hasDoc = readU1();
  if (hasDoc) {
    uint32_t descriptionCpIndex __attribute__((unused)) = readS4be();
    uint32_t returnValueDescriptionCpIndex __attribute__((unused)) = readS4be();
    uint32_t parameterCount __attribute__((unused)) = readS4be();
  }
  uint32_t typeCpIndex = readS4be();
  TypeDecl *typeDecl =
      constantPool->getTypeCp(typeCpIndex, constantPool, false);
  varDecl->setTypeDecl(typeDecl);
  birPackage->addGlobalVar(varDecl);
}

VarDecl *BIRReader::readLocalVar() {
  VarDecl *varDecl = new VarDecl();
  uint8_t kind = readU1();
  varDecl->setVarKind((VarKind)kind);

  uint32_t typeCpIndex = readS4be();
  TypeDecl *typeDecl =
      constantPool->getTypeCp(typeCpIndex, constantPool, false);
  varDecl->setTypeDecl(typeDecl);

  uint32_t nameCpIndex = readS4be();
  varDecl->setVarName(constantPool->getStringCp(nameCpIndex, constantPool));

  if (kind == ARG_VAR_KIND) {
    uint32_t metaVarNameCpIndex __attribute__((unused)) = readS4be();
  } else if (kind == LOCAL_VAR_KIND) {
    uint32_t metaVarNameCpIndex __attribute__((unused)) = readS4be();
    uint32_t endBbIdCpIndex __attribute__((unused)) = readS4be();
    uint32_t startBbIdCpIndex __attribute__((unused)) = readS4be();
    uint32_t instructionOffset __attribute__((unused)) = readS4be();
  }
  return varDecl;
}

// Read Local Variable and return VarDecl pointer
Operand *BIRReader::readOperand() {
  VarDecl *varDecl = new VarDecl();
  uint8_t ignoredVar = readU1();
  varDecl->setIgnore((bool)ignoredVar);

  uint8_t kind = readU1();
  varDecl->setVarKind((VarKind)kind);

  uint8_t scope = readU1();
  varDecl->setVarScope((VarScope)scope);

  uint32_t varDclNameCpIndex = readS4be();
  varDecl->setVarName(
      (constantPool->getStringCp(varDclNameCpIndex, constantPool)));

  if (varDecl->getVarKind() == GLOBAL_VAR_KIND) {
    uint32_t packageIndex __attribute__((unused)) = readS4be();
    uint32_t typeCpIndex = readS4be();
    varDecl->setTypeDecl(
        constantPool->getTypeCp(typeCpIndex, constantPool, false));
  }

  Operand *operand = new Operand(varDecl);
  return operand;
}

// Read TYPEDESC Insn
void BIRReader::readTypeDescInsn() {
  uint8_t ignoredVar __attribute__((unused)) = readU1();
  uint8_t kind __attribute__((unused)) = readU1();
  uint8_t scope __attribute__((unused)) = readU1();
  uint32_t varDclNameCpIndex __attribute__((unused)) = readS4be();
  uint32_t typeCpIndex __attribute__((unused)) = readS4be();
}

// Read STRUCTURE Insn
void BIRReader::readStructureInsn() {
  uint8_t ignoredVar __attribute__((unused)) = readU1();
  uint8_t kind __attribute__((unused)) = readU1();
  uint8_t scope __attribute__((unused)) = readU1();
  uint32_t varDclNameCpIndex __attribute__((unused)) = readS4be();
  uint8_t ignoredVar1 __attribute__((unused)) = readU1();
  uint8_t kind1 __attribute__((unused)) = readU1();
  uint8_t scope1 __attribute__((unused)) = readU1();
  uint32_t varDclNameCpIndex1 __attribute__((unused)) = readS4be();
  uint32_t packageIndex1 __attribute__((unused)) = readS4be();
  uint32_t typeCpIndex1 __attribute__((unused)) = readS4be();
}

// Read CONST_LOAD Insn
void BIRReader::readConstInsn(ConstantLoadInsn *constantloadInsn) {
  Operand *lhsOperand = new Operand();
  VarDecl *varDecl = new VarDecl();
  lhsOperand->setVarDecl(varDecl);

  uint32_t typeCpIndex1 = readS4be();
  TypeDecl *typeDecl =
      constantPool->getTypeCp(typeCpIndex1, constantPool, false);
  varDecl->setTypeDecl(typeDecl);

  uint8_t ignoredVar = readU1();
  lhsOperand->getVarDecl()->setIgnore((bool)ignoredVar);

  uint8_t kind = readU1();
  lhsOperand->getVarDecl()->setVarKind((VarKind)kind);

  uint8_t scope = readU1();
  lhsOperand->getVarDecl()->setVarScope((VarScope)scope);

  uint32_t varDclNameCpIndex = readS4be();
  lhsOperand->getVarDecl()->setVarName(
      (constantPool->getStringCp(varDclNameCpIndex, constantPool)));

  if (lhsOperand->getVarDecl()->getVarKind() == GLOBAL_VAR_KIND) {
    uint32_t packageIndex __attribute__((unused)) = readS4be();
    uint32_t typeCpIndex __attribute__((unused)) = readS4be();
  }

  typeTagEnum typeTag = constantPool->getTypeTag(typeCpIndex1, constantPool);
  if (typeTag == TYPE_TAG_INT) {
    uint32_t valueCpIndex = readS4be();
    constantloadInsn->setValue(
        constantPool->getIntCp(valueCpIndex, constantPool));
  }
  if (typeTag == TYPE_TAG_BOOLEAN) {
    uint8_t valueCpIndex = readU1();
    constantloadInsn->setValue(valueCpIndex);
  }

  constantloadInsn->setLhsOperand(lhsOperand);
}

// Read GOTO Insn
uint32_t BIRReader::readGotoInsn() {
  uint32_t targetBBNameCpIndex = readS4be();
  return targetBBNameCpIndex;
}

// Read Unary Operand
void BIRReader::readUnaryOpInsn(UnaryOpInsn *unaryOpInsn) {
  Operand *rhsOp = readOperand();
  Operand *lhsOp = readOperand();

  unaryOpInsn->setRhsOp(rhsOp);
  unaryOpInsn->setLhsOperand(lhsOp);
}

// Read Binary Operand
void BIRReader::readBinaryOpInsn(BinaryOpInsn *binaryOpInsn) {
  Operand *rhsOp1 = readOperand();
  Operand *rhsOp2 = readOperand();
  Operand *lhsOp = readOperand();

  binaryOpInsn->setRhsOp1(rhsOp1);
  binaryOpInsn->setRhsOp2(rhsOp2);
  binaryOpInsn->setLhsOperand(lhsOp);
}

// Read BRANCH Insn
void BIRReader::readConditionalBrInsn(ConditionBrInsn *conditionBrInsn) {
  Operand *lhsOp = readOperand();
  conditionBrInsn->setLhsOperand(lhsOp);
  uint32_t trueBbIdNameCpIndex = readS4be();
  uint32_t falseBbIdNameCpIndex = readS4be();

  BIRBasicBlock *trueDummybasicBlock = new BIRBasicBlock(
      constantPool->getStringCp(trueBbIdNameCpIndex, constantPool));
  conditionBrInsn->setIfThenBB(trueDummybasicBlock);

  BIRBasicBlock *falseDummybasicBlock = new BIRBasicBlock(
      constantPool->getStringCp(falseBbIdNameCpIndex, constantPool));
  conditionBrInsn->setElseBB(falseDummybasicBlock);

  conditionBrInsn->setPatchStatus(true);
  conditionBrInsn->setNextBB(NULL);
}

// Read MOV Insn
void BIRReader::readMoveInsn(MoveInsn *moveInsn) {
  Operand *rhsOp = readOperand();
  Operand *lhsOp = readOperand();

  moveInsn->setRhsOp(rhsOp);
  moveInsn->setLhsOperand(lhsOp);
}

// Read Function Call
void BIRReader::readFunctionCall(FunctionCallInsn *functionCallInsn) {
  uint8_t isVirtual = readU1();
  functionCallInsn->setIsVirtual((bool)isVirtual);

  uint32_t packageIndex __attribute__((unused)) = readS4be();
  uint32_t callNameCpIndex = readS4be();
  functionCallInsn->setFunctionName(
      constantPool->getStringCp(callNameCpIndex, constantPool));

  uint32_t argumentsCount = readS4be();
  functionCallInsn->setArgCount(argumentsCount);
  for (unsigned int i = 0; i < argumentsCount; i++) {
    Operand *param = readOperand();
    functionCallInsn->addArgumentToList(param);
  }
  uint8_t hasLhsOperand = readU1();
  if (hasLhsOperand) {
    Operand *lhsOp = readOperand();
    functionCallInsn->setLhsOperand(lhsOp);
  }
  uint32_t thenBbIdNameCpIndex = readS4be();
  BIRBasicBlock *dummybasicBlock = new BIRBasicBlock(
      constantPool->getStringCp(thenBbIdNameCpIndex, constantPool));
  functionCallInsn->setNextBB(dummybasicBlock);
  functionCallInsn->setPatchStatus(true);
}

// Search basic block based on the basic block ID
BIRBasicBlock *BIRReader::searchBb(vector<BIRBasicBlock *> basicBlocks,
                                   std::string name) {
  std::vector<BIRBasicBlock *>::iterator itr;
  for (itr = basicBlocks.begin(); itr != basicBlocks.end(); itr++) {
    if ((*itr)->getId() == name) {
      return (*itr);
    }
  }
  return NULL;
}

// Read NonTerminatorInsn from the BIR
NonTerminatorInsn *BIRReader::readInsn(BIRFunction *birFunction,
                                       BIRBasicBlock *basicBlock) {
  NonTerminatorInsn *nonTerminatorInsn = new NonTerminatorInsn();
  nonTerminatorInsn->setFunction(birFunction);
  uint32_t sLine = readS4be();
  uint32_t eLine = readS4be();
  uint32_t sCol = readS4be();
  uint32_t eCol = readS4be();
  uint32_t sourceFileCpIndex = readS4be();

  Location *location =
      new Location(constantPool->getStringCp(sourceFileCpIndex, constantPool),
                   (int)sLine, (int)eLine, (int)sCol, (int)eCol);
  nonTerminatorInsn->setLocation(location);

  InstructionKind insnKind = (InstructionKind)readU1();
  switch (insnKind) {
  case INSTRUCTION_KIND_NEW_TYPEDESC: {
    readTypeDescInsn();
    break;
  }
  case INSTRUCTION_KIND_NEW_STRUCTURE: {
    readStructureInsn();
    break;
  }
  case INSTRUCTION_KIND_CONST_LOAD: {
    ConstantLoadInsn *constantloadInsn = new ConstantLoadInsn();
    constantloadInsn->setFunction(birFunction);
    constantloadInsn->setInstKind(insnKind);

    readConstInsn(constantloadInsn);
    nonTerminatorInsn = constantloadInsn;
    break;
  }
  case INSTRUCTION_KIND_GOTO: {
    GoToInsn *gotoInsn = new GoToInsn();
    uint32_t nameId = readGotoInsn();
    BIRBasicBlock *dummybasicBlock =
        new BIRBasicBlock(constantPool->getStringCp(nameId, constantPool));
    gotoInsn->setNextBB(dummybasicBlock);
    gotoInsn->setPatchStatus(true);
    gotoInsn->setInstKind(insnKind);
    basicBlock->setTerminatorInsn(gotoInsn);
    nonTerminatorInsn = NULL;
    break;
  }
  case INSTRUCTION_KIND_RETURN: {
    ReturnInsn *returnInsn = new ReturnInsn();
    returnInsn->setInstKind(insnKind);
    basicBlock->setTerminatorInsn(returnInsn);
    nonTerminatorInsn = NULL;
    break;
  }
  case INSTRUCTION_KIND_BINARY_ADD:
  case INSTRUCTION_KIND_BINARY_SUB:
  case INSTRUCTION_KIND_BINARY_MUL:
  case INSTRUCTION_KIND_BINARY_DIV:
  case INSTRUCTION_KIND_BINARY_EQUAL:
  case INSTRUCTION_KIND_BINARY_NOT_EQUAL:
  case INSTRUCTION_KIND_BINARY_GREATER_THAN:
  case INSTRUCTION_KIND_BINARY_GREATER_EQUAL:
  case INSTRUCTION_KIND_BINARY_LESS_THAN:
  case INSTRUCTION_KIND_BINARY_LESS_EQUAL:
  case INSTRUCTION_KIND_BINARY_BITWISE_XOR:
  case INSTRUCTION_KIND_BINARY_MOD: {
    BinaryOpInsn *binaryOpInsn = new BinaryOpInsn();
    binaryOpInsn->setInstKind(insnKind);
    readBinaryOpInsn(binaryOpInsn);
    nonTerminatorInsn = binaryOpInsn;
    break;
  }
  case INSTRUCTION_KIND_UNARY_NEG:
  case INSTRUCTION_KIND_UNARY_NOT: {
    UnaryOpInsn *unaryOpInsn = new UnaryOpInsn();
    unaryOpInsn->setInstKind(insnKind);
    readUnaryOpInsn(unaryOpInsn);
    nonTerminatorInsn = unaryOpInsn;
    break;
  }
  case INSTRUCTION_KIND_CONDITIONAL_BRANCH: {
    ConditionBrInsn *conditionBrInsn = new ConditionBrInsn();
    conditionBrInsn->setInstKind(insnKind);
    readConditionalBrInsn(conditionBrInsn);
    basicBlock->setTerminatorInsn(conditionBrInsn);
    nonTerminatorInsn = NULL;
    break;
  }
  case INSTRUCTION_KIND_MOVE: {
    MoveInsn *moveInsn = new MoveInsn();
    moveInsn->setInstKind(insnKind);
    readMoveInsn(moveInsn);
    nonTerminatorInsn = (moveInsn);
    break;
  }
  case INSTRUCTION_KIND_CALL: {
    FunctionCallInsn *functionCallInsn = new FunctionCallInsn();
    functionCallInsn->setInstKind(insnKind);
    readFunctionCall(functionCallInsn);
    basicBlock->setTerminatorInsn(functionCallInsn);
    nonTerminatorInsn = NULL;
    break;
  }
  default:
    break;
  }
  return nonTerminatorInsn;
}

// Read Basic Block from the BIR
BIRBasicBlock *BIRReader::readBasicBlock(BIRFunction *birFunction) {
  BIRBasicBlock *basicBlock = new BIRBasicBlock();
  uint32_t nameCpIndex = readS4be();
  basicBlock->setId(constantPool->getStringCp(nameCpIndex, constantPool));
  basicBlock->setBIRFunction(birFunction);

  uint32_t insnCount = readS4be();
  for (unsigned int i = 0; i < insnCount; i++) {
    NonTerminatorInsn *nonTerminatorInsn = readInsn(birFunction, basicBlock);
    if (nonTerminatorInsn)
      basicBlock->addNonTermInsn(nonTerminatorInsn);
  }
  return basicBlock;
}

// Patches the Terminator Insn with destination Basic Block
void BIRReader::patchInsn(vector<BIRBasicBlock *> basicBlocks) {
  for (unsigned int i = 0; i < basicBlocks.size(); i++) {
    BIRBasicBlock *basicBlock = basicBlocks[i];
    TerminatorInsn *terminator = basicBlock->getTerminatorInsn();
    if (terminator && terminator->getPatchStatus()) {
      switch (terminator->getInstKind()) {
      case INSTRUCTION_KIND_CONDITIONAL_BRANCH: {
        ConditionBrInsn *Terminator =
            (static_cast<ConditionBrInsn *>(terminator));
        BIRBasicBlock *trueBB =
            searchBb(basicBlocks, Terminator->getIfThenBB()->getId());
        BIRBasicBlock *falseBB =
            searchBb(basicBlocks, Terminator->getElseBB()->getId());
        BIRBasicBlock *danglingTrueBB = Terminator->getIfThenBB();
        BIRBasicBlock *danglingFalseBB = Terminator->getElseBB();
        delete danglingTrueBB;
        delete danglingFalseBB;
        Terminator->setIfThenBB(trueBB);
        Terminator->setElseBB(falseBB);
        Terminator->setPatchStatus(false);
        break;
      }
      case INSTRUCTION_KIND_GOTO: {
        BIRBasicBlock *destBB =
            searchBb(basicBlocks, terminator->getNextBB()->getId());
        BIRBasicBlock *danglingBB = terminator->getNextBB();
        delete danglingBB;
        terminator->setNextBB(destBB);
        terminator->setPatchStatus(false);
        break;
      }
      case INSTRUCTION_KIND_CALL: {
        BIRBasicBlock *destBB =
            searchBb(basicBlocks, terminator->getNextBB()->getId());
        BIRBasicBlock *danglingBB = terminator->getNextBB();
        delete danglingBB;
        terminator->setNextBB(destBB);
        break;
      }
      default:
        fprintf(stderr, "%s:%d Invalid Insn Kind for Instruction Patching.\n",
                __FILE__, __LINE__);
        break;
      }
    }
  }
}

// Reads BIR function
BIRFunction *BIRReader::readFunction() {
  BIRFunction *birFunction = new BIRFunction();
  uint32_t sLine = readS4be();
  uint32_t eLine = readS4be();
  uint32_t sCol = readS4be();
  uint32_t eCol = readS4be();

  uint32_t sourceFileCpIndex = readS4be();
  birPackage->setSrcFileName(
      constantPool->getStringCp(sourceFileCpIndex, constantPool));

  Location *location =
      new Location(constantPool->getStringCp(sourceFileCpIndex, constantPool),
                   (int)sLine, (int)eLine, (int)sCol, (int)eCol);
  birFunction->setLocation(location);

  uint32_t nameCpIndex = readS4be();
  birFunction->setName(constantPool->getStringCp(nameCpIndex, constantPool));
  std::string initFuncName = "..<init>";
  std::string startFuncName = "..<start>";
  std::string stopFuncName = "..<stop>";
  if (!(initFuncName.compare(birFunction->getName()) == 0 ||
        startFuncName.compare(birFunction->getName()) == 0 ||
        stopFuncName.compare(birFunction->getName()) == 0))
    birPackage->addFunctionLookUpEntry(birFunction->getName(), birFunction);

  uint32_t workdernameCpIndex = readS4be();
  birFunction->setWorkerName(
      constantPool->getStringCp(workdernameCpIndex, constantPool));

  uint32_t flags = readS4be();
  birFunction->setFlags(flags);

  uint32_t typeCpIndex = readS4be();
  birFunction->setInvokableType(
      constantPool->getInvokableType(typeCpIndex, constantPool));

  uint64_t annotationLength __attribute__((unused));
  annotationLength = readS8be();

  uint32_t annotationAttachments __attribute__((unused));
  annotationAttachments = readS4be();

  uint32_t requiredParamCount = readS4be();
  birFunction->setNumParams(requiredParamCount);
  // Set function param here and then fill remaining values from the default
  // Params
  for (unsigned int i = 0; i < requiredParamCount; i++) {
    uint32_t paramNameCpIndex = readS4be();
    VarDecl *varDecl = new VarDecl();
    varDecl->setVarName(
        constantPool->getStringCp(paramNameCpIndex, constantPool));
    Operand *param = new Operand(varDecl);
    uint32_t paramFlags __attribute__((unused)) = readS4be();
    birFunction->setParam(param);
  }

  uint8_t hasRestParam = readU1();
  if (!((bool)hasRestParam))
    birFunction->setRestParam(NULL);

  uint8_t hasReceiver = readU1();
  if (!((bool)hasReceiver))
    birFunction->setReceiver(NULL);

  uint64_t taintTableLength = readS8be();

  std::vector<char> result(taintTableLength);
  is.read(&result[0], taintTableLength);

  uint32_t docLength __attribute__((unused));
  docLength = readS4be();

  std::vector<char> result1(docLength);
  is.read(&result1[0], docLength);

  uint64_t functionBodyLength __attribute__((unused));
  functionBodyLength = readS8be();

  uint32_t argsCount __attribute__((unused));
  argsCount = readS4be();

  uint8_t hasReturnVar = readU1();

  if (hasReturnVar) {
    VarDecl *varDecl = new VarDecl();

    uint8_t kind = readU1();
    varDecl->setVarKind((VarKind)kind);

    uint32_t typeCpIndex = readS4be();
    TypeDecl *typeDecl =
        constantPool->getTypeCp(typeCpIndex, constantPool, false);
    varDecl->setTypeDecl(typeDecl);

    uint32_t nameCpIndex = readS4be();
    varDecl->setVarName(constantPool->getStringCp(nameCpIndex, constantPool));

    birFunction->setReturnVar(varDecl);
  }

  uint32_t defaultParamValue = readS4be();
  for (unsigned int i = 0; i < defaultParamValue; i++) {
    uint8_t kind = readU1();
    uint32_t typeCpIndex = readS4be();
    Operand *param = birFunction->getParam(i);
    param->getVarDecl()->setTypeDecl(
        constantPool->getTypeCp(typeCpIndex, constantPool, false));
    param->getVarDecl()->setVarKind((VarKind)kind);
    uint32_t nameCpIndex __attribute__((unused)) = readS4be();
    if (kind == ARG_VAR_KIND) {
      uint32_t metaVarNameCpIndex __attribute__((unused)) = readS4be();
    }
    uint8_t hasDefaultExpr __attribute__((unused)) = readU1();
  }

  uint32_t localVarCount = readS4be();

  std::vector<VarDecl *> localvars;
  for (unsigned int i = 0; i < localVarCount; i++) {
    VarDecl *varDecl = readLocalVar();
    localvars.push_back(varDecl);
  }
  birFunction->setLocalVars(localvars);
  for (unsigned int i = 0; i < defaultParamValue; i++) {
    uint32_t basicBlocksCount __attribute__((unused)) = readS4be();
  }

  uint32_t BBCount = readS4be();
  for (unsigned int i = 0; i < BBCount; i++) {
    BIRBasicBlock *basicBlock = readBasicBlock(birFunction);
    birFunction->addBIRBasicBlock(basicBlock);
  }

  // Patching the insn
  vector<BIRBasicBlock *> basicBlocks = birFunction->getBasicBlocks();
  patchInsn(basicBlocks);

  // Create links between Basic Block
  for (unsigned int i = 0; i < BBCount - 1; i++) {
    BIRBasicBlock *basicBlock = birFunction->getBasicBlock(i);
    basicBlock->setNextBB(birFunction->getBasicBlock(i + 1));
  }

  uint32_t errorEntriesCount __attribute__((unused));
  errorEntriesCount = readS4be();

  uint32_t channelsLength __attribute__((unused));
  channelsLength = readS4be();

  return birFunction;
}

void StringCpInfo::read(BIRReader *reader) {
  stringLength = reader->readS4be();
  std::vector<char> result(stringLength);
  reader->is.read(&result[0], stringLength);
  value = std::string(result.begin(), result.end());
}

void ShapeCpInfo::read(BIRReader *reader) {
  shapeLength = reader->readS4be();
  typeTag = static_cast<typeTagEnum>(reader->readU1());
  nameIndex = reader->readS4be();
  typeFlag = reader->readS4be();
  typeSpecialFlag = reader->readS4be();

  uint32_t shapeLengthTypeInfo = shapeLength - 13;

  switch (typeTag) {
  case TYPE_TAG_INVOKABLE: {
    paramCount = reader->readS4be();
    for (int i = 0; i < paramCount; i++) {
      uint32_t paramTypeCpIndex = reader->readS4be();
      addParam(paramTypeCpIndex);
    }
    hasRestType = reader->readU1();
    if (hasRestType) {
      restTypeIndex = reader->readS4be();
    }
    returnTypeIndex = reader->readS4be();
    break;
  }
  case TYPE_TAG_INT:
  case TYPE_TAG_BYTE:
  case TYPE_TAG_FLOAT:
  case TYPE_TAG_DECIMAL:
  case TYPE_TAG_STRING:
  case TYPE_TAG_BOOLEAN:
  case TYPE_TAG_JSON:
  case TYPE_TAG_XML:
  case TYPE_TAG_TABLE:
  case TYPE_TAG_NIL:
  case TYPE_TAG_ANYDATA:
  case TYPE_TAG_RECORD:
  case TYPE_TAG_TYPEDESC:
  case TYPE_TAG_STREAM:
  case TYPE_TAG_MAP:
  case TYPE_TAG_ANY:
  case TYPE_TAG_ENDPOINT:
  case TYPE_TAG_ARRAY:
  case TYPE_TAG_UNION:
  case TYPE_TAG_INTERSECTION:
  case TYPE_TAG_PACKAGE:
  case TYPE_TAG_NONE:
  case TYPE_TAG_VOID:
  case TYPE_TAG_XMLNS:
  case TYPE_TAG_ANNOTATION:
  case TYPE_TAG_SEMANTIC_ERROR:
  case TYPE_TAG_ERROR:
  case TYPE_TAG_ITERATOR:
  case TYPE_TAG_TUPLE:
  case TYPE_TAG_FUTURE:
  case TYPE_TAG_FINITE:
  case TYPE_TAG_OBJECT:
  case TYPE_TAG_SERVICE:
  case TYPE_TAG_BYTE_ARRAY:
  case TYPE_TAG_FUNCTION_POINTER:
  case TYPE_TAG_HANDLE:
  case TYPE_TAG_READONLY:
  case TYPE_TAG_SIGNED32_INT:
  case TYPE_TAG_SIGNED16_INT:
  case TYPE_TAG_SIGNED8_INT:
  case TYPE_TAG_UNSIGNED32_INT:
  case TYPE_TAG_UNSIGNED16_INT:
  case TYPE_TAG_UNSIGNED8_INT:
  case TYPE_TAG_CHAR_STRING:
  case TYPE_TAG_XML_ELEMENT:
  case TYPE_TAG_XML_PI:
  case TYPE_TAG_XML_COMMENT:
  case TYPE_TAG_XML_TEXT:
  case TYPE_TAG_NEVER:
  case TYPE_TAG_NULL_SET:
  case TYPE_TAG_PARAMETERIZED_TYPE: {
    std::vector<char> result(shapeLengthTypeInfo);
    reader->is.read(&result[0], shapeLengthTypeInfo);
    break;
  }
  default:
    fprintf(stderr, "%s:%d Invalid Type Tag in shape.\n", __FILE__, __LINE__);
    fprintf(stderr, "%d is the Type Tag.\n", (typeTagEnum)typeTag);
    break;
  }
}

void PackageCpInfo::read(BIRReader *reader) {
  orgIndex = reader->readS4be();
  nameIndex = reader->readS4be();
  versionIndex = reader->readS4be();
}

void IntCpInfo::read(BIRReader *reader) { value = reader->readS8be(); }

void BooleanCpInfo::read(BIRReader *reader) { value = reader->readU1(); }

void FloatCpInfo::read(BIRReader *reader) { value = reader->readS8be(); }

void ByteCpInfo::read(BIRReader *reader) { value = reader->readU1(); }

void ConstantPoolSet::read(BIRReader *reader) {
  poolCount = reader->readS4be();
  int constantPoolEntries = getConstantPoolCount();
  poolEntries = new std::vector<ConstantPoolEntry *>();
  poolEntries->reserve(constantPoolEntries);
  for (int i = 0; i < constantPoolEntries; i++) {
    ConstantPoolEntry::tagEnum tag =
        static_cast<ConstantPoolEntry::tagEnum>(reader->readU1());
    switch (tag) {
    case ConstantPoolEntry::tagEnum::TAG_ENUM_CP_ENTRY_PACKAGE: {
      PackageCpInfo *poolEntry = new PackageCpInfo();
      poolEntry->read(reader);
      poolEntries->push_back(poolEntry);
      break;
    }
    case ConstantPoolEntry::tagEnum::TAG_ENUM_CP_ENTRY_SHAPE: {
      ShapeCpInfo *poolEntry = new ShapeCpInfo();
      poolEntry->read(reader);
      poolEntries->push_back(poolEntry);
      break;
    }
    case ConstantPoolEntry::tagEnum::TAG_ENUM_CP_ENTRY_STRING: {
      StringCpInfo *poolEntry = new StringCpInfo();
      poolEntry->read(reader);
      poolEntries->push_back(poolEntry);
      break;
    }
    case ConstantPoolEntry::tagEnum::TAG_ENUM_CP_ENTRY_INTEGER: {
      IntCpInfo *poolEntry = new IntCpInfo();
      poolEntry->read(reader);
      poolEntries->push_back(poolEntry);
      break;
    }
    case ConstantPoolEntry::tagEnum::TAG_ENUM_CP_ENTRY_FLOAT: {
      FloatCpInfo *poolEntry = new FloatCpInfo();
      poolEntry->read(reader);
      poolEntries->push_back(poolEntry);
      break;
    }
    case ConstantPoolEntry::tagEnum::TAG_ENUM_CP_ENTRY_BOOLEAN: {
      BooleanCpInfo *poolEntry = new BooleanCpInfo();
      poolEntry->read(reader);
      poolEntries->push_back(poolEntry);
      break;
    }
    case ConstantPoolEntry::tagEnum::TAG_ENUM_CP_ENTRY_BYTE: {
      ByteCpInfo *poolEntry = new ByteCpInfo();
      poolEntry->read(reader);
      poolEntries->push_back(poolEntry);
      break;
    }
    default:
      break;
    }
  }
}

void BIRReader::readModule() {
  int32_t idCpIndex = readS4be();
  ConstantPoolEntry *poolEntry = constantPool->getEntry(idCpIndex);
  switch (poolEntry->getTag()) {
  case ConstantPoolEntry::tagEnum::TAG_ENUM_CP_ENTRY_PACKAGE: {
    PackageCpInfo *packageEntry = static_cast<PackageCpInfo *>(poolEntry);
    poolEntry = constantPool->getEntry(packageEntry->getOrgIndex());
    StringCpInfo *stringCp = static_cast<StringCpInfo *>(poolEntry);
    birPackage->setOrgName(stringCp->getValue());

    poolEntry = constantPool->getEntry(packageEntry->getNameIndex());
    stringCp = static_cast<StringCpInfo *>(poolEntry);
    birPackage->setPackageName(stringCp->getValue());

    poolEntry = constantPool->getEntry(packageEntry->getVersionIndex());
    stringCp = static_cast<StringCpInfo *>(poolEntry);
    birPackage->setVersion(stringCp->getValue());
    break;
  }
  default:
    break;
  }

  // The following three are read into unused variables so that the file
  // pointer advances to the data that we need next.
  uint32_t importCount __attribute__((unused));
  uint32_t constCount __attribute__((unused));
  uint32_t typeDefinitionCount __attribute__((unused));

  importCount = readS4be();
  constCount = readS4be();
  typeDefinitionCount = readS4be();

  uint32_t globalVarCount = readS4be();
  if (globalVarCount > 0) {
    for (unsigned int i = 0; i < globalVarCount; i++) {
      readGlobalVar();
    }
  }

  uint32_t typeDefinitionBodiesCount __attribute__((unused)) = readS4be();
  uint32_t functionCount = readS4be();

  // Push all the functions in BIRpackage except __init, __start & __stop
  std::string initFuncName = "..<init>";
  std::string startFuncName = "..<start>";
  std::string stopFuncName = "..<stop>";
  for (unsigned int i = 0; i < functionCount; i++) {
    BIRFunction *curFunc = readFunction();
    if (!(initFuncName.compare(curFunc->getName()) == 0 ||
          startFuncName.compare(curFunc->getName()) == 0 ||
          stopFuncName.compare(curFunc->getName()) == 0))
      birPackage->addFunction(curFunc);
    else
      delete curFunc;
  }

  uint32_t annotationsSize __attribute__((unused));
  annotationsSize = readS4be();

  // Assign typedecl to function param of call Insn
  for (size_t i = 0; i < birPackage->numFunctions(); i++) {
    BIRFunction *curFunc = birPackage->getFunction(i);
    for (size_t i = 0; i < curFunc->numBasicBlocks(); i++) {
      BIRBasicBlock *birBasicBlock = curFunc->getBasicBlock(i);
      for (size_t i = 0; i < birBasicBlock->numInsns(); i++) {
        TerminatorInsn *terminator = birBasicBlock->getTerminatorInsn();
        if (terminator && terminator->getPatchStatus()) {
          switch (terminator->getInstKind()) {
          case INSTRUCTION_KIND_CALL: {
            FunctionCallInsn *Terminator =
                (static_cast<FunctionCallInsn *>(terminator));
            for (int i = 0; i < Terminator->getArgCount(); i++) {
              BIRFunction *patchCallFunction =
                  birPackage->getFunctionLookUp(Terminator->getFunctionName());
              InvokableType *invokableType =
                  patchCallFunction->getInvokableType();
              for (size_t i = 0; i < invokableType->getParamTypeCount(); i++) {
                TypeDecl *typeDecl = invokableType->getParamType(i);
                Operand *param = Terminator->getArgumentFromList(i);
                param->getVarDecl()->setTypeDecl(typeDecl);
              }
            }
            break;
          }
          default:
            fprintf(stderr,
                    "%s:%d Invalid Insn Kind for assigning TypeDecl to "
                    "FuncCall Insn.\n",
                    __FILE__, __LINE__);
            break;
          }
        }
      }
    }
  }
}

void BIRReader::deserialize(BIRPackage *birPackageReader) {
  // Read Constant Pool
  ConstantPoolSet *constantPoolSet = new ConstantPoolSet();
  constantPoolSet->read(this);

  setConstantPool(constantPoolSet);
  setBIRPackage(birPackageReader);

  // Read Module
  readModule();
}
