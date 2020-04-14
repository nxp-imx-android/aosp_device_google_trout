/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "iio_utils.h"
#include <errno.h>
#include <limits.h>
#include <log/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>
#include <fstream>
#include <iostream>

static const char* IIO_DEVICE_BASE = "iio:device";
static const char* DEVICE_IIO_DIR = "/sys/bus/iio/devices/";
static const char* IIO_SCAN_ELEMENTS_EN = "_en";
static const char* IIO_SFA_FILENAME = "sampling_frequency_available";
static const char* IIO_SCALE_FILENAME = "_scale";
static const char* IIO_SAMPLING_FREQUENCY = "_sampling_frequency";
static const char* IIO_BUFFER_ENABLE = "buffer/enable";

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace subhal {
namespace implementation {

static bool str_has_prefix(const char* s, const char* prefix) {
    if (!s || !prefix) return false;

    const auto len_s = strlen(s);
    const auto len_prefix = strlen(prefix);
    if (len_s < len_prefix) return false;
    return std::equal(s, s + len_prefix, prefix);
}

static bool str_has_suffix(const char* s, const char* suffix) {
    if (!s || !suffix) return false;

    const auto len_s = strlen(s);
    const auto len_suffix = strlen(suffix);
    if (len_s < len_suffix) return false;
    return std::equal(s + len_s - len_suffix, s + len_s, suffix);
}

static int sysfs_opendir(const std::string& name, DIR** dp);
static int sysfs_write_uint(const std::string& file, const unsigned int val);
static int sysfs_read_uint8(const std::string& file, uint8_t* val);
static int sysfs_read_str(const std::string& file, std::string* str);
static int sysfs_read_float(const std::string& file, float* val);
static int get_scan_type(const std::string& device_dir, struct iio_info_channel* chanInfo);
static int get_sampling_frequency_available(const std::string& device_dir, std::vector<float>* sfa);
static int get_scale(const std::string& device_dir, float* resolution);
static int check_file(const std::string& filename);

int sysfs_opendir(const std::string& name, DIR** dp) {
    struct stat sb;
    DIR* tmp;

    if (dp == nullptr) {
        return -EINVAL;
    }

    /*
     * Check if path exists, if a component of path does not exist,
     * or path is an empty string return ENOENT
     * If path is not accessible return EACCES
     */
    if (stat(name.c_str(), &sb) == -1) {
        return -errno;
    }

    /* Open sysfs directory */
    tmp = opendir(name.c_str());
    if (tmp == nullptr) return -errno;

    *dp = tmp;

    return 0;
}

int sysfs_write_uint(const std::string& file, const unsigned int val) {
    FILE* fp = fopen(file.c_str(), "r+");
    if (nullptr == fp) return -errno;

    fprintf(fp, "%u", val);
    fclose(fp);

    return 0;
}

int sysfs_read_uint8(const std::string& file, uint8_t* val) {
    int ret;
    if (val == nullptr) {
        return -EINVAL;
    }

    FILE* fp = fopen(file.c_str(), "r");
    if (nullptr == fp) return -errno;

    ret = fscanf(fp, "%hhu\n", val);
    fclose(fp);

    return ret;
}

int sysfs_read_str(const std::string& file, std::string* str) {
    int err = 0;

    std::ifstream infile(file);
    if (!infile.is_open()) return -EINVAL;

    if (!std::getline(infile, *str)) {
        err = -EINVAL;
    }

    infile.close();

    return err;
}
int sysfs_read_float(const std::string& file, float* val) {
    int ret;
    if (val == nullptr) {
        return -EINVAL;
    }

    FILE* fp = fopen(file.c_str(), "r");
    if (nullptr == fp) return -errno;

    ret = fscanf(fp, "%f", val);
    ret = (ret == 1) ? 0 : -EINVAL;
    fclose(fp);

    return ret;
}

int check_file(const std::string& filename) {
    struct stat info;
    return stat(filename.c_str(), &info);
}

int enable_sensor(const std::string& device_dir, const bool enable) {
    int err = check_file(device_dir);
    if (!err) {
        std::string enable_file = device_dir;
        enable_file += "/";
        enable_file += IIO_BUFFER_ENABLE;
        err = sysfs_write_uint(enable_file, enable);
    }

    return err;
}

int get_sampling_frequency_available(const std::string& device_dir, std::vector<float>* sfa) {
    int ret = 0;
    char* rest;
    std::string line;

    std::string filename = device_dir;
    filename += "/";
    filename += IIO_SFA_FILENAME;

    ret = sysfs_read_str(filename, &line);
    if (ret < 0) return ret;
    char* pch = strtok_r(const_cast<char*>(line.c_str()), " ,", &rest);
    while (pch != nullptr) {
        sfa->push_back(atof(pch));
        pch = strtok_r(nullptr, " ,", &rest);
    }

    return ret < 0 ? ret : 0;
}

int set_sampling_frequency(const std::string& device_dir, const unsigned int frequency) {
    DIR* dp;
    const struct dirent* ent;

    int ret = sysfs_opendir(device_dir, &dp);
    if (ret) return ret;
    while (ent = readdir(dp), ent != nullptr) {
        if (str_has_suffix(ent->d_name, IIO_SAMPLING_FREQUENCY)) {
            std::string filename = device_dir;
            filename += "/";
            filename += ent->d_name;
            ret = sysfs_write_uint(filename, frequency);
        }
    }
    closedir(dp);
    return ret;
}

int get_scale(const std::string& device_dir, float* resolution) {
    DIR* dp;
    const struct dirent* ent;
    int err;
    std::string filename;
    if (resolution == nullptr) {
        return -EINVAL;
    }
    err = sysfs_opendir(device_dir, &dp);
    if (err) return err;
    while (ent = readdir(dp), ent != nullptr) {
        if (str_has_suffix(ent->d_name, IIO_SCALE_FILENAME)) {
            filename = device_dir;
            filename += "/";
            filename += ent->d_name;
            err = sysfs_read_float(filename, resolution);
        }
    }
    closedir(dp);
    return err;
}

static bool is_supported_sensor(const std::string& path,
                                const std::vector<sensors_supported_hal>& supported_sensors,
                                std::string* name, sensors_supported_hal* sensor) {
    std::string name_file = path + "/name";
    std::ifstream iio_file(name_file.c_str());
    if (!iio_file) return false;
    std::string iio_name;
    std::getline(iio_file, iio_name);
    auto iter = std::find_if(
            supported_sensors.begin(), supported_sensors.end(),
            [&iio_name](const auto& candidate) -> bool { return candidate.name == iio_name; });
    if (iter == supported_sensors.end()) return false;
    *sensor = *iter;
    *name = iio_name;
    return true;
}

int load_iio_devices(std::vector<iio_device_data>* iio_data,
                     const std::vector<sensors_supported_hal>& supported_sensors) {
    DIR* dp;
    const struct dirent* ent;
    int err;

    std::ifstream iio_file;
    const auto iio_base_len = strlen(IIO_DEVICE_BASE);
    err = sysfs_opendir(DEVICE_IIO_DIR, &dp);
    if (err) return err;
    while (ent = readdir(dp), ent != nullptr) {
        if (!str_has_prefix(ent->d_name, IIO_DEVICE_BASE)) continue;

        std::string path_device = DEVICE_IIO_DIR;
        path_device += ent->d_name;
        sensors_supported_hal sensor_match;
        std::string iio_name;
        if (!is_supported_sensor(path_device, supported_sensors, &iio_name, &sensor_match))
            continue;

        ALOGI("found sensor %s at path %s", iio_name.c_str(), path_device.c_str());
        iio_device_data iio_dev_data;
        iio_dev_data.name = iio_name;
        iio_dev_data.type = sensor_match.type;
        iio_dev_data.sysfspath.append(path_device, 0, strlen(DEVICE_IIO_DIR) + strlen(ent->d_name));
        err = get_sampling_frequency_available(iio_dev_data.sysfspath,
                                               &iio_dev_data.sampling_freq_avl);
        if (err) {
            ALOGE("get_sampling_frequency_available for %s returned error %d", path_device.c_str(),
                  err);
            continue;
        }

        std::sort(iio_dev_data.sampling_freq_avl.begin(), iio_dev_data.sampling_freq_avl.end());
        err = get_scale(iio_dev_data.sysfspath, &iio_dev_data.resolution);
        if (err) {
            ALOGE("get_scale for %s returned error %d", path_device.c_str(), err);
            continue;
        }
        sscanf(ent->d_name + iio_base_len, "%hhu", &iio_dev_data.iio_dev_num);

        iio_data->push_back(iio_dev_data);
    }
    closedir(dp);
    return err;
}

int get_scan_type(const std::string& device_dir, struct iio_info_channel* chanInfo) {
    DIR* dp;
    const struct dirent* ent;
    FILE* fp;
    int ret;
    std::string scan_dir;
    std::string filename;
    std::string type_name;
    char signchar, endianchar;
    unsigned int storage_bits;

    if (chanInfo == nullptr) {
        return -EINVAL;
    }
    scan_dir = device_dir;
    scan_dir += "/scan_elements";
    ret = sysfs_opendir(scan_dir, &dp);
    if (ret) return ret;
    type_name = chanInfo->name;
    type_name += "_type";
    while (ent = readdir(dp), ent != nullptr) {
        if (strcmp(ent->d_name, type_name.c_str()) == 0) {
            filename = scan_dir;
            filename += "/";
            filename += ent->d_name;
            fp = fopen(filename.c_str(), "r");
            if (fp == nullptr) continue;
            ret = fscanf(fp, "%ce:%c%hhu/%u>>%hhu", &endianchar, &signchar, &chanInfo->bits_used,
                         &storage_bits, &chanInfo->shift);
            if (ret < 0) {
                fclose(fp);
                continue;
            }
            chanInfo->big_endian = (endianchar == 'b');
            chanInfo->sign = (signchar == 's');
            chanInfo->storage_bytes = (storage_bits >> 3);
            fclose(fp);
        }
    }
    closedir(dp);
    return 0;
}

int scan_elements(const std::string& device_dir, struct iio_device_data* iio_data) {
    DIR* dp;
    const struct dirent* ent;
    std::string scan_dir;
    std::string filename;
    uint8_t temp;
    int ret;

    if (iio_data == nullptr) {
        return -EINVAL;
    }
    scan_dir = device_dir;
    scan_dir += "/scan_elements";
    ret = sysfs_opendir(scan_dir, &dp);
    if (ret) return ret;
    while (ent = readdir(dp), ent != nullptr) {
        if (str_has_suffix(ent->d_name, IIO_SCAN_ELEMENTS_EN)) {
            filename = scan_dir;
            filename += "/";
            filename += ent->d_name;
            ret = sysfs_write_uint(filename, ENABLE_CHANNEL);
            if (ret == 0) {
                ret = sysfs_read_uint8(filename, &temp);
                if ((ret > 0) && (temp == 1)) {
                    iio_info_channel chan_info;
                    chan_info.name = strndup(ent->d_name,
                                             strlen(ent->d_name) - strlen(IIO_SCAN_ELEMENTS_EN));
                    filename = scan_dir;
                    filename += "/";
                    filename += chan_info.name;
                    filename += "_index";
                    sysfs_read_uint8(filename, &chan_info.index);
                    ret = get_scan_type(device_dir, &chan_info);
                    iio_data->channelInfo.push_back(chan_info);
                }
            }
        }
    }
    closedir(dp);
    return ret;
}
}  // namespace implementation
}  // namespace subhal
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
