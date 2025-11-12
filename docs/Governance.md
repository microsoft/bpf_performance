# Project Governance

## Overview

The BPF Performance project follows a collaborative governance model focused on performance testing and benchmarking of BPF programs and maps.

## Roles and Responsibilities

### Contributors

Anyone can contribute to the BPF Performance project by:
- Reporting issues
- Submitting pull requests
- Providing feedback on existing issues and pull requests
- Participating in discussions

### Maintainers

Project maintainers have write access to the repository and are responsible for:
- Reviewing and merging pull requests
- Triaging issues and assigning labels
- Ensuring code quality and test coverage
- Managing releases

Current maintainers are listed in the [CODEOWNERS](.github/CODEOWNERS) file.

## Pull Request Review Process

All pull requests must be reviewed before merging. The review process includes:

1. **Technical Review**: Ensure the code is correct, follows coding standards, and doesn't introduce regressions.

2. **Performance Impact**: For changes that might affect performance, verify that:
   - Existing benchmarks continue to pass
   - New benchmarks are added for new functionality
   - Performance characteristics are documented

3. **Testing Requirements**: Verify that:
   - Appropriate tests are included
   - All existing tests continue to pass
   - Test coverage is maintained or improved

4. **Documentation**: Ensure that:
   - User-facing changes are documented
   - Code comments are clear and helpful
   - Examples are provided where appropriate

## Approval Requirements

- Pull requests require at least **one approval** from a maintainer
- Pull requests that significantly change performance characteristics require **two approvals**
- The author of a pull request cannot approve their own changes

## Issue Management

### Issue Labels

We use the following labels to categorize issues:

- `bug`: Something isn't working correctly
- `enhancement`: New feature or improvement
- `performance`: Performance-related issue or improvement
- `documentation`: Documentation improvements
- `testing`: Test-related changes
- `help wanted`: Good for contributors looking to help

### Priority Levels

- **High Priority**: Critical bugs, security issues, or blocking problems
- **Medium Priority**: Important features or non-critical bugs
- **Low Priority**: Nice-to-have improvements or minor issues

## Release Process

Releases are managed by maintainers and follow semantic versioning:

- **Major versions** (X.0.0): Breaking changes or significant new features
- **Minor versions** (X.Y.0): New features that are backward compatible
- **Patch versions** (X.Y.Z): Bug fixes and minor improvements

## Communication

- Use GitHub issues for bug reports and feature requests
- Use GitHub discussions for general questions and community discussions
- Follow the [Code of Conduct](CODE_OF_CONDUCT.md) in all interactions

## Decision Making

- Most decisions are made through discussion on GitHub issues or pull requests
- For significant changes, maintainers may request broader community input
- Maintainers aim for consensus but may make decisions when needed to keep the project moving forward