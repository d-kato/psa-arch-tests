
# PSA Firmware Framework Architecture Test Suite

## PSA Firmware Framework

*PSA Firmware Framework* (PSA-FF) defines a standard interface and framework to isolate trusted functionality within constrained IoT devices.

The framework provides:
- Architecture that describes isolated runtime environments (partitions) for trusted and untrusted firmware.
- A standard model for describing the functionality and resources in each partition.
- A Secure IPC interface to request services from other partitions.
- A model that describes how the partitions can interact with one another, as well as the hardware and the firmware framework implementation itself.
- A standard interface for the PSA RoT services such as PSA RoT lifecycle service.

This specification enables the development of Secure firmware functionality which can be reused on different devices that use any conforming implementation of the Firmware Framework. For more information, download the [PSA-FF Specification](https://pages.arm.com/psa-resources-ff.html?_ga=2.97388575.1220230133.1540547473-1540784585.1540547382).

### Architecture test suite

The architecture test suite is a set of examples of the invariant behaviors that are specified by the PSA-FF specification. Use this suite to verify whether these behaviors are implemented correctly in your system. This suite contains self-checking and portable C-based tests with directed stimulus. The tests are available as open source. The tests and the corresponding abstraction layers are available with an Apache v2.0 license allowing for external contribution.

This test suite is not a substitute for design verification. To review the test logs, Arm licensees can contact Arm directly through their partner managers.

For more information on architecture test suite specification, refer to the [Validation Methodology](../docs/Arm_PSA_APIs_Arch_Test_Validation_Methodology.pdf) document.

## Tests scenarios

The mapping of the rules in the specification to the test cases and the steps followed in the tests are mentioned in the [Scenario Document](../docs/) present in the **docs/** folder.


## Getting started

Follow the instructions in the subsequent sections to get a copy of the source code on your local machine and build the tests. Make sure you have all required software installed as explained in the [Software Requirements](../docs/sw_requirements.md).

### Porting steps

Refer to the [PSA-FF Test Suite Porting Guide](../docs/porting_guide_ff.md) document for porting steps.

### Build steps

To build the test suite for your target platform, perform the following steps.

1. Execute `cd api-tests`.

2. Using your Secure partition build tool, parse the following test suite partition manifest files and generate manifest output files. The manifest parsing tool must be compliant with the manifest rules defined in the PSA FF specification.<br />
   <br />The test suite manifests to be parsed are:<br />
   - **platform/targets/<platform_name>/manifests/common/driver_partition_psa.json**
   - **platform/targets/<platform_name>/manifests/ipc/client_partition_psa.json**
   - **platform/targets/<platform_name>/manifests/ipc/server_partition_psa.json**

3. Compile the tests as shown below. <br />
```
    cd api-tests
    mkdir <build_dir>
    cd <build_dir>
    cmake ../ -G"<generator-name> -DTARGET=<platform_name> -DCPU_ARCH=<cpu_architecture_version> -DSUITE=<suite_name> -DPSA_INCLUDE_PATHS="<include_path1>;<include_path2>;...;<include_pathn>"
    cmake --build .
```
<br />  where:

-   <generator-name> "Unix Makefiles" to generate Makefiles for Linux and Cygwin <br />
                     "MinGW Makefiles" to generate Makefiles for cmd.exe on Windows  <br />
-   <platform_name> is the same as the name of the target specific directory created in the **platform/targets/** directory.  <br />
-   <cpu_architecture_version> is the Arm Architecture version name for which the tests should be compiled. For example, Armv7M, Armv8M-Baseline and Armv8M-Mainline Architecture.  <br />
-   <suite_name> is the suite name which is the same as the suite name available in **ff/** directory. <br >
-   <include_path1>;<include_path2>;...;<include_pathn> is an additional directory to be included into the compiler search path. To compile IPC tests, the include path must point to the path where **psa/client.h**, **psa/service.h**,  **psa/lifecycle.h** and test partition manifest output files(**psa_manifest/sid.h**, **psa_manifest/pid.h** and **psa_manifest/<manifestfilename>.h**) are located in your build system.<br />

To compile IPC tests for **tgt_ff_tfm_an521** platform, execute the following commands:
```
    cd api-tests
    mkdir BUILD
    cd  BUILD
    cmake ../ -G"Unix Makefiles" -DTARGET=tgt_ff_tfm_an521 -DCPU_ARCH=armv8m_ml -DSUITE=IPC -DPSA_INCLUDE_PATHS="<include_path1>;<include_path2>;...;<include_pathn>"
    cmake --build .
```
**Note**: The default compilation flow includes the functional API tests to build the test suite. It does not include panic tests that check for the API's PROGRAMMER ERROR conditions as defined in the PSA-FF specification. You can include the panic tests for building the test suite just by passing **-DINCLUDE_PANIC_TESTS=1** to CMake.

### Build output
The test suite build generates the following binaries:<br />

NSPE libraries:<br />
1. **<build_dir>/val/val_nspe.a**
2. **<build_dir>/platform/pal_nspe.a**
3. **<build_dir>/ff/<suite_name>/test_combine.a**

SPE libraries explicitly for IPC test suite:<br />
1. **<build_dir>/partition/driver_partition.a**
2. **<build_dir>/partition/client_partition.a**
3. **<build_dir>/partition/server_partition.a**

### Integrating the libraries into your target platform

1. Integrate the test partition (SPE archives) with your software stack containing SPM so that the partition code gets access to PSA-defined client and Secure partition APIs. This forms an SPE binary.
2. Integrate **val_nspe.a**, **pal_nspe.a** and **test_combine.a** libraries with your Non-secure OS so that these libraries get access to PSA client APIs. This forms an NSPE binary.
3. Load NSPE binary into Non-secure memory.
4. Load SPE binary into Secure memory.

## Test suite execution
The following steps describe the execution flow before the test execution: <br />

1. The target platform must load the above binaries into appropriate memory. <br />
2. The *System Under Test* (SUT) boots to an environment that initializes the SPM and the test suite partitions are ready to accept requests. <br />
3. On the Non-secure side, the SUT boot software gives control to the test suite entry point **void val_entry(void);** as an application entry point. <br />
4. The tests are executed sequentially in a loop in the test_dispatcher function. <br />

For details on test suite integration, refer to the **Integrating the test suite with the SUT** section of [Validation Methodology](../docs/Arm_PSA_APIs_Arch_Test_Validation_Methodology.pdf).

## License

Arm PSA test suite is distributed under Apache v2.0 License.


## Feedback, contributions, and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, send an email to support-psa-arch-tests@arm.com with details.
 - Arm licensees can contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests.

--------------

*Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.*
