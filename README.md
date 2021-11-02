# UEFI bootloader
This UEFI boot-loader is designed to work as part of my OS-dev project but it's in a separate codebase for several reasons. One I find it easier to work on this project this way to compartmentalize each piece and have them be independent of each other, leading in to point two making it more easy to apply or fork this code else where in my other projects and I feel it leads to much nicer layout of the projects than if I had just made them all in one repo and makes maintaining and modifying parts independently of each other easier.

___

# How To Build:
Below are the steps and tools required to build this project.

### Submodules:
So firstly you need to download and make the gnu-efi-code submodule and also make the edk2/OvmfPkg submodule if you are testing on QEMU virtual machine. If you are testing on a different setup you may skip this and do the steps needed for your test setup.
```
    git submodule init 
    git submodule update
```

### Compiling Code:
So to actually build this project it should just be as simple as running `make`. However if you aren't on system with make as an available application. I'd suggest just reading the make file and typing out the commands manually or emulating them in another build system.

### Tools Used:
Below are the tools and version numbers I use for building. before filling out a issue due to an error building I would suggest trying to assemble with the below versions. As it's rare but sometimes differences in tool versions can cause issues. Especially when there is a large difference in version.

- Make (Version: 4.3)
- gcc (Version: 10.2.1)
- ld (Version: 2.35.1)
- objcopy (Version: 2.35.1)

___

# Issues:

### Building Issues:
If issues are encountered during building make sure you have the tools installed. If you are using your own tools make sure you've set up your build tools correctly. Otherwise file an issue with the output of your makefile commands or your build commands as this could be useful for me when debugging


### Other/Runtime Issues:
So if you encounter a problem during running and you suspect it's a bug or mistake please file a detailed report. Describing the bug and anything you did to cause it and the type of media you booted off of and any other details that are relevant for example a display/graphics issue. please include details about you display/graphics setup.

### Known Issues:
- On some B450 motherboards GOP doesn't seem to be set correctly and it seems to always revert to 1024x768 or just report as such(not sure yet)

---

# Contributing: 

Help with the project and code contributions are always welcome. Though I do have a few requests for anyone that wants to do this. Please test you code before submitting a pull request or issue as I will be testing it as well to make sure it merges well and doesn't break anything less obvious. So not 100% needed but potentially could save me time.

If your code does have issues/isn't bug free and you don't know how to fix it. Feel free to still submit a request others including myself can potentially help solve the bug.

For small changes pull requests are more than welcome but for larger changes to the code base I would prefer for you to file an issue to being with [MERGE REQUEST] in the title with a detailed description of what you code is. Why you think it should be changed and what exactly the changed code does. E.G. "[MERGE REQUEST] Code update to fix reading disk on certain BIOS"

___

# FAQ
Currently empty but any questions I find myself answering a lot will be added as time goes on.

___

# Branches

- `Master` this branch is what I consider to be the up to date stable code and a good effort to make it bug free and working will always be taken. Along with giving me a clean empty branch to fall back on if I make a huge mistake on other branches.
- `Unstable` like the name suggest this branch is where I push the most recent and unstable/untested code. There is less guarantee about this branch actually working but of course an effort will still be made to ensure it "works". But this branch will also reflect what I'm currently working on.
