# Testing Guide

This document describes the testing approach for the BPF Performance project.

## Overview

The BPF Performance project focuses on benchmarking and performance testing of BPF programs and maps. Our testing strategy ensures that:

- Performance benchmarks are reliable and consistent
- New features don't introduce performance regressions
- The codebase remains maintainable and correct

## Test Categories

### Performance Tests

Performance tests are the core of this project. They measure the execution time and resource usage of various BPF operations.

**Current test categories include:**
- **Map Operations**: Testing different map types (hash, array, LRU, etc.)
- **Program Execution**: Benchmarking BPF program execution performance
- **Helper Functions**: Testing the performance of BPF helper functions
- **Tail Calls**: Measuring tail call performance and overhead

**When adding new performance tests:**
- Use consistent measurement methodologies
- Run tests multiple times and report statistical measures (mean, median, std dev)
- Document expected performance characteristics
- Include both small and large dataset tests where appropriate

### Unit Tests

Unit tests verify the correctness of individual components:

- Test harness functionality
- Data processing and analysis code
- Configuration parsing
- Result formatting and output

### Integration Tests

Integration tests verify that different components work together:

- End-to-end test execution
- Result collection and aggregation
- Cross-platform compatibility
- Different BPF runtime environments

### Regression Tests

Regression tests ensure that performance doesn't degrade over time:

- Automated comparison with baseline performance metrics
- Detection of significant performance changes
- Historical performance tracking

## Test Structure

### Test Organization

Tests are organized by functionality:

```
tests/
├── performance/          # Performance benchmark tests
│   ├── maps/            # Map performance tests
│   ├── programs/        # Program execution tests
│   └── helpers/         # Helper function tests
├── unit/                # Unit tests
├── integration/         # Integration tests
└── regression/          # Regression tests
```

### Test Data

- Use representative test data that reflects real-world usage
- Include both synthetic and real-world datasets
- Ensure test data is reproducible and deterministic
- Document data characteristics and expected performance

## Running Tests

### Local Development

To run all tests locally:

```bash
# Build the project
cmake --build build

# Run performance tests
./build/bin/runner --test-suite performance

# Run specific test categories
./build/bin/runner --test-suite maps
./build/bin/runner --test-suite programs
```

### Continuous Integration

All tests run automatically on:
- Pull request creation and updates
- Commits to the main branch
- Scheduled daily runs for regression testing

### Performance Baselines

- Baseline performance metrics are maintained for each supported platform
- Significant deviations from baselines trigger alerts
- Baselines are updated when intentional performance changes are made

## Adding New Tests

### Performance Test Guidelines

When adding new performance tests:

1. **Define clear objectives**: What performance aspect are you measuring?
2. **Use appropriate metrics**: Choose metrics that matter to users
3. **Ensure reproducibility**: Tests should produce consistent results
4. **Document expectations**: Include expected performance ranges
5. **Consider platform differences**: Account for hardware and OS variations

### Test Implementation

```c
// Example performance test structure
typedef struct test_config {
    const char* name;
    size_t iterations;
    size_t data_size;
    // Test-specific configuration
} test_config_t;

int run_performance_test(const test_config_t* config) {
    // Setup test environment

    // Warm-up runs

    // Measured runs
    for (size_t i = 0; i < config->iterations; i++) {
        start_timer();
        // Execute test operation
        end_timer();
    }

    // Calculate and report statistics

    // Cleanup
}
```

### Test Documentation

Each test should include:

- Clear description of what is being tested
- Expected performance characteristics
- Hardware/software requirements
- Known limitations or variations

## Performance Analysis

### Metrics Collection

Standard metrics collected for each test:
- **Execution time**: Mean, median, standard deviation
- **Throughput**: Operations per second
- **Resource usage**: CPU, memory utilization
- **Latency**: Response time distribution

### Statistical Analysis

- Use appropriate statistical methods for performance comparison
- Account for measurement noise and system variations
- Report confidence intervals where applicable
- Identify and handle outliers appropriately

### Reporting

Test results are reported in standardized formats:
- CSV files for raw data
- JSON for structured results
- Grafana dashboards for visualization
- Summary reports for human consumption

## Best Practices

### Test Reliability

- Run tests in isolated environments when possible
- Use consistent test harnesses and measurement tools
- Account for system noise and background processes
- Validate test setup before measurement

### Performance Regression Detection

- Establish clear thresholds for performance changes
- Use statistical tests to identify significant regressions
- Automate regression detection in CI/CD pipelines
- Provide clear feedback when regressions are detected

### Cross-Platform Testing

- Test on multiple operating systems and architectures
- Account for platform-specific performance characteristics
- Document platform differences and limitations
- Use platform-appropriate optimization techniques

## Troubleshooting

### Common Issues

- **Inconsistent results**: Check for background processes, thermal throttling
- **Test failures**: Verify test environment setup and dependencies
- **Performance regressions**: Compare with known baselines, check for configuration changes

### Debugging Performance Issues

- Use profiling tools to identify bottlenecks
- Compare results across different environments
- Check for resource contention or system limitations
- Validate test methodology and measurement accuracy