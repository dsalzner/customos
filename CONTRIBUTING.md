## Fork, then clone

```
git clone git@github.com:<your-username>/customos.git
```

## Dependencies

Make sure you have installed

* QEmu

The rest is relatively self contained and was developed on a standard Ubuntu 20.x machine.

## Build Compiler

```
cd Pt1-CrossCompiler/
./gccbuild.sh
```

## Build

Each part either has a script

```
./build-and-boot.sh
```

or it's own Makefile. With these Makefiles it's often best to clean first.

```
make clean && make
```
