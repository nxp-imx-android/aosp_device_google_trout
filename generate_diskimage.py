#!/usr/bin/env python3
import os
import subprocess
import parted
import logging
import json
import shutil
import sys
import argparse

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

def create_disk_image(partition_data, output_image_path):
    settings = partition_data['settings']
    partitions = partition_data['partitions']

    disk_size = settings['disk_size']
    with open(output_image_path, 'wb') as f:
        f.seek(disk_size - 1)
        f.write(b'\0')

    device = parted.getDevice(output_image_path)
    disk = parted.freshDisk(device, 'gpt')
    disk.deleteAllPartitions()

    start_sector = 2048
    for idx, partition in enumerate(partitions, start=1):
        label = partition['label']
        size = partition['size']

        end_sector = start_sector + (size // device.sectorSize) - 1
        geometry = parted.Geometry(device=device, start=start_sector, end=end_sector)
        filesystem = parted.FileSystem(type='ext2', geometry=geometry)
        partition = parted.Partition(disk=disk, type=parted.PARTITION_NORMAL, geometry=geometry, fs=filesystem)
        partition.setFlag(parted.PARTITION_BOOT)
        partition.set_name(label)

        constraint = parted.Constraint(exactGeom=geometry)
        disk.addPartition(partition, constraint)

        start_sector = end_sector + 1

    disk.commit()
    logger.info("Successfully created GPT partition table")

def parse_gpt_image(image_path):
    device = parted.getDevice(image_path)
    disk = parted.newDisk(device)

    if disk.type != 'gpt':
        raise ValueError("The disk image is not GPT formatted.")

    partitions = disk.partitions
    partition_map = {
        'super': 'super_raw.img',
        'dtbo_a': 'dtb.img',
        'dtbo_b': 'dtb.img',
        'init_boot_a': 'init_boot.img',
        'init_boot_b': 'init_boot.img',
        'vendor_boot_a': 'vendor_boot.img',
        'vendor_boot_b': 'vendor_boot.img',
        'vbmeta_a': 'vbmeta.img',
        'vbmeta_b': 'vbmeta.img',
        'vbmeta_system_a': 'vbmeta_system.img',
        'vbmeta_system_b': 'vbmeta_system.img',
        'boot_a': 'boot.img',
        'boot_b': 'boot.img'
    }

    return partitions, partition_map

def process_partitions(image_path, partitions, partition_map, image_dir):
    for partition in partitions:
        partition_name = partition.name
        logger.info(f"Processing partition: {partition_name}")
        if partition_name in partition_map:
            partition_image_path = os.path.join(image_dir, partition_map[partition_name])
            logger.info(f"Writing {partition_image_path} to partition {partition_name}")
            write_partition_image(image_path, partition, partition_image_path)
        else:
            logger.info(f"Filling partition {partition_name} with zero")
            fill_partition_with_zero(image_path, partition)

def write_partition_image(image_path, partition, partition_image_path):
    device = parted.getDevice(image_path)
    start_sector = partition.geometry.start
    sector_size = device.sectorSize

    with open(image_path, 'rb+') as disk_file, open(partition_image_path, 'rb') as partition_file:
        disk_file.seek(start_sector * sector_size)
        shutil.copyfileobj(partition_file, disk_file)

    logger.info(f"Successfully wrote {partition_image_path} to partition")

def fill_partition_with_zero(image_path, partition):
    device = parted.getDevice(image_path)
    start_sector = partition.geometry.start
    sector_size = device.sectorSize
    partition_size = partition.getLength() * sector_size

    with open(image_path, 'rb+') as disk_file:
        disk_file.seek(start_sector * sector_size)
        disk_file.write(b'\0' * partition_size)

    logger.info(f"Successfully filled partition with zero")

def is_sparse_image(file_path):
    with open(file_path, 'rb') as f:
        header = f.read(12)
        return header[:4] == b'\x3a\xff\x26\xed'

def convert_sparse_image(sparse_file, raw_file, simg2img_path):
    try:
        subprocess.run([simg2img_path, sparse_file, raw_file], check=True)
        print(f"Successfully converted {sparse_file} to {raw_file}")
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Failed to convert {sparse_file} to {raw_file}: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Create and process disk image.")
    parser.add_argument('--input', required=True, help="Path to the partition description file.")
    parser.add_argument('--output', required=True, help="Path to the output disk image file.")
    parser.add_argument('--image-dir', required=False, default='.', help="Directory containing the .img files.")
    parser.add_argument('--simg2img-path', required=False, default='simg2img', help="Path to the simg2img tool.")
    args = parser.parse_args()

    partition_description_file = args.input
    output_image_path = args.output
    image_dir = args.image_dir
    simg2img_path = args.simg2img_path

    sparse_file = os.path.join(image_dir, 'super.img')
    raw_file = os.path.join(image_dir, 'super_raw.img')

    if not os.path.isfile(sparse_file):
        raise FileNotFoundError(f"No such file: '{sparse_file}'")

    if not is_sparse_image(sparse_file):
        raise ValueError(f"'{sparse_file}' is not an Android sparse image")

    convert_sparse_image(sparse_file, raw_file, simg2img_path)

    with open(partition_description_file, 'r') as f:
        partition_data = json.load(f)

    logger.info("Starting to create disk image")
    create_disk_image(partition_data, output_image_path)
    logger.info("Disk image created successfully")

    logger.info(f"Starting to process disk image: {output_image_path}")
    partitions, partition_map = parse_gpt_image(output_image_path)
    process_partitions(output_image_path, partitions, partition_map, image_dir)
    logger.info("Processing completed")
