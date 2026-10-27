// Copyright 2026 Brawlback Team
// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include "Core/NetPlayProto.h"

namespace NetPlay
{
TEST(NetPlayMapping, ManagedControllerAlwaysUsesFirstLocalSource)
{
  constexpr unsigned int slot_count = 4;

  EXPECT_EQ(GetLocalInputSourceForSlot(0, 0, slot_count), 0u);

  EXPECT_EQ(GetLocalInputSourceForSlot(0, 1, slot_count), 1u);
  EXPECT_EQ(GetLocalInputSourceForSlot(1, 1, slot_count), 0u);
  EXPECT_EQ(GetLocalInputSourceForSlot(2, 1, slot_count), 2u);
  EXPECT_EQ(GetLocalInputSourceForSlot(3, 1, slot_count), 3u);

  EXPECT_EQ(GetLocalInputSourceForSlot(0, 3, slot_count), 1u);
  EXPECT_EQ(GetLocalInputSourceForSlot(1, 3, slot_count), 2u);
  EXPECT_EQ(GetLocalInputSourceForSlot(2, 3, slot_count), 3u);
  EXPECT_EQ(GetLocalInputSourceForSlot(3, 3, slot_count), 0u);
}

TEST(NetPlayMapping, InvalidManagedSlotsRemainUnchanged)
{
  EXPECT_EQ(GetLocalInputSourceForSlot(4, 0, 4), 4u);
  EXPECT_EQ(GetLocalInputSourceForSlot(2, 4, 4), 2u);
}
}  // namespace NetPlay
