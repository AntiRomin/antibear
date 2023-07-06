# Debug in VSCode

- Openocd requied
- VScode requied

## Installing Openocd
Please install the Openocd manually. You can download the openocd at here: https://github.com/xpack-dev-tools/openocd-xpack/releases. Remember to download the "xpack-openocd-version-win32-x64.zip" for windows. I highly recommend you add the openocd to your enviroment path. Otherwise you will nedd to use absolute addresses when writing vscode script.
>/your openocd install path/bin

Enter the following script in your cmd:
>openocd -v

You can see openocd version information

## Installing plug-in for VSCode
You need to install "Cortex-Debug" for VSCode. You can find in Extensions when you type "Cortex-Debug" in search bar.

## Write c_cpp_properties.json for C language support
We should change the intelliSensMode to "windows-gcc-arm" for arm style C language typing. Also we should add GNU ARM Toolchain's c lib to includePath for introducing header file. A simple version is following:

```
{
    "env": {
        "gnuArmIncludePath": [
            "your gnu arm toolchain install path/lib/gcc/arm-none-eabi/10.3.1/include",
            "your gnu arm toolchain install path/lib/gcc/arm-none-eabi/10.3.1/include-fixed"
        ],
        "gnuArmCompilerPath": [
            "your gnu arm toolchain install path/bin/arm-none-eabi-gcc.exe"
        ]
    },
    "configurations": [
        {
            "name": "GNU Arm",
            "intelliSenseMode": "windows-gcc-arm",
            "includePath": [
                "${gnuArmIncludePath}",
                "${workspaceFolder}",
                "/another/path"
            ],
            "defines": [
                "DEBUG"
            ],
            "compilerPath": "${gnuArmCompilerPath}"
        }

    ],
    "version": 4
}
```

## Write tasks.json for VSCode
```
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "clean",
            "type": "shell",
            "detail": "clean the current target",
            "command": "make clean",
            "problemMatcher": "$gcc"
        },
        {
            "label": "clean all",
            "type": "shell",
            "detail": "Clean all target",
            "command": "make clean_all",
            "problemMatcher": "$gcc"
        },
        {
            "label": "make release",
            "type": "shell",
            "detail": "Build the target",
            "command": "make",
            "problemMatcher": "$gcc"
        },
        {
            "label": "make debug",
            "type": "shell",
            "detail": "Build with DEBUG=GDB",
            "command": "make",
            "args": [
                "DEBUG=GDB"
            ],
            "problemMatcher": "$gcc"
        },
        {
            "label": "download with openocd connect",
            "type": "shell",
            "detail": "download elf to mcu",
            "command": "openocd",
            "args": [
                "-f",
                "interface/stlink.cfg",
                "-f",
                "target/stm32h7x.cfg",
                "-c",
                "program ./obj/debug/debug.elf verify reset exit"
            ],
            "problemMatcher": "$gcc"
        },
        {
            "label": "debug with download",
            "type": "shell",
            "dependsOn": [
                "make debug",
                "download with openocd connect"
            ],
            "problemMatcher": "$gcc"
        }
    ]
}
```

## Write launch.json for Debug
```
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug with OpenOCD",
            "type": "cortex-debug",
            "request": "launch",
            "executable": "./obj/debug/debug.elf",
            "cwd": "${workspaceRoot}",
            "servertype": "openocd",
            "configFiles": [
                "interface/stlink.cfg",
                "target/stm32h7x.cfg"
            ],
            "runToEntryPoint": "SystemInit",
            "showDevDebugOutput": "none",

            "preLaunchTask": "debug with download"
        }
    ]
}
```