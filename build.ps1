Set-StrictMode -Version Latest;

function Show-Menu {
    Clear-Host
    Write-Host "================ Options ================" 
    Write-Host "1: Build binary."
    Write-Host "2: ST-Link Flash."
    Write-Host "Q: Press 'Q' to quit."
}

function Build-Binary{
    mingw32-make.exe;
}

function Flash-Firmware{
    st-flash.exe --reset write .\out\bin\firmware.bin 0x8000000;
}

do {

    Show-Menu;
    $choice = Read-Host "Please choose an action";
    switch($choice){
        '1' {
            Build-Binary;
            pause;
            break;
        }
        '2' {
            Flash-Firmware;
            pause;
            break;
        }
    }
    
}
while ($choice -ne 'q')
Clear-Host;
Write-Host "Terminating...Bye.";
