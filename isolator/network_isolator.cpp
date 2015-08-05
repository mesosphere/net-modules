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

#include <mesos/hook.hpp>
#include <mesos/mesos.hpp>
#include <mesos/module.hpp>

#include <mesos/module/hook.hpp>
#include <mesos/module/isolator.hpp>

#include <mesos/slave/isolator.hpp>

#include <process/future.hpp>
#include <process/owned.hpp>
#include <process/process.hpp>
#include <process/subprocess.hpp>
#include <process/io.hpp>

#include <stout/try.hpp>
#include <stout/stringify.hpp>
#include <stout/hashmap.hpp>
#include <stout/option.hpp>

#include "network_isolator.hpp"

using namespace mesos;
using namespace process;

using mesos::slave::ContainerPrepareInfo;
using mesos::slave::Isolator;

const char* initializationKey = "initialization_command";
const char* cleanupKey = "cleanup_command";
const char* isolateKey = "isolate_command";
const char* ipamKey = "ipam_command";
const char* pythonPath = "/usr/bin/python";
const char* ipAddressLabelKey = "MesosContainerizer.NetworkSettings.IPAddress";

hashmap<ContainerID, Info*> *infos = NULL;
hashmap<ExecutorID, ContainerID> *executors = NULL;


Try<Isolator*> CalicoIsolatorProcess::create(const Parameters& parameters)
{
  std::string ipamPath = "";
  foreach (const Parameter& parameter, parameters.parameter()) {
    if (parameter.key() == ipamKey) {
      ipamPath = parameter.value();
    }
  }
  if (ipamPath == "") {
    LOG(WARNING) << "IPAM path not specified";
    return Error("IPAM path not specified.");
  }
  return new CalicoIsolator(process::Owned<CalicoIsolatorProcess>(
      new CalicoIsolatorProcess(ipamPath, parameters)));
}


process::Future<Option<ContainerPrepareInfo>> CalicoIsolatorProcess::prepare(
    const ContainerID& containerId,
    const ExecutorInfo& executorInfo,
    const std::string& directory,
    const Option<std::string>& rootfs,
    const Option<std::string>& user)
{
  LOG(INFO) << "CalicoIsolator::prepare";

  std::string ipAddress = "auto";
  std::string profile = "none";
  foreach (const Environment_Variable& var,
      executorInfo.command().environment().variables()) {
    LOG(INFO) << "ENV: " << var.name() << "=" << var.value();
    if (var.name() == "CALICO_IP") {
      ipAddress = var.value();
    }
    else if (var.name() == "CALICO_PROFILE") {
      profile = var.value();
    }
  }

  if (ipAddress == "auto") {
    std::vector<std::string> argv(3);
    argv[0] = "python";
    argv[1] = ipamPath;
    argv[2] = "assign_ipv4";
    Try<process::Subprocess> child = process::subprocess(
        pythonPath,
        argv,
        process::Subprocess::PIPE(),
        process::Subprocess::PIPE(),
        process::Subprocess::PIPE());
    CHECK_SOME(child);
    waitpid(child.get().pid(), NULL, 0);
    ipAddress = process::io::read(child.get().out().get()).get();
    LOG(INFO) << "Got IP " << ipAddress << " from IPAM.";
  }

  LOG(INFO) << "LIBPROCESS_IP=" << ipAddress;

  ContainerPrepareInfo prepareInfo;

  Environment::Variable* variable =
    prepareInfo.mutable_environment()->add_variables();
  variable->set_name("LIBPROCESS_IP");
  variable->set_value(ipAddress);

  foreach (const Parameter& parameter, parameters.parameter()) {
    if (parameter.key() == initializationKey) {
      prepareInfo.add_commands()->set_value(parameter.value());
    }
  }

  (*infos)[containerId] = new Info(ipAddress, profile);
  (*executors)[executorInfo.executor_id()] = containerId;

  return prepareInfo;
}


