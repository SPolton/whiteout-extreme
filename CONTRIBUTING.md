# Contributing

### Development environment

The main development environment is Windows 10/11.
The prefered editor is Visual Studio 2022.

### Coding standards

Please refer to the [Coding standards](CODING_STANDARDS.md).

### Clone the project

- HTTPS: `git clone https://github.com/SPolton/racing-game.git`
- SSH: `git clone git@github.com:SPolton/racing-game.git`

### Check out a branch

Before working on a change, pull the latest changes and check out a new branch:
```
git pull origin
git checkout -b name-of-branch
```

### Commits

It is best to commit often with one action per commit: either fix a bug, refactor code, add a new feature, etc.
Try not to combine multiple of these activities in a single commit.

```
git add <files>
git commit -m "fix: a descriptive commit message"
```

Write a Good Commit Message:
* When fixing a bug, explain what the bug was and how it was fixed.
* When implementing a feature, explain the feature at high-level, referring to classes or methods if applicable.
* When improving performance, explain what caused the bad performance, and how it was improved.
* And so on...

It is recommended to format your commit message according to
[Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/)
and the
[Commits Cheatsheet](https://gist.github.com/qoomon/5dfcdf8eec66a051ecd85625518cfd13)

### Push to remote

Push your changes often to reduce merge conflicts:
```
git fetch origin
git push origin name-of-branch
```

### Create a Pull Request

When your changes are ready for review, create a Pull Request (PR) on GitHub.

* Open PRs often to integrate changes early.
* Briefly describe all the changes in the PR description.
