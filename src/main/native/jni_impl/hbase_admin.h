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
#ifndef HBASE_JNI_IMPL_ADMIN_H_
#define HBASE_JNI_IMPL_ADMIN_H_

/**
 * https://hbase.apache.org/devapidocs/org/apache/hadoop/hbase/client/HBaseAdmin.html
 *
 *   HBaseAdmin is no longer a client API.
 *   It is marked InterfaceAudience. Private indicating that this is an HBase-internal class as defined in:
 *     https://hadoop.apache.org/docs/current/hadoop-project-dist/hadoop-common/InterfaceClassification.html
 *
 *   There are no guarantees for backwards source / binary compatibility and methods or class
 *     can change or go away without deprecation.
 *
 *   Use Connection.getAdmin() to obtain an instance of Admin instead of constructing an HBaseAdmin directly.
 */

#include <jni.h>

#include "hbase_coldesc.h"
#include "hbase_config.h"
#include "hbase_status.h"

namespace hbase {

class HBaseAdmin : public JniObject {
public:
  HBaseAdmin();

  ~HBaseAdmin();

  Status Init(HBaseConfiguration *conf, JNIEnv *current_env=NULL);

  Status Close(JNIEnv *current_env=NULL);

  Status TableExists(const char *name_space, const char *tableName, JNIEnv *current_env=NULL);

  Status TableEnabled(const char *name_space, const char *tableName, JNIEnv *current_env=NULL);

  Status CreateTable(const char *name_space, const char *tableName,
      const HColumnDescriptor *families[], const size_t numFamilies, JNIEnv *current_env=NULL);

  Status EnableTable(const char *name_space, const char *tableName, JNIEnv *current_env=NULL);

  Status DisableTable(const char *name_space, const char *tableName, JNIEnv *current_env=NULL);

  Status DeleteTable(const char *name_space, const char *tableName, JNIEnv *current_env=NULL);

private:
  pthread_mutex_t admin_mutex;
};

} /* namespace hbase */

#endif /* HBASE_JNI_IMPL_ADMIN_H_ */
