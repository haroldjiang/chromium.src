// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DISPLAY_MANAGER_CHROMEOS_UPDATE_DISPLAY_CONFIGURATION_TASK_H_
#define UI_DISPLAY_MANAGER_CHROMEOS_UPDATE_DISPLAY_CONFIGURATION_TASK_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "ui/display/manager/chromeos/configure_displays_task.h"
#include "ui/display/manager/chromeos/display_configurator.h"

namespace display {

class DisplaySnapshot;
class NativeDisplayDelegate;

class DISPLAY_MANAGER_EXPORT UpdateDisplayConfigurationTask {
 public:
  typedef base::Callback<void(
      bool /* success */,
      const std::vector<DisplaySnapshot*>& /* displays */,
      const gfx::Size& /* framebuffer_size */,
      MultipleDisplayState /* new_display_state */,
      chromeos::DisplayPowerState /* new_power_state */)>
      ResponseCallback;

  UpdateDisplayConfigurationTask(NativeDisplayDelegate* delegate,
                                 DisplayLayoutManager* layout_manager,
                                 MultipleDisplayState new_display_state,
                                 chromeos::DisplayPowerState new_power_state,
                                 int power_flags,
                                 uint32_t background_color_argb,
                                 bool force_configure,
                                 const ResponseCallback& callback);
  ~UpdateDisplayConfigurationTask();

  // The pointers to the DisplaySnapshots in this vector are owned by
  // DisplayConfigurator.
  void SetVirtualDisplaySnapshots(
      const std::vector<std::unique_ptr<DisplaySnapshot>>& snapshots);

  void Run();

 private:
  // Callback to NativeDisplayDelegate::GetDisplays().
  void OnDisplaysUpdated(const std::vector<DisplaySnapshot*>& displays);

  // Callback to ConfigureDisplaysTask used to process the result of a display
  // configuration run.
  void OnStateEntered(ConfigureDisplaysTask::Status status);

  // If the initial display configuration run failed due to errors entering
  // mirror more, another configuration run is executed to enter software
  // mirroring. This is the callback used to process the result of that
  // configuration.
  void OnEnableSoftwareMirroring(ConfigureDisplaysTask::Status status);

  // Starts the configuration process. |callback| is used to continue the task
  // after |configure_taks_| finishes executing.
  void EnterState(const ConfigureDisplaysTask::ResponseCallback& callback);

  // Finishes display configuration and runs |callback_|.
  void FinishConfiguration(bool success);

  // Returns true if the DPMS state should be force to on.
  bool ShouldForceDpms() const;

  // Returns true if a display configuration is required.
  bool ShouldConfigure() const;

  // Returns a display state based on the power state.
  MultipleDisplayState ChooseDisplayState() const;

  NativeDisplayDelegate* delegate_;       // Not owned.
  DisplayLayoutManager* layout_manager_;  // Not owned.

  // Requested display state.
  MultipleDisplayState new_display_state_;

  // Requested power state.
  chromeos::DisplayPowerState new_power_state_;

  // Bitwise-or-ed values for the kSetDisplayPower* values defined in
  // DisplayConfigurator.
  int power_flags_;

  uint32_t background_color_argb_;

  bool force_configure_;

  // Used to signal that the task has finished.
  ResponseCallback callback_;

  bool force_dpms_;

  // List of updated displays.
  std::vector<DisplaySnapshot*> cached_displays_;

  // Vector of unowned VirtualDisplaySnapshots to be added when doing the task.
  std::vector<DisplaySnapshot*> virtual_display_snapshots_;

  gfx::Size framebuffer_size_;

  std::unique_ptr<ConfigureDisplaysTask> configure_task_;

  base::WeakPtrFactory<UpdateDisplayConfigurationTask> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(UpdateDisplayConfigurationTask);
};

}  // namespace display

#endif  // UI_DISPLAY_MANAGER_CHROMEOS_UPDATE_DISPLAY_CONFIGURATION_TASK_H_
