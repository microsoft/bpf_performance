/*
Copyright (c) Microsoft Corporation
SPDX-License-Identifier: MIT

This script checks the performance results of the eBPF for Windows repository
and returns the results that are 2 standard deviations away from the mean.

The script uses the following parameters:
- platform: The platform to filter the results by.
- max_sigma: The number of standard deviations away from the mean to consider a regression.
- look_back: The number of days to look back for results. Format is 'X days'.
- repository: The repository to filter the results by.
*/

\if :{?platform}
    \set platform :platform
\else
    \set platform 'Windows 2019'
\endif

\if :{?max_sigma}
    \set max_sigma :max_sigma
\else
    \set max_sigma '2'
\endif

\if :{?look_back}
    \set look_back :look_back
\else
    \set look_back '30 days'
\endif

\if :{?repository}
    \set repository :repository
\else
    \set repository 'microsoft/ebpf-for-windows'
\endif

WITH samples AS (
    SELECT metric, value, "timestamp",
           ROW_NUMBER() OVER (PARTITION BY metric ORDER BY "timestamp" DESC) AS row_num
    FROM benchmarkresults
    WHERE platform = :'platform'
      AND repository = :'repository'
      AND "timestamp" >= NOW() - INTERVAL :'look_back'
),
stats AS (
    SELECT metric,
           AVG(value) as mean_value,
           STDDEV(value) as stddev_value
    FROM benchmarkresults
    WHERE platform = :'platform'
      AND repository = :'repository'
      AND "timestamp" >= NOW() - INTERVAL :'look_back'
    GROUP BY metric
)
SELECT samples.timestamp, samples.metric, samples.value, stats.mean_value, stats.stddev_value
FROM samples
INNER JOIN stats ON samples.metric = stats.metric
WHERE ABS(samples.value - stats.mean_value) >= :max_sigma * stats.stddev_value AND row_num = 1;