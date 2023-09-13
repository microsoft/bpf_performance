# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT
#!/bin/bash

# Define the path where you want to search for CSV files.
search_path="$1"
# The script will append a number to the end of the prefix if the output file already exists.
output_file_prefix="$2"

# Use find to locate CSV files and loop through them
find "$search_path" -type f -name "*.csv" -print0 | while IFS= read -r -d '' csv_file; do
    # Assume output file does not exist.
    output_file="$output_file_prefix.csv"
    # If output file already exists, append count to filename and try again.
    if [ -f "$output_file" ]; then
        count=2
        while [ -f "$output_file" ]; do
            output_file="$output_file_prefix-$count.csv"
            count=$((count + 1))
        done
    fi
    # Copy the CSV file to the output file.
    cp "$csv_file" "$output_file"
done
