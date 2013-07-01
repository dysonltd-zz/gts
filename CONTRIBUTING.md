# How to contribute #

The GTS project welcomes new contributors. Please use this document as a guideline for raising issue tickets and making contributions.

## Getting Started ##

* Submit an issue if one does not already exist for bugs or feature requests.
* Describe the issue including steps to reproduce if it is a bug.
* Fork the repository on GitHub.
* Make the changes.
* Create a Pull Request.
* Add details of this Pull Request to the Issue.
* Sign the Contributors License Agreement.

## Making Changes ##

* Create a branch from where you want to base your work.
    * Typically the development branch.
    * Release branches if you are certain your fix must be on that branch.
    * To quickly create a topic branch based on master; `git branch
    fix/master/my_contribution master` then checkout the new branch with `git
    checkout fix/master/my_contribution`. Please avoid working directly on the
    `master` branch.

* Add unit tests where possible.
* Compile with `GTS_TESTS=ON` and run `gts_tests` the tests to ensure everthing works.

## Submitting Changes ##

* Sign the [Contributor License Agreement](INDIVIDUAL_CLA.txt).
* Push your changes to a new branch in your fork of the repository e.g. `dev-issue44`.
* Submit a Pull Request.
* Update the issue with a link to the Pull Request.

## Reporting Issues ##

Issues will only be accepted if they are bugs or feature requests.

## Branches ##

- `master` is the latest, deployed version.
- `develop` is the official work in progress branch for the next release. 

## Pull Requests ##

The use of the typical GitHub flow is recommended, where contributors fork the GroundTruthSystem repository and submit a Pull Request.

A description of the changes in the Pull Request would be ideal so that we can quickly understand  the changes.

Some Unit Tests have been provided, but any additions with respect to your contributions when creating a Pull Request would be most welcome.

Pull requests will only be considered if the author has completed an [Individual Contributor License Agreement](CLA/IndividualCLA.pdf). If you work for a corporation, your employer must also complete the [Corporate Contributors Licensing Agreement](CLA/CorporateCLA.pdf). This can be attained by emailing EMAIL_ADDRESS.

## Contributors License Agreement ##

Please download, sign, scan and send a copy of the [Individual Contributor License Agreement](CLA/IndividualCLA.pdf) to ADDRESS. You will only need to complete this once.
