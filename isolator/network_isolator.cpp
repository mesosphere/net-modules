/**
 * This file is © 2015 Mesosphere, Inc. ("Mesosphere"). Mesosphere
 * licenses this file to you solely pursuant to the agreement between
 * Mesosphere and you (if any).  If there is no such agreement between
 * Mesosphere, the following terms apply (and you may not use this
 * file except in compliance with such terms):
 *
 * 1) Subject to your compliance with the following terms, Mesosphere
 * hereby grants you a nonexclusive, limited, personal,
 * non-sublicensable, non-transferable, royalty-free license to use
 * this file solely for your internal business purposes.
 *
 * 2) You may not (and agree not to, and not to authorize or enable
 * others to), directly or indirectly:
 *   (a) copy, distribute, rent, lease, timeshare, operate a service
 *   bureau, or otherwise use for the benefit of a third party, this
 *   file; or
 *
 *   (b) remove any proprietary notices from this file.  Except as
 *   expressly set forth herein, as between you and Mesosphere,
 *   Mesosphere retains all right, title and interest in and to this
 *   file.
 *
 * 3) Unless required by applicable law or otherwise agreed to in
 * writing, Mesosphere provides this file on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
 * including, without limitation, any warranties or conditions of
 * TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * 4) In no event and under no legal theory, whether in tort
 * (including negligence), contract, or otherwise, unless required by
 * applicable law (such as deliberate and grossly negligent acts) or
 * agreed to in writing, shall Mesosphere be liable to you for
 * damages, including any direct, indirect, special, incidental, or
 * consequential damages of any character arising as a result of these
 * terms or out of the use or inability to use this file (including
 * but not limited to damages for loss of goodwill, work stoppage,
 * computer failure or malfunction, or any and all other commercial
 * damages or losses), even if Mesosphere has been advised of the
 * possibility of such damages.
 */

#include <mesos/mesos.hpp>
#include <mesos/module.hpp>

#include <mesos/module/isolator.hpp>

#include <mesos/slave/isolator.hpp>

#include <process/future.hpp>
#include <process/owned.hpp>
#include <process/subprocess.hpp>

#include <stout/try.hpp>

using namespace mesos;

using mesos::slave::Isolator;

const char* initializationKey = "initialization_command";
const char* cleanupKey = "cleanup_command";

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
        Try<process::Subprocess> child = process::subprocess(parameter.value());
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
