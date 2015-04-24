/**
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
 */

#include <mesos/mesos.hpp>
#include <mesos/module.hpp>

#include <mesos/module/isolator.hpp>

#include <mesos/slave/isolator.hpp>

#include <process/future.hpp>
#include <process/owned.hpp>
#include <process/subprocess.hpp>

#include <stout/try.hpp>
#include <stout/stringify.hpp>

using namespace mesos;

using mesos::slave::Isolator;

const char* initializationKey = "initialization_command";
const char* cleanupKey = "cleanup_command";
const char* isolateKey = "isolate_command";
const char* pythonPath = "/usr/bin/python";

class MetaswitchNetworkIsolatorProcess : public mesos::slave::IsolatorProcess
{
public:
  static Try<mesos::slave::Isolator*> create(const Parameters& parameters)
  {
    return new Isolator(process::Owned<IsolatorProcess>(
        new MetaswitchNetworkIsolatorProcess(parameters)));
  }

  virtual ~MetaswitchNetworkIsolatorProcess() {}

  virtual process::Future<Nothing> recover(
      const std::list<mesos::slave::ExecutorRunState>& states)
  {
    return Nothing();
  }

  virtual process::Future<Option<CommandInfo>> prepare(
      const ContainerID& containerId,
      const ExecutorInfo& executorInfo,
      const std::string& directory,
      const Option<std::string>& user)
  {
    LOG(INFO) << "MetaswitchNetworkIsolator::prepare";
    foreach (const Parameter& parameter, parameters.parameter()) {
      if (parameter.key() == initializationKey) {
        CommandInfo commandInfo;
        commandInfo.set_value(parameter.value());
        return commandInfo;
      }
    }
    return None();
  }

  virtual process::Future<Nothing> isolate(
      const ContainerID& containerId,
      pid_t pid)
  {
    foreach (const Parameter& parameter, parameters.parameter()) {
      if (parameter.key() == isolateKey) {
        std::vector<std::string> argv(5);
        argv[0] = "python";
        argv[1] = parameter.value();
        argv[2] = "isolate";
        argv[3] = stringify(pid);
        argv[4] = containerId.value();
        Try<process::Subprocess> child = process::subprocess(pythonPath, argv);
        CHECK_SOME(child);
        waitpid(child.get().pid(), NULL, 0);
        break;
      }
    }
    return Nothing();
  }

  virtual process::Future<mesos::slave::Limitation> watch(
      const ContainerID& containerId)
  {
    return process::Future<mesos::slave::Limitation>();
  }

  virtual process::Future<Nothing> update(
      const ContainerID& containerId,
      const Resources& resources)
  {
    return Nothing();
  }

  virtual process::Future<ResourceStatistics> usage(
      const ContainerID& containerId)
  {
    return ResourceStatistics();
  }

  virtual process::Future<Nothing> cleanup(
      const ContainerID& containerId)
  {
    foreach (const Parameter& parameter, parameters.parameter()) {
      if (parameter.key() == cleanupKey) {
        std::vector<std::string> argv(5);
        argv[0] = "python";
        argv[1] = parameter.value();
        argv[2] = "cleanup";
        argv[3] = stringify(pid);
        argv[4] = containerId.value();
        Try<process::Subprocess> child = process::subprocess(pythonPath, argv);
        CHECK_SOME(child);
        waitpid(child.get().pid(), NULL, 0);
        break;
      }
    }
    return Nothing();
  }

private:
  MetaswitchNetworkIsolatorProcess(const Parameters& parameters_)
    : parameters(parameters_) {}

  const Parameters parameters;
};


static Isolator* createMetaswitchNetworkIsolator(const Parameters& parameters)
{
  LOG(INFO) << "Loading Metaswitch Network Isolator module";
  Try<Isolator*> result = MetaswitchNetworkIsolatorProcess::create(parameters);
  if (result.isError()) {
    return NULL;
  }
  return result.get();
}


// Declares the Metaswitch network isolator named
// 'org_apache_mesos_MetaswitchNetworkIsolator'.
mesos::modules::Module<Isolator> com_mesosphere_mesos_MetaswitchNetworkIsolator(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Mesosphere",
    "support@mesosphere.com",
    "Metaswitch Network Isolator module.",
    NULL,
    createMetaswitchNetworkIsolator);
