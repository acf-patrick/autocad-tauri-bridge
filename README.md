# AutoCAD and Tauri bridge

Create brdige between AutoCAD and Tauri application, allowing GUI creation using web technologies instead of native framework like Qt.
Get more control on the development process and get the possibility to use Rust as backend service. Of course, one can still use C++.

The frontend application make an API call to the main ObjectARX application (AutoCAD) using VBScript.

## Setup

- Decide where to put the files related to GUI created with Tauri.
  - Sample codes in this repo assume that the GUI repo is located at `D:\GUI` **make sure to set yours**
- Setup the AutoCAD command that the VBScript will use :

> You may have to adapt the following code according to your AutoCAD SDK version.

```C++
void initCmd(AcString sCmdName, GcRxFunctionPtr FunctionAddr) {
	//Declarer le global name
	AcString sGlobalName = "your_command_group" + "_" + sCmdName;

	//Creer le nom
	acedRegCmds->addCommand(
		"your_command_group",
		sGlobalName,
		sCmdName,
		ACRX_CMD_MODAL + ACRX_CMD_USEPICKSET + ACRX_CMD_REDRAW,
		FunctionAddr);
}

...

// Command name MUST be "TAURI"
initCmd("TAURI", cmdCommunicateTauri);
```

- Copy `exec_cmd.vbs` script and the executable built with Tauri to the GUI folder.

## Usage
Create an AutoCAD command wrapper for your new executable.

```C++
void cmdTauriDemo() {
  system("start /B D:\\GUI\\tauri_demo.exe");
}

initCmd("TAURIDEMO", cmdTauriDemo);
```

You can now call **TAURIDEMO** inside AutoCAD and the Tauri executable will be run as separated process.

## API
The system just uses two files for IPC : *IPC.req.json* and *IPC.res.json*
The C++ command and Rust backend will read and write those files during communication.

### Why use Web technologies ?
> Because I had enough fucking around with Qt and any of the so called framework for native interfaces.