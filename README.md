# CoconutShack
The goal of this project was to make simpler game development tool that could be used by beginner to intermediate developer. 
A use case for our tool:
A teacher who wants to teach their students loops. They want the game to have sound effects, graphics, and motion control.  

### Prerequisites
1. Visual Studio

### Steps to build
1. Put the cocos2d folder in the root directory of the project (This is just the folder with all of the components of the cocos2d library, Link: 
   Make sure this folder is called cocos2d
2. Put the Resources folder in the root directory of the project (This just contains all of the assets for the game. Link: )
3. Go to proj.win32 and open up CSP.sln in Visual Studio.
4. F7 to build.
### Solutions to Common Build Errors
1. Open up Visual Studio Installer
2. Go to your installed Visual Studio version and click "Modify"
3. Go to the "Individual Components" tab
4. Make sure the following components are installed:
   -Under "SDKs, libraries, and frameworks":
     -The Windows SDK for whatever version of Windows you're developing on (e.g. "Windows 8.1 SDK" for a Windows 8.1 machine)
     -"Windows Universal C Runtime"
   -Under "Compilers, build tools, and runtimes":
     -"Windows Universal CRT SDK"

# Current Functions in the UI
* Checkbox(String, pointer* boolean) return boolean
* float_edit(String, pointer* float) return boolean
* float_edit(String, pointer* int min (optional), int max (optional)) return boolean
* text(format String ...) return;
* button(String) return boolean (clicked)
* text_field(String name, string pointer, boolean multiple lines, ) return boolean(string edit)
* unsigned8_edit(String name, u8 value,(optional) int min, (optional) int max) return boolean
 (u8 value means an unnsigned 8 bit integer)
* colorEdit(name,rbga values pointer) return boolean
# Features we are working on for the UI
- [ ] Dropdown menu
# Features we are working on 
- [ ] Control Schemes
- [ ] Adding duplication functionality
- [ ] Changing placemenet of asset
- [ ] allowing user to select size of asset
- [ ] grid locking
## Authors

* **Shivani Kohli** 

* **Parker Berry** 

* **Hannah** 

* **Eric** 



## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details







