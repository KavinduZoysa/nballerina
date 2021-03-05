/*
 * Copyright (c) 2021, WSO2 Inc. (http://www.wso2.org) All Rights Reserved.
 *
 * WSO2 Inc. licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef __BALOPERAND__H__
#define __BALOPERAND__H__

#include "interfaces/VariableKind.h"
#include <string>

namespace nballerina {

// Forward Declaration
class Type;

class Operand : public VariableKind {
  public:
    Operand() = delete;
    Operand(std::string name, VarKind kind);
    virtual ~Operand() = default;
};

class FunctionParam : public Operand {
  private:
    Type *type;

  public:
    FunctionParam() = delete;
    FunctionParam(std::string name);
    ~FunctionParam() = default;

    Type *getType();
    void setType(Type *type);
};

} // namespace nballerina

#endif //!__BALOPERAND__H__
