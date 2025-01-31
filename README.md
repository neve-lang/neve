# Neve

This repository hosts the implementation of the Neve interpreter virtual 
machine.  PRs are welcome!

## What's Next

Implementation of UTF-16 and UTF-32 strings has been delayed; however, Neve now supports UTF-8 strings.

The next feature on the horizon for the Neve virtual machine will be **named constants**.  
Something like this:

```rb
const
  Some = 10

  # optional type annotation
  Thing Int = 20

  Else = 30
end

const Named
  Some = 40
  Thing = 50
  Else = 60
end

# temporary "print" keyword--this won't compile in future updates
print Some + Thing + Else
print Named.Some + Named.Thing + Named.Else
```

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

And the **neve** binary will be output to `build/neve`.
