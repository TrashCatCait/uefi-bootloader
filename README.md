# UEFI bootloader
designed to work as part of my os project is aslo able to be forked and or used for other projects.

# Submodules 
Please before running make. Check you have all the Submodules needed by this project you can do this manually or download them with 
```
    git submodule update --init --remote --recursive
```
only git submodule update --init is technically needed. However recursive will ensure that any Submodules of sub modules will be downloaded and --remote just ensure you get the latest version.

# how to make 
First you need to make gnuefi. So `cd gnuefi dir` then run `make` inside that folder. Then simply return to this repos folder and run the make command.
