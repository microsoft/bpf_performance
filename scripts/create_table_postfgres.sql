-- Copyright (c) Microsoft Corporation
-- SPDX-License-Identifier: MIT

CREATE TABLE BenchmarkResults (
    Timestamp TIMESTAMPTZ,
    Metric VARCHAR(255),
    Value NUMERIC,
    CommitHash VARCHAR(255),
    Platform VARCHAR(255),
    Repository VARCHAR(255)
);