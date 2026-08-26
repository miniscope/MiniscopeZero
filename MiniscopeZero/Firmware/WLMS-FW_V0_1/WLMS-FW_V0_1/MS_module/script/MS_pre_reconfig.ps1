$currentpath = $PWD
Write-Host ("original path: "+$currentpath)
$scriptpath = Split-Path $MyInvocation.MyCommand.Path
Write-Host ("command path: "+$scriptpath)
Set-Location $scriptpath

# Convert path to absolute path
$prjroot = Resolve-Path "..\..\"
#$submodulepath = Resolve-Path "..\..\MS_module\"

# Array of excluded c files and header files
$cfilepatharray = @("sd_mmc\sd_mmc.c", "hal\src\hal_mci_sync.c", "hpl\dmac\hpl_dmac.c", "hpl\sdhc\hpl_sdhc.c")
$headerpatharray = @("sd_mmc\sd_mmc.h", "hal\include\hal_mci_sync.h", "hal\include\hpl_mci_sync.h", "hal\include\hpl_dma.h")


function Uninstall-Drivers {
    [CmdletBinding()]
    param(
        [Parameter()]
        [String[]]$DriverList
    )
    For ($i=0; $i -lt $DriverList.Length; $i++) {
        $originalfiledir = Split-Path -Path ($prjroot.Path + "\" + $DriverList[$i]) #directory of original file
        $originalfilename = Split-Path -Path ($prjroot.Path + "\" + $DriverList[$i]) -Leaf #name of original file
    
        # change original .c file to .ctmp file to avoid compile
        if ((Test-Path -Path ($originalfiledir + "\" + $originalfilename) -PathType Leaf) -And (Test-Path -Path ($originalfiledir + "\" + $originalfilename + "tmp") -PathType Leaf)) {
            try {
                # delete .c file first
                Remove-Item ($originalfiledir + "\" + $originalfilename)
                Write-Host ($originalfiledir + "\" + $originalfilename + " has been deleted.")
                # change .ctmp file to .c file
                Rename-Item -Path ($originalfiledir + "\" + $originalfilename + "tmp") -NewName $originalfilename
                Write-Host ($originalfilename + " has been created.")
            }
            catch {
                throw $_.Exception.Message
            }
        }
    }
}


Uninstall-Drivers -DriverList $cfilepatharray
Write-Host (".c files uninstalled")
Uninstall-Drivers -DriverList $headerpatharray
Write-Host (".h files uninstalled")
