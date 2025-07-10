# unified_installation

## 1 Overview

To better manage SSGX versions in practice, SSGX supports a unified installation mode, which installs all external
libraries (except Poco) into a single directory.

## 2 Usage

### 2.1 When installing external dependencies

- New method:

```shell
    ./build_install_external.sh --trusted-install-prefix /opt/safeheron/ssgx --install_libs_together true
```

- Original method:

```shell
    ./build_install_external.sh
```

### 2.2 When compiling with CMake

Modify CMakePresets.json and add the following path to CMAKE_PREFIX_PATH:
"/opt/safeheron/ssgx/__untrusted_dependencies"

### 2.3 When referencing in CMake

Modify CMakePresets.json and add the following path to CMAKE_PREFIX_PATH:
"/opt/safeheron/ssgx/__untrusted_dependencies"

## 3 Notes

- **Avoid Conflicts**: Do not install the same dependencies repeatedly under system paths in the untrusted environment (
  e.g., /usr/local/), as it may lead to library version conflicts.
- **Keep Poco Consistent**: Ensure the Poco library version remains consistent across different image environments.
- **Avoid Duplicate Library Names:** During development, make sure that the **trusted** and **untrusted** environments
  do **not contain libraries with the same name**, to prevent linkage ambiguity and unexpected behavior.

