#ifndef BIR_H
#define BIR_H

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "BalAbstractInsn.h"
#include "BalBasicBlock.h"
#include "BalConstantLoad.h"
#include "BalEnums.h"
#include "BalFuncParam.h"
#include "BalFunction.h"
#include "BalFunctionCallInsn.h"
#include "BalInvokableType.h"
#include "BalMoveInsn.h"
#include "BalNonTerminatorInsn.h"
#include "BalOperand.h"
#include "BalPackage.h"
#include "BalParam.h"
#include "BalStructureInsn.h"
#include "BalTerminatorInsn.h"
#include "BalTypeDecl.h"
#include "BalVarDecl.h"
#include "Debuggable.h"
#include "PackageNode.h"
#include "Translatable.h"
#include "llvm-c/Core.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/MC/StringTableBuilder.h"
#include "llvm/Support/raw_ostream.h"

#ifndef unix
#define __attribute__(unused)
#endif

#define DEFAULT_VERSION 0
using namespace std;
using namespace llvm;
using namespace nballerina;

enum SymbolKind { LOCAL_SYMBOL_KIND, GLOBAL_SYMBOL_KIND };












#endif // BIR_H
