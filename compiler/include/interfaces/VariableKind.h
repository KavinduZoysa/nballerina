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

#ifndef __VARIABLEKIND__H__
#define __VARIABLEKIND__H__

#include <string>

namespace nballerina {

enum VarKind {
    LOCAL_VAR_KIND = 1,
    ARG_VAR_KIND = 2,
    TEMP_VAR_KIND = 3,
    RETURN_VAR_KIND = 4,
    GLOBAL_VAR_KIND = 5,
    SELF_VAR_KIND = 6,
    CONSTANT_VAR_KIND = 7
};

class VariableKind {
  private:
    std::string name;
    VarKind kind;

  public:
    VariableKind() = delete;
    VariableKind(std::string _name, VarKind _kind) : name(_name), kind(_kind) {}
    virtual ~VariableKind() = default;

    VarKind getKind() { return kind; }
    std::string &getName() { return name; };
};

} // namespace nballerina

#endif //!__VARIABLEKIND__H__
