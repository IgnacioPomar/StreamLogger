# CI and releases

StreamLogger uses Forgejo Actions. The workflows are in `.forgejo/workflows/`:

| Workflow      | Trigger                           | Result                                                       |
|---------------|-----------------------------------|--------------------------------------------------------------|
| `ci.yml`      | Any push to a branch, any PR      | Unit tests and Conan package check. Nothing is published     |
| `release.yml` | Push of a tag `vX.Y.Z`            | Same checks, then the recipe is uploaded to the Conan registry |

Both run in a `node:22-trixie` container (Debian 13, gcc 14, the same as the devcontainer; Node is needed by the
actions), on a runner with the `docker` label.

## A normal push (`ci.yml`)

Every push to any branch, and every pull request, runs these steps:

1. **Checkout** of the commit.
2. **Toolchain**: `build-essential`, `cmake` and Conan 2 (in a Python venv).
3. **Conan cache**: `~/.conan2/p` is restored between runs (Catch2 and Trompeloeil are not built again). The key
   is the hash of `conanfile.py`.
4. **Conan profile**: `conan profile detect`.
5. **Unit tests**: the same commands as in local:
   ```bash
   conan install . --build=missing -s compiler.cppstd=20 -o build_tests=True
   cmake --preset conan-release
   cmake --build --preset conan-release
   ctest --preset conan-release --output-on-failure
   ```
   The tester (`StreamLoggerTester`) links the library (shared by default), and also uses its internal classes.
6. **Conan package**: `conan create . --build=missing -s compiler.cppstd=20` builds the package with the default
   options (shared library) and runs `test_package/`: a small program that finds, links and loads
   `libStreamLogger.so`, as a consumer would.

A failure in any step marks the commit as failed. **Nothing is uploaded**: a normal push never publishes anything,
in any branch (including `main`).

## Versioning

### Where the version is

The version is written in **one place only**:

```cmake
# CMakeLists.txt
project(StreamLogger VERSION 0.1.0 LANGUAGES CXX)
```

Everything else is derived from it:

| What                    | How                                                  | Example (0.1.0)              |
|-------------------------|------------------------------------------------------|------------------------------|
| Conan package version   | `set_version()` in `conanfile.py` reads CMakeLists   | `streamlogger/0.1.0`         |
| Shared library file     | CMake `VERSION`                                      | `libStreamLogger.so.0.1.0`   |
| SONAME (ABI version)    | CMake `SOVERSION` (see below)                        | `libStreamLogger.so.0.1`     |
| Git tag                 | Written by hand, checked by the release              | `v0.1.0`                     |

`conan create --version X` fails if `X` is not the CMakeLists version, so the tag, the package and the library can
not get out of sync.

### Numbering: `MAJOR.MINOR.PATCH`

The version follows [Semantic Versioning](https://semver.org), applied to the **public API and the ABI** of the
shared library: what the installed headers (`include/StreamLogger/`) declare, and the layout of their classes.
The internal classes (`src/`) are not installed, so the consumers can not depend on them.

**From 1.0.0:**

| Increase  | When                                                                          | SONAME           |
|-----------|-------------------------------------------------------------------------------|------------------|
| `PATCH`   | Bug fixes. API and ABI unchanged                                              | Same             |
| `MINOR`   | New features, compatible: new functions, new `Config` options                 | Same             |
| `MAJOR`   | Incompatible changes: removed or changed functions, changed exported classes  | `.so.MAJOR` changes |

**Before 1.0.0 (now):** the API is not stable yet, so the minor version plays the role of the major one:

| Increase  | When                                                                          | SONAME                |
|-----------|-------------------------------------------------------------------------------|-----------------------|
| `PATCH`   | Bug fixes and compatible additions. The ABI is unchanged                      | Same (`.so.0.1`)      |
| `MINOR`   | Anything that breaks the API or the ABI                                       | Changes (`.so.0.2`)   |

That is why the SONAME is `libStreamLogger.so.0.MINOR` before 1.0 and `libStreamLogger.so.MAJOR` after it: two
versions with the same SONAME must be interchangeable without recompiling the program.

Changes that break the ABI even if the source still compiles: adding or reordering members of an exported class,
adding virtual functions, changing default arguments or inline functions in the public headers.

### Consumers: version ranges

A consumer can follow the compatible versions with a range:

```python
self.requires("streamlogger/[~0.1]")    # Before 1.0: >=0.1.0 <0.2.0
self.requires("streamlogger/[^1.2]")    # From 1.0: >=1.2.0 <2.0.0
```

## Creating a release (`release.yml`)

### Steps

1. Choose the new version with the rules above, and change it in `CMakeLists.txt`:
   ```cmake
   project(StreamLogger VERSION 0.2.0 LANGUAGES CXX)
   ```
2. Commit it in `main` (usually through a PR) and wait for the CI to pass.
3. Create the tag on that commit and push it:
   ```bash
   git tag v0.2.0
   git push origin v0.2.0
   ```

The tag must be exactly `v` + the CMakeLists version. A tag that does not match makes the release fail, and nothing
is uploaded.

### What the release workflow does

1. Checkout, toolchain and Conan profile, as in `ci.yml`.
2. Adds the Conan remote `forgejo`: `<forgejo url>/api/packages/<repository owner>/conan`
   (`.../api/packages/Commons/conan`).
3. **Validation**: `conan create --version <tag without v>` twice, for the shared (default) and the static
   (`shared=False`) library, each one with its `test_package`.
4. **Upload**: `conan upload streamlogger/<version> -r forgejo --only-recipe`.

Only the **recipe and its sources** (`exports_sources`: `CMakeLists.txt`, `include/`, `src/`, `test/`, `examples/`)
are uploaded, **never binaries**: a C++ binary is only valid for the same compiler, standard library and flags, so
each consumer builds StreamLogger with its own toolchain. The recipe has `build_policy = "missing"`, so the
consumer does not need to pass `--build`: Conan builds it the first time and keeps it in its cache.

Forgejo also offers the source archive (`.tar.gz` / `.zip`) of every tag in the repository page.

### Required configuration in Forgejo

- Actions enabled in the repository, and a runner with the `docker` label.
- Repository (or organization) secrets:
  - `CONAN_USER`: the Forgejo user that uploads the package.
  - `CONAN_TOKEN`: an access token of that user with the `write:package` scope.

  The workflow passes them to Conan as `CONAN_LOGIN_USERNAME_FORGEJO` / `CONAN_PASSWORD_FORGEJO`.

### Mistakes

- **The release failed**: fix the cause, delete the tag (`git push --delete origin vX.Y.Z` and `git tag -d vX.Y.Z`)
  and tag again. Nothing was uploaded.
- **A published version is wrong**: do not reuse the version. Publish a new `PATCH` (or `MINOR`) version. Uploading
  the same version again creates a new *recipe revision* in Conan: the consumers that already have the previous one
  in their cache keep using it (unless they run with `--update`), so two builds of "the same version" may differ.

## Using the published package

```bash
conan remote add forgejo https://<forgejo>/api/packages/Commons/conan
conan remote login forgejo <user> -p <token>    # Token with the read:package scope
```

```python
def requirements(self):
    self.requires("streamlogger/[~0.1]")
```

The library is shared by default; `-o "streamlogger/*:shared=False"` gives the static one. With the shared library
the program needs `libStreamLogger.so` at run time: run it with the Conan run environment
(`source conanrun.sh`, or the `VirtualRunEnv` generator), or deploy the library with the program.
