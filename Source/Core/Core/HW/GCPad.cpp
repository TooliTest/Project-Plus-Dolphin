// Copyright 2010 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Core/HW/GCPad.h"

#include <atomic>
#include <limits>

#include "Common/Common.h"
#include "Core/Config/MainSettings.h"
#include "Core/Core.h"
#include "Core/CoreTiming.h"
#include "Core/HW/GCPadEmu.h"
#include "Core/Movie.h"
#include "Core/System.h"
#include "InputCommon/ControllerEmu/ControlGroup/ControlGroup.h"
#include "InputCommon/GCPadStatus.h"
#include "InputCommon/InputConfig.h"

namespace Pad
{
static InputConfig s_config("GCPadNew", _trans("Pad"), "GCPad", "Pad");
static std::atomic_bool s_replay_boot_input_enabled{false};
static constexpr u64 REPLAY_MENU_A_TARGET_FRAME = 60 * 12;
static constexpr u64 REPLAY_MENU_A_FRAME_UNSET = std::numeric_limits<u64>::max();
static std::atomic<u64> s_replay_menu_a_frame{REPLAY_MENU_A_FRAME_UNSET};
InputConfig* GetConfig()
{
  return &s_config;
}

void Shutdown()
{
  s_config.UnregisterHotplugCallback();

  s_config.ClearControllers();
}

void Initialize()
{
  if (s_config.ControllersNeedToBeCreated())
  {
    for (unsigned int i = 0; i < 4; ++i)
      s_config.CreateController<GCPad>(i);
  }

  s_config.RegisterHotplugCallback();

  // Load the saved controller config
  s_config.LoadConfig();
}

void LoadConfig()
{
  s_config.LoadConfig();
}

void GenerateDynamicInputTextures()
{
  s_config.GenerateControllerTextures();
}

bool IsInitialized()
{
  return !s_config.ControllersNeedToBeCreated();
}

GCPadStatus GetStatus(int pad_num)
{
  GCPadStatus status = static_cast<GCPad*>(s_config.GetController(pad_num))->GetInput();
  ApplyReplayBootInput(pad_num, &status);
  return status;
}

void SetReplayBootInputEnabled(bool enabled)
{
  s_replay_menu_a_frame.store(REPLAY_MENU_A_FRAME_UNSET, std::memory_order_release);
  s_replay_boot_input_enabled.store(enabled, std::memory_order_release);
  Core::SetReplayBootFastForwardActive(enabled);
}

void ApplyReplayBootInput(int pad_num, GCPadStatus* status)
{
  if (!IsReplayBootInputActive(pad_num))
    return;

  auto& system = Core::System::GetInstance();
  const u64 frame = system.GetMovie().GetCurrentFrame();
  u64 a_frame = s_replay_menu_a_frame.load(std::memory_order_acquire);
  if (frame >= REPLAY_MENU_A_TARGET_FRAME && a_frame == REPLAY_MENU_A_FRAME_UNSET)
  {
    s_replay_menu_a_frame.compare_exchange_strong(a_frame, frame, std::memory_order_acq_rel);
    a_frame = s_replay_menu_a_frame.load(std::memory_order_acquire);
  }
  const bool pressing_a = frame == a_frame;

  if (pressing_a && Core::IsReplayBootFastForwardActive())
  {
    Core::SetReplayBootFastForwardActive(false);
    system.GetCoreTiming().ResetThrottleToNow();
  }

  // Keep the user's live P1 input available while adding the replay-menu startup sequence.
  status->isConnected = true;
  status->button |= pressing_a ? PAD_BUTTON_A : PAD_TRIGGER_Z;
}

bool IsReplayBootInputActive(int pad_num)
{
  if (pad_num != 0 || !s_replay_boot_input_enabled.load(std::memory_order_acquire))
    return false;

  auto& system = Core::System::GetInstance();
  const u64 frame = system.GetMovie().GetCurrentFrame();
  const u64 a_frame = s_replay_menu_a_frame.load(std::memory_order_acquire);
  return frame < REPLAY_MENU_A_TARGET_FRAME || a_frame == REPLAY_MENU_A_FRAME_UNSET ||
         frame <= a_frame;
}

ControllerEmu::ControlGroup* GetGroup(int pad_num, PadGroup group)
{
  return static_cast<GCPad*>(s_config.GetController(pad_num))->GetGroup(group);
}

void Rumble(const int pad_num, const ControlState strength)
{
  static_cast<GCPad*>(s_config.GetController(pad_num))->SetOutput(strength);
}

void ResetRumble(const int pad_num)
{
  static_cast<GCPad*>(s_config.GetController(pad_num))->SetOutput(0.0);
}

bool GetMicButton(const int pad_num)
{
  return static_cast<GCPad*>(s_config.GetController(pad_num))->GetMicButton();
}
}  // namespace Pad
