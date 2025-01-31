// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "paths.hpp"

#include <fstream>

#include <gtest/gtest.h>

// Create test equivalents of /sys/class/fsi-master/ and /dev/
class CFAMSDevice : public ::testing::Test
{
  public:
    CFAMSDevice() :
        fsiMasterDir(paths::getFSIMasterDir()), deviceDir(paths::getDeviceDir())
    {
        // Create the fake /sys/class/fsi-master/ files
        auto link0 = fsiMasterDir / "fsi0/slave@00:00/00:00:00:13";
        std::filesystem::create_directories(link0);

        auto link0CFAM = link0 / "mbox-cfam-s10";
        std::ofstream fsi0{link0CFAM.c_str()};
        fsi0 << "foo";
        fsi0.close();

        auto link1 = fsiMasterDir / "fsi1/slave@00:00/01:00:00:13";
        std::filesystem::create_directories(link1);

        auto link1CFAM = link1 / "mbox-cfam-s11";
        std::ofstream fsi1{link1CFAM.c_str()};
        fsi1 << "foo";
        fsi1.close();

        // Create the fake /dev files
        link0Device = deviceDir / "mbox-cfam-s10";
        std::ofstream dev0(link0Device.c_str());
        dev0 << "foo";
        dev0.close();

        link1Device = deviceDir / "mbox-cfam-s11";
        std::ofstream dev1(link1Device.c_str());
        dev1 << "foo";
        dev1.close();
    }

    virtual ~CFAMSDevice()
    {
        std::filesystem::remove_all(fsiMasterDir);
        std::filesystem::remove_all(deviceDir);
    }

    std::filesystem::path fsiMasterDir;
    std::filesystem::path deviceDir;
    std::filesystem::path link0Device;
    std::filesystem::path link1Device;
};
