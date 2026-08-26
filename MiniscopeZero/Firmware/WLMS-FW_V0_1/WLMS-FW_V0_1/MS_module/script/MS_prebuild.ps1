$currentpath = $PWD
Write-Host ("original path: " + $currentpath)
$scriptpath = Split-Path $MyInvocation.MyCommand.Path
Write-Host ("command path: " + $scriptpath)
Set-Location $scriptpath

# Convert path to absolute path
$prjroot = Resolve-Path "..\..\"
$submodulepath = Resolve-Path "..\..\MS_module\"

# Array of excluded c files and header files
$cfilepatharray = @("sd_mmc\sd_mmc.c", "hal\src\hal_mci_sync.c", "hpl\dmac\hpl_dmac.c", "hpl\sdhc\hpl_sdhc.c")
$headerpatharray = @("sd_mmc\sd_mmc.h", "hal\include\hal_mci_sync.h", "hal\include\hpl_mci_sync.h", "hal\include\hpl_dma.h")

function Install-Drivers {
    [CmdletBinding()]
    param(
        [Parameter()]
        [String[]]$DriverList
    )
    For ($i = 0; $i -lt $DriverList.Length; $i++) {
        $originalfiledir = Split-Path -Path ($prjroot.Path + "\" + $DriverList[$i]) #directory of original file
        $originalfilename = Split-Path -Path ($prjroot.Path + "\" + $DriverList[$i]) -Leaf #name of original file
        
        # check if driver is installed by Atmel START
        if ((Test-Path -Path ($originalfiledir + "\" + $originalfilename) -PathType Leaf) -or (Test-Path -Path ($originalfiledir + "\" + $originalfilename + "tmp") -PathType Leaf)) {
            # change original .c file to .ctmp file to avoid compile
            if (Test-Path -Path ($originalfiledir + "\" + $originalfilename) -PathType Leaf) {
                try {
                    # $filepath = Get-ChildItem ($originalfiledir + "\" + $originalfilename)
    
                    # if .ctmp file exists, delete first
                    if (Test-Path -Path ($originalfiledir + "\" + $originalfilename + "tmp") -PathType Leaf) {
                        Remove-Item ($originalfiledir + "\" + $originalfilename + "tmp")
                        Write-Host ($originalfiledir + "\" + $originalfilename + "tmp has been deleted.")
                    }
                    # change .c file to .ctmp file
                    Rename-Item -Path ($originalfiledir + "\" + $originalfilename) -NewName ($originalfilename + "tmp")
                    Write-Host ($originalfilename + "tmp has been created.")
                }
                catch {
                    throw $_.Exception.Message
                }
            }
            # copy custom file into original directory
            try {
                $customfile = Get-Childitem $submodulepath -Filter ($originalfilename + "src") -Recurse
                New-Item -ItemType HardLink -Path ($originalfiledir + "\" + $originalfilename) -Value $customfile.FullName
                Write-Host ($originalfiledir + "\" + $originalfilename + " linked to " + $customfile.Name)            
            }
            catch {
                throw $_.Exception.Message
            }

        }
    }
}

function Install-Main {
    [CmdletBinding()]
    param(
        [Parameter()]
        [System.Management.Automation.PathInfo]$FilePath,

        [Parameter()]
        [System.Management.Automation.PathInfo]$HardLinkPath
    )
    $CustomMain = Get-Childitem $FilePath -Filter ("main.csrc")
    $DefaultMain = Get-Childitem $HardLinkPath -Filter ("main.c")
    if (Test-Path -Path ($DefaultMain.FullName) -PathType Leaf) {
        Remove-Item $DefaultMain.FullName
        Write-Host ($DefaultMain.FullName + "\main.c has been deleted.")
    }
    try {
        New-Item -ItemType HardLink -Path $DefaultMain.FullName -Value $CustomMain.FullName
        Write-Host ($CustomMain.FullName + " linked to " + $DefaultMain.FullName)                    
    }
    catch {
        throw $_.Exception.Message
    }
}

Install-Drivers -DriverList $cfilepatharray
Write-Host (".c files installed")
Install-Drivers -DriverList $headerpatharray
Write-Host (".h files installed")
Install-Main -FilePath $submodulepath -HardLinkPath $prjroot
Write-Host ("main.c installed")
