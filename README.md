# MultiplayerSample Project - Project Spectra Private Preview 

## Confidentiality; Pre-Release Access  

Welcome to the Project Spectra Private Preview.  This is a confidential pre-release project; your use is subject to the nondisclosure agreement between you (or your organization) and Amazon.  Do not disclose the existence of this project, your participation in it, or any of the  materials provided, to any unauthorized third party.  To request access for a third party, please contact [Royal O'Brien, obriroya@amazon.com](mailto:obriroya@amazon.com).

## Download and Install

This repository uses Git LFS for storing large binary files.  You will need to create a Github personal access token to authenticate with the LFS service.


### Create a Git Personal Access Token

You will need your personal access token credentials to authenticate when you clone the repository.

[Create a personal access token with the 'repo' scope.](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token)


### (Recommended) Verify you have a credential manager installed to store your credentials 

Recent versions of Git install a credential manager to store your credentials so you don't have to put in the credentials for every request.  
It is highly recommended you check that you have a [credential manager installed and configured](https://github.com/microsoft/Git-Credential-Manager-Core)



### Step 1. Clone the repository 

You can clone the project to any folder locally, including inside the engine folder. If you clone the project inside an existing Git repository (e.g. o3de) you should add the project folder to the Git exclude file for the existing repository.

#### Option #1 (Recommended) - cloning into a folder outside the engine repository folder

```shell
# clone the project into a folder outside your engine repository folder
> git clone https://github.com/aws/o3de-multiplayersample.git
Cloning into 'o3de-multiplayersample'...
```

#### Option #2 - cloning into the engine repository folder

```shell
# clone the project into a folder named 'o3de-multiplayersample' in your existing engine repository folder
> git clone https://github.com/aws/o3de-multiplayersample.git c:/path/to/o3de/o3de-multiplayersample
Cloning into 'o3de-multiplayersample'...

# modify the local engine git exclude file to ignore the project folder
> echo o3de-multiplayersample > c:/path/to/o3de/.git/info/exclude
```

If you have a Git credential helper configured, you should not be prompted for your credentials anymore.

### Step 2. Register the engine and project 

```shell
# register the engine (only need to do this once)
> c:/path/to/o3de/scripts/o3de register --this-engine

# register the project 
> c:/path/to/o3de/scripts/o3de register -p c:/path/to/o3de-multiplayersample
```

### Step 3. Configure and build 

#### Option #1 (Recommended) -  Project-centric approach 

This option will output all the project binaries in the project's build folder e.g. c:/path/to/o3de-multiplayersample/build

```shell
# example configure command
> cmake c:/path/to/o3de -B c:/path/to/o3de-multiplayersample/build -G "Visual Studio 16" -DLY_3RDPARTY_PATH="c:/3rdparty" -DLY_PROJECTS="c:/path/to/o3de-multiplayersample" 

# example build command
> cmake --build c:/path/to/o3de-multiplayersample/build --target Editor MultiplayerSample.GameLauncher --config profile -- /m /nologo 
```

#### Option #2 - Engine-centric approach to building a project 

This option will output all the project and engine binaries in the engine's build folder e.g. c:/path/to/o3de/build

```shell
# example configure command
> cmake c:/path/to/o3de -B c:/path/to/o3de/build -G "Visual Studio 16" -DLY_3RDPARTY_PATH="c:/3rdparty" -DLY_PROJECTS="c:/path/to/o3de-multiplayersample"

# example build command
> cmake --build c:/path/to/o3de/build --target Editor MultiplayerSample.GameLauncher --config profile -- /m /nologo 

```



## License

For terms please see the LICENSE*.TXT file at the root of this distribution.


