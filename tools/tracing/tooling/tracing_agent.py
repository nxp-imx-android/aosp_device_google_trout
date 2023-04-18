
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
#
import argparse
from datetime import datetime
import os
import sys
import subprocess
import time
import traceback

# May import this package in the workstation with:
# pip install paramiko
from paramiko import SSHClient
from paramiko import AutoAddPolicy

# This script works on Linux workstation.
# We haven't tested on Windows/macOS.

# Demonstration of tracing QNX host and get tracelogger output as a binary file.
#
# Prerequirements for run the script:
# Install  traceprinter utility in QNX Software and setup proper path for it.
# One can Install QNX Software Center from the following location:
# https://www.qnx.com/download/group.html?programid=29178
# Define an environment varialbe QNX_DEV_DIR, the script will read this environment variable.
# export QNX_DEV_DIR=/to/qns_dev_dir/
#
# Usage:
# python3 tracing_agent.py --guest_serial 10.42.0.235 --host_ip
# 10.42.0.235 --host_tracing_file_name qnx.trace --out_dir test_trace
# --duration 2 --host_username root
#

def parseArguments():
    parser = argparse.ArgumentParser(
        prog = 'vm_tracing_driver.py',
        description='VM Tracing Driver')
    parser.add_argument('--guest_serial', required=True,
                             help = 'guest VM serial number')
    parser.add_argument('--host_ip', required=True,
                             help = 'host IP address')
    #TODO(b/267675642): read user name from user ssh_config.
    parser.add_argument('--host_username', required=True,
                             help = 'host username')
    parser.add_argument('--host_tracing_file_name', required=True,
                             help = 'host trace file name')
    parser.add_argument('--out_dir', required=True,
                             help = 'directory to store output file')
    parser.add_argument('--duration', type=int, required=True,
                             help = 'tracing time')
    return parser.parse_args()

# TODO(b/267675642): add HostTracingAgent as an interface and
# QnxHostTracingAgent.

# HostTracingAgent for QNX
class HostTracingAgent:
    def __init__(self, args):
        self.ip = args.host_ip
        self.username = args.host_username
        self.out_dir = args.out_dir
        self.duration = args.duration
        self.tracing_kev_file_path = os.path.join(args.out_dir, f'{args.host_tracing_file_name}.kev')
        self.tracing_printer_file_path = os.path.join(args.out_dir, args.host_tracing_file_name)

    def clientExecuteCmd(self, cmd_str):
        print(f'sshclient executing command {cmd_str}')
        (stdin, stdout, stderr) = self.client.exec_command(cmd_str)
        if stdout.channel.recv_exit_status():
            raise Exception(stderr.read())
        elif stderr.channel.recv_exit_status():
            raise Exception(stderr.read())

    def subprocessRun(self, cmd):
        print(f'Subprocess executing command {cmd}')
        result = subprocess.run(cmd)
        return result

    def doesDirExist(self, dirpath):
        cmd = f'ls -d {dirpath}'
        (stdin, stdout, stderr) = self.client.exec_command(cmd)
        error_str = stderr.read()
        if len(error_str) == 0:
            return True
        return False

    def startTracing(self):
        print('**********start tracing host vm')
        try:

            # start a sshclien to start tracing
            with SSHClient() as sshclient:
                sshclient = SSHClient()
                sshclient.load_system_host_keys()
                sshclient.set_missing_host_key_policy(AutoAddPolicy())
                sshclient.connect(self.ip, username=self.username)
                self.client = sshclient

                # create directory at the host to store tracing config and tracing output
                if self.doesDirExist(self.out_dir) == False:
                    mkdir_cmd = f'mkdir {self.out_dir}'
                    self.clientExecuteCmd(mkdir_cmd)

                # TODO(b/267675642):
                # read the trace configuration file to get the tracing parameters
                # add wrapper function for command execution to report tracing status.
                # split the main() function into incremental steps and report tracing status.

                # start tracing host
                tracing_cmd = f'on -p15 tracelogger  -s {self.duration} -f {self.tracing_kev_file_path}'
                self.clientExecuteCmd(tracing_cmd)
        except Exception as e:
            traceresult = traceback.format_exc()
            error_msg = f'Caught an exception: {traceback.format_exc()}'
            sys.exit(error_msg)

    def copyTracingFile(self):
        print('\n**********start to copy host tracing file to workstation')
        # copy tracing output file from host to workstation
        os.makedirs(self.out_dir, exist_ok=True)
        scp_cmd = ['scp', '-F', '/dev/null',
                   f'{self.username}@{self.ip}:{self.tracing_kev_file_path}',
                   f'{self.tracing_kev_file_path}']
        result = self.subprocessRun(scp_cmd)
        if result.returncode != 0:
            raise Exception(result.stderr)

    def parseTracingFile(self):
        print('\n**********start to parse host tracing file')
        # using traceprinter to convert binary file to text file
        # for traceprinter options, reference:
        # http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.utilities/topic/t/traceprinter.html
        qnx_dev_dir = os.environ.get('QNX_DEV_DIR')
        traceprinter = os.path.join(qnx_dev_dir, 'traceprinter')
        traceprinter_cmd = [traceprinter,
                            '-p', '%C %t %Z %z',
                            '-f', f'{self.tracing_kev_file_path}',
                            '-o', f'{self.tracing_printer_file_path}']
        result = self.subprocessRun(traceprinter_cmd)
        if result.returncode != 0:
            raise Exception(result.stderr)

        # convert tracing file in text format to json format:
        convert_cmd = ['qnx_perfetto.py',
                       f'{self.tracing_printer_file_path}']
        result = self.subprocessRun(convert_cmd)
        if result.returncode != 0:
            raise Exception(result.stderr)

    def run(self):
        try:
            self.startTracing()
            self.copyTracingFile()
            self.parseTracingFile()

        except Exception as e:
            traceresult = traceback.format_exc()
            error_msg = f'Caught an exception: {traceback.format_exc()}'
            sys.exit(error_msg)

def main():
    args = parseArguments()
    host_agent = HostTracingAgent(args)
    host_agent.run()

if __name__ == "__main__":
    main()
