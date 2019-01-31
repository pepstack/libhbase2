/**
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements. See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership. The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#line 19 "hbase_tablename.cc" // ensures short filename in logs.

#include <jni.h>
#include <errno.h>

#include "hbase_tablename.h"

#include "hbase_consts.h"
#include "hbase_msgs.h"
#include "jnihelper.h"

namespace hbase {

Status
HBaseTableName::Init(
    const char *tableName,
    JNIEnv *current_env) {
  JNI_GET_ENV(current_env);
  jstring tableNameString = env->NewStringUTF(tableName);
  JniResult result = JniHelper::NewObject(env, HBASE_TABLENAME,
      JMETHOD1(JPARAM(JAVA_STRING), "V"),
      tableNameString);
  if (result.ok()) {
    jobject_ = env->NewGlobalRef(result.GetObject());
  }
  return result;
}

HBaseTableName::~HBaseTableName() {
  if (jobject_ != NULL) {
    JNIEnv *env = JniHelper::GetJNIEnv();
    if (env != NULL) {
      env->DeleteGlobalRef(jobject_);
    }
  }
}

} /* namespace hbase */
