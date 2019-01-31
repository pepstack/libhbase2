/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
package org.apache.hadoop.hbase.jni;

import java.io.Closeable;
import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hbase.HBaseConfiguration;

import org.apache.hadoop.hbase.client.Connection;
import org.apache.hadoop.hbase.client.ConnectionFactory;

import org.apache.hadoop.hbase.client.Admin;
import org.apache.hadoop.hbase.client.HBaseAdmin;


public final class AdminProxy implements Closeable {
    protected Connection connection_;

    protected Admin admin_;

    public AdminProxy(Configuration conf) throws IOException {
        // try-with-resources only for Java7+
        try (Connection connection = ConnectionFactory.createConnection(conf)) {
            admin_ = connection.getAdmin();
            connection_ = connection;
        }
    }
    @Override
    public void close() throws IOException {
        admin_.close();
        connection_.close();
    }

    /*
    public void close(final long callback, final long client, final long extra) throws IOException {
        this.client_.shutdown().addBoth(new ClientShutdownCallback<Object, Object>(callback, client, extra));
    }
    */
}
