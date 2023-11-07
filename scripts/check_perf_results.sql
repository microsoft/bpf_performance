/*
Copyright (c) Microsoft Corporation
SPDX-License-Identifier: MIT
*/
WITH samples AS (
    SELECT metric, value, "timestamp",
           ROW_NUMBER() OVER (PARTITION BY metric ORDER BY "timestamp" DESC) AS row_num
    FROM benchmarkresults
    WHERE platform = 'Windows 2019'
      AND repository = 'microsoft/ebpf-for-windows'
      AND "timestamp" >= NOW() - INTERVAL '30 days'
),
stats AS (
    SELECT metric,
           AVG(value) as mean_value,
           STDDEV(value) as stddev_value
    FROM benchmarkresults
    WHERE platform = 'Windows 2019'
      AND repository = 'microsoft/ebpf-for-windows'
      AND "timestamp" >= NOW() - INTERVAL '30 days'
    GROUP BY metric
)
SELECT samples.timestamp, samples.metric, samples.value, stats.mean_value, stats.stddev_value
FROM samples
INNER JOIN stats ON samples.metric = stats.metric
WHERE ABS(samples.value - stats.mean_value) >= 2 * stats.stddev_value AND row_num = 1;