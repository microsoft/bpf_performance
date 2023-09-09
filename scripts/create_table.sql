-- Copyright (c) Microsoft Corporation
-- SPDX-License-Identifier: MIT

-- This script creates the table that will hold the benchmark results.
CREATE TABLE [dbo].[BenchmarkResults] (
    [id]         INT           IDENTITY (1, 1) NOT NULL,
    [Timestamp]  DATETIME      NOT NULL,
    [Metric]     NVARCHAR (50) NOT NULL,
    [Value]      INT           NOT NULL,
    [CommitHash] NVARCHAR (50) NOT NULL,
    [Platform]   NVARCHAR (50) NOT NULL,
    CONSTRAINT [PK_BenchmarkResults] PRIMARY KEY CLUSTERED ([id] ASC)
);