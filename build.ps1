Set-StrictMode -Version Latest;

function Show-Menu {
    Clear-Host
    Write-Host "================ Options ================" 
    Write-Host "1: build binary."
    Write-Host "2: flash."
    Write-Host "Q: Press 'Q' to quit."
}

function Build-Binary{
    mingw32-make.exe;
}

function Flash-Firmware{
    st-flash.exe;
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