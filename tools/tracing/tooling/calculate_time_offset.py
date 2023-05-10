#!/usr/bin/env python3
#
# Copyright (C) 2023 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
from datetime import datetime
from threading import Thread
import os
import re
import sys
import subprocess
import time
import traceback

# May import this package in the workstation with:
# pip install paramiko
from paramiko import SSHClient
from paramiko import AutoAddPolicy

# Usage:
# ./calculate_time_offset.py --host_username root --host_ip 10.42.0.247
# --guest_serial 10.42.0.247 --clock_name CLOCK_REALTIME
# or
# ./calculate_time_offset.py --host_username root --host_ip 10.42.0.247
# --guest_serial 10.42.0.247 --clock_name CLOCK_REALTIME --mode trace

def subprocessRun(cmd):
    try :
        result = subprocess.run(cmd,  stdout=subprocess.PIPE, check=True)
    except Exception as e:
        traceresult = traceback.format_exc()
        error_msg = f'subprocessRun caught an exception: {traceback.format_exc()}'
        sys.exit(error_msg)
    return result.stdout.decode('utf-8')

class Device:
    # Get the machine time
    def __init__(self, args):
        if args.clock_name != None:
            self.time_cmd += f' {args.clock_name}'
        if args.trace:
            if args.clock_name == None:
                raise SystemExit("Error: with trace mode, clock_name must be specified")
            self.time_cmd = f'{self.time_cmd} --trace'
    def GetTime(self):
        pass

    def ParseTime(self, time_str):
        pattern = r'\d+'
        match = re.search(pattern, time_str)
        if match is None:
            traceresult = traceback.format_exc()
            error_msg = f'ParseTime no match time string({time_str}): {traceback.format_exc()}'
            sys.exit(error_msg)
        return int(match.group())

    def TraceTime(self, ts_str):
        lines = ts_str.split("\n")
        if len(lines) != 3:
            sys.exit(f'{ts_str} should be three lines')
        self.cpu_ts = int(lines[0])
        self.clock_ts = int(lines[1])
        self.cpu_cycles = float(lines[2])

class QnxDevice(Device):
    def __init__(self, args):
        self.sshclient = SSHClient()
        self.sshclient.load_system_host_keys()
        self.sshclient.set_missing_host_key_policy(AutoAddPolicy())
        self.sshclient.connect(args.host_ip, username=args.host_username)
        self.time_cmd = "/bin/QnxClocktime"
        super().__init__(args)

    def GetTime(self):
        (stdin, stdout, stderr) = self.sshclient.exec_command(self.time_cmd)
        return stdout
    def ParseTime(self, time_str):
        time_decoded_str = time_str.read().decode()
        return super().ParseTime(time_decoded_str)

    def TraceTime(self):
        result_str = self.GetTime()
        ts_str = result_str.read().decode()
        super().TraceTime(ts_str)

class AndroidDevice(Device):
    def __init__(self, args):
        subprocessRun(['adb', 'connect', args.guest_serial])
        self.time_cmd =  "/vendor/bin/android.automotive.time_util"
        self.serial = args.guest_serial
        super().__init__(args)
    def GetTime(self):
        ts = subprocessRun(['adb', '-s',  self.serial, 'shell', self.time_cmd])
        return ts
    def TraceTime(self):
        super().TraceTime(self.GetTime())
# measure the time offset between device1 and device2 with ptp,
# return the average value over cnt times.
def Ptp(device1, device2):
    # set up max delay as 100 milliseconds
    max_delay_ms = 100000000
    # set up max offset as 2 milliseconds
    max_offset_ms = 2000000
    max_retry = 20
    for i in range(max_retry):
        time1_d1_str = device1.GetTime()
        time1_d2_str = device2.GetTime()
        time2_d2_str = device2.GetTime()
        time2_d1_str = device1.GetTime()

        time1_d1 = device1.ParseTime(time1_d1_str)
        time2_d1 = device1.ParseTime(time2_d1_str)
        time1_d2 = device2.ParseTime(time1_d2_str)
        time2_d2 = device2.ParseTime(time2_d2_str)

        offset = (time1_d2 + time2_d2 - time1_d1 - time2_d1)/2
        if time2_d1 - time1_d1 > max_delay_ms or time2_d2 - time2_d2 > max_delay_ms or abs(offset) > max_offset_ms:
            print(f'Network delay is too big, ignore this measure {offset}')
        else:
            return int(offset)
    raise SystemExit(f"Network delay is still too big after {max_retry} retries")

def TraceTimeOffset(device1, device2):
    device1.TraceTime()
    device2.TraceTime()
    offset = device2.clock_ts - device1.clock_ts - ((device2.cpu_ts - device1.cpu_ts)/device2.cpu_cycles)
    return int(offset)

def ParseArguments():
    parser = argparse.ArgumentParser(
        prog = 'ptp_qnx_android.py',
        description='Test PTP')
    parser.add_argument('--host_ip', required=True,
                             help = 'host IP address')
    parser.add_argument('--host_username', required=True,
                             help = 'host username')
    parser.add_argument('--guest_serial', required=True,
                        help = 'guest VM serial number')
    parser.add_argument('--clock_name', required=False, choices =['CLOCK_REALTIME','CLOCK_MONOTONIC'],
                        help = 'clock that will be used for the measument. By default CPU counter is used.')
    parser.add_argument('--mode', choices=['ptp', 'trace'], default='ptp',
                    help='select the mode of operation. trace option meaning using CPU counter to calculate the time offset between devices')
    return parser.parse_args()

def main():
    args = ParseArguments()
    qnx = QnxDevice(args)
    android = AndroidDevice(args)
    if args.mode == "trace":
        offset = TraceTimeOffset(qnx, android)
    else:
        offset = Ptp(qnx, android)
    print(f'Time offset between {type(qnx).__name__} and {type(qnx).__name__} is {offset} nanoseconds')
if __name__ == "__main__":
    main()
