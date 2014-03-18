# How to contribute

The GTS project welcomes new contributors. Please use this document as a guideline for raising issue tickets and making contributions.

## Getting Started

* Create a Github account.
* If you find a bug or want to request a feature, submit an [issue](https://github.com/dysonltd/gts/issues/new)
* Describe the issue including steps to reproduce if it is a bug.
* If you wish to make changes: [Fork](https://github.com/dysonltd/gts/fork) the repository to your account.
* Make the changes (see below)
* Create a [Pull Request](https://github.com/dysonltd/gts/compare/)
* Add details of this Pull Request to the Issue.
* Sign and email the Contributors License Agreement (see below).

## Making Changes

* Create a branch from where you want to base your work.
    * Typically the development branch.
    * To quickly create a topic branch based on develop; `git branch dev-issue44 master` then checkout the new branch with `git checkout dev-issue44`. 
    Please avoid working directly on the`master` branch.

* Add unit tests where possible.
* Compile with `GTS_TESTS=ON` and run `gts_tests` the tests to ensure everthing works.

## Submitting Changes

* Sign the Contributor License Agreement (see below for options) and email a PDF copy to akram.hussein@dyson.com
* Push your changes to a new branch in your fork of the repository e.g. `dev-issue44`.
* Submit a [Pull Request](https://github.com/dysonltd/gts/compare/)
* Update the issue with a link to the Pull Request.

These changes will then be reviewed and if deemed appropriate, committed to the `develop` branch and then eventually to `master`.

## Reporting Issues

Issues will only be accepted if they are bugs or feature requests.

## Branches 

- `master` is the latest, deployed version.
- `develop` is the official work in progress branch for the next release. 

## Pull Requests 

The use of the typical GitHub flow is recommended, where contributors fork the GroundTruthSystem repository and submit a Pull Request.

A description of the changes in the Pull Request would be ideal so that we can quickly understand  the changes.

Some Unit Tests have been provided, but any additions with respect to your contributions when creating a Pull Request would be most welcome.

Pull requests will only be considered if the author has completed an [Individual Contributor License Agreement](cla/IndividualCLA.pdf). If you work for a corporation, your employer must also complete the [Corporate Contributors Licensing Agreement](cla/CorporateCLA.pdf).

## Contributors License Agreement

Please download, sign, scan and send a copy of the [Individual Contributor License Agreement](cla/IndividualCLA.pdf) to akram.hussein@dyson.com. You will only need to complete this once.
