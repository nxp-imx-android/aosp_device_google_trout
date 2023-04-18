/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <time.h>
#include <cstring>
#include <iostream>
#include <optional>
#include <sstream>
#include <unordered_map>

uint64_t s2ns(uint64_t s) {
    return s * 1000000000ull;
}

int GetTime(int type, uint64_t* ts_ns) {
    struct timespec ts;
    int res = clock_gettime(type, &ts);
    if (!res) {
        *ts_ns = s2ns(ts.tv_sec) + ts.tv_nsec;
    }
    return res;
}

uint64_t GetCPUTicks() {
#if defined(__x86_64__) || defined(__amd64__)
    uint32_t hi, lo;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
#elif defined(__aarch64__)
    uint64_t vct;
    asm volatile("mrs %0, cntvct_el0" : "=r"(vct));
    return vct;
#else
#error "no cpu tick support."
#endif
}

void PrintHelpAndExit(const std::string& error_msg = "") {
    int exit_error = 0;
    if (!error_msg.empty()) {
        std::cout << error_msg << "\n";
        exit_error = 1;
    }

    std::cout << "Usage: ClockTime [CLOCK_ID]\n"
              << "CLOCK_ID can be  CLOCK_REALTIME or CLOCK_MONOTONIC \n"
              << "if omitted, it will obtain the processors's time-stamp counter \n"
              << "on x86 it will use RDTSC, on arm64 it will use MRS CNTCVT. \n"
              << "-h, --help      Print this help message\n";

    exit(exit_error);
}

int main(int argc, char* argv[]) {
    std::unordered_map<std::string, clockid_t> clock_map = {
            std::make_pair("CLOCK_REALTIME", CLOCK_REALTIME),
            std::make_pair("CLOCK_MONOTONIC", CLOCK_MONOTONIC)};

    if (argc == 1) {
        std::cout << GetCPUTicks() << "\n";
    } else if (argc == 2) {
        if (!(strcmp(argv[1], "-h") && strcmp(argv[1], "--help"))) {
            PrintHelpAndExit();
        }

        uint64_t ts_ns;
        auto it = clock_map.find(argv[1]);
        if (it == clock_map.end()) {
            PrintHelpAndExit("Wrong CLOCK_ID");
        }

        int res = GetTime(it->second, &ts_ns);
        if (res) {
            std::stringstream err_msg("GetTime() got error");
            err_msg << res;
            PrintHelpAndExit(err_msg.str());
        }

        std::cout << ts_ns << "\n";
    } else {
        PrintHelpAndExit("Wrong number of arguments");
    }

    return EXIT_SUCCESS;
}
