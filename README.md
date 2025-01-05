# Neve

This repository hosts the implementation of the Neve interpreter virtual 
machine.  PRs are welcome!

## What's Next

The next feature in the horizon for the Neve virtual machine is 
**Tables (hash tables).**

However, any PR is welcome, whether it is related to the implementation of 
the next feature or something completely different that benefits the project.

## How to build

### Requirements

In order to build the Neve VM, you must have installed:

```
make
cmake
clang-tidy
```

### Building the VM

Once you have the build dependencies installed, you can simply run the 
commands below:

```
mkdir build && cd build
cmake ..
cd .. && ./build.sh
```

And the **neve** binary will be outputted to `build/neve`.
