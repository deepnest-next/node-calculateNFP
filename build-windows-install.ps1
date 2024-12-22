# PowerShell Script: Check and Install VS Components for All Editions of VS2022

# Define the path to the .vsconfig file
$vsconfigPath = ".\.vsconfig"

# Ensure the Visual Studio Installer (vswhere) is available
$vsInstallerBasePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer"
$vswherePath = Join-Path $vsInstallerBasePath "vswhere.exe"
$vsInstallerPath = Join-Path $vsInstallerBasePath "vs_installer.exe"

if (-Not (Test-Path $vswherePath)) {
  Write-Error "vswhere.exe not found. Please ensure Visual Studio Installer is installed."
  exit 1
}

# Check if vs_installer.exe is available
if (-Not (Test-Path $vsInstallerPath)) {
  Write-Host "Visual Studio Installer not found. Downloading..."
  $vsInstallerDownloadUrl = "https://aka.ms/vs/17/release/vs_installer.exe"
  Invoke-WebRequest -Uri $vsInstallerDownloadUrl -OutFile $vsInstallerPath -UseBasicParsing
  Write-Host "Visual Studio Installer downloaded to $vsInstallerPath"
}

# Get all installed instances of Visual Studio 2022
$vsInstallations = & $vswherePath -prerelease -products * -requires Microsoft.VisualStudio.Component.CoreEditor -format json | ConvertFrom-Json
if (-Not $vsInstallations) {
  Write-Error "No Visual Studio 2022 installations found."
  exit 1
}

# Validate the .vsconfig file
if (-Not (Test-Path $vsconfigPath)) {
  Write-Error "The specified .vsconfig file does not exist: $vsconfigPath"
  exit 1
}

# Iterate through each Visual Studio installation
foreach ($vsInstallation in $vsInstallations) {
  Write-Host "Checking Visual Studio installation at: $($vsInstallation.installationPath)"
    
  # Retrieve the product ID and channel ID
  $productId = $vsInstallation.productId
  $channelId = $vsInstallation.channelId

  if (-Not $productId -or -Not $channelId) {
    Write-Error "Could not retrieve productId or channelId for Visual Studio installation at $($vsInstallation.installationPath). Skipping."
    continue
  }

  # Debug: Output the product ID, channel ID, and installation path
  Write-Host "Product ID: $productId"
  Write-Host "Channel ID: $channelId"
  Write-Host "Using Visual Studio Installer at: $vsInstallerPath"
  Write-Host "Applying configuration for Visual Studio installation at: $($vsInstallation.installationPath)"

  # Install or update components using the .vsconfig file

  Write-Host "Installing or updating components using: $vsconfigPath"
  try {
    Start-Process -FilePath $vsInstallerPath -ArgumentList "install --productId `"$productId`" --channelId `"$channelId`" --installPath `"$($vsInstallation.installationPath)`" --config `"$vsconfigPath`" --quiet" -Wait -ErrorAction Stop
  }
  catch {
    Write-Error "The command failed. Ensure the GitHub runner has administrative permissions."
    throw
  }
}

Write-Host "All Visual Studio 2022 instances have been checked and updated."