process::Future<Nothing> CalicoIsolatorProcess::isolate(
    const ContainerID& containerId,
    pid_t pid)
{
  const Info* info = (*infos)[containerId];
  foreach (const Parameter& parameter, parameters.parameter()) {
    if (parameter.key() == isolateKey) {
      std::vector<std::string> argv(7);
      argv[0] = "python";
      argv[1] = parameter.value();
      argv[2] = "isolate";
      argv[3] = stringify(pid);
      argv[4] = containerId.value();
      argv[5] = stringify(info->ipAddress.get());
      argv[6] = stringify(info->profile.get());
      Try<process::Subprocess> child = process::subprocess(pythonPath, argv);
      CHECK_SOME(child);
      waitpid(child.get().pid(), NULL, 0);
      break;
    }
  }
  return Nothing();
}


process::Future<Nothing> CalicoIsolatorProcess::cleanup(
    const ContainerID& containerId)
{
  if (!infos->contains(containerId)) {
    LOG(WARNING) << "Ignoring cleanup for unknown container " << containerId;
    return Nothing();
  }
  infos->erase(containerId);
  foreach (const Parameter& parameter, parameters.parameter()) {
    if (parameter.key() == cleanupKey) {
      std::vector<std::string> argv(4);
      argv[0] = "python";
      argv[1] = parameter.value();
      argv[2] = "cleanup";
      argv[3] = containerId.value();
      Try<process::Subprocess> child = process::subprocess(pythonPath, argv);
      CHECK_SOME(child);
      waitpid(child.get().pid(), NULL, 0);
      break;
    }
  }
  return Nothing();
}


static Isolator* createCalicoIsolator(const Parameters& parameters)
{
  LOG(INFO) << "Loading Calico Isolator module";

  if (infos == NULL) {
    infos = new hashmap<ContainerID, Info*>();
    CHECK(executors == NULL);
    executors = new hashmap<ExecutorID, ContainerID>();
  }

  Try<Isolator*> result = CalicoIsolatorProcess::create(parameters);

  if (result.isError()) {
    return NULL;
  }

  return result.get();
}


// TODO(karya): Use the hooks for Task Status labels.
class CalicoHook : public Hook
{
  virtual Result<Labels> slaveTaskStatusLabelDecorator(
      const FrameworkID& frameworkId,
      const TaskStatus& status)
  {
    LOG(INFO) << "CalicoHook::task status label decorator";

    if (!status.has_executor_id()) {
      LOG(WARNING) << "CalicoHook:: task status has no valid executor id";
      return None();
    }

    const ExecutorID executorId = status.executor_id();
    if (!executors->contains(executorId)) {
      LOG(WARNING) << "CalicoHook:: no valid container id for: " << executorId;
      return None();
    }

    const ContainerID containerId = executors->at(executorId);
    if (infos == NULL || !infos->contains(containerId)) {
      LOG(WARNING) << "CalicoHook:: no valid infos for: " << containerId;
      return None();
    }

    const Info* info = (*infos)[containerId];
    if (info->ipAddress.isNone()) {
      LOG(WARNING) << "CalicoHook:: no valid IP address";
      return None();
    }

    Labels labels;
    if (status.has_labels()) {
      labels.CopyFrom(status.labels());
    }

    // Set IPAddress label.
    Label* label = labels.add_labels();
    label->set_key(ipAddressLabelKey);
    label->set_value(info->ipAddress.get());

    LOG(INFO) << "CalicoHook:: added label "
              << label->key() << ":" << label->value();
    return labels;
  }
};


static Hook* createCalicoHook(const Parameters& parameters)
{
  return new CalicoHook();
}


// Declares the Calico isolator named
// 'org_apache_mesos_CalicoIsolator'.
mesos::modules::Module<Isolator> com_mesosphere_mesos_CalicoIsolator(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Mesosphere",
    "support@mesosphere.com",
    "Calico Isolator module.",
    NULL,
    createCalicoIsolator);


// Declares the Calico hook module 'org_apache_mesos_CalicoHook'.
mesos::modules::Module<Hook> com_mesosphere_mesos_CalicoHook(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Mesosphere",
    "support@mesosphere.com",
    "Calico Hook module.",
    NULL,
    createCalicoHook);
