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

import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hbase.HBaseConfiguration;

import org.apache.hadoop.hbase.TableName;

import org.apache.hadoop.hbase.client.Connection;
import org.apache.hadoop.hbase.client.ConnectionFactory;

// HTable is no longer a client API. Use Table instead.
import org.apache.hadoop.hbase.client.Table;

import org.apache.hadoop.hbase.client.Put;
import org.apache.hadoop.hbase.client.Row;

import org.apache.hadoop.hbase.client.BufferedMutator;


/**
 * https://data-flair.training/blogs/hbase-client-api/
 * http://hbase.apache.org/devapidocs/org/apache/hadoop/hbase/client/HTable.html
 * https://hbase.apache.org/apidocs/org/apache/hadoop/hbase/client/package-summary.html
 */
public final class TableProxy implements Closeable {
    protected boolean autoFlush = false;

    // https://docs.oracle.com/javase/tutorial/essential/exceptions/tryResourceClose.html
    protected Connection connection_;

    // Instances of Table class SHOULD NOT be constructed directly.
    // Obtain an instance via Connection.
    // Use the table as needed, for a single operation and a single thread
    protected Table table_;

    protected BufferedMutator mutator_;


    public TableProxy(Configuration conf, String tabeName) throws IOException {
        // try-with-resources only for Java7+
        try (Connection connection = ConnectionFactory.createConnection(conf)) {
            table_ = connection.getTable(TableName.valueOf(tabeName));
            mutator_ = connection_.getBufferedMutator(table_.getName());

            connection_ = connection;
        }
    }

    public void setAutoFlush(boolean autoFlush) {
        this.autoFlush = autoFlush;
    }

    @Override
    public void close() throws IOException {
        table_.close();
        mutator_.close();
    }

    public void flushCommits() throws IOException {
        mutator_.flush();
    }

    public void put(final List<PutProxy> putProxies) throws IOException {
        List<Put> puts = new ArrayList<Put>(putProxies.size());
        for (PutProxy putProxy : putProxies) {
            puts.add((Put) putProxy.toHBaseMutation());
        }

        if (autoFlush) {
            table_.put(puts);
        } else {
            mutator_.mutate(puts);            
        }
    }

    public Object[] batch(final List<MutationProxy> mutations) throws InterruptedException, IOException {
        Object[] results = new Object[mutations.size()];

        List<Row> actions = new ArrayList<Row>(mutations.size());

        for (MutationProxy mutation : mutations) {
            actions.add(mutation.toHBaseMutation());
        }

        table_.batch(actions, results);
        return results;
    }
}
