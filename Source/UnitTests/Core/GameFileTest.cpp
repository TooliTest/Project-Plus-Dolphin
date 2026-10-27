// Copyright 2026 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <string>

#include <gtest/gtest.h>

#include "Common/FileUtil.h"
#include "UICommon/GameFile.h"

TEST(GameFile, WindowsExecutablePathUsesFileNameForGameID)
{
  const std::string temp_dir = File::CreateTempDir();
  ASSERT_FALSE(temp_dir.empty());

  const std::string launcher_path = temp_dir + "/vBrawl Netplay Launcher.dol";
  ASSERT_TRUE(File::CreateEmptyFile(launcher_path));

  std::string windows_path = launcher_path;
  std::ranges::replace(windows_path, '/', '\\');
  const UICommon::GameFile game_file(windows_path);

  EXPECT_TRUE(game_file.IsValid());
  EXPECT_EQ(game_file.GetFileName(), "vBrawl Netplay Launcher.dol");
  EXPECT_EQ(game_file.GetGameID(), "ID-vBrawl Netplay Launcher");

  File::DeleteDirRecursively(temp_dir);
}
