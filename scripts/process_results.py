# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT

# This script processes the results of the benchmarking runs and generates
# a SQL script that can be used to update the results in the database.

import argparse
import csv
import datetime
import os
import re
import sys
import time

from collections import defaultdict
from pathlib import Path

# The following are the names of the columns in the CSV file.
# The first column is the name of the timestamp column.
TIMESTAMP_COLUMN_NAME = "Timestamp"
# The third column is the name of the metric.
METRIC_COLUMN_NAME = "Test"
# The fourth column is the name of the value.
VALUE_COLUMN_NAME = "Average Duration (ns)"

# The following are the names of the columns in the SQL script.
# The first column is the timestamp column.
TIMESTAMP_SQL_COLUMN_NAME = "Timestamp"
# The second column is the metric column.
METRIC_SQL_COLUMN_NAME = "Metric"
# The third column is the value column.
VALUE_SQL_COLUMN_NAME = "Value"
# The fourth column is the commit hash column.
COMMIT_HASH_SQL_COLUMN_NAME = "CommitHash"
# The fifth column is the platform column.
PLATFORM_SQL_COLUMN_NAME = "Platform"
# The sixth column is the repository column.
REPOSITORY_SQL_COLUMN_NAME = "Repository"

# Parse all CSV files in the given directory and return a dictionary.
# The keys of the dictionary are the names of the CSV files.
# The values of the dictionary are the contents of the CSV files.

def parse_csv_files(csv_directory):
    csv_files = {}
    for csv_file in csv_directory.glob("*.csv"):
        csv_files[csv_file.resolve()] = parse_csv_file(csv_file)
    return csv_files

# Parse the given CSV file and return a list of dictionaries.
# Each dictionary represents a row in the CSV file.
# The keys of the dictionary are the names of the columns.
# The values of the dictionary are the values of the columns.

def parse_csv_file(csv_file):
    csv_rows = []
    with open(csv_file, "r") as csv_file_handle:
        csv_reader = csv.DictReader(csv_file_handle)
        for csv_row in csv_reader:
            csv_rows.append(csv_row)
    return csv_rows

# Convert the given CSV file to a SQL script and write it to the given file.
# The CSV file is assumed to be a dictionary.
# The keys of the dictionary are the names of the columns.
# The values of the dictionary are the values of the columns.

def convert_csv_file_to_sql_script(csv_file, sql_script_file, commit_id, platform, repository):
    csv_rows = parse_csv_file(csv_file)
    sql_script_file.write("INSERT INTO BenchmarkResults (")
    sql_script_file.write(f"{TIMESTAMP_SQL_COLUMN_NAME}, ")
    sql_script_file.write(f"{METRIC_SQL_COLUMN_NAME}, ")
    sql_script_file.write(f"{VALUE_SQL_COLUMN_NAME}, ")
    sql_script_file.write(f"{COMMIT_HASH_SQL_COLUMN_NAME}, ")
    sql_script_file.write(f"{PLATFORM_SQL_COLUMN_NAME}, ")
    sql_script_file.write(f"{REPOSITORY_SQL_COLUMN_NAME}")
    sql_script_file.write(")\n")
    sql_script_file.write("VALUES\n")
    for csv_row in csv_rows:
        # Skip rows that do not have a metric.
        if csv_row[METRIC_COLUMN_NAME] == None or csv_row[METRIC_COLUMN_NAME] == "":
            continue
        sql_script_file.write("(")
        sql_script_file.write(f"'{csv_row[TIMESTAMP_COLUMN_NAME]}', ")
        sql_script_file.write(f"'{csv_row[METRIC_COLUMN_NAME]}', ")
        sql_script_file.write(f"{csv_row[VALUE_COLUMN_NAME]}, ")
        sql_script_file.write(f"'{commit_id}', ")
        sql_script_file.write(f"'{platform}',")
        sql_script_file.write(f"'{repository}'")
        # Write a comma if this is not the last row.
        if csv_row != csv_rows[-1]:
            sql_script_file.write("),\n")
        else:
            sql_script_file.write(")")
    sql_script_file.write(";\n")

# Convert the given CSV files to a SQL script and write it to the given file.
# The CSV files are assumed to be a dictionary.
# The keys of the dictionary are the names of the CSV files.
# The values of the dictionary are the contents of the CSV files.

def convert_csv_files_to_sql_script(csv_files, sql_script_file, commit_id, platform, repository):
    for csv_file_name, csv_file in csv_files.items():
        convert_csv_file_to_sql_script(csv_file_name, sql_script_file, commit_id, platform, repository)

# Main entry point.

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv-directory", type=Path, required=True)
    parser.add_argument("--sql-script-file", type=Path, required=True)
    parser.add_argument("--commit_id", type=str, required=True)
    parser.add_argument("--platform", type=str, required=True)
    parser.add_argument("--repository", type=str, required=True)
    args = parser.parse_args()

    csv_files = parse_csv_files(args.csv_directory)
    with open(args.sql_script_file, "w") as sql_script_file:
        convert_csv_files_to_sql_script(csv_files, sql_script_file, args.commit_id, args.platform, args.repository)

if __name__ == "__main__":
    main()