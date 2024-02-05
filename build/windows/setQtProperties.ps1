#!/usr/bin/env pwsh
#
# Set Qt properties in MSVC property sheet
#
# syntax:
#     setQtProperties.ps1 -inFile <property-sheet-file> -version <qt-version>
#
# Environment variables:
#
#     QTDIR is set to Qt installation directory

param (
    [string]$inFile = $(throw "inFile is required."),
    [string]$version = $(throw "version is required.")
)

$outFile = $infile.TrimEnd(".in")
$major, $null, $null = $version -split "\."

(Get-Content $inFile) | 
Foreach-Object {$_ -replace 'QTDIRPLACEHOLDER',$Env:QTDIR}  |
Foreach-Object {$_ -replace 'QTVERSIONPLACEHOLDER',$version}  |
Foreach-Object {$_ -replace 'QTMAJORPLACEHOLDER',$major}  |
Out-File $outFile